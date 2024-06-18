#include <stdio.h>
#include <stdlib.h>
#include "bigint.h"

int main(int argc, char* argv[]) {
	BigInt* x = BigIntNewU(9000000000000000000);
	BigInt* y = BigIntNewU(9000000000000000000);
	BigInt* z = BigIntAdd(x, y);
	BigInt* zz = BigIntAdd(z, y);
	char* p = BigInt2String(zz);
	printf("%s\n", p);
	free(p);
	BigIntFree(x);
	BigIntFree(y);
	BigIntFree(z);
	BigIntFree(zz);
	return 0;
}