#include <mupdf/fitz.h>
#include <mupdf/pdf.h>
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

static void handle_pdf(fz_context *ctx, fz_document *doc);
static void handle_page(fz_context *ctx, fz_document *doc, fz_page *page);

int main(int argc, char **argv) {
#ifdef _WIN32
  // Windows控制台，需要设置成UTF8输出编码，以免显示乱码
  SetConsoleOutputCP(65001);
#endif

  if (argc < 3) {
    printf("需要传入参数：格式：%s <源pdf> <目标pdf>\n", argv[0]);
    return EXIT_FAILURE;
  }
  // 首先创建一个fz_context
  fz_context *ctx = fz_new_context(nullptr, nullptr, FZ_STORE_UNLIMITED);
  if (!ctx) {
    printf("cannot create mupdf context\n");
    return EXIT_FAILURE;
  }

  // 注册默认的文档处理器
  fz_try(ctx) { fz_register_document_handlers(ctx); }
  fz_catch(ctx) {
    fz_report_error(ctx);
    printf("cannot register document handlers\n");
    fz_drop_context(ctx);
    return EXIT_FAILURE;
  }

  fz_document *doc = nullptr;
  // 打开文档
  fz_try(ctx) { doc = fz_open_document(ctx, argv[1]); }
  fz_catch(ctx) {
    fz_report_error(ctx);
    printf("cannot open document\n");
    fz_drop_context(ctx);
    return EXIT_FAILURE;
  }

  fz_try(ctx) {
    // 处理文档
    handle_pdf(ctx, doc);
    // 保存文档
    pdf_save_document(ctx, (pdf_document *)doc, argv[2],
                      &pdf_default_write_options);
  }
  fz_catch(ctx) {
    fz_report_error(ctx);
    printf("cannot count number of pages\n");
    fz_drop_document(ctx, doc);
    fz_drop_context(ctx);
    return EXIT_FAILURE;
  }

  // 清理资源
  fz_drop_document(ctx, doc);
  fz_drop_context(ctx);
  return EXIT_SUCCESS;
}

static void handle_pdf(fz_context *ctx, fz_document *doc) {
  // 获取文档总页数
  int page_count = fz_count_pages(ctx, doc);
  // 遍历每一页
  for (int i = 0; i < page_count; ++i) {
    // 加载页面
    fz_page *page = fz_load_page(ctx, doc, i);
    // 处理页面
    handle_page(ctx, doc, page);
    // 释放页面
    fz_drop_page(ctx, page);
  }
}

static void handle_page(fz_context *ctx, fz_document *doc, fz_page *page) {
  fz_stext_options opts = {FZ_STEXT_PRESERVE_IMAGES |
                           FZ_STEXT_PRESERVE_LIGATURES};
  // 根据选项获取页面的结构化页面数据
  fz_stext_page *stext_page = fz_new_stext_page_from_page(ctx, page, &opts);

  // 文本转换用的临时空间
  // 由于文本字符的存储是一个unicode字符值，以int存储的所以一个8字节的空间足够了
  char buf[8];
  // 遍历结构化页面的块
  for (fz_stext_block *text_block = stext_page->first_block; text_block;
       text_block = text_block->next) {
    // 如果不是文本块，则不管它，只需要文本块
    if (text_block->type != FZ_STEXT_BLOCK_TEXT) {
      continue;
    }
    // 遍历文本块中的行
    for (fz_stext_line *text_line = text_block->u.t.first_line; text_line;
         text_line = text_line->next) {
      // 遍历行中的每一个字符
      for (fz_stext_char *text_char = text_line->first_char; text_char;
           text_char = text_char->next) {
        // 获取字符的值，是以Unicode存储的
        const int c = text_char->c;
        // 转换成UTF8编码
        const int num = fz_runetochar(buf, c);
        // 设置结束符
        buf[num] = 0;
        // 输出UTF8字符
        printf("%s", buf);
      }
    }
    printf("\n");
  }

  // 清理当前页资源
  fz_drop_stext_page(ctx, stext_page);
}
