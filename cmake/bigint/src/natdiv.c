#include "priv.h"
#include "bigint.h"
#include "nat.h"

static uint64_t nlz(Word x) {
	return LeadingZeros(x);
}

static Word reciprocalWord(Word d1) {
	uint64_t u = d1 << nlz(d1);
	uint64_t x1 = ~u;
	uint64_t x0 = _M;
	uint64_t rem = 0;
	return (Word)UIntDiv64(x1, x0, u, &rem);
}

static Word divWW(Word x1, Word x0, Word y, Word m, Word* r) {
	uint64_t s = nlz(y);
	if (s != 0) {
		x1 = x1 << s | x0 >> (_W - s);
		x0 <<= s;
		y <<= s;
	}
	uint64_t d = y;
	uint64_t t1 = 0;
	uint64_t t0 = UIntMul64(m, x1, &t1);
	uint64_t c = 0;
	UIntAdd64(t0, x0, &c);
	t1 = UIntAdd64(t1, x1, &c);
	uint64_t qq = t1;
	uint64_t dq1 = 0;
	uint64_t dq0 = UIntMul64(d, qq, &dq1);
	uint64_t b = 0;
	uint64_t r0 = UIntSub64(x0, dq0, &b);
	uint64_t r1 = UIntSub64(x1, dq1, &b);
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
static Word natDivWVW(nat* z, Word xn, nat* x, Word y) {
	Word r = xn;
	if (x->len == 1) {
		uint64_t rem = 0;
		uint64_t q = UIntDiv64(r, x->nat[0], y, &rem);
		r = rem;
		z->nat[0] = q;
		return r;
	}
	Word rec = reciprocalWord(y);
	for (uint64_t i = z->len - 1; i >= 0; --i) {
		z->nat[i] = divWW(r, x->nat[i], y, rec, &r);
	}
	return r;
}

static nat* natDivW(nat* x, Word y, Word* r) {
	uint64_t m = x->len;
	assert(y != 0);
	if (y == 1) {
		return natCopy(x);
	}
	if (m == 0) {
		return natNew(0);
	}
	nat* z = natNewLen(m);
	*r = natDivWVW(z, 0, x, y);
	return natNorm(z);
}

static bool greaterThan(Word x1, Word x2, Word y1, Word y2) {
	return x1 > y1 || x1 == y1 && x2 > y2;
}

static void natDivBasic(nat* q, nat* u, nat* v) {
	int64_t n = v->len;
	int64_t m = u->len - n;

	nat* qhatv = natNewLen(n + 1);

	Word vn1 = v->nat[n - 1];
	Word rec = reciprocalWord(vn1);

	for (int64_t j = m; j >= 0; --j) {
		Word qhat = _M;
		Word ujn = 0;
		if (j + n < u->len) {
			ujn = u->nat[j + n];
		}
		if (ujn != vn1) {
			Word rhat = 0;
			qhat = divWW(ujn, u->nat[j + n - 1], vn1, rec, &rhat);
			Word vn2 = v->nat[n - 2];
			Word x1 = 0;
			Word x2 = mulWW(qhat, vn2, &x1);
			Word ujn2 = u->nat[j + n - 2];
			while (greaterThan(x1, x2, rhat, ujn2)) {
				qhat--;
				uint64_t preRhat = rhat;
				rhat += vn1;
				if (rhat < preRhat) {
					break;
				}
				x2 = mulWW(qhat, vn2, &x1);
			}
		}

		qhatv->nat[n] = mulAddVWW(qhatv, v, qhat, 0);
		int64_t qhl = qhatv->len;
		if (j + qhl > u->len && qhatv->nat[n] == 0) {
			qhl--;
		}
		nat* u1 = natPart(u, j, j + qhl);
		nat* u2 = natPart(u, j, u->len);
		Word c = subVV(u1, u2, qhatv);
		if (c != 0) {
			nat* u3 = natPart(u, j, j + n);
			Word cc = addVV(u3, u2, v);
			if (n < qhl) {
				u->nat[j + n] += cc;
			}
			--qhat;
		}
		if (j == m && m == q->len && qhat == 0) {
			continue;
		}
		q->nat[j] = qhat;
	}
}

static void natDivRecursive(nat* q, nat* u, nat* v) {

}

static Word shlVU(nat* z, nat* x, uint64_t s) {
	if (s == 0) {
		natCopy2(z, x);
		return 0;
	}
	if (z->len == 0) {
		return 0;
	}
	s &= _W - 1;
	uint64_t s1 = _W - s;
	s1 &= _W - 1;
	Word c = x->nat[z->len - 1] >> s1;
	for (int64_t i = z->len - 1; i > 0; --i) {
		z->nat[i] = x->nat[i] << s | x->nat[i - 1] >> s1;
	}
	z->nat[0] = x->nat[0] << s;
	return c;
}

static Word shrVU(nat* z, nat* x, uint64_t s) {
	assert(x->len == z->len);
	if (s == 0) {
		natCopy2(z, x);
		return 0;
	}
	if (z->len == 0) {
		return 0;
	}
	s &= _W - 1;
	uint64_t s1 = _W - s;
	s1 &= _W - 1;
	Word c = x->nat[0] << s1;
	for (int64_t i = 1; i < z->len; ++i) {
		z->nat[i - 1] = x->nat[i - 1] >> s | x->nat[i] << s1;
	}
	z->nat[z->len - 1] = x->nat[z->len - 1] >> s;
	return c;
}


#define divRecursiveThreshold 100
static nat* natDivLarge(nat* uIn, nat* vIn, nat** r) {
	uint64_t n = vIn->len;
	uint64_t m = uIn->len - n;

	uint64_t shift = nlz(vIn->nat[n - 1]);
	nat* v = natNewLen(n);
	shlVU(v, vIn, shift);
	nat* u = natNewLen(uIn->len + 1);
	u->nat[uIn->len] = shlVU(u, uIn, shift);

	nat* q = natNewLen(m + 1);
	if (n < divRecursiveThreshold) {
		natDivBasic(q, u, v);
	}
	else {
		natDivRecursive(q, u, v);
	}
	natNorm(q);
	shrVU(u, u, shift);
	natNorm(u);
	*r = u;
	return q;
}

nat* natDiv(nat* u, nat* v, nat** r) {
	if (natCmp(u, v) < 0) {
		*r = natCopy(u);
		return natNew(0);
	}
	if (v->len == 1) {
		Word rr;
		nat* q = natDivW(u, v->nat[0], &rr);
		*r = natNew(rr);
		return q;
	}
	return natDivLarge(u, v, r);
}

nat* natRem(nat* u, nat* v) {
	nat* r = NULL;
	natDiv(u, v, &r);
	return r;
}