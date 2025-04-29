#include <mupdf/fitz.h>
#include <mupdf/fitz/glyph-cache.h>
#include <mupdf/pdf.h>
#include <mupdf/pdf/object.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#endif

#ifndef __cplusplus
#if __STDC_VERSION__ < 202311L
#ifdef nullptr
#undef nullptr
#endif

#define nullptr NULL
#endif
#endif

static void handle_pdf(fz_context *ctx, pdf_document *doc);
static void handle_page(fz_context *ctx, pdf_document *doc, pdf_page *page,
                        pdf_obj *font_obj);

int main(int argc, char **argv) {
#ifdef _WIN32
  SetConsoleOutputCP(65001);
#endif
  fz_context *ctx = fz_new_context(nullptr, nullptr, FZ_STORE_UNLIMITED);
  if (!ctx) {
    printf("cannot create mupdf context\n");
    return EXIT_FAILURE;
  }

  fz_try(ctx) fz_register_document_handlers(ctx);
  fz_catch(ctx) {
    fz_report_error(ctx);
    printf("cannot register document handlers\n");
    fz_drop_context(ctx);
    return EXIT_FAILURE;
  }

  pdf_document *doc = nullptr;
  fz_try(ctx) { doc = pdf_open_document(ctx, argv[1]); }
  fz_catch(ctx) {
    fz_report_error(ctx);
    printf("cannot open document\n");
    fz_drop_context(ctx);
    return EXIT_FAILURE;
  }

  fz_try(ctx) {
    handle_pdf(ctx, doc);
    pdf_save_document(ctx, doc, argv[2], &pdf_default_write_options);
  }
  fz_catch(ctx) {
    fz_report_error(ctx);
    printf("cannot count number of pages\n");
    pdf_drop_document(ctx, doc);
    fz_drop_context(ctx);
    return EXIT_FAILURE;
  }

  pdf_drop_document(ctx, doc);
  fz_drop_context(ctx);
  return EXIT_SUCCESS;
}

static pdf_obj *build_ckj_font(fz_context *ctx, pdf_document *doc) {
  // 创建简体中文字体
  fz_font *font = fz_new_cjk_font(ctx, FZ_ADOBE_GB);
  // wmode 决定编码
  // encoding = wmode ? "UniGB-UTF16-V" : "UniGB-UTF16-H";
  // serif 决定字体的basefont，1为宋体，0为黑体，但是实际上还是宋体
  // basefont = serif ? "Song" : "Heiti";
  pdf_obj *font_obj = pdf_add_cjk_font(ctx, doc, font, FZ_ADOBE_GB, 0, 1);
  // 创建的字体默认是"UniGB-UTF16-V"或者"UniGB-UTF16-H"，
  // 这里需要"GBK-EUC-H"
  pdf_dict_puts(ctx, font_obj, "Encoding", pdf_new_name(ctx, "GBK-EUC-H"));
  return font_obj;
}

static void handle_pdf(fz_context *ctx, pdf_document *doc) {
  pdf_obj *font_obj = build_ckj_font(ctx, doc);
  int page_count = pdf_count_pages(ctx, doc);
  for (int i = 0; i < page_count; ++i) {
    pdf_page *page = pdf_load_page(ctx, doc, i);
    handle_page(ctx, doc, page, font_obj);
    // 释放页面
    pdf_drop_page(ctx, page);
  }
}

static void handle_page(fz_context *ctx, pdf_document *doc, pdf_page *page,
                        pdf_obj *font_obj) {
  pdf_obj *resources = pdf_page_resources(ctx, page);
  if (resources && pdf_is_dict(ctx, resources)) {
    pdf_obj *fonts = pdf_dict_gets(ctx, resources, "Font");
    if (fonts && pdf_is_dict(ctx, fonts)) {
      /* Iterate over all font entries */
      int fontCount = pdf_dict_len(ctx, fonts);
      for (int j = 0; j < fontCount; j++) {
        pdf_obj *key_obj = pdf_dict_get_key(ctx, fonts, j);
        const char *key = pdf_to_name(ctx, key_obj);
        pdf_obj *font = pdf_dict_gets(ctx, fonts, key);

        if (!pdf_is_dict(ctx, font))
          continue;

        pdf_obj *enc = pdf_dict_gets(ctx, font, "Encoding");
        const char *encoding = pdf_to_name(ctx, enc);
        // 如果不是WinAnsiEncoding则continue
        if (strcmp(encoding, "WinAnsiEncoding") != 0) {
          continue;
        }

        /* Read the BaseFont name */
        pdf_obj *bf = pdf_dict_gets(ctx, font, "BaseFont");
        if (!bf || !pdf_is_name(ctx, bf))
          continue;

        const char *fontname = pdf_to_name(ctx, bf);
        uint32_t v = *(uint32_t *)fontname;
        switch (v) {
        case 0xe5cccecb: // 宋体
          fontname = "SimSun";
          break;
        case 0xe5ccdaba: // 黑体
          fontname = "SimHei";
          break;
        case 0xe5ccacbf: // 楷体
          fontname = "SimKai";
          break;
        default:
          continue;
          break;
        }

        // 替换成内置的CJK字体
        pdf_dict_puts(ctx, fonts, key, font_obj);
      }
    }
  }
}
