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
        addr = val.GetValue()  # Ensure val is valid
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
            pdf_ctx = pdf_ctx.GetValue()  # Ensure pdf_ctx is a valid value
        if args.__len__() > 0:
            args_str = ', '.join([str(arg) for arg in args])
            expr = f"{cast}{func_name}((fz_context*){pdf_ctx},{addr}, {args_str})"
        else:
            expr = f"{cast}{func_name}((fz_context*){pdf_ctx},(pdf_obj*){addr})"
        result = target.EvaluateExpression(expr)
        #print(f"call_mupdf_api Var:{val.GetName()} {expr} {result}")
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

def call_pdf_api_1(func_name, val, args, rettype=int):
    return call_mupdf_api(func_name, val, rettype, args)


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
        addr = valobj.GetValue()
        #print(f"[LLDB pdf_obj summary] Called {valobj.GetName()} at {addr}")
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

        if kind == "int":
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
        self.target = valobj.GetTarget()
        self.ptr_expr = valobj.GetValue()
        self.kind = detect_pdf_obj_kind(self.valobj)

    def has_children(self):
        if self.kind in ["array", "dict"]:
            return True
        print(f"num_children {self.valobj.GetName()} kind={self.kind} no children")
        return False

    def num_children(self):
        if self.kind in ["array", "dict"]:
            length = call_pdf_api(f"pdf_{self.kind}_len", self.valobj)
            return int(length) if length else 0

        return 0

    def get_child_index(self, name):
        try:
            print(f"get_child_index {self.valobj.GetName()} name={name}")
            return int(name.lstrip("[").rstrip("]"))
        except:
            return -1

    def get_child_at_index(self, index):
        if index < 0:
            return None
        if index >= self.num_children():
            return None
        try:
            if self.kind == "array":
                v = call_pdf_api_1(f"pdf_array_get", self.valobj, index, object)
                print(f"get_array_at_index {self.valobj.GetName()} index={index} => {v}")
                # expr = f"(pdf_obj *){v.GetValue()}"
                # kind = detect_pdf_obj_kind(v.GetValue())
                # print("kind:", kind)
                # if kind in ["array", "dict", "null"]:
                #     # 返回SBValue对象
                #     return self.valobj.CreateValueFromExpression(f"[{index}]", expr)
                # else:
                return self.valobj.CreateValueFromExpression(f"[{index}]", v.GetSummary())
            if self.kind == "dict":
                key = call_pdf_api_1("pdf_dict_get_key", self.valobj, index, object)
                val = call_pdf_api_1("pdf_dict_get_val", self.valobj, index, object)
                print(f"get_dict_at_index {self.valobj.GetName()} index={index} => {key}:{val}")
                # 返回SBValue对象
                return self.valobj.CreateValueFromExpression(f"[{key}]", val)
        except:
            return None

def __lldb_init_module(debugger, internal_dict):
    # ✅ 正确匹配 (pdf_obj *) 指针
    debugger.HandleCommand(r'type summary add -x ".*pdf_obj.*\*" --python-function mupdf_printer.PDFObjAPISummary'  )
    debugger.HandleCommand(r'type synthetic add -x ".*pdf_obj.*\*" --python-class mupdf_printer.PDFObjAPIPrinter')

    print("[+] MuPDF pdf_obj summary provider (via API) loaded.")
