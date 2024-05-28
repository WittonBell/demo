#include <iostream>
#include <format>
#include <string>
#include <set>
import libA;
import libB;
import stdm;

#ifdef _MSC_VER
#define __FUNC__ __FUNCSIG__
#else
#define __FUNC__ __PRETTY_FUNCTION__
#endif

template<typename T>
concept is_array = std::is_array<T>();

template<typename T>
bool isArray() {
	return std::is_array_v<T>;
}

template<typename T, T v>
const char* fun_sig()
{
	return __FUNC__;
}

enum EA
{
	EAA = 1,
};

int main(int argc, char* argv[])
{
	int ver = __clang_major__;
	std::string str = "test";
	std::set<std::string> set;
	set.insert(str);
	setConsoleOutputCP(65001);
	std::cout << fun_sig<EA,EAA>() << std::endl;
	std::array ar = {3,2,1};
	auto v = isArray<decltype(ar.__elems_)>();
	auto s = std::format("Test{2},{0},{1}", 1, 2, "abc");
	std::cout << "sum is:" << plus(10,20) << std::endl;
	minus(10, 5);
	Test a;
	a.foo();
	T1<int> b;
	b.m_a = 1;
	Player p;

	return 0;
}
