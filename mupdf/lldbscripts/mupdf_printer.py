import lldb

pdf_ctx = None

def get_mupdf_version_from_symbol(target):
    try:
        version = target.EvaluateExpression('(const char*)mupdf_version')
        return version.GetSummary()
    except Exception as e:
        return f"<symbol not found: {e}>"

def call_mupdf_api(func_name, val, retType, *args):
    try:
        target = val.GetTarget()
        addr = val.GetValueAsAddress()  # 使用GetValueAsAddress获取指针值
        cast = {
            int: "(int)",
            float: "(float)",
            str: "(const char*)",  # for functions returning const char*
            object: "(pdf_obj *)",  # for pdf_obj pointers
        }.get(retType, "(void)")
        global pdf_ctx
        if pdf_ctx is None:
            ver = get_mupdf_version_from_symbol(target)
            print(f"[LLDB] MuPDF version: {ver}")
            pdf_ctx = target.EvaluateExpression(f"(fz_context*)fz_new_context_imp(0,0,0,{ver})")
            pdf_ctx = pdf_ctx.GetValueAsAddress()  # 保存为整数地址
        if args:
            args_str = ', '.join([str(arg) for arg in args])
            expr = f"{cast}{func_name}((fz_context*){pdf_ctx},{addr}, {args_str})"
        else:
            expr = f"{cast}{func_name}((fz_context*){pdf_ctx},(pdf_obj*){addr})"
        result = target.EvaluateExpression(expr)
        if retType == int:
            return int(result.GetValue())
        elif retType == float:
            return float(result.GetValue())
        elif retType == str:
            return result.GetSummary()
        else:
            return result
    except Exception as e:
        print(f"<error calling {func_name}: {e}>")
        return f"<error calling {func_name}: {e}>"

def call_pdf_api(func_name, val, rettype=int):
    return call_mupdf_api(func_name, val, rettype)

def call_pdf_api_1(func_name, val, arg, rettype=int):
    return call_mupdf_api(func_name, val, rettype, arg)

def detect_pdf_obj_kind(val):
    try:
        if call_pdf_api("pdf_is_null", val):
            return "null"
        if call_pdf_api("pdf_is_int", val):
            return "int"
        elif call_pdf_api("pdf_is_real", val):
            return "real"
        elif call_pdf_api("pdf_is_bool", val):
            return "bool"
        elif call_pdf_api("pdf_is_string", val):
            return "string"
        elif call_pdf_api("pdf_is_name", val):
            return "name"
        elif call_pdf_api("pdf_is_array", val):
            return "array"
        elif call_pdf_api("pdf_is_dict", val):
            return "dict"
        else:
            return "unknown"
    except Exception as e:
        print(f"<error detecting pdf_obj kind: {e}>")
        return "<error>"

def PDFObjAPISummary(valobj : lldb.value, internal_dict):
    try:
        addr = valobj.GetValueAsAddress()
        if not addr:
            return "<null>"

        ref = ""
        if call_pdf_api("pdf_is_indirect", valobj):
            num = call_pdf_api(f"pdf_to_num", valobj)
            gen = call_pdf_api(f"pdf_to_gen", valobj)
            ref = f"(ref) {num} {gen} => "

        kind = detect_pdf_obj_kind(valobj)
        if kind == "null":
            return f"{ref}<null>"
        elif kind == "int":
            val = call_pdf_api("pdf_to_int", valobj)
            return f"{ref}{val}"
        elif kind == "real":
            val = call_pdf_api(f"pdf_to_real", valobj, float)
            return f"{ref}{val}"
        elif kind == "bool":
            val = call_pdf_api(f"pdf_to_bool", valobj)
            return f"{ref}{'true' if val else 'false'}"
        elif kind == "string":
            val = call_pdf_api("pdf_to_text_string", valobj, str)
            return f'{ref}{val}'
        elif kind == "name":
            val = call_pdf_api("pdf_to_name", valobj, str)
            return f'{ref}{val}'
        elif kind == "array":
            length = call_pdf_api("pdf_array_len", valobj)
            return f"{ref}[{length} elements]"
        elif kind == "dict":
            length = call_pdf_api(f"pdf_dict_len", valobj)
            return f"{ref}{{{length} pairs}}"
        return f"{addr}"

    except Exception as e:
        return f"<error: {e}>"

class PDFObjAPIPrinter:
    def __init__(self, valobj, internal_dict):
        self.valobj = valobj
        self.kind = detect_pdf_obj_kind(self.valobj)

    def has_children(self):
        # 只在array/dict类型时允许展开
        return self.kind in ["array", "dict"]

    def num_children(self):
        if self.kind == "array":
            length = call_pdf_api(f"pdf_array_len", self.valobj)
            return int(length) if length else 0
        elif self.kind == "dict":
            length = call_pdf_api(f"pdf_dict_len", self.valobj)
            return int(length) if length else 0
        return 0

    def get_child_index(self, name):
        try:
            if self.kind == "array":
                return int(name.strip("[]"))
            elif self.kind == "dict":
                # 字典子节点名称格式为[key]
                return int(name.split("]")[0].strip("["))
        except:
            return -1

    def get_child_at_index(self, index):
        try:
            if index < 0 or index >= self.num_children():
                return None

            if self.kind == "array":
                v = call_pdf_api_1(f"pdf_array_get", self.valobj, index, object)
                addr = v.GetValueAsAddress()
                expr = f"(pdf_obj *){addr}"
                return self.valobj.CreateValueFromExpression(f"[{index}]", expr)

            elif self.kind == "dict":
                key = call_pdf_api_1("pdf_dict_get_key", self.valobj, index, object)
                val = call_pdf_api_1("pdf_dict_get_val", self.valobj, index, object)
                key_str = call_pdf_api("pdf_to_name", key, str)
                addr = val.GetValueAsAddress()
                expr = f"(pdf_obj *){addr}"
                return self.valobj.CreateValueFromExpression(f"[{key_str}]", expr)
        except Exception as e:
            print(f"Error in get_child_at_index: {e}")
        return None

def __lldb_init_module(debugger, internal_dict):
    debugger.HandleCommand(r'type summary add -x ".*pdf_obj.*\*" --python-function mupdf_printer.PDFObjAPISummary')
    debugger.HandleCommand(r'type synthetic add -x ".*pdf_obj.*\*" --python-class mupdf_printer.PDFObjAPIPrinter')
    print("[+] MuPDF pdf_obj summary provider (via API) loaded.")
