#include <stdio.h>
#include <stdlib.h>
#include "bigint.h"

const char* res100W;

void checkres(size_t count, const char* file_name, const char* res) {
	FILE* fp = fopen(file_name, "r");
	if (NULL == fp) {
		printf("cannot found file:%s\n", file_name);
		return;
	}
	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char* std = (char*)malloc(size * sizeof(char));
	if (NULL == std) {
		printf("out of memory\n");
		fclose(fp);
		return;
	}
	size_t n = fread(std, size, 1, fp);
	if (n != 1) {
		return;
	}
	size_t i = 0;
	while (*std && *res) {
		if (*std != *res) {
			printf("count:%zdW index:%zd is not match, total:%d\n", count, i, size);
			break;
		}
		std++;
		res++;
		i++;
	}
	fclose(fp);
}

void Fibonacci(size_t n) {
	BigInt* a = BigIntNewU(1);
	BigInt* b = BigIntNewU(1);
	BigInt* sum = NULL;
	for (size_t i = 0; i < n*10000; ++i) {
		sum = BigIntAdd(a, b);
		BigIntFree(a);
		a = b;
		b = sum;
	}
	char* p = BigInt2String(sum);
	char path[256];
	sprintf(path, "../../../%zdW.txt", n);
	checkres(n, path, p);
	free(p);
	BigIntFree(a);
	BigIntFree(b);
}

int main(int argc, char* argv[]) {
	//Fibonacci(1);
	Fibonacci(10);
	BigInt* x = BigIntNewU(9000000000000000000);
	BigInt* y = BigIntNewU(9000000000000000000);
	BigInt* z = BigIntAdd(x, y);
	BigInt* zz = BigIntAdd(z, y);
	BigInt* s1 = BigIntSub(zz, z);
	BigInt* m1 = BigIntMul(x, y);
	BigInt* q1 = BigIntDiv(m1, x);
	char*p = BigInt2String(zz);
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