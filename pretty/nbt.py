# ---------------------------------------------------------------------------
#  pretty/nbt_printer.py   –   Pretty-printer for BetterNBT::NBTBase
# ---------------------------------------------------------------------------
#
#  Author:  you
#  Tested:  GDB 14.2 (MinGW) + CLion 2023.3  –  libstdc++ pretty-printers on
#
#  NBTBase layout (simplified):
#
#     struct NBTBase {
#         eNBT      type;   // enum class eNBT : uint8_t { NONE, INT8, … }
#         NBTValue  value;  // std::variant<…>
#     };
#
#  This printer:
#     * shows a concise   “NBT<STRING>”   summary
#     * flattens `value` so the user sees   value = "Steve"
#       (or 42, 3.14f, etc.) without the “[contained value]” wrapper
#     * still allows expanding LIST / COMPOUND children
# ---------------------------------------------------------------------------

import gdb
import gdb.printing

# ─────────────────────────────────────────────────────────────────────────────
# helpers
# ─────────────────────────────────────────────────────────────────────────────

eNBT_NONE       = 0
eNBT_INT8       = 1
eNBT_INT16      = 2
eNBT_INT32      = 3
eNBT_INT64      = 4
eNBT_FLOAT      = 5
eNBT_DOUBLE     = 6
eNBT_BYTE_ARRAY = 7
eNBT_STRING     = 8
eNBT_LIST       = 9
eNBT_COMPOUND   = 10
eNBT_INT_ARRAY  = 11
eNBT_LONG_ARRAY = 12
eNBT_PRIMITIVE  = 99


PRIMITIVE_MAP = {
    1: ("signed char",   int),    # eNBT_INT8
    2: ("short",         int),    # eNBT_INT16
    3: ("int",           int),    # eNBT_INT32
    4: ("long long",     int),    # eNBT_INT64
    5: ("float",         float),  # eNBT_FLOAT
    6: ("double",        float),  # eNBT_DOUBLE
    8: ("std::string",   str),    # eNBT_STRING
}

def _enum_name(enum_val: gdb.Value) -> str:
    """Return bare enum constant name (eNBT::STRING → STRING)."""
    s = str(enum_val)
    return s.split("::")[-1] if "::" in s else s

# In libstdc++, a std::variant<Ts...> is:
#   struct { unsigned _M_index; storage union; ... }
# The exact field names are implementation-specific but consistent.
def _variant_active(v: gdb.Value) -> gdb.Value:
    """
    Given v of type std::variant<...>, return the gdb.Value
    for the currently‐selected alternative, handling both
    the "tuple" and "binary‐tree" libstdc++ layouts.
    """
    # 1) Read the runtime index (_M_index lives either at v or at v._M_u)
    try:
        idx = int(v['_M_index'])
    except gdb.error:
        idx = int(v['_M_u']['_M_index'])

    # 2) Try the flat‐tuple layout
    for path in [
        ['_M_u', '_M_union',   '_M_storage', '_M_tuple'],
        ['_M_u', '_M_u',      '_M_storage', '_M_tuple'],
    ]:
        try:
            node = v
            for fld in path:
                node = node[fld]
            return node[idx]
        except gdb.error:
            pass

    # 3) Fall back to the binary‐tree layout
    node = v['_M_u']
    while True:
        if idx == 0:
            return node['_M_first']
        idx -= 1
        node = node['_M_rest']
    # ─────────────────────────────────────────────────────────────────────────────
# the printer
# ─────────────────────────────────────────────────────────────────────────────


# ------------------------------------------------------------------
#  Printer for the *new* NBTCompound  (parallel vectors layout)
# ------------------------------------------------------------------



class NBTListPrinter:
    """
    NBTList layout in your code:

        eNBT                      m_subType       // element kind
        std::vector<NBTBase>      m_elements

    We simply walk m_elements.  For primitive sub-types we flatten the
    value so the Type column is   {int} / {float} / {std::string}.
    """

    def __init__(self, val: gdb.Value):
        self.val = val                       # the NBTList object

        self.sub_kind = int(val["m_subType"])
        elems_impl    = val["m_elements"]["_M_impl"]
        self.start    = elems_impl["_M_start"]
        self.finish   = elems_impl["_M_finish"]
        self.count    = int(self.finish - self.start)

    # ─ summary line ────────────────────────────────────────────
    def to_string(self):
        kind_name = _enum_name(self.val["m_subType"])
        return f"{{{kind_name}, size={self.count}}}"

    def display_hint(self):
        return "array"

    # ─ children (index, value) ─────────────────────────────────
    def children(self):
        for i in range(self.count):
            elt = (self.start + i).dereference()

            # Primitive?  Flatten
            if self.sub_kind in (
                    eNBT_INT8,  eNBT_INT16,  eNBT_INT32,
                    eNBT_INT64, eNBT_FLOAT,  eNBT_DOUBLE
            ):
                active  = _variant_active(elt["m_value"])
                typname, _ = PRIMITIVE_MAP[self.sub_kind]
                real_t  = gdb.lookup_type(typname).strip_typedefs()
                yield f"[{i}]", active.cast(real_t)
                continue

            if self.sub_kind == eNBT_STRING:
                active = _variant_active(elt["m_value"])
                try:
                    s = active["_M_storage"]
                except gdb.error:
                    s = active
                yield f"[{i}]", s
                continue

            # list-of-compound / list-of-list etc. → hand back NBTBase
            yield f"[{i}]", elt





