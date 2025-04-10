#ifdef _WIN32
#include <windows.h>
#endif
#include <stdio.h>

extern void asmFoo();

int main()
{
#ifdef _WIN32
    printf("CodePage:%d\n", GetConsoleOutputCP());
#endif
    asmFoo();
    return 0;
}
