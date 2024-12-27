export module libA.minus;

export int minus(int x, int y)
{
	return x - y;
}

#if __clang__
module :private;
#endif

void func11()
{

}
