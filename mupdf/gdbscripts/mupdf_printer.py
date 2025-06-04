from tkinter import E
import gdb

pdf_ctx = None

# 获取mupdf的版本号
def get_mupdf_version_from_symbol():
    try:
        version = gdb.parse_and_eval('(const char*)mupdf_version')
        return version.string()
    except gdb.error as e:
        return f"<symbol not found: {e}>"

# 调用mupdf的API
def call_mupdf_api(func_name : str, val : gdb.Value, retType : type, *args):
    try:
        # 获取地址
        if val.type.code == gdb.TYPE_CODE_PTR:
            # 如果本身是指针，则直接转成int，就是地址
            addr = int(val)
        else:
            # 如果不是指针，则需要取地址，再转成int
            addr = int(val.address)
        cast = {
            int: "(int)",
            float: "(float)",
            str: "(const char*)",
            object: "(pdf_obj *)",
        }.get(retType, "(void)")
        global pdf_ctx
        if pdf_ctx is None:
            # 获取mupdf的版本号
            ver = get_mupdf_version_from_symbol()
            print(f"[GDB] MuPDF version: {ver}")
            # 创建一个fz_context，并保存在Python变量中
            pdf_ctx = gdb.parse_and_eval(f"(fz_context*)fz_new_context_imp(0,0,0,\"{ver}\")")
        if args.__len__() > 0:
            # 有参数的情况
            args_str = ', '.join([str(arg) for arg in args])
            expr = f"{cast}{func_name}({pdf_ctx},{addr},{args_str})"
        else:
            # 没参数的情况
            expr = f"{cast}{func_name}({pdf_ctx},{addr})"
        # 执行表达式
        result = gdb.parse_and_eval(expr)
        # 使用全局 pdf_ctx 则此处不需要释放
        #gdb.parse_and_eval(f"(void)fz_drop_context({ctx})")  # Clean up context
        # 根据返回值进行相应的转换
        if retType == int:
            return int(result.cast(gdb.lookup_type("int")))
        elif retType == float:
            return float(result.cast(gdb.lookup_type("float")))
        elif retType == str:
            return result.string()
        else:
            # Object不进行转换，直接返回，由调用者处理
            return result
    except Exception as e:
        print(f"<error calling {func_name}: {e}>")
        return f"<error calling {func_name}: {e}>"

def call_pdf_api(func_name : str, val : gdb.Value, rettype=int):
    return call_mupdf_api(func_name, val, rettype)

def call_pdf_api_1(func_name : str, val : gdb.Value, args):
    return call_mupdf_api(func_name, val, object, args)

# 检测除了间隔引用外的所有类型
def detect_pdf_obj_kind(val : gdb.Value):
    try:
        if call_pdf_api("pdf_is_null", val):
            return "null"
        elif call_pdf_api("pdf_is_int", val):
            return "int"
        elif call_pdf_api("pdf_is_real", val):
            return "real"
        elif call_pdf_api("pdf_is_string", val):
            return "string"
        elif call_pdf_api("pdf_is_name", val):
            return "name"
        elif call_pdf_api("pdf_is_array", val):
            return "array"
        elif call_pdf_api("pdf_is_dict", val):
            return "dict"
        elif call_pdf_api("pdf_is_bool", val):
            return "bool"
        elif call_pdf_api("pdf_is_stream", val):
            return "stream"
        else:
            return "unknown"
    except Exception as e:
        print(f"<error detecting pdf_obj kind: {e}>")
        return "error"

# 整数类型
class PDFObjIntPrinter:
    def __init__(self, val : gdb.Value, ref : str):
        self.val = val
        self.ref = ref

    def to_string(self):
        return f"{self.ref}{call_pdf_api('pdf_to_int', self.val, int)}"

    def display_hint(self):
        return None

# 浮点类型
class PDFObjRealPrinter:
    def __init__(self, val : gdb.Value, ref : str):
        self.val = val
        self.ref = ref

    def to_string(self):
        return f"{self.ref}{call_pdf_api('pdf_to_real', self.val, float)}"

    def display_hint(self):
        return None

# 字符串类型
class PDFObjStringPrinter:
    def __init__(self, val : gdb.Value, ref : str):
        self.val = val
        self.ref = ref

    def to_string(self):
        if self.ref is not None:
            return f"{self.ref}{call_pdf_api('pdf_to_text_string', self.val, str)}"
        return f"{call_pdf_api('pdf_to_text_string', self.val, str)}"

    def display_hint(self):
        return "string"

# 名字类型
class PDFObjNamePrinter:
    def __init__(self, val : gdb.Value, ref : str):
        self.val = val
        self.ref = ref

    def to_string(self):
        return f"{self.ref}/{call_pdf_api('pdf_to_name', self.val, str)}"

    def display_hint(self):
        return None

