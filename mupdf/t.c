#include <mupdf/fitz.h>
#include <mupdf/pdf.h>
#include <stdbool.h>

#ifndef __cplusplus
#if __STDC_VERSION__ < 202311L
#ifdef nullptr
#undef nullptr
#endif

#define nullptr NULL
#endif
#endif

// 为GDB调试器使用，不能设置为static
// 这将使得GDB可以在运行时获取mupdf的版本信息
const char* mupdf_version = FZ_VERSION;

#if defined(_MSC_VER) && defined(_DEBUG)
// VC调试器需要ref2obj辅助函数来展开查看间接引用的值
fz_context* mupdf_ctx = nullptr;

pdf_obj* ref2obj(pdf_obj* obj) {
  if (mupdf_ctx == nullptr) {
    return obj;
  }
  if (pdf_is_indirect(mupdf_ctx, obj)) {
    return pdf_resolve_indirect_chain(mupdf_ctx, obj);
  }
  return obj;
}
#endif

int main(int argc, char* argv[]) {
  const char* p = "hello";
  bool b = true;
  pdf_obj* nil = nullptr;
  fz_context* ctx = fz_new_context(nullptr, nullptr, FZ_STORE_UNLIMITED);
#if defined(_MSC_VER) && defined(_DEBUG)
  mupdf_ctx = ctx;
#endif
  pdf_document* doc = nullptr;
  fz_try(ctx) {
    doc = pdf_open_document(ctx, "t.pdf");
  }
  fz_catch(ctx) {
    fz_report_error(ctx);
    printf("cannot open document\n");
    fz_drop_context(ctx);
    return EXIT_FAILURE;
  }

  pdf_obj* Int = pdf_new_int(ctx, 10);
  pdf_obj* Real = pdf_new_real(ctx, 3.14F);
  pdf_obj* Str = pdf_new_text_string(ctx, "hello");
  pdf_obj* Name = pdf_new_name(ctx, "name");
  pdf_obj* True = PDF_TRUE;
  pdf_obj* False = PDF_FALSE;

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

  pdf_drop_document(ctx, doc);
  fz_drop_context(ctx);
}
