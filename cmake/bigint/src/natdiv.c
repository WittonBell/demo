#include "priv.h"
#include "bigint.h"
#include "nat.h"

#define divRecursiveThreshold 100

size_t nlz(Word x) {
	return LeadingZeros(x);
}

static Word reciprocalWord(Word d1) {
	size_t u = d1 << nlz(d1);
	size_t x1 = ~u;
	size_t x0 = _M;
	size_t rem = 0;
	return (Word)UIntDiv(x1, x0, u, &rem);
}

static Word divWW(Word x1, Word x0, Word y, Word m, Word* r) {
	size_t s = nlz(y);
	if (s != 0) {
		x1 = (x1 << s) | (x0 >> (_W - s));
		x0 <<= s;
		y <<= s;
	}
	size_t d = y;
	size_t t1 = 0;
	size_t t0 = UIntMul(m, x1, &t1);
	size_t c = 0;
	UIntAdd(t0, x0, &c);
	t1 = UIntAdd(t1, x1, &c);
	size_t qq = t1;
	size_t dq1 = 0;
	size_t dq0 = UIntMul(d, qq, &dq1);
	size_t b = 0;
	size_t r0 = UIntSub(x0, dq0, &b);
	size_t r1 = UIntSub(x1, dq1, &b);
	if (r1 != 0) {
		++qq;
		r0 -= d;
	}
	if (r0 >= d) {
		++qq;
		r0 -= d;
	}
	*r = r0 >> s;
	return qq;
}

// 返回余数
static Word natDivWVW(nat z, Word xn, nat x, Word y) {
	Word r = xn;
	if (x.len == 1) {
		size_t rem = 0;
		z.data[0] = UIntDiv(r, x.data[0], y, &r);
		return r;
	}
	Word rec = reciprocalWord(y);
	for (ssize_t i = z.len - 1; i >= 0; --i) {
		z.data[i] = divWW(r, x.data[i], y, rec, &r);
	}
	return r;
}

nat natDivW(nat x, Word y, Word* r) {
	ssize_t m = x.len;
	assert(y != 0);
	if (y == 1) {
		return natCopy(x);
	}
	if (m == 0) {
		return natNew(0);
	}
	nat z = natNewLen(m);
	*r = natDivWVW(z, 0, x, y);
	return natNorm(z);
}

static bool greaterThan(Word x1, Word x2, Word y1, Word y2) {
	return x1 > y1 || x1 == y1 && x2 > y2;
}

static void natDivBasic(nat q, nat u, nat v) {
	ssize_t n = v.len;
	ssize_t m = u.len - n;

	nat qhatv = natNewLen(n + 1);

	Word vn1 = v.data[n - 1];
	Word rec = reciprocalWord(vn1);

	for (int64_t j = m; j >= 0; --j) {
		Word qhat = _M;
		Word ujn = 0;
		if (j + n < u.len) {
			ujn = u.data[j + n];
		}
		if (ujn != vn1) {
			Word rhat = 0;
			qhat = divWW(ujn, u.data[j + n - 1], vn1, rec, &rhat);
			Word vn2 = v.data[n - 2];
			Word x1 = 0;
			Word x2 = mulWW(qhat, vn2, &x1);
			Word ujn2 = u.data[j + n - 2];
			while (greaterThan(x1, x2, rhat, ujn2)) {
				qhat--;
				Word preRhat = rhat;
				rhat += vn1;
				if (rhat < preRhat) {
					break;
				}
				x2 = mulWW(qhat, vn2, &x1);
			}
		}

		qhatv.data[n] = mulAddVWW(natPart(qhatv, 0, n), v, qhat, 0);
		ssize_t qhl = qhatv.len;
		if (j + qhl > u.len && qhatv.data[n] == 0) {
			qhl--;
		}
		Word c = subVV(natPart(u, j, j + qhl), natPart(u, j, u.len), qhatv);
		if (c != 0) {
			nat u3 = natPart(u, j, j + n);
			Word cc = addVV(u3, natPart(u, j, u.len), v);
			if (n < qhl) {
				u.data[j + n] += cc;
			}
			--qhat;
		}
		if (j == m && m == q.len && qhat == 0) {
			continue;
		}
		q.data[j] = qhat;
	}
}

