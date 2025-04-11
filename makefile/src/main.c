#ifdef _WIN32
#include <windows.h>
#endif

extern void asmFoo();

int main()
{
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    asmFoo();
    return 0;
}
