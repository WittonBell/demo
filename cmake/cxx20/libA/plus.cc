module;
#include <stdio.h>
export module libA:plus;

export int plus(int x, int y)
{
	printf("%d + %d = %d\n", x, y, x + y);
	//std::cout << x << "+" << y << std::endl;
	return x + y;
}

export void foo()
{
	//std::string s;
}
