#ifndef _NAT_H_INCLUDE_
#define _NAT_H_INCLUDE_

#include "priv.h"

// C/C++左移是常量时，移位长度shift超过数据长度len时，直接变为0；
// 如果是变量，实际移位长度shift会与数据长度len作模运算，即：shift % len，以32为int数据为例，如果左移33位，实际上移位 33 % 32 = 1位。
// 这里需要当移位长度shift超过数据长len时为0。
#define UintSize (32U << (~(size_t)0 >> 63U)) // 32或者64
#define _W (UintSize)
#define _B (((size_t)1) << (_W))
#define _M (_B - 1)

typedef size_t Word;
typedef struct nat {
	Word* data;
	ssize_t len;
	ssize_t cap;
	bool part;
}nat;

typedef struct divisor divisor;

nat natNew(Word v);
nat natNewLen(ssize_t len);
nat natDup(nat x);
nat natPart(nat p, ssize_t from, ssize_t to);
nat natNorm(nat p);
int natCmp(nat x, nat y);
nat natSetWord(nat z, Word x);
nat natMake(nat z, ssize_t n);
void natClear(nat x);
void natFree(nat* p);

static inline void natSetValue(nat t, ssize_t index, Word value) {
	assert(index >= 0 && index < t.len);
	t.data[index] = value;
}

static inline void natCopy(nat dst, nat src) {
	assert(dst.cap >= src.len);
	memcpy(dst.data, src.data, src.len * sizeof(dst.data[0]));
}

static inline nat natSet(nat z, nat x) {
	z = natMake(z, x.len);
	natCopy(z, x);
	return z;
}

static inline void natSwap(nat* x, nat* y) {
	nat t = *x;
	*x = *y;
	*y = t;
}

nat natAdd(nat x, nat y);
Word addVV(nat z, nat x, nat y);
Word addVW(nat z, nat x, Word y);

nat natSub(nat x, nat y);
Word subVW(nat z, nat x, Word y);
Word subVV(nat z, nat x, nat y);

nat natMul(nat x, nat y);
Word mulWW(Word x, Word y, Word* z1);
Word mulAddVWW(nat z, nat x, Word y, Word r);
void addAt(nat z, nat x, ssize_t i);
nat natSqr(nat x);

// u/v, 返回值为商，余数为r
nat natDiv(nat z, nat z2, nat u, nat v, nat* r);
nat natRem(nat z, nat u, nat v);
nat natDivW(nat x, Word y, Word* r);
Word shlVU(nat z, nat x, size_t s);

divisor* divisors(ssize_t m, Word b, ssize_t ndigits, Word bb, int* divisorNum);
char* natI2a(nat x, bool neg, int base);

size_t nlz(Word x);
ssize_t Len(size_t x);
ssize_t LeadingZeros(size_t x);
ssize_t LeadingZeros64(uint64_t x);
ssize_t TrailingZeros(size_t x);
ssize_t TrailingZeros64(uint64_t x);


#endif
