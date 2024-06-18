#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include "nat.h"

int LeadingZeros(size_t x) {
	return UintSize - len64(x);
}

int LeadingZeros64(uint64_t x) {
	return 64 - len64(x);
}

uint32_t UIntAdd32(uint32_t x, uint32_t y, uint32_t* carray) {
	uint64_t sum64 = (uint64_t)(x)+(uint64_t)(y);
	sum64 += *carray;
	uint32_t sum = (uint32_t)sum64;
	*carray = sum64 >> 32U;
	return sum;
}

uint64_t UIntAdd64(uint64_t x, uint64_t y, uint64_t* carray) {
	uint64_t sum = x + y + *carray;
	*carray = ((x & y) | ((x | y) & (~sum))) >> 63U;
	return sum;
}

uint32_t UIntSub32(uint32_t a, uint32_t b, uint32_t* borrow) {
	uint32_t diff = a - b - *borrow;
	*borrow = ((~a & b) | (~(a ^ b) & diff)) >> 31U;
	return diff;
}

uint64_t UIntSub64(uint64_t x, uint64_t y, uint64_t* borrow) {
	uint64_t diff = x - y - *borrow;
	*borrow = ((~x & y) | (~(x ^ y) & diff)) >> 63U;
	return diff;
}

uint32_t UIntMul32(uint32_t x, uint32_t y, uint32_t* hi) {
	uint64_t tmp = (uint64_t)x * (uint64_t)y;
	*hi = tmp >> 32U;
	return (uint32_t)tmp;
}

uint64_t UIntMul64(uint64_t x, uint64_t y, uint64_t* hi) {
	const uint64_t mask32 = (1ULL << 32U) - 1;
	uint64_t x0 = x & mask32;
	uint64_t x1 = x >> 32U;
	uint64_t y0 = y & mask32;
	uint64_t y1 = y >> 32U;
	uint64_t w0 = x0 * y0;
	uint64_t t = x1 * y0 + (w0 >> 32U);
	uint64_t w1 = t & mask32;
	uint64_t w2 = t >> 32U;
	w1 += x0 * y1;
	*hi = x1 * y1 + w2 + (w1 >> 32U);
	return x * y;
}

uint32_t UIntDiv32(uint64_t x, uint32_t y, uint32_t* rem) {
	uint32_t hi = x >> 32U;
	uint32_t lo = (uint32_t)x;
	assert(y != 0); // divideError
	assert(y <= hi); // "overflowError"
	uint64_t z = (uint64_t)hi << 32U | (uint64_t)lo;
	*rem = z % (uint64_t)y;
	return z / (uint64_t)y;
}

uint64_t UIntDiv64(uint64_t hi, uint64_t lo, uint64_t y, uint64_t* rem) {
	assert(y != 0); // divideError
	assert(y <= hi); // "overflowError"
	if (hi == 0) {
		*rem = lo % y;
		return lo / y;
	}
	uint32_t s = LeadingZeros64(y);
	y <<= s;

	const uint64_t two32 = 1ULL << 32U;
	const uint64_t mask32 = two32 - 1;

	uint64_t yn1 = y >> 32U;
	uint64_t yn0 = y & mask32;
	uint64_t un32 = hi << s | lo >> (64 - s);
	uint64_t un10 = lo << s;
	uint64_t un1 = un10 >> 32U;
	uint64_t un0 = un10 & mask32;
	uint64_t q1 = un32 / yn1;
	uint64_t rhat = un32 - q1 * yn1;

	while (q1 >= two32 || q1 * yn0 > two32 * rhat + un1) {
		q1--;
		rhat += yn1;
		if (rhat >= two32) {
			break;
		}
	}

	uint64_t un21 = un32 * two32 + un1 - q1 * y;
	uint64_t q0 = un21 / yn1;
	rhat = un21 - q0 * yn1;

	while (q0 >= two32 || q0 * yn0 > two32 * rhat + un0) {
		q0--;
		rhat += yn1;
		if (rhat >= two32) {
			break;
		}
	}
	*rem = (un21 * two32 + un0 - q0 * y) >> s;
	return q1 * two32 + q0;
}

uint32_t UIntRem32(uint64_t x, uint32_t y) {
	uint32_t rem = 0;
	UIntDiv32(x, y, &rem);
	return rem;
}

uint64_t UIntRem64(uint64_t hi, uint64_t lo, uint64_t y) {
	uint64_t rem = 0;
	UIntDiv64(hi, lo, y, &rem);
	return rem;
}

