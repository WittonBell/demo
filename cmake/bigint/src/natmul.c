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
		Word cc = 0;
		Word lo = UIntAdd(z0, c, &cc);
		z.data[i] = lo;
		c = cc + z1;
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

static const ssize_t karatsubaThreshold = 40;

ssize_t karatsubaLen(ssize_t n, ssize_t threshold) {
	ssize_t i = 0;
	while (n > threshold) {
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
	ssize_t n = y.len;
	if ((n & 1) != 0 || n < karatsubaThreshold || n < 2) {
		basicMul(z, x, y);
		return;
	}
	ssize_t n2 = n >> 1;
	nat x0 = natPart(x, 0, n2);
	nat x1 = natPart(x, n2, x.len);
	nat y0 = natPart(y, 0, n2);
	nat y1 = natPart(y, n2, y.len);

	karatsuba(z, x0, y0);
	nat z1 = natPart(z, n, z.len);
	karatsuba(z1, x1, y1);

	ssize_t s = 1;
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

void addAt(nat z, nat x, ssize_t i) {
	if (x.len > 0) {
		nat z1 = natPart(z, i, i + x.len);
		nat z2 = natPart(z, i, z.len);
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
	ssize_t k = karatsubaLen(n, karatsubaThreshold);
	nat x0 = natPart(x, 0, k);
	nat y0 = natPart(y, 0, k);
	nat z = natNewLen(fmax(6 * k, m + n));
	karatsuba(z, x0, y0);

	z = natPart(z, 0, m + n);
	natClear(natPart(z, 2 * k, z.len));

	if (k < n || m != n) {
		x0 = natNorm(x0);
		nat y1 = natPart(y, k, y.len);
		nat t = natMul(x0, y0);
		addAt(z, t, k);

		y0 = natNorm(y0);
		for (ssize_t i = k; i < x.len; i += k) {
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

static const ssize_t basicSqrThreshold = 20;
static const ssize_t karatsubaSqrThreshold = 260;

nat basicSqr(nat z,nat x) {
	ssize_t n = x.len;
	nat t = natNewLen(2 * n);
	z.data[0] = mulWW(x.data[0], x.data[0], &z.data[1]);
	for (ssize_t i = 1; i < n; ++i) {
		Word d = x.data[i];
		z.data[2 * i] = mulWW(d, d, &z.data[2 * i + 1]);
		t.data[2 * i] = addMulVVW(natPart(t, i, 2 * i), natPart(x, 0, i), d);
	}
	nat t1 = natPart(t, 1, 2 * n - 1);
	t.data[2 * n - 1] = shlVU(t1, t1, 1);
	addVV(z, z, t);
	return z;
}

void karatsubaSqr(nat z, nat x) {
	ssize_t n = x.len;
	if (n & 1 != 0 || n < karatsubaSqrThreshold || n < 2) {
		basicSqr(natPart(z, 0, 2 * n), x);
		return;
	}

	ssize_t n2 = n >> 1;
	nat x0 = natPart(x, 0, n2);
	nat x1 = natPart(x, n2, x.len);

	karatsubaSqr(z, x0);
	nat z1 = natPart(z, n, z.len);
	karatsubaSqr(z1, x1);

	nat xd = natPart(z, 2 * n, 2 * n + n2);
	if (subVV(xd, x1, x0) != 0) {
		subVV(xd, x0, x1);
	}
	nat p = natPart(z, n * 3, z.len);
	karatsubaSqr(p, xd);

	nat r = natPart(z, n * 4, z.len);
	natCopy2(r, natPart(z, 0, n * 2));

	nat z2 = natPart(z, n2, z.len);
	karatsubaAdd(z2, r, n);
	karatsubaAdd(z2, natPart(r, n, r.len), n);
	karatsubaSub(z2, p, n);
}

// 计算平方
nat natSqr(nat x) {
	ssize_t n = x.len;
	if (n == 0) {
		return natNewLen(0);
	}
	if (n == 1) {
		Word d = x.data[0];
		nat z = natNewLen(2);
		z.data[0] = mulWW(d, d, &z.data[1]);
		return natNorm(z);
	}
	if (n < basicSqrThreshold) {
		nat z = natNewLen(2 * n);
		basicMul(z, x, x);
		return natNorm(z);
	}
	if (n < karatsubaSqrThreshold) {
		nat z = natNewLen(2 * n);
		z = basicSqr(z, x);
		return natNorm(z);
	}
	ssize_t k = karatsubaLen(n, karatsubaSqrThreshold);
	nat x0 = natPart(x, 0, k);
	nat z = natNewLen(fmax(6 * k, 2 * n));
	karatsubaSqr(z, x0);
	z = natPart(z, 0, 2 * n);
	natClear(natPart(z, 2 * k, z.len));

	if (k < n) {
		x0 = natNorm(x0);
		nat x1 = natPart(x, k, x.len);
		nat t = natMul(x0, x1);
		addAt(z, t, k);
		addAt(z, t, k);
		t = natSqr(x1);
		addAt(z, t, 2 * k);
	}
	return natNorm(z);
}