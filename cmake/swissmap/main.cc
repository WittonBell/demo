#include <assert.h>
#include <string>
#include <windows.h>
#include "swissmap.h"

template <class L, class R, class... Args>
static auto impl_impl(L&& l) {
	static_assert(!std::is_same<L, std::function<R(Args...)>>::value,
		"Only lambdas are supported, it is unsafe to use std::function or other non-lambda callables");

	static L lambda_s = std::move(l);
	return +[](Args... args) -> R {
		return lambda_s(args...);
	};
}

template <class L>
struct to_f_impl : public to_f_impl<decltype(&L::operator())> {
};

template <class ClassType, class R, class... Args>
struct to_f_impl<R(ClassType::*)(Args...) const> {
	template <class L>
	static auto impl(L&& l) {
		return impl_impl<L, R, Args...>(std::move(l));
	}
};

template <class ClassType, class R, class... Args>
struct to_f_impl<R(ClassType::*)(Args...)> {
	template <class L>
	static auto impl(L&& l) {
		return impl_impl<L, R, Args...>(std::move(l));
	}
};

template <class L>
auto to_f(L&& l) {
	return to_f_impl<L>::impl(std::move(l));
}


int main(int argc, char* argv[]) {
	auto t1 = timeGetTime();
	char buf[32];
	CSwissMap<int, std::string> m;
	int num = 1'0000;
	for (int i = 1; i <= num; ++i) {
		sprintf(buf, "str%d", i);
		m.insert(i, buf);
	}
	m.foreach(to_f([&](const int& k, std::string& v) {
		buf[0] = buf[0];
		t1 = t1;
		printf("%d %s\n", k, v.c_str());
		return false;
		}));
	for (int i = 1; i <= num; ++i) {
		m.erase(i);
	}
	auto t2 = timeGetTime();
	printf("swissMap %d次，耗时:%lu\n", num, t2 - t1);
	return 0;
}
