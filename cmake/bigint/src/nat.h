#ifndef _NAT_H_INCLUDE_
#define _NAT_H_INCLUDE_

#include <stdint.h>

#define UintSize (32U << (~(size_t)0 >> 63U)) // 32或者64
#define _W (UintSize)
#define _B (1ULL << (_W))
#define _M (_B - 1)

typedef uint64_t Word;
typedef struct nat {
	Word* nat;
	int64_t len;
	int64_t cap;
}nat;

typedef struct divisor divisor;

nat* natNew(Word v);
nat* natNewLen(int64_t len);
void natSet(nat* p, uint64_t index, Word value);
nat* natCopy(nat* x);
void natCopy2(nat* dst, nat* src);
nat* natPart(nat* p, uint64_t from, uint64_t to);
nat* natNorm(nat* p);
int natCmp(nat* x, nat* y);
void natFree(nat* p);

nat* natAdd(nat* x, nat* y);
Word addVV(nat* z, nat* x, nat* y);
Word addVW(nat* z, nat* x, Word y);

nat* natSub(nat* x, nat* y);
Word subVW(nat* z, nat* x, Word y);
Word subVV(nat* z, nat* x, nat* y);

Word mulWW(Word x, Word y, Word* z1);
Word mulAddVWW(nat* z, nat* x, Word y, Word r);

// u/v, 返回值为商，余数为r
nat* natDiv(nat* u, nat* v, nat** r);
nat* natRem(nat* u, nat* v);

divisor* divisors(int64_t m, Word b, int64_t ndigits, Word bb, int* divisorNum);
char* natI2a(nat* x, bool neg, int base);

int len64(uint64_t x);
int LeadingZeros(uint64_t x);
int LeadingZeros64(uint64_t x);
int TrailingZeros64(uint64_t x);


#endif
