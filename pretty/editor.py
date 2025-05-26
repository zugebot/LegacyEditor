import gdb
import gdb.printing

# ──────────────────────────────────────────────────────────────────────────────
#  Pretty‑printers for editor::LCEFile, editor::LCEFile*, and the common STL
#  containers that hold them.  Goal: show a concise summary such as
#        {LCEFile} ["file1", lce::FILETYPE::STRUCTURE, size= 12904]
#  while still letting the user expand the node to inspect children.
# ──────────────────────────────────────────────────────────────────────────────

# Helper – extract std::string data (libstdc++ layout)

def _string_data(s: gdb.Value) -> str:
    try:
        return s['_M_dataplus']['_M_p'].string()
    except gdb.error:
        return "<err>"


class LCEFilePrinter:
    """Pretty‑printer for editor::LCEFile (non‑pointer)."""
    def __init__(self, val: gdb.Value):
        self.val = val

    # summary line
    def to_string(self):
        # try:
            name   = _string_data(self.val['m_internalName'])
            size   = int(self.val['m_data']['m_size'])
            return f"{{\"{name}\", size={size}}}"
        # except gdb.error as e:
        #     print(e)
        #     return "{Invalid}"

    def display_hint(self):
        return "string"

    def children(self):
        for fld in self.val.type.fields():
            try:
                yield fld.name, self.val[fld.name]
            except gdb.error:
                pass



class ChunkManagerPrinter:
    """Pretty‑printer for editor::LCEFile (non‑pointer)."""
    def __init__(self, val: gdb.Value):
        self.val = val

    # summary line
    def to_string(self):

        size   = int(self.val['buffer']['m_size'])
        if size == 0:
            return f"{{size={size}}}"
        else:
            chunk_x = int(self.val['chunkData']['chunkX'])
            chunk_z = int(self.val['chunkData']['chunkZ'])
            return f"{{size={size}, pos=({chunk_x}, {chunk_z})}}"



    def display_hint(self):
        return "object"

    def children(self):
        for fld in self.val.type.fields():
            try:
                yield fld.name, self.val[fld.name]
            except gdb.error:
                pass


# ───────────────────────── lookup() ────────────────────────────

def lookup(val):
    try:
        tag = val.type.unqualified().strip_typedefs().name
        if tag == 'editor::LCEFile':
            return LCEFilePrinter(val)
        if tag == 'editor::ChunkManager':
            return ChunkManagerPrinter(val)
    except gdb.error:
        pass
    return None


gdb.pretty_printers.append(lookup)
