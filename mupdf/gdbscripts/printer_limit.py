# -*- coding: utf-8 -*-
import gdb

def read_c_string(addr, length=None):
    """读取内存中的字符串内容，支持 flexible array 和 char*"""
    inferior = gdb.selected_inferior()
    try:
        raw = inferior.read_memory(addr, length or 256)
        s = raw.tobytes()
        if length is not None:
            return s[:length].decode("utf-8", errors="replace")
        else:
            return s.split(b'\x00', 1)[0].decode("utf-8", errors="replace")
    except Exception:
        return "<invalid string>"

class PDFObjPrinter:
    def __init__(self, val):
        self.val = val
        val_type = val.type.strip_typedefs()
        self.is_ptr = val_type.code == gdb.TYPE_CODE_PTR
        self.obj = val.dereference() if self.is_ptr else val

    def cast_obj(self, typename):
        t = gdb.lookup_type(typename)
        if self.is_ptr:
            return self.val.cast(t.pointer()).dereference()
        else:
            return self.val.address.cast(t.pointer()).dereference()

    def to_string(self):
        try:
            if not self.val:
                return "<null>"

            kind = chr(int(self.obj['kind']))
            if kind == 'i':  # PDF_INT
                num = self.cast_obj('pdf_obj_num')
                return f"{int(num['u']['i'])}"
            elif kind == 'f':  # PDF_REAL
                num = self.cast_obj('pdf_obj_num')
                return f"{float(num['u']['f'])}"
            elif kind == 's':  # PDF_STRING
                s = self.cast_obj('pdf_obj_string')
                length = int(s['len'])
                try:
                    if s['text']:
                        addr = int(s['text'])
                    else:
                        addr = int(s.address) + int(s.type.sizeof)  # buf 就在结构体尾部
                    text = read_c_string(addr, length)
                except Exception:
                    text = "<invalid string>"
                return f"\"{text}\""
            elif kind == 'n':  # PDF_NAME
                n = self.cast_obj('pdf_obj_name')
                try:
                    addr = int(n['n'].address)
                    name = read_c_string(addr)
                except Exception:
                    name = "<invalid name>"
                return f"\"{name}\""
            elif kind == 'a':  # PDF_ARRAY
                a = self.cast_obj('pdf_obj_array')
                length = int(a['len'])
                items = []
                try:
                    for i in range(length):
                        item = a['items'][i]
                        items.append(str(item))
                except Exception:
                    items.append("...")
                return f"[size]={length}, [{', '.join(items)}]"
            elif kind == 'd':  # PDF_DICT
                d = self.cast_obj('pdf_obj_dict')
                length = int(d['len'])
                items = []
                try:
                    for i in range(length):
                        entry = d['items'][i]
                        key = entry['k']
                        val = entry['v']
                        items.append(f"{key}: {val}")
                except Exception as e:
                    items.append("...")
                return f"[size]={int(d['len'])}, {{{', '.join(items)}}}"
            elif kind == 'r':  # PDF_INDIRECT
                return "<PDF indirect ref>"
            else:
                return f"<PDF unknown kind '{kind}'>"
        except Exception as e:
            return f"{e}"

    def children(self):
        try:
            kind = chr(int(self.obj['kind']))
        except Exception:
            return
        if kind == 'a':  # array
            a = self.cast_obj('pdf_obj_array')
            length = int(a['len'])
            for i in range(length):
                try:
                    item = a['items'][i]
                    yield (f"[{i}]", item)
                except Exception:
                    yield (f"[{i}]", "<invalid>")
        elif kind == 'd':  # dict
            d = self.cast_obj('pdf_obj_dict')
            length = int(d['len'])
            try:
                for i in range(length):
                    entry = d['items'][i]
                    key = entry['k']
                    val = entry['v']
                    yield (f"{i}:key", key)
                    yield (f"{i}:val", val)
            except Exception:
                yield ("dict", "<invalid>")
        return (None, None)

    def display_hint(self):
        try:
            kind = chr(int(self.obj['kind']))
            if kind == 'a':
                return 'array'
            elif kind == 'd':
                return 'map'
            else:
                return None
        except Exception:
            return None

def pdf_lookup(val):
    t = val.type.strip_typedefs()
    if t.code == gdb.TYPE_CODE_PTR:
        t = t.target().strip_typedefs()
    if t.tag and t.tag.startswith('pdf_obj'):
        return PDFObjPrinter(val)
    return None

def register_printers(objfile=None):
    if objfile is None:
        objfile = gdb
    gdb.printing.register_pretty_printer(objfile, pdf_lookup, replace=True)
    print("load mupdf printers")

register_printers(None)
