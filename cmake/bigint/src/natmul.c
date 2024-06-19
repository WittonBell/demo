#include "priv.h"
#include "bigint.h"
#include "nat.h"

Word mulWW(Word x, Word y, Word* z1) {
	return UIntMul(x, y, z1);
}

static Word mulAddWWW(Word x, Word y, Word c, Word* z1) {
	size_t hi = 0;
	size_t lo = UIntMul(x, y, &hi);
	size_t cc = 0;
	lo = UIntAdd(lo, c, &cc);
	*z1 = hi + cc;
	return lo;
}

static Word addMulVVW(nat z, nat x, Word y) {
	Word c = 0;
	for (ssize_t i = 0; i < z.len && i < x.len; ++i) {
		Word z1 = 0;
		Word z0 = mulAddWWW(x.data[i], y, z.data[i], &z1);
		Word lo = UIntAdd(z0, c, &c);
		z.data[i] = lo;
		c += z1;
	}
	return c;
}

static void basicMul(nat z, nat x, nat y) {
	for (ssize_t i = 0; i < y.len; ++i) {
		if (y.data[i] != 0) {
			nat zz = natPart(z, i, i + x.len);
			z.data[x.len + i] = addMulVVW(zz, x, y.data[i]);
		}
	}
}

static Word mulAddWW(Word x, Word y, Word* z1) {
	size_t hi = 0;
	size_t lo = UIntMul(x, y, &hi);
	*z1 = hi;
	return lo;
}

Word mulAddVWW(nat z, nat x, Word y, Word r) {
	Word c = r;
	for (ssize_t i = 0; i < z.len && i < x.len; ++i) {
		z.data[i] = mulAddWWW(x.data[i], y, c, &c);
	}
	return c;
}

static nat natMulAddWW(nat x, Word y, Word r) {
	ssize_t m = x.len;
	if (m == 0 || y == 0) {
		return natNew(r);
	}
	nat z = natNewLen(m + 1);
	nat zz = natPart(z, 0, m);
	z.data[m] = mulAddVWW(zz, x, y, r);
	return natNorm(z);
}

static const uint64_t karatsubaThreshold = 40;

static uint64_t karatsubaLen(uint64_t n, uint64_t threshold) {
	uint64_t i = 0;
	while (n > threshold)
	{
		n >>= 1;
		++i;
	}
	return n << i;
}

static void karatsubaAdd(nat z, nat x, uint64_t n) {
	nat z1 = natPart(z, 0, n);
	Word c = addVV(z1, z, x);
	if (c != 0) {
		nat z2 = natPart(z, n, n + n >> 1);
		nat z3 = natPart(z, n, z.len);
		addVW(z2, z3, c);
	}
}

static void karatsubaSub(nat z, nat x, uint64_t n) {
	nat z1 = natPart(z, 0, n);
	Word c = subVV(z1, z, x);
	if (c != 0) {
		nat z2 = natPart(z, n, n + n >> 1);
		nat z3 = natPart(z, n, z.len);
		subVW(z2, z3, c);
	}
}

static void karatsuba(nat z, nat x, nat y) {
	uint64_t n = y.len;
	if ((n & 1) != 0 || n < karatsubaThreshold || n < 2) {
		basicMul(z, x, y);
		return;
	}
	uint64_t n2 = n >> 1;
	nat x0 = natPart(x, 0, n2);
	nat x1 = natPart(x, n2, x.len);
	nat y0 = natPart(y, 0, n2);
	nat y1 = natPart(y, n2, y.len);

	karatsuba(z, x0, y0);
	nat z1 = natPart(z, n, z.len);
	karatsuba(z1, x1, y1);

	uint64_t s = 1;
	nat xd = natPart(z, 2 * n, 2 * n + n2);
	if (subVV(xd, x1, x0) != 0) {
		s = -s;
		subVV(xd, x0, x1);
	}

	nat yd = natPart(z, 2 * n + n2, 3 * n);
	if (subVV(yd, y0, y1) != 0) {
		s = -s;
		subVV(yd, y1, y0);
	}

	nat p = natPart(z, n * 3, z.len);
	karatsuba(p, xd, yd);

	nat r = natPart(z, n * 4, z.len);
	memcpy(r.data, &z.data[0], n * 2 * sizeof(Word));

	nat z2 = natPart(z, n2, z.len);
	karatsubaAdd(z2, r, n);
	nat r1 = natPart(r, n, r.len);
	karatsubaAdd(z2, r1, n);
	if (s > 0) {
		karatsubaAdd(z2, p, n);
	}
	else {
		karatsubaSub(z2, p, n);
	}
}

static void addAt(nat z, nat x, uint64_t i) {
	if (x.len > 0) {
		nat z1 = natPart(z, i, i + x.len);
		nat z2 = natPart(z, i, x.len);
		Word c = addVV(z1, z2, x);
		if (c != 0) {
			ssize_t j = i + x.len;
			if (j < z.len) {
				nat z3 = natPart(z, j, z.len);
				addVW(z3, z3, c);
			}
		}
	}
}

nat natMul(nat x, nat y) {
	ssize_t m = x.len;
	ssize_t n = y.len;
	if (m < n) {
		return natMul(y, x);
	}
	if (m == 0 || n == 0) {
		return natNew(0);
	}
	if (n == 1) {
		return natMulAddWW(x, y.data[0], 0);
	}
	if (n < karatsubaThreshold) {
		nat z = natNewLen(m + n);
		basicMul(z, x, y);
		return natNorm(z);
	}
	uint64_t k = karatsubaLen(n, karatsubaThreshold);
	nat x0 = natPart(x, 0, k);
	nat y0 = natPart(y, 0, k);
	nat z = natNewLen(fmax(6 * k, m + n));
	karatsuba(z, x0, y0);

	nat z1 = natPart(z, 0, m + n);
	nat z2 = natPart(z1, 2 * k, z1.len);
	memset(z2.data, 0, z2.len * sizeof(z2.data[0]));

	if (k < n || m != n) {
		x0 = natNorm(x0);
		nat y1 = natPart(y, k, y.len);
		nat t = natMul(x0, y0);
		addAt(z, t, k);

		y0 = natNorm(y0);
		for (uint64_t i = k; i < x.len; i += k) {
			nat xi = natPart(x, i, x.len);
			if (xi.len > k) {
				xi = natPart(xi, 0, k);
			}
			xi = natNorm(xi);
			t = natMul(xi, y0);
			addAt(z, t, i);
			t = natMul(xi, y1);
			addAt(z, t, i + k);
		}
	}
	return natNorm(z);
}