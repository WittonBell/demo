#include "priv.h"
#include "bigint.h"
#include "nat.h"

struct BigInt {
	bool neg;
	nat abs;
};

static Word word1 = 1;
static BigInt intOne = { false, { &word1, 1, 1, true } };

BigInt* BigIntNewI(int64_t v) {
	BigInt* p = (BigInt*)malloc(sizeof(BigInt));
	if (p == NULL) {
		return NULL;
	}
	uint64_t x = (uint64_t)v;
	if (v < 0) {
		x = -x;
		p->neg = true;
	} else {
		p->neg = false;
	}
	p->abs = natNew(x);
	return p;
}

BigInt* BigIntNewU(uint64_t v) {
	BigInt* p = (BigInt*)malloc(sizeof(BigInt));
	if (p == NULL) {
		return NULL;
	}
	p->abs = natNew(v);
	p->neg = false;
	return p;
}

void BigIntFree(BigInt* t) {
	natFree(&t->abs);
	free(t);
}

BigInt* BigIntAdd(BigInt* x, BigInt *y) {
	bool neg = x->neg;
	BigInt* z = (BigInt*)malloc(sizeof(BigInt));
	if (z == NULL) {
		return NULL;
	}
	if (x->neg == y->neg) {
		z->abs = natAdd(x->abs, y->abs);
	} else {
		if (natCmp(x->abs, y->abs) >= 0) {
			z->abs = natSub(x->abs, y->abs);
		} else {
			neg = !neg;
			z->abs = natSub(y->abs, x->abs);
		}
	}
	z->neg = z->abs.len > 0 && neg;
	return z;
}

BigInt* BigIntSub(BigInt* x, BigInt* y) {
	BigInt* z = (BigInt*)malloc(sizeof(BigInt));
	if (z == NULL) {
		return NULL;
	}
	bool neg = x->neg;
	if (x->neg != y->neg) {
		z->abs = natAdd(x->abs, y->abs);
	}
	else {
		if (natCmp(x->abs, y->abs) >= 0) {
			z->abs = natSub(x->abs, y->abs);
		}
		else {
			neg = !neg;
			z->abs = natSub(y->abs, x->abs);
		}
	}
	z->neg = z->abs.len > 0 && neg;
	return z;
}

BigInt* BigIntMul(BigInt* x, BigInt* y) {
	BigInt* z = (BigInt*)malloc(sizeof(BigInt));
	if (z == NULL) {
		return NULL;
	}
	if (x == y) {
		//z->abs = natSqr(x->abs);
		//z->neg = false;
		//return z;
	}
	z->abs = natMul(x->abs, y->abs);
	z->neg = z->abs.len > 0 && x->neg != y->neg;
	return z;
}

BigInt* QuoRem(BigInt* x, BigInt* y, BigInt** r) {
	BigInt* z = (BigInt*)malloc(sizeof(BigInt));
	if (z == NULL) {
		return NULL;
	}
	nat rr;
	z->abs = natDiv(x->abs, y->abs, &rr);
	z->neg = z->abs.len > 0 && x->neg != y->neg;
	rr = natNorm(rr);
	(*r)->abs = rr;
	(*r)->neg = rr.len > 0 && x->neg;
	return z;
}

BigInt* BigIntDiv(BigInt* x, BigInt* y) {
	BigInt* r = (BigInt*)malloc(sizeof(BigInt));
	if (r == NULL) {
		return NULL;
	}
	bool yneg = y->neg;
	BigInt* z = QuoRem(x, y, &r);
	BigIntFree(r);
	if (z == NULL) {
		return NULL;
	}
	if (r->neg) {
		if (yneg) {
			z = BigIntAdd(z, &intOne);
		}
		else {
			z = BigIntSub(z, &intOne);
		}
	}
	return z;
}

char* BigInt2String(BigInt* x) {
	return natI2a(x->abs, x->neg, 10);
}

char* BigInt2HexStr(BigInt* x) {
	return natI2a(x->abs, x->neg, 16);
}

char* BigInt2Text(BigInt* x, int base) {
	return natI2a(x->abs, x->neg, base);
}