const uint8_t ntz8tab[] = "" \
"\x08\x00\x01\x00\x02\x00\x01\x00\x03\x00\x01\x00\x02\x00\x01\x00" \
"\x04\x00\x01\x00\x02\x00\x01\x00\x03\x00\x01\x00\x02\x00\x01\x00" \
"\x05\x00\x01\x00\x02\x00\x01\x00\x03\x00\x01\x00\x02\x00\x01\x00" \
"\x04\x00\x01\x00\x02\x00\x01\x00\x03\x00\x01\x00\x02\x00\x01\x00" \
"\x06\x00\x01\x00\x02\x00\x01\x00\x03\x00\x01\x00\x02\x00\x01\x00" \
"\x04\x00\x01\x00\x02\x00\x01\x00\x03\x00\x01\x00\x02\x00\x01\x00" \
"\x05\x00\x01\x00\x02\x00\x01\x00\x03\x00\x01\x00\x02\x00\x01\x00" \
"\x04\x00\x01\x00\x02\x00\x01\x00\x03\x00\x01\x00\x02\x00\x01\x00" \
"\x07\x00\x01\x00\x02\x00\x01\x00\x03\x00\x01\x00\x02\x00\x01\x00" \
"\x04\x00\x01\x00\x02\x00\x01\x00\x03\x00\x01\x00\x02\x00\x01\x00" \
"\x05\x00\x01\x00\x02\x00\x01\x00\x03\x00\x01\x00\x02\x00\x01\x00" \
"\x04\x00\x01\x00\x02\x00\x01\x00\x03\x00\x01\x00\x02\x00\x01\x00" \
"\x06\x00\x01\x00\x02\x00\x01\x00\x03\x00\x01\x00\x02\x00\x01\x00" \
"\x04\x00\x01\x00\x02\x00\x01\x00\x03\x00\x01\x00\x02\x00\x01\x00" \
"\x05\x00\x01\x00\x02\x00\x01\x00\x03\x00\x01\x00\x02\x00\x01\x00" \
"\x04\x00\x01\x00\x02\x00\x01\x00\x03\x00\x01\x00\x02\x00\x01\x00";

const uint8_t pop8tab[] = "" \
"\x00\x01\x01\x02\x01\x02\x02\x03\x01\x02\x02\x03\x02\x03\x03\x04" \
"\x01\x02\x02\x03\x02\x03\x03\x04\x02\x03\x03\x04\x03\x04\x04\x05" \
"\x01\x02\x02\x03\x02\x03\x03\x04\x02\x03\x03\x04\x03\x04\x04\x05" \
"\x02\x03\x03\x04\x03\x04\x04\x05\x03\x04\x04\x05\x04\x05\x05\x06" \
"\x01\x02\x02\x03\x02\x03\x03\x04\x02\x03\x03\x04\x03\x04\x04\x05" \
"\x02\x03\x03\x04\x03\x04\x04\x05\x03\x04\x04\x05\x04\x05\x05\x06" \
"\x02\x03\x03\x04\x03\x04\x04\x05\x03\x04\x04\x05\x04\x05\x05\x06" \
"\x03\x04\x04\x05\x04\x05\x05\x06\x04\x05\x05\x06\x05\x06\x06\x07" \
"\x01\x02\x02\x03\x02\x03\x03\x04\x02\x03\x03\x04\x03\x04\x04\x05" \
"\x02\x03\x03\x04\x03\x04\x04\x05\x03\x04\x04\x05\x04\x05\x05\x06" \
"\x02\x03\x03\x04\x03\x04\x04\x05\x03\x04\x04\x05\x04\x05\x05\x06" \
"\x03\x04\x04\x05\x04\x05\x05\x06\x04\x05\x05\x06\x05\x06\x06\x07" \
"\x02\x03\x03\x04\x03\x04\x04\x05\x03\x04\x04\x05\x04\x05\x05\x06" \
"\x03\x04\x04\x05\x04\x05\x05\x06\x04\x05\x05\x06\x05\x06\x06\x07" \
"\x03\x04\x04\x05\x04\x05\x05\x06\x04\x05\x05\x06\x05\x06\x06\x07" \
"\x04\x05\x05\x06\x05\x06\x06\x07\x05\x06\x06\x07\x06\x07\x07\x08";

