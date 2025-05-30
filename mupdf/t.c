#include <mupdf/fitz.h>
#include <mupdf/pdf.h>

// 为GDB调试器使用，不能设置为static
// 这将使得GDB可以在运行时获取mupdf的版本信息
const char* mupdf_version = FZ_VERSION;

int main(int argc, char* argv[]) {
  pdf_obj* nil = nullptr;
  fz_context* ctx = fz_new_context(nullptr, nullptr, FZ_STORE_UNLIMITED);
#ifdef _MSC_VER
  pdf_document* doc = pdf_open_document(ctx, "../../../t.pdf");
#else
  pdf_document* doc = pdf_open_document(ctx, "t.pdf");
#endif
  pdf_obj* Int = pdf_new_int(ctx, 10);
  pdf_obj* Real = pdf_new_real(ctx, 3.14F);
  pdf_obj* Str = pdf_new_text_string(ctx, "hello");
  pdf_obj* Name = pdf_new_name(ctx, "name");

  pdf_obj* ar = pdf_new_array(ctx, doc, 10);
  pdf_array_put(ctx, ar, 0, Int);
  pdf_array_put(ctx, ar, 1, Real);
  pdf_array_put(ctx, ar, 2, Str);
  pdf_array_push_bool(ctx, ar, 1);
  pdf_array_push_bool(ctx, ar, 0);
  pdf_array_push(ctx, ar, PDF_NULL);

  pdf_obj* dict = pdf_new_dict(ctx, doc, 10);
  pdf_dict_puts(ctx, dict, "int", Int);
  pdf_dict_puts(ctx, dict, "real", Real);
  pdf_dict_puts(ctx, dict, "str", Str);
  pdf_dict_puts(ctx, dict, "name", Name);
  pdf_dict_puts(ctx, dict, "array", ar);

  pdf_obj* ref = pdf_new_indirect(ctx, doc, 3633, 0);

  pdf_drop_obj(ctx, Int);
  pdf_drop_obj(ctx, Real);
  pdf_drop_obj(ctx, Str);
  pdf_drop_obj(ctx, Name);
  pdf_drop_obj(ctx, ar);
  pdf_drop_obj(ctx, dict);
  pdf_drop_obj(ctx, ref);

  fz_drop_context(ctx);
}
