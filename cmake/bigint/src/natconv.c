#include "priv.h"
#include "nat.h"

typedef struct divisor {
	nat     bbb;
	ssize_t nbits;
	ssize_t ndigits;
}divisor;

static const int leafSize = 8;

static uint64_t natBitLen(nat x) {
	ssize_t i = x.len - 1;
	if (i >= 0) {
		uint64_t top = x.data[i];
		top |= top >> 1;
		top |= top >> 2;
		top |= top >> 4;
		top |= top >> 8;
		top |= top >> 16;
		top |= top >> 16 >> 16;
		return i * _W + Len(top);
	}
	return 0;
}

// 当x为2的i次方时，返回指数i；否则返回-1
static ssize_t natIsPower2(nat x) {
	ssize_t i = 0;
	while (x.data[i] == 0) {
		++i;
	}
	if (i == x.len - 1 && (x.data[i] & (x.data[i] - 1)) == 0) {
		return i * _W + (size_t)TrailingZeros(x.data[i]);
	}
	return -1;
}

static nat natExpNNMontgomery(nat z, nat x, nat y, nat m) {
	return natNorm(z);
}

static nat natExpNNWindowed(nat z, nat x, nat y, size_t logM) {
	return natNorm(z);
}

static nat natExpNNMontgomeryEven(nat z, nat x, nat y, nat m) {
	return natNorm(z);
}

static nat natExpNN(nat x, nat y, nat m, bool slow) {
	if (m.len == 1 && m.data[0] == 1) {
		return natNew(0);
	}
	if (y.len == 0) {
		return natNew(1);
	}
	if (x.len == 0) {
		return natNew(0);
	}
	if (x.len == 1 && x.data[0] == 1) {
		return natNew(1);
	}
	if (y.len == 1 && y.data[0] == 1) {
		if (m.len != 0) {
			return natRem(x, m);
		}
		return natCopy(x);
	}
	// y > 1
	if (m.len != 0) {
		nat z = natNewLen(m.len);
		if (y.len > 1 && !slow) {
			if ((m.data[0] & 1) == 1) {
				return natExpNNMontgomery(z, x, y, m);
			}
			ssize_t logM = natIsPower2(m);
			if (logM >= 0) {
				return natExpNNWindowed(z, x, y, logM);
			}
			return natExpNNMontgomeryEven(z, x, y, m);
		}
	}

	nat z = natCopy(x);
	Word v = y.data[y.len - 1];
	size_t shift = nlz(v) + 1;
	v <<= shift;

	const size_t mask = (size_t)1 << (_W - 1);
	size_t w = _W - (ssize_t)shift;

	nat q, zz, r;
	for (size_t j = 0; j < w; ++j) {
		zz = natSqr(z);
		natSwap(&z, &zz);

		if (v & mask != 0) {
			zz = natMul(z, x);
			natSwap(&z, &zz);
		}
		if (m.len != 0) {
			nat r;
			zz = natDiv(z, m, &r);
			natSwap(&zz, &q);
			natSwap(&z, &r);
		}
		v <<= 1;
	}
	for (ssize_t i = y.len - 2; i >= 0; --i) {
		v = y.data[i];
		for (ssize_t j = 0; j < _W; ++j) {
			zz = natSqr(z);
			natSwap(&z, &zz);
			if (v & mask != 0) {
				zz = natMul(z, x);
				natSwap(&z, &zz);
			}
			if (m.len != 0) {
				zz = natDiv(z, m, &r);
				natSwap(&zz, &q);
				natSwap(&r, &z);
			}
			v <<= 1;
		}
	}
	return natNorm(z);
}

nat natExpWW(Word x, Word y) {
	nat m = natNewLen(0);
	nat xx = natNew(x);
	nat yy = natNew(y);
	nat z = natExpNN(xx, yy, m, false);
	return z;
}

