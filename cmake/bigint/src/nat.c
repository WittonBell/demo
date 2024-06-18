#include "priv.h"
#include "bigint.h"
#include "nat.h"

nat* natNewLen(int64_t len) {
	nat* p = (nat*)calloc(1, sizeof(nat) + len * sizeof(Word));
	if (p == NULL) {
		return NULL;
	}
	p->cap = len;
	p->len = len;
	p->nat = (Word*)&p[1];
	return p;
}

nat* natNew(Word v) {
	const int cap = 8;
	nat* p = natNewLen(cap);
	*p->nat = v;
	p->len = 1;
	return p;
}

void natSet(nat* p, uint64_t index, Word value) {
	assert(index < p->len);
	p->nat[index] = value;
}

nat* natCopy(nat* x) {
	nat* z = natNewLen(x->len);
	memcpy(z->nat, x->nat, x->len);
	z->cap = x->len;
	z->len = x->len;
	return z;
}

void natCopy2(nat* dst, nat* src) {
	assert(dst->cap >= src->len);
	memcpy(dst->nat, src->nat, src->len * sizeof(dst->nat[0]));
}

nat* natPart(nat* p, uint64_t from, uint64_t to) {
	nat* t = (nat*)malloc(sizeof(nat));
	if (t == NULL) {
		return NULL;
	}
	t->cap = to - from;
	t->len = t->cap;
	t->nat = &p->nat[from];
	return t;
}

nat* natNorm(nat* p) {
	int64_t i = p->len;
	while (i > 0 && p->nat[i - 1] == 0) {
		--i;
	}
	p->len = i;
	return p;
}

int natCmp(nat* x, nat* y) {
	int64_t m = x->len;
	int64_t n = y->len;
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
	while (i > 0 && x->nat[i] == y->nat[i]) {
		--i;
	}
	if (x->nat[i] < y->nat[i]) {
		return -1;
	}
	if (x->nat[i] > y->nat[i]) {
		return 1;
	}
	return 0;
}

void natFree(nat* p) {
	free(p);
}
