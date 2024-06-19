#include <stdio.h>
#include <stdlib.h>
#include "bigint.h"

int main(int argc, char* argv[]) {
	BigInt* x = BigIntNewU(9000000000000000000);
	BigInt* y = BigIntNewU(9000000000000000000);
	BigInt* z = BigIntAdd(x, y);
	BigInt* zz = BigIntAdd(z, y);
	BigInt* s1 = BigIntSub(zz, z);
	BigInt* m1 = BigIntMul(x, y);
	BigInt* q1 = BigIntDiv(m1, x);
	char* p = BigInt2String(zz);
	if (strcmp(p, "27000000000000000000")) {
		printf("error\n");
	}
	printf("%s\n", p);
	free(p);
	p = BigInt2HexStr(zz);
	if (strcmp(p, "176b344f2a78c0000")) {
		printf("error\n");
	}
	printf("%s\n", p);
	free(p);
	p = BigInt2Text(zz, 62);
	if (strcmp(p, "wawhL6Tc75u")) {
		printf("error\n");
	}
	printf("%s\n", p);
	free(p);
	BigIntFree(x);
	BigIntFree(y);
	BigIntFree(z);
	BigIntFree(zz);
	BigIntFree(s1);
	BigIntFree(m1);
	BigIntFree(q1);
	return 0;
}