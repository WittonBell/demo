import lldb

pdf_ctx : int | None = None

def get_mupdf_version_from_symbol(target : lldb.SBTarget):
    try:
        version = target.EvaluateExpression('(const char*)mupdf_version')
        return version.GetSummary()
    except Exception as e:
        return f"<symbol not found: {e}>"

def call_mupdf_api(func_name : str, val : lldb.SBValue, retType : type, *args):
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
            ctx : lldb.SBValue = target.EvaluateExpression(f"(fz_context*)fz_new_context_imp(0,0,0,{ver})")
            pdf_ctx = ctx.GetValueAsAddress()  # 保存为整数地址
        if args:
            args_str = ', '.join([str(arg) for arg in args])
            expr = f"{cast}{func_name}((fz_context*){pdf_ctx},{addr}, {args_str})"
        else:
            expr = f"{cast}{func_name}((fz_context*){pdf_ctx},(pdf_obj*){addr})"
        result : lldb.SBValue = target.EvaluateExpression(expr)
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

def call_pdf_api(func_name : str, val : lldb.SBValue, rettype=int):
    return call_mupdf_api(func_name, val, rettype)

def call_pdf_api_1(func_name : str, val : lldb.SBValue, arg, rettype=int):
    return call_mupdf_api(func_name, val, rettype, arg)

# 检测除间隔引用外的数据类型
def detect_pdf_obj_kind(val : lldb.SBValue):
    try:
        if call_pdf_api("pdf_is_null", val):
            return "null"
        elif call_pdf_api("pdf_is_int", val):
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
        return "unknown"
    except Exception as e:
        print(f"<error detecting pdf_obj kind: {e}>")
        return "<error>"

def PDFObjAPISummary(val : lldb.SBValue, internal_dict : dict):
    try:
        addr = val.GetValueAsAddress()
        if not addr:
            return "<null>"

        ref = ""
        if call_pdf_api("pdf_is_indirect", val):
            num = call_pdf_api(f"pdf_to_num", val)
            #gen = call_pdf_api(f"pdf_to_gen", valobj)
            ref = f"<Ref {num}> => "

        kind = detect_pdf_obj_kind(val)
        if kind == "null":
            return f"{ref}<null>"
        elif kind == "int":
            return f"{ref}{call_pdf_api("pdf_to_int", val)}"
        elif kind == "real":
            return f"{ref}{call_pdf_api(f"pdf_to_real", val, float)}"
        elif kind == "bool":
            v = call_pdf_api(f"pdf_to_bool", val)
            return f"{ref}{'true' if v else 'false'}"
        elif kind == "string":
            return f'{ref}{call_pdf_api("pdf_to_text_string", val, str)}'
        elif kind == "name":
            v = call_pdf_api("pdf_to_name", val, str)
            return f'{ref}/{v.strip('"')}'
        elif kind == "array":
            length = call_pdf_api("pdf_array_len", val)
            return f"{ref}[size]={length}"
        elif kind == "dict":
            length = call_pdf_api(f"pdf_dict_len", val)
            return f"{ref}[pairs]={length}"
        return f"{ref}{addr}"

    except Exception as e:
        return f"<error: {e}>"
class PDFObjAPIPrinter:
    def __init__(self, val : lldb.SBValue, internal_dict : dict):
        self.val = val
        self.kind = detect_pdf_obj_kind(val)
        self.size = self.num_children()

    def has_children(self):
        # 只在array/dict类型时允许展开
        return self.kind in ["array", "dict"]

    def num_children(self):
        if self.kind == "array":
            length = call_pdf_api(f"pdf_array_len", self.val)
            return int(length) if length else 0
        elif self.kind == "dict":
            length = call_pdf_api(f"pdf_dict_len", self.val)
            return int(length) if length else 0
        return 0

    def get_child_at_index(self, index):
        try:
            if index < 0 or index >= self.size:
                return None

            if self.kind == "array":
                v = call_pdf_api_1(f"pdf_array_get", self.val, index, object)
                # 根据索引取到pdf_obj对象了，需要获取其地址
                addr = v.GetValueAsAddress()
                # 再构造一个表达式，将这个地址强制转为pdf_obj的指针
                expr = f"(pdf_obj *){addr}"
                # 最后根据这个表达式创建一个新的值，LLDB会自动重新根据规则显示这个值
                return self.val.CreateValueFromExpression(f"[{index}]", expr)

            elif self.kind == "dict":
                key = call_pdf_api_1("pdf_dict_get_key", self.val, index, object)
                val = call_pdf_api_1("pdf_dict_get_val", self.val, index, object)
                # 将pdf_obj中字典的Key一定是一个name，取name的值
                key_str = call_pdf_api("pdf_to_name", key, str).strip('"')
                # 将字典的value取地址，构造一个新的表达式
                addr = val.GetValueAsAddress()
                expr = f"(pdf_obj *){addr}"
                # 最后根据这个表达式创建一个新的值，LLDB会自动重新根据规则显示这个值
                return self.val.CreateValueFromExpression(f"[/{key_str}]", expr)
        except Exception as e:
            print(f"Error in get_child_at_index: {e}")
        return None

def __lldb_init_module(debugger : lldb.SBDebugger, internal_dict : dict):
    debugger.HandleCommand(r'type summary add -x ".*pdf_obj.*\*" --python-function mupdf_printer.PDFObjAPISummary')
    debugger.HandleCommand(r'type synthetic add -x ".*pdf_obj.*\*" --python-class mupdf_printer.PDFObjAPIPrinter')
    print("MuPDF pdf_obj summary and synthetic provider (via API) loaded.")
