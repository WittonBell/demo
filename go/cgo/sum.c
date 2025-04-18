#include "cgo.h"
#include <errno.h>

int32_t sum_positive(int32_t a, int32_t b) {
	if (a <= 0 || b <= 0) {
		// 可以在C语言中直接设置错误码，Go中获取错误码
		errno = EINVAL;
		return 0;
	}
	return a + b;
}
