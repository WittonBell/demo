#include <direct.h>
#include <io.h>
#include <mupdf/fitz.h>
#include <mupdf/fitz/version.h>
#include <mupdf/pdf.h>
#include <mupdf/pdf/document.h>
#include <mupdf/pdf/font.h>
#include <mupdf/pdf/object.h>
#include <mupdf/pdf/page.h>
#include <mupdf/pdf/xref.h>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

const char* mupdf_version = FZ_VERSION;

using namespace std;

#if 0
extern "C" {
struct pdf_obj {
  short refs;
  unsigned char kind;
  unsigned char flags;
};

struct keyval
{
	pdf_obj *k;
	pdf_obj *v;
};

typedef struct {
  pdf_obj super;
  union {
    int64_t i;
    float f;
  } u;
} pdf_obj_num;

typedef struct {
  pdf_obj super;
  char *text; /* utf8 encoded text string */
  size_t len;
  char buf[];
} pdf_obj_string;

typedef struct {
  pdf_obj super;
  char n[];
} pdf_obj_name;

typedef struct {
  pdf_obj super;
  pdf_document *doc;
  int parent_num;
  int len;
  int cap;
  pdf_obj **items;
} pdf_obj_array;

typedef struct {
  pdf_obj super;
  pdf_document *doc;
  int parent_num;
  int len;
  int cap;
  struct keyval *items;
} pdf_obj_dict;

typedef struct {
  pdf_obj super;
  pdf_document *doc; /* Only needed for arrays, dicts and indirects */
  int num;
  int gen;
} pdf_obj_ref;
}
#endif

int main(int argc, char* argv[]) {
  const char* p = _getcwd(NULL, 0);
#if 0
  pdf_obj_num *a = nullptr;
  pdf_obj_string *b = nullptr;
  pdf_obj_name *c = nullptr;
  pdf_obj_array *d = nullptr;
  pdf_obj_dict *e = nullptr;
  pdf_obj_ref *f = nullptr;
#endif
  fz_context* ctx = fz_new_context(nullptr, nullptr, FZ_STORE_UNLIMITED);
#ifdef _MSC_VER
  pdf_document* doc = pdf_open_document(ctx, "../../../t.pdf");
#else
  pdf_document* doc = pdf_open_document(ctx, "t.pdf");
#endif
  pdf_obj* Int = pdf_new_int(ctx, 10);
  pdf_obj* Real = pdf_new_real(ctx, 3.14);
  pdf_obj* Str = pdf_new_string(ctx, "hello", 5);
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
  pdf_obj* ref1 = pdf_new_indirect(ctx, doc, 4234, 0);
  int ret = pdf_is_stream(ctx, ref1);
  fz_buffer* buff = nullptr;
  if (ret) {
    buff = pdf_load_stream(ctx, ref1);
  }
  pdf_obj* nil = nullptr;
  std::vector<int> vec = {10, 20, 30};
  string str = "测试";
  vector<int> v;
  for (int i = 0; i < 1000; ++i) {
    v.push_back(i);
  }
  map<int, int> m;
  m.insert(std::make_pair(1, 10));
  cout << str << "\n";
}
