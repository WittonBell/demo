#include "priv.h"
#include "bigint.h"
#include "nat.h"

Word addVV(nat z, nat x, nat y) {
	Word c = 0;
	for (int i = 0; i < z.len && i < x.len && i < y.len; ++i) {
		size_t zi = UIntAdd(x.data[i], y.data[i], &c);
		z.data[i] = zi;
	}
	return c;
}

static Word addVWlarge(nat z, nat x, Word y) {
	Word c = y;
	Word oc = 0;
	for (ssize_t i = 0; i < z.len && i < x.len; ++i) {
		if (c == 0) {
			memcpy(&z.data[i], &x.data[i], x.len * sizeof(Word));
			return c;
		}
		size_t zi = UIntAdd(x.data[i], c, &oc);
		z.data[i] = (Word)zi;
		c = oc;
	}
	return c;
}

Word addVW(nat z, nat x, Word y) {
	if (z.len > 32) {
		return addVWlarge(z, x, y);
	}
	Word c = y;
	Word oc = 0;
	for (ssize_t i = 0; i < z.len && i < x.len; ++i) {
		size_t zi = UIntAdd(x.data[i], c, &oc);
		z.data[i] = (Word)zi;
		c = oc;
	}
	return c;
}

nat natAdd(nat x, nat y) {
	ssize_t m = x.len;
	ssize_t n = y.len;

	if (m < n) {
		return natAdd(y, x);
	}
	if (m == 0 || n == 0) {
		return x;
	}
	nat z = natNewLen(m + 1);
	nat z1 = natPart(z, 0, n);
	Word c = addVV(z1, x, y);
	if (m > n) {
		nat zz = natPart(z, n, m);
		nat xx = natPart(x, n, x.len);
		c = addVW(zz, xx, c);
	}
	natSet(z, m, c);
	return natNorm(z);
}