const uint8_t rev8tab[] = "" \
"\x00\x80\x40\xc0\x20\xa0\x60\xe0\x10\x90\x50\xd0\x30\xb0\x70\xf0" \
"\x08\x88\x48\xc8\x28\xa8\x68\xe8\x18\x98\x58\xd8\x38\xb8\x78\xf8" \
"\x04\x84\x44\xc4\x24\xa4\x64\xe4\x14\x94\x54\xd4\x34\xb4\x74\xf4" \
"\x0c\x8c\x4c\xcc\x2c\xac\x6c\xec\x1c\x9c\x5c\xdc\x3c\xbc\x7c\xfc" \
"\x02\x82\x42\xc2\x22\xa2\x62\xe2\x12\x92\x52\xd2\x32\xb2\x72\xf2" \
"\x0a\x8a\x4a\xca\x2a\xaa\x6a\xea\x1a\x9a\x5a\xda\x3a\xba\x7a\xfa" \
"\x06\x86\x46\xc6\x26\xa6\x66\xe6\x16\x96\x56\xd6\x36\xb6\x76\xf6" \
"\x0e\x8e\x4e\xce\x2e\xae\x6e\xee\x1e\x9e\x5e\xde\x3e\xbe\x7e\xfe" \
"\x01\x81\x41\xc1\x21\xa1\x61\xe1\x11\x91\x51\xd1\x31\xb1\x71\xf1" \
"\x09\x89\x49\xc9\x29\xa9\x69\xe9\x19\x99\x59\xd9\x39\xb9\x79\xf9" \
"\x05\x85\x45\xc5\x25\xa5\x65\xe5\x15\x95\x55\xd5\x35\xb5\x75\xf5" \
"\x0d\x8d\x4d\xcd\x2d\xad\x6d\xed\x1d\x9d\x5d\xdd\x3d\xbd\x7d\xfd" \
"\x03\x83\x43\xc3\x23\xa3\x63\xe3\x13\x93\x53\xd3\x33\xb3\x73\xf3" \
"\x0b\x8b\x4b\xcb\x2b\xab\x6b\xeb\x1b\x9b\x5b\xdb\x3b\xbb\x7b\xfb" \
"\x07\x87\x47\xc7\x27\xa7\x67\xe7\x17\x97\x57\xd7\x37\xb7\x77\xf7" \
"\x0f\x8f\x4f\xcf\x2f\xaf\x6f\xef\x1f\x9f\x5f\xdf\x3f\xbf\x7f\xff";

static const uint8_t len8tab[] = "" \
"\x00\x01\x02\x02\x03\x03\x03\x03\x04\x04\x04\x04\x04\x04\x04\x04" \
"\x05\x05\x05\x05\x05\x05\x05\x05\x05\x05\x05\x05\x05\x05\x05\x05" \
"\x06\x06\x06\x06\x06\x06\x06\x06\x06\x06\x06\x06\x06\x06\x06\x06" \
"\x06\x06\x06\x06\x06\x06\x06\x06\x06\x06\x06\x06\x06\x06\x06\x06" \
"\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07" \
"\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07" \
"\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07" \
"\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07\x07" \
"\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08" \
"\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08" \
"\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08" \
"\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08" \
"\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08" \
"\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08" \
"\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08" \
"\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08\x08";

int len64(uint64_t x) {
	int n = 0;
	if (x >= 1ULL << 32U) {
		x >>= 32U;
		n = 32;
	}
	if (x >= 1ULL << 16U) {
		x >>= 16U;
		n += 16;
	}
	if (x >= 1ULL << 8U) {
		x >>= 8U;
		n += 8;
	}
	return n + (int)(len8tab[x]);
}

static const uint64_t deBruijn64 = 0x03f79d71b4ca8b09;
static uint8_t deBruijn64tab[64] = {
	0, 1, 56, 2, 57, 49, 28, 3, 61, 58, 42, 50, 38, 29, 17, 4,
	62, 47, 59, 36, 45, 43, 51, 22, 53, 39, 33, 30, 24, 18, 12, 5,
	63, 55, 48, 27, 60, 41, 37, 16, 46, 35, 44, 21, 52, 32, 23, 11,
	54, 26, 40, 15, 34, 20, 31, 10, 25, 14, 19, 9, 13, 8, 7, 6,
};

int TrailingZeros64(uint64_t x) {
	if (x == 0) {
		return 64;
	}
	return deBruijn64tab[(x & -x) * deBruijn64 >> (64 - 6)];
}