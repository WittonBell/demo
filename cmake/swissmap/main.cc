#include <assert.h>
#include <string>
#include <windows.h>
#include "swissmap.h"

int main(int argc, char* argv[]) {
	auto t1 = timeGetTime();
	char buf[32];
	CSwissMap<int, std::string> m;
	int num = 100'0000;
	for (int i = 1; i <= num; ++i){
		sprintf(buf, "str%d", i);
		m.insert(i, buf);
	}
	for (int i = 1; i <= num; ++i){
		m.erase(i);
	}
	auto t2 = timeGetTime();
	printf("swissMap %d次，耗时:%lu\n", num, t2 - t1);
	return 0;
}