divisor* divisors(ssize_t m, Word b, ssize_t ndigits, Word bb, int* divisorNum) {
	if (leafSize == 0 || m <= leafSize) {
		return NULL;
	}

	ssize_t k = 1;
	for (ssize_t words = leafSize; words < (m>>1U); words <<= 1U) {
		++k;
	}
	divisor* table = NULL;
	table = (divisor*)calloc(k, sizeof(divisor));
	if (table == NULL) {
		return NULL;
	}

	*divisorNum = k;
	if (table[k - 1].ndigits == 0) {
		for (uint64_t i = 0; i < k; ++i) {
			if (table[i].ndigits == 0) {
				if (i == 0) {
					table[0].bbb = natExpWW(bb, (Word)leafSize);
					table[0].ndigits = ndigits * leafSize;
				}
				else {
					table[i].bbb = natSqr(table[i - 1].bbb);
					table[i].ndigits = 2 * table[i - 1].ndigits;
				}
				nat larger = natCopy(table[i].bbb);
				while (mulAddVWW(larger, larger, b, 0) == 0) {
					natCopy2(table[i].bbb, larger);
					table[i].ndigits++;
				}
				table[i].nbits = natBitLen(table[i].bbb);
			}
		}
	}
	return table;
}

static const int MaxBase = 10 + ('z' - 'a' + 1) + ('Z' - 'A' + 1);
static const char* digits = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

static Word maxPow(Word b, ssize_t* n) {
	Word p = b;
	*n = 1;
	for (ssize_t max = _M / b; p < max;) {
		p *= b;
		++*n;
	}
	return p;
}

void convertWords(nat q, char* s, ssize_t slen, Word b, int64_t ndigits, Word bb, divisor* table, int tableNum) {
	if (table != NULL) {
		int index = tableNum - 1;
		nat r;
		while (q.len > leafSize) {
			ssize_t maxLen = natBitLen(q);
			ssize_t minLen = maxLen >> 1;
			while (index > 0 && table[index - 1].nbits > minLen) {
				--index;
			}
			if (table[index].nbits >= maxLen && natCmp(table[index].bbb, q) >= 0) {
				--index;
				assert(index >= 0);
			}
			q = natDiv(q, table[index].bbb, &r);
			ssize_t h = slen - table[index].ndigits;
			convertWords(r, &s[h], table[index].ndigits, b, ndigits, bb, table, index);
			slen = h;
		}
	}
	ssize_t i = slen;
	if (b == 10) {
		while (q.len > 0) {
			Word r = 0;
			q = natDivW(q, bb, &r);
			for (ssize_t j = 0; j < ndigits && i > 0; j++) {
				--i;
				Word t = r / 10;
				s[i] = '0' + (r - t * 10);
				r = t;
			}
		}
	}
	else {
		while (q.len > 0) {
			Word r = 0;
			q = natDivW(q, bb, &r);
			for (ssize_t j = 0; j < ndigits && i > 0; j++) {
				--i;
				s[i] = digits[r % b];
				r /= b;
			}
		}
	}
	while (i > 0) {
		--i;
		s[i] = '0';
	}
}

char* natI2a(nat x, bool neg, int base) {
	assert(base >= 2 && base <= MaxBase);
	if (x.len == 0) {
		char* p = malloc(2);
		if (p == NULL) {
			return NULL;
		}
		p[0] = '0';
		p[1] = 0;
		return p;
	}
	ssize_t i = ((double)(natBitLen(x)) / log2((double)base)) + 1;
	if (neg) {
		++i;
	}
	ssize_t slen = i;
	char* s = calloc(1, slen+1);
	if (s == NULL) {
		return NULL;
	}
	Word b = (Word)base;
	if (b == b & -b) {
		ssize_t shift = TrailingZeros(b);
		size_t mask = 1ULL << shift - 1;
		Word w = x.data[0];
		size_t nbits = _W;

		for (ssize_t k = 1; k < x.len; ++k) {
			while (nbits >= shift) {
				--i;
				s[i] = digits[w & mask];
				w >>= shift;
				nbits -= shift;
			}
			if (nbits == 0) {
				w = x.data[k];
				nbits = _W;
			}
			else {
				w |= x.data[k] << nbits;
				--i;
				s[i] = digits[w & mask];

				w = x.data[k] >> (shift - nbits);
				nbits = _W - (shift - nbits);
			}
		}

		while (w != 0) {
			--i;
			s[i] = digits[w & mask];
			w >>= shift;
		}
	}
	else {
		ssize_t ndigits = 0;
		Word bb = maxPow(b, &ndigits);
		int tableNum = 0;
		divisor* table = divisors(x.len, b, ndigits, bb, &tableNum);
		nat q = natCopy(x);
		convertWords(q, s, slen, b, ndigits, bb, table, tableNum);
		i = 0;
		while (s[i] == '0') {
			++i;
		}
	}
	if (neg) {
		--i;
		s[i] = '-';
	}
	memmove(s, &s[i], slen - i);
	s[slen - i] = 0;
	return s;
}