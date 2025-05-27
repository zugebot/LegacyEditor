import os
import sys
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

def _fs_path_to_string(path_val: gdb.Value) -> str:
    try:
        string = str(path_val.cast(gdb.lookup_type('std::wstring')))
        string = string[2:-1]
        string = string.replace("\\\\", "\\")
        return string
    except Exception:
        return "<err>"


class LCEFilePrinter:
    """Pretty‑printer for editor::LCEFile (non‑pointer)."""
    def __init__(self, val: gdb.Value):
        self.val = val

    # summary line
    def to_string(self):
        try:
            name = _fs_path_to_string(self.val['m_fileName'])
            folder = _fs_path_to_string(self.val['m_folderPath'])
            console = str(self.val['m_console']).replace("lce::CONSOLE::", "")
            base_dir = os.getcwd()

            full_path = os.path.join(base_dir, folder, name)
            print(full_path)

            try:
                size = os.path.getsize(full_path)
            except OSError:
                size = "<not found>"

            return f"{{{console}, size={size}, \"{name}\"}}"
        except gdb.error:
            return "{Invalid}"

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
