#include <signal.h>
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
    CATCH(SIGSEGV) {
      printf("test SIGSEGV exception\n");
    }
    div0();
  }
  CATCH(SIGFPE) {
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
      CATCH(SIGSEGV) {
        printf("main SIGSEGV exception\n");
      }
      div0();
    }
    CATCH(SIGFPE) {
      printf("main SIGFPE exception\n");
    }
    test();
    int* p = 0;
    *p = 0;
  }
  CATCH(SIGSEGV) {
    printf("main SIGSEGV exception\n");
  }
  else {
    printf("main other exception\n");
  }

  TRY {
    THROW(1);
  }
  CATCH(1) {
    printf("except 1\n");
  }
  TRY {
    THROW(2);
  }
  CATCH(2) {
    printf("except 2\n");
  }
  TRY {
    THROW(3);
  }
  EXCEPT {
    printf("except other\n");
  }
  printf("ok\n");
  return 0;
}
