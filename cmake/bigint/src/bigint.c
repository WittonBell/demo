#include "priv.h"
#include "bigint.h"
#include "nat.h"

struct BigInt {
    bool neg;
    nat *abs;
};

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
    natFree(t->abs);
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
    z->neg = z->abs->len > 0 && neg;
    return z;
}

char* BigInt2String(BigInt* x) {
    return natI2a(x->abs, x->neg, 10);
}