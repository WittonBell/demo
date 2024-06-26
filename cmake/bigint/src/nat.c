#include "priv.h"
#include "bigint.h"
#include "nat.h"

nat natNewLen(ssize_t len) {
	nat t;
	if (len > 0) {
		t.data = (Word*)calloc(len, sizeof(Word));
		if (t.data == NULL) {
			len = 0;
		}
	}
	t.cap = len;
	t.len = len;
	t.part = false;
	return t;
}

nat natNew(Word v) {
	const ssize_t cap = 8;
	nat t = natNewLen(cap);
	*t.data = v;
	t.len = 1;
	return t;
}

nat natMake(nat z, ssize_t n) {
	if (n < z.cap) {
		return natPart(z, 0, n);
	}
	if (n == 1) {
		return natNewLen(1);
	}
	const ssize_t e = 4;
	nat t = natNewLen(n+e);
	t.len = n;
	return t;
}

nat natSetWord(nat z, Word x) {
	if (x == 0) {
		return natPart(z, 0, 0);
	}
	z = natMake(z, 1);
	z.data[0] = x;
	return z;
}

nat natCopy(nat x) {
	nat z = natNewLen(x.len);
	memcpy(z.data, x.data, x.len * sizeof(Word));
	z.cap = x.len;
	z.len = x.len;
	return z;
}

nat natPart(nat p, ssize_t from, ssize_t to) {
	assert(to >= from);
	assert(from >= 0);
	nat t;
	t.cap = to - from;
	t.len = t.cap;
	t.data = &p.data[from];
	t.part = true;
	return t;
}

nat natNorm(nat p) {
	ssize_t i = p.len;
	while (i > 0 && p.data[i - 1] == 0) {
		--i;
	}
	p.len = i;
	return p;
}

int natCmp(nat x, nat y) {
	ssize_t m = x.len;
	ssize_t n = y.len;
	if (m != n || m == 0) {
		if (m < n) {
			return -1;
		}
		if (m > n) {
			return 1;
		}
		return 0;
	}
	ssize_t i = m - 1;
	while (i > 0 && x.data[i] == y.data[i]) {
		--i;
	}
	if (x.data[i] < y.data[i]) {
		return -1;
	}
	if (x.data[i] > y.data[i]) {
		return 1;
	}
	return 0;
}

void natClear(nat x) {
	for (ssize_t i = 0; i < x.len; ++i) {
		x.data[i] = 0;
	}
}

void natFree(nat *p) {
	if (p->part) {
		return;
	}
	free(p->data);
	p->data = NULL;
	p->cap = 0;
	p->len = 0;
	p->part = false;
}
