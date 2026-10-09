#include <stdio.h>
#include "foo/foo.h"

int main(int argc, char* argv[]) {
	(void)argc;
	(void)argv;
	printf("test\n");
	foo();
	return 0;
}