class PDFObjBoolPrinter:
    def __init__(self, val : gdb.Value, ref : str):
        self.val = val
        self.ref = ref

    def to_string(self):
        ret = call_pdf_api("pdf_to_bool", self.val, int)
        return f"{self.ref}{'true' if ret else 'false'}"

    def display_hint(self):
        return None

class PDFObjNullPrinter:
    def __init__(self, val : gdb.Value, ref : str):
        self.val = val
        self.ref = ref

    def to_string(self):
        return f"{self.ref}<null>"

    def display_hint(self):
        return None

# 为了让数组和字典中基本数据类型的元素不展开
# 根据kind先把基本数据类型的元素取出来
# 但是bool,string,name,null即使预先取出来，也是以字符串返回，会有展开箭头
# 所以这些类型依旧返回原值
def getItemValue(item : gdb.Value):
    try:
        kind = detect_pdf_obj_kind(item)
        if kind == "int":
            return call_pdf_api('pdf_to_int', item, int)
        if kind == "real":
            return call_pdf_api('pdf_to_real', item, float)
        if kind == "bool":
            return item
        if kind == "string":
            return item
        if kind == "name":
            return item
        if kind == "array":
            return item
        if kind == "dict":
            return item
        if kind == "null":
            return item
        return item
    except Exception as e:
        print(f"getItemValue {e}")

# 数组类型
class PDFArrayPrinter:
    def __init__(self, val, ref):
        self.val = val
        self.ref = ref
        self.count = call_pdf_api("pdf_array_len", self.val)

    def to_string(self):
        return f"{self.ref}[size]={self.count}"

    class _interator:
        def __init__(self, val, count):
            self.val = val
            self.count = count
            self.index = 0

        def __iter__(self):
            return self

        def __next__(self):
            if self.index >= self.count:
                raise StopIteration
            i = self.index
            self.index += 1
            try:
                item = call_pdf_api_1("pdf_array_get", self.val, i)
                return f"{i}", getItemValue(item)
            except Exception:
                raise StopIteration

    def children(self):
        return self._interator(self.val, self.count)

    def display_hint(self):
        return "array"

# 字典类型
class PDFDictPrinter:
    def __init__(self, val, ref):
        self.val = val
        self.ref = ref

    def to_string(self):
        count = call_pdf_api("pdf_dict_len", self.val)
        return f"{self.ref}[pairs]={count}"

    def children(self):
        count = call_pdf_api("pdf_dict_len", self.val)
        for i in range(count):
            try:
                key = call_pdf_api_1("pdf_dict_get_key",self.val, i)
                val = call_pdf_api_1("pdf_dict_get_val",self.val, i)
                # 下面两个yield语句中，元组的第一个元素不能相同
                yield (f"{i}:k", key) # 返回键值对的键
                yield (f"{i}:v", getItemValue(val)) # 返回键值对的值
            except Exception as e:
                print(f"PDFDictPrinter: error getting key {i} {e}")
                yield f"<key:{i}>", "<invalid>"

    def display_hint(self):
        return "map"

def pdf_obj_lookup(val : gdb.Value):
    try:
        # 获取val的类型
        t = val.type
        # 如果是指针，则需要取目标
        if t.code == gdb.TYPE_CODE_PTR:
            t = t.target()

        # 判断名字是否为 pdf_obj
        if t.name == "pdf_obj":
            # 首先判断是否是间隔引用，
            # 如果是，mupdf会在后面的其它类型判断时会自动缓存对象
            ref = ""
            if call_pdf_api("pdf_is_indirect", val):
                ref_num = call_pdf_api("pdf_to_num", val, int)
                #gen_num = call_pdf_api("pdf_to_gen", val, int)
                ref = f"<Ref {ref_num}> => "
            kind = detect_pdf_obj_kind(val)
            if kind == "int":
                return PDFObjIntPrinter(val, ref)
            elif kind == "real":
                return PDFObjRealPrinter(val, ref)
            elif kind == "string":
                return PDFObjStringPrinter(val, ref)
            elif kind == "name":
                return PDFObjNamePrinter(val, ref)
            elif kind == "array":
                return PDFArrayPrinter(val, ref)
            elif kind == "dict":
                return PDFDictPrinter(val, ref)
            elif kind == "bool":
                return PDFObjBoolPrinter(val, ref)
            elif kind == "null":
                return PDFObjNullPrinter(val, ref)
            else:
                print(f"<unknown pdf_obj kind: {kind}>")
                return None
    except:
        return None

gdb.pretty_printers.append(pdf_obj_lookup)
print("MuPDF GDB pretty printers loaded.")