static void natDivRecursiveStep(nat z, nat u, nat v, ssize_t depth, nat tmp, nat* temps, ssize_t tempsNum) {
	u = natNorm(u);
	v = natNorm(v);
	if (u.len == 0) {
		natClear(z);
		return;
	}
	ssize_t n = v.len;
	if (n < divRecursiveThreshold) {
		natDivBasic(z, u, v);
		return;
	}
	ssize_t m = u.len - n;
	if (m < 0) {
		return;
	}

	ssize_t B = n / 2;
	if (temps[depth].data == NULL) {
		temps[depth] = natNewLen(n);
	}
	else {
		temps[depth] = natMake(temps[depth], B + 1);
	}
	ssize_t j = m;
	while (j > B) {
		ssize_t s = B - 1;
		nat uu = natPart(u, j - B, u.len);
		nat qhat = temps[depth];
		natClear(qhat);
		assert(depth + 1 < tempsNum);
		natDivRecursiveStep(qhat, natPart(uu, s, B + n), natPart(v, s, v.len), depth + 1, tmp, temps, tempsNum);
		qhat = natNorm(qhat);

		nat qhatv = natMul(qhat, natPart(v, 0, s));
		for (int i = 0; i < 2; ++i) {
			if (natCmp(qhatv, natNorm(uu)) <= 0) {
				break;
			}
			subVW(qhat, qhat, 1);
			Word c = subVV(natPart(qhatv, 0, s), natPart(qhatv, 0, s), natPart(v, 0, s));
			if (qhatv.len > s) {
				subVW(natPart(qhatv, s, qhatv.len), natPart(qhatv, s, qhatv.len), c);
			}
			addAt(natPart(uu, s, uu.len), natPart(v, s, v.len), 0);
		}
		assert(natCmp(qhatv, natNorm(uu)) <= 0);
		Word c = subVV(natPart(uu, 0, qhatv.len), natPart(uu, 0, qhatv.len), qhatv);
		if (c > 0) {
			subVW(natPart(uu, qhatv.len, uu.len), natPart(uu, qhatv.len, uu.len), c);
		}
		addAt(z, qhat, j - B);
		j -= B;
	}

	ssize_t s = B - 1;
	nat qhat = temps[depth];
	natClear(qhat);
	assert(depth + 1 < tempsNum);
	natDivRecursiveStep(qhat, natNorm(natPart(u, s, u.len)), natPart(v, s, v.len), depth + 1, tmp, temps, tempsNum);
	qhat = natNorm(qhat);
	nat qhatv = natMul(qhat, natPart(v, 0, s));
	for (int i = 0; i < 2; ++i) {
		if (natCmp(qhatv, natNorm(u)) > 0) {
			subVW(qhat, qhat, 1);
			Word c = subVV(natPart(qhatv, 0, s), natPart(qhatv, 0, s), natPart(v, 0, s));
			if (qhatv.len > s) {
				subVW(natPart(qhatv, s, qhatv.len), natPart(qhatv, s, qhatv.len), c);
			}
			addAt(natPart(u, s, u.len), natPart(v, s, v.len), 0);
		}
	}
	assert(natCmp(qhatv, natNorm(u)) <= 0);
	Word c = subVV(natPart(u, 0, qhatv.len), natPart(u, 0, qhatv.len), qhatv);
	if (c > 0) {
		c = subVW(natPart(u, qhatv.len, u.len), natPart(u, qhatv.len, u.len), c);
	}
	assert(c <= 0);
	addAt(z, natNorm(qhat), 0);
}

static void natDivRecursive(nat z, nat u, nat v) {
	ssize_t recDepth = 2 * Len((size_t)v.len);
	nat tmp = natNewLen(3 * v.len);
	nat* temps = calloc(recDepth, sizeof(nat));
	if (temps == NULL) {
		printf("%s\n", "out of memory in natDivRecursive");
		return;
	}
	natClear(z);
	natDivRecursiveStep(z, u, v, 0, tmp, temps, recDepth);
	for (ssize_t i = 0; i < recDepth; ++i) {
		natFree(&temps[i]);
	}
	free(temps);
}

Word shlVU(nat z, nat x, size_t s) {
	if (s == 0) {
		natCopy2(z, x);
		return 0;
	}
	if (z.len == 0) {
		return 0;
	}
	s &= _W - 1;
	size_t s1 = _W - s;
	s1 &= _W - 1;
	Word c = x.data[z.len - 1] >> s1;
	for (ssize_t i = z.len - 1; i > 0; --i) {
		z.data[i] = x.data[i] << s | x.data[i - 1] >> s1;
	}
	z.data[0] = x.data[0] << s;
	return c;
}

static Word shrVU(nat z, nat x, size_t s) {
	assert(x.len == z.len);
	if (s == 0) {
		natCopy2(z, x);
		return 0;
	}
	if (z.len == 0) {
		return 0;
	}
	s &= _W - 1;
	size_t s1 = _W - s;
	s1 &= _W - 1;
	Word c = x.data[0] << s1;
	for (ssize_t i = 1; i < z.len; ++i) {
		z.data[i - 1] = x.data[i - 1] >> s | x.data[i] << s1;
	}
	z.data[z.len - 1] = x.data[z.len - 1] >> s;
	return c;
}

static nat natDivLarge(nat z, nat u, nat uIn, nat vIn, nat* r) {
	ssize_t n = vIn.len;
	ssize_t m = uIn.len - n;

	size_t shift = nlz(vIn.data[n - 1]);
	nat v = natNewLen(n);
	shlVU(v, vIn, shift);
	u = natMake(u, uIn.len + 1);
	u.data[uIn.len] = shlVU(natPart(u, 0, uIn.len), uIn, shift);

	nat q = natMake(z, m + 1);
	if (n < divRecursiveThreshold) {
		natDivBasic(q, u, v);
	}
	else {
		natDivRecursive(q, u, v);
	}
	q = natNorm(q);
	shrVU(u, u, shift);
	*r = natNorm(u);
	return q;
}

nat natDiv(nat z, nat z2, nat u, nat v, nat* r) {
	assert(v.len > 0);
	if (natCmp(u, v) < 0) {
		nat q = natPart(z, 0, 0);
		*r = natSet(z2, u);
		return q;
	}
	if (v.len == 1) {
		Word r2;
		nat q = natDivW(u, v.data[0], &r2);
		*r = natSetWord(z2, r2);
		return q;
	}
	return natDivLarge(z, z2, u, v, r);
}

// returns r such that r = u%v.
// It uses z as the storage for r.
nat natRem(nat z, nat u, nat v) {
	nat r;
	nat qp = natNewLen(0);
	natDiv(qp, z, u, v, &r);
	return r;
}
