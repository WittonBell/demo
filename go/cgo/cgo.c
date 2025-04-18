#ifdef _WIN32
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include "cgo.h"
#include "_cgo_export.h"

static GoString pp;

void sayHello(char *str) {
#ifdef _WIN32
	// 让Windows的控制台使用UTF-8编码输出，否则会显示成乱码
	UINT codePage = GetConsoleOutputCP();
	if (codePage != 65001) {
		SetConsoleOutputCP(65001);
	}
#endif
  printf("C语言：%s\n", str);
  // Go语言中使用C.CString转换的字符串需要手动释放内存
  free(str);
  //在VSCode的调试控制台中，printf可能不会立即输出，需要调用一次flush才会立即显示
  fflush(NULL);
  char *p = goTest("测试");
  printf("%s\n", p);
  // Go语言中使用C.CString转换的字符串需要手动释放内存
  free(p);
  pp = goString();
  // 由于Go字符串不是以null结束，所以需要按指定长度输出字符串
  printf("%.*s\n", (int)(pp.n), pp.p);
  fflush(NULL);
}

void testGoString() {
	printf("测试之前保存的Go中的字符串\n");
	printf("%.*s\n", (int)(pp.n), pp.p);
	fflush(NULL);
}