class NBTCompoundPrinter:
    """
    Pretty-printer for the custom hash-map:

        std::vector<std::string>            _keys
        std::vector<NBTBase>                _values
        std::vector< std::vector<size_t> >  _buckets   ← not needed here

    We enumerate the entries by simply walking the two vectors
    in parallel (index 0 … size-1).  The Value-column shows either
    a flattened primitive (int / float / std::string) or a nested
    NBTBase (“NBT<LIST>”, “NBT<COMPOUND>”…).
    """

    # ── boiler-plate ────────────────────────────────────────────
    def __init__(self, val: gdb.Value):
        self.val = val          # gdb.Value of the NBTCompound

    def to_string(self):
        sz = int(self.val["_size"])
        return f"{{size={sz}}}"

    def display_hint(self):           # let CLion render as an array
        return "array"

    # ── the interesting part: children() ───────────────────────
    def children(self):
        # 1) pull out _keys and _values vectors
        keys_vec   = self.val["_keys"]["_M_impl"]
        vals_vec   = self.val["_values"]["_M_impl"]

        k_start, k_finish = keys_vec["_M_start"], keys_vec["_M_finish"]
        v_start, _        = vals_vec["_M_start"], vals_vec["_M_finish"]

        count = int(k_finish - k_start)
        assert count == int(self.val["_size"])   # sanity

        # iterate 0 … count-1
        for i in range(count):
            key_gdb = (k_start + i).dereference()
            val_gdb = (v_start + i).dereference()

            key_str = key_gdb  # already a std::string gdb.Value

            # decide how we want to show the value
            kind      = int(val_gdb["m_type"])
            variant   = val_gdb["m_value"]
            active    = _variant_active(variant)

            # ─ primitives / string → flatten ─
            if kind in (eNBT_INT8,  eNBT_INT16,  eNBT_INT32,
                        eNBT_INT64, eNBT_FLOAT, eNBT_DOUBLE):

                cpp_name, _ = PRIMITIVE_MAP[kind]
                real_t      = gdb.lookup_type(cpp_name).strip_typedefs()
                yield key_str, active.cast(real_t)
                continue

            if kind == eNBT_STRING:
                # libstdc++ keeps the std::string object in _M_storage
                try:
                    s = active["_M_storage"]
                except gdb.error:
                    s = active
                yield key_str, s
                continue

            if kind == eNBT_LIST:
                real_t      = gdb.lookup_type("NBTList").strip_typedefs()
                yield key_str, active.cast(real_t)
                continue

            if kind == eNBT_COMPOUND:
                real_t      = gdb.lookup_type("NBTCompound").strip_typedefs()
                yield key_str, active.cast(real_t)
                continue

            yield key_str, val_gdb






class NBTBasePrinter:
    """Pretty-printer for BetterNBT::NBTBase"""

    def __init__(self, val: gdb.Value):
        self.val = val

    # summary shown in the Value column / CLI print
    def to_string(self):
        string = f"NBT<{_enum_name(self.val['m_type'])}>"
        kind = int(self.val["m_type"])
        if kind == eNBT_COMPOUND:
            string += ""
        return string

    # CLion / GDB uses this to decide whether to format as array / map / string
    def display_hint(self):
        kind = int(self.val["m_type"])
        if kind in (1,2,3,4,5,6,8):
            return "string"     # PRIMITIVES
        elif kind == 9:
            return "array"      # LIST
        elif kind == 10:
            return "string"        # COMPOUND
        return None             # primitives – default formatting

    # ── children() – return (name , gdb.Value) pairs ──────────────────
    def children(self):
        kind    = int(self.val['m_type'])
        variant = self.val['m_value']
        active  = _variant_active(variant)

        # always show the tag
        # yield "type", self.val["type"]

        # ——————————————— STRING ———————————————
        if kind in (1,2,3,4,5,6,8):
            # active is the union node; extract its _M_storage member
            try:
                actual = active['_M_storage']
            except gdb.error:
                actual = active
            yield "m_value", actual
            return

        # ————————————— BYTE/INT/LONG ARRAY —————————————
        if kind in (7,11,12):
            vec    = active['_M_impl']
            start  = vec['_M_start']
            finish = vec['_M_finish']
            cnt    = int(finish - start)
            yield "size", cnt
            for i in range(cnt):
                yield f"[{i}]", (start + i).dereference()
            return


        # ————————————— FALLBACK —————————————
        yield "raw", active

# ─────────────────────────────────────────────────────────────────────────────
# registration boiler-plate
# ─────────────────────────────────────────────────────────────────────────────


def lookup(val):
    tag = val.type.strip_typedefs().tag or ""

    if tag == "NBTList":
        return NBTListPrinter(val)

    if tag == "NBTCompound":
        return NBTCompoundPrinter(val)

    if tag.endswith("NBTBase"):
        try:
            k = int(val["m_type"])
            if k == eNBT_COMPOUND:
                cmpd = _variant_active(val["m_value"])["_M_storage"]
                return NBTCompoundPrinter(cmpd)
            if k == eNBT_LIST:
                lst  = _variant_active(val["m_value"])["_M_storage"]
                return NBTListPrinter(lst)
        except gdb.error:
            pass
        return NBTBasePrinter(val)

    return None


gdb.pretty_printers.append(lookup)

