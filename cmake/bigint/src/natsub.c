#include "priv.h"
#include "bigint.h"
#include "nat.h"


Word subVV(nat* z, nat* x, nat* y) {
	Word c = 0;
	for (uint64_t i = 0; i < z->len && i < x->len && i < y->len; ++i) {
		uint64_t zi = UIntSub64(x->nat[i], y->nat[i], &c);
		z->nat[i] = zi;
	}
	return c;
}

static Word subVWlarge(nat* z, nat* x, Word y) {
	Word c = y;
	Word oc = 0;
	for (uint64_t i = 0; i < z->len && i < x->len; ++i) {
		if (c == 0) {
			memcpy(&z->nat[i], &x->nat[i], x->len * sizeof(Word));
			return c;
		}
		uint64_t zi = UIntSub64(x->nat[i], c, &oc);
		z->nat[i] = zi;
		c = oc;
	}
	return c;
}

Word subVW(nat* z, nat* x, Word y) {
	if (z->len > 32) {
		return subVWlarge(z, x, y);
	}
	Word c = y;
	Word oc = 0;
	for (uint64_t i = 0; i < z->len && i < x->len; ++i) {
		uint64_t zi = UIntSub64(x->nat[i], c, &oc);
		z->nat[i] = zi;
		c = oc;
	}
	return c;
}

nat* natSub(nat* x, nat* y) {
	uint64_t m = x->len;
	uint64_t n = y->len;
	assert(m >= n);
	if (m == 0 || n == 0) {
		return x;
	}
	nat* z = natNewLen(m);
	nat* z1 = natPart(z, 0, n);
	Word c = subVV(z1, x, y);
	if (m > n) {
		nat* zz = natPart(z, n, z->len);
		nat* xx = natPart(x, n, x->len);
		c = subVW(zz, xx, c);
		natFree(zz);
		natFree(xx);
	}
	natFree(z1);
	assert(c == 0);
	return natNorm(z);
}