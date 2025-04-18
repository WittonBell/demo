//go:build ignore
#include "cgo.h"
#include <stdio.h>

int main(int argc, char* argv[]) {
	sum_positive(1,2);
	printf("test\n");
	return 0;
}
