#ifndef _BIG_INT_H_
#define _BIG_INT_H_

#include <stdint.h>

// 带进位32位加法，返回值=x+y+carry, 进位保存在carray
uint32_t U32Add(uint32_t x, uint32_t y, uint32_t* carray);
// 带进位64位加法，返回值=x+y+carry, 进位保存在carray
uint64_t U64Add(uint64_t x, uint64_t y, uint64_t* carray);

size_t UIntAdd(size_t x, size_t y, size_t* carry);

// 带借位32位加法，返回值=x-y-carry, 借位保存在borrow
uint32_t U32Sub(uint32_t x, uint32_t y, uint32_t* borrow);
// 带借位64位加法，返回值=x-y-carry, 借位保存在borrow
uint64_t U64Sub(uint64_t x, uint64_t y, uint64_t* borrow);

size_t UIntSub(size_t x, size_t y, size_t* borrow);

// 32位乘法，x * y, 返回低32位，高32位在hi中
uint32_t U32Mul(uint32_t x, uint32_t y, uint32_t* hi);
// 64位乘法，x * y, 返回低64位，高64位在hi中
uint64_t U64Mul(uint64_t x, uint64_t y, uint64_t* hi);
size_t UIntMul(size_t x, size_t y, size_t* hi);

// 32位除法，返回值为商 = (hi, lo) / y, rem = (hi, lo) % y
uint32_t U32Div(uint32_t hi, uint32_t lo, uint32_t y, uint32_t* rem);
// 64位除法，返回值为商 = (hi, lo) / y, rem = (hi, lo) % y
uint64_t U64Div(uint64_t hi, uint64_t lo, uint64_t y, uint64_t* rem);
size_t UIntDiv(size_t hi, size_t lo, size_t y, size_t* rem);

// 32位模运算，返回值为 (hi, lo) % y
uint32_t U32Rem(uint32_t hi, uint32_t lo, uint32_t y);
// 64位模运算，返回值为 (hi, lo) % y
uint64_t U64Rem(uint64_t hi, uint64_t lo, uint64_t y);
size_t UIntRem(size_t hi, size_t lo, size_t y);

typedef struct BigInt BigInt;
BigInt* BigIntNewI(int64_t v);
BigInt* BigIntNewU(uint64_t v);
void    BigIntFree(BigInt* t);

BigInt* BigIntAdd(BigInt* x, BigInt* y);
BigInt* BigIntSub(BigInt* x, BigInt* y);
BigInt* BigIntMul(BigInt* x, BigInt* y);
BigInt* BigIntDiv(BigInt* x, BigInt* y);

char* BigInt2String(BigInt* x);

#endif
