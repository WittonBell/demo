import gdb

pdf_ctx = None

def get_mupdf_version_from_symbol():
    try:
        version = gdb.parse_and_eval('(const char*)mupdf_version')
        return version.string()
    except gdb.error as e:
        return f"<symbol not found: {e}>"


def call_mupdf_api(func_name, val, retType, *args):
    try:
        # 获取地址
        if val.type.code == gdb.TYPE_CODE_PTR:
            addr = int(val)
        else:
            addr = int(val.address)
        cast = {
            int: "(int)",
            float: "(float)",
            str: "(const char*)",  # for functions returning const char*
            object: "(pdf_obj *)",  # for pdf_obj pointers
        }.get(retType, "")
        global pdf_ctx
        if pdf_ctx is None:
            ver = get_mupdf_version_from_symbol()
            pdf_ctx = gdb.parse_and_eval(f"(fz_context*)fz_new_context_imp(0,0,0,\"{ver}\")")
        if args.__len__() > 0:
            args_str = ', '.join([str(arg) for arg in args])
            expr = f"{cast}{func_name}({pdf_ctx},(pdf_obj *){addr}u, {args_str})"
        else:
            expr = f"{cast}{func_name}({pdf_ctx},(pdf_obj *){addr}u)"
        result = gdb.parse_and_eval(expr)
        # 使用全局 pdf_ctx 则此处不需要释放
        #gdb.parse_and_eval(f"(void)fz_drop_context({ctx})")  # Clean up context
        if retType == int:
            return int(result.cast(gdb.lookup_type("int")))
        elif retType == float:
            return float(result.cast(gdb.lookup_type("float")))
        elif retType == str:
            return str(f"{result.string()}")
        else:
            return result
    except Exception as e:
        print(f"<error calling {func_name}: {e}>")
        return f"<error calling {func_name}: {e}>"

# -------- Utility to call MuPDF C APIs from GDB --------
def call_pdf_api(func_name, val, rettype=int):
    return call_mupdf_api(func_name, val, rettype)

def call_pdf_api_1(func_name, val, args, rettype=int):
    return call_mupdf_api(func_name, val, rettype, args)

# -------- Array Pretty Printer --------
class PDFArrayPrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        count = call_pdf_api("pdf_array_len", self.val)
        return f"<PDF array[{count}]>"

    def children(self):
        count = call_pdf_api("pdf_array_len", self.val)
        for i in range(count):
            try:
                item = call_pdf_api_1("pdf_array_get", self.val, i, object)
                yield f"{i}", item
            except Exception:
                yield f"{i}", "<invalid>"

    def display_hint(self):
        return "array"

# -------- Dictionary Pretty Printer --------
class PDFDictPrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        count = call_pdf_api("pdf_dict_len", self.val)
        return f"<PDF dict[{count}]>"

    def children(self):
        count = call_pdf_api("pdf_dict_len", self.val)
        for i in range(count):
            try:
                key = call_pdf_api_1("pdf_dict_get_key",self.val, i, object)
                val = call_pdf_api_1("pdf_dict_get_val",self.val, i, object)
                # 下面两个yield语句中，元组的第一个元素不能相同
                yield (f"{i}:k", key) # 返回键值对的键
                yield (f"{i}:v", val) # 返回键值对的值
            except Exception as e:
                print(f"PDFDictPrinter: error getting key {i} {e}")
                yield f"<key:{i}>", "<invalid>"

    def display_hint(self):
        return "map"

# -------- Main Pretty Printer for pdf_obj --------
class PDFObjPrinter:
    def __init__(self, val):
        self.val = val
        val_type = val.type.strip_typedefs()
        self.is_ptr = val_type.code == gdb.TYPE_CODE_PTR
        self.obj = val.dereference() if self.is_ptr else val


    def to_string(self):
        v = self.val
        try:
            if self.is_ptr and not self.val:
                return "<null>"

            ref = ""
            if call_pdf_api("pdf_is_indirect", v):
                ref_num = call_pdf_api("pdf_to_num", v, int)
                ref = f"<PDF indirect ref {ref_num}> => "

            if call_pdf_api("pdf_is_int", v):
                return f"{ref}{call_pdf_api('pdf_to_int', v, int)}"
            elif call_pdf_api("pdf_is_real", v):
                return f"{ref}{call_pdf_api('pdf_to_real', v, float)}"
            elif call_pdf_api("pdf_is_string", v):
                ret = call_pdf_api("pdf_to_text_string", v, str)
                return f"{ref}{ret}"
            elif call_pdf_api("pdf_is_name", v):
                ret = call_pdf_api("pdf_to_name", v, str)
                return f"{ref}\"{ret}\""
            elif call_pdf_api("pdf_is_array", v):
                return f"{ref}{PDFArrayPrinter(v).to_string()}"
            elif call_pdf_api("pdf_is_dict", v):
                return f"{ref}{PDFDictPrinter(v).to_string()}"
            elif call_pdf_api("pdf_is_bool", v):
                ret = call_pdf_api("pdf_to_bool", v, int)
                return f"{ref}true" if ret else f"{ref}false"
            elif call_pdf_api("pdf_is_stream", v):
                stream_len = call_pdf_api("pdf_stream_len", v, int)
                if stream_len > 0:
                    return f"<PDF stream[{stream_len}]>"
                else:
                    return "<PDF empty stream>"
            elif call_pdf_api("pdf_is_null", v):
                return "<PDF_NULL>"
            else:
                return "<PDF unknown>"
        except Exception as e:
            return f"<invalid pdf_obj: {e}>"

    def children(self):
        v = self.val
        try:
            if call_pdf_api("pdf_is_array", v):
                return PDFArrayPrinter(v).children()
            elif call_pdf_api("pdf_is_dict", v):
                return PDFDictPrinter(v).children()
        except:
            pass
        # 如果不是数组或字典，则返回空
        return []

    def display_hint(self):
        v = self.val
        try:
            if call_pdf_api("pdf_is_array", v):
                return "array"
            if call_pdf_api("pdf_is_dict", v):
                return "map"
            if call_pdf_api("pdf_is_string", v):
                return "string"
        except:
            pass
        return None

# -------- Pretty Printer Entry Point --------
def pdf_obj_lookup(val):
    try:
        t = val.type.strip_typedefs()
        if t.code == gdb.TYPE_CODE_PTR:
            t = t.target().strip_typedefs()
        if t.name == "pdf_obj" or t.name == "struct pdf_obj":
            return PDFObjPrinter(val)
    except:
        return None

gdb.pretty_printers.append(pdf_obj_lookup)
print("MuPDF GDB pretty printers loaded.")
