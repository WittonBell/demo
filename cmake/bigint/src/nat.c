#include "priv.h"
#include "bigint.h"
#include "nat.h"

nat natNewLen(ssize_t len) {
	nat t;
	t.data = (Word*)calloc(len, sizeof(Word));
	if (t.data == NULL) {
		len = 0;
	}
	t.cap = len;
	t.len = len;
	t.part = false;
	return t;
}

nat natNew(Word v) {
	const size_t cap = 8;
	nat t = natNewLen(cap);
	*t.data = v;
	t.len = 1;
	return t;
}

void natSet(nat t, ssize_t index, Word value) {
	assert(index >= 0 && index < t.len);
	t.data[index] = value;
}

nat natCopy(nat x) {
	nat z = natNewLen(x.len);
	memcpy(z.data, x.data, x.len * sizeof(Word));
	z.cap = x.len;
	z.len = x.len;
	return z;
}

void natCopy2(nat dst, nat src) {
	assert(dst.cap >= src.len);
	memcpy(dst.data, src.data, src.len * sizeof(dst.data[0]));
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
	uint64_t i = m - 1;
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
