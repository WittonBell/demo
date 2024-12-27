export module libA;

export import :plus;
export import libA.minus;
export import :test;

#if __clang__
module :private;
#endif

void func()
{

}
