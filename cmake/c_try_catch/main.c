#include <stdio.h>
#include "ctry/except.h"

void segv() {
  int* v = 0;
  *v = 1;
  printf("%p\n", v);
}

void div0() {
  int v = 0;
  printf("%d\n", 1 / v);
}

void test() {
  TRY {
    TRY {
      segv();
    }
    CATCH {
      printf("test SIGSEGV exception\n");
    }
    div0();
  }
  CATCH {
    printf("test SIGFPE exception\n");
  }
}

int main(int argc, char* argv[]) {
  init_try_catch();
  TRY {
    TRY {
      TRY {
        segv();
      }
      CATCH {
        printf("main SIGSEGV exception\n");
      }
      div0();
    }
    CATCH {
      printf("main SIGFPE exception\n");
    }
    test();
    int* p = 0;
    *p = 0;
  }
  CATCH {
    printf("main exception\n");
  }
  printf("ok\n");
  return 0;
}
