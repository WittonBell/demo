#ifndef _BIG_INT_H_
#define _BIG_INT_H_

#include <stdint.h>

// 带进位32位加法，返回值=x+y+carry, 进位保存在carray
uint32_t UIntAdd32(uint32_t x, uint32_t y, uint32_t* carray);
// 带进位64位加法，返回值=x+y+carry, 进位保存在carray
uint64_t UIntAdd64(uint64_t x, uint64_t y, uint64_t* carray);

// 带借位32位加法，返回值=x-y-carry, 借位保存在borrow
uint32_t UIntSub32(uint32_t x, uint32_t y, uint32_t* borrow);
// 带借位64位加法，返回值=x-y-carry, 借位保存在borrow
uint64_t UIntSub64(uint64_t x, uint64_t y, uint64_t* borrow);

// 32位乘法，x * y, 返回低32位，高32位在hi中
uint32_t UIntMul32(uint32_t x, uint32_t y, uint32_t* hi);
// 64位乘法，x * y, 返回低64位，高64位在hi中
uint64_t UIntMul64(uint64_t x, uint64_t y, uint64_t* hi);

// 32位除法，返回值为商 = x / y, rem = x % y
uint32_t UIntDiv32(uint64_t x, uint32_t y, uint32_t* rem);
// 64位除法，返回值为商 = (hi, lo) / y, rem = (hi, lo) % y
uint64_t UIntDiv64(uint64_t hi, uint64_t lo, uint64_t y, uint64_t* rem);

// 32位模运算，返回值为 x % y
uint32_t UIntRem32(uint64_t x, uint32_t y);
// 64位模运算，返回值为 (hi, lo) % y
uint64_t UIntRem64(uint64_t hi, uint64_t lo, uint64_t y);

typedef struct BigInt BigInt;
BigInt* BigIntNewI(int64_t v);
BigInt* BigIntNewU(uint64_t v);
void    BigIntFree(BigInt* t);

BigInt* BigIntAdd(BigInt* a, BigInt* b);
BigInt* BigIntSub(BigInt* a, BigInt* b);
BigInt* BigIntMul(BigInt* a, BigInt* b);
BigInt* BigIntDiv(BigInt* a, BigInt* b);

char* BigInt2String(BigInt* x);

#endif
