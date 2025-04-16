#ifdef _WIN32
#include <Windows.h>
#endif
#include "shape.h"
#include <stdio.h>

void testCC();
void asmFoo();

int main(){
#ifdef _WIN32
	SetConsoleOutputCP(65001);
#endif
	printf("测试\n");
	testCC();
	asmFoo();
	printf("\n\n");
	Rect* r = newRect(10, 20);
	Circle* c = newCircle(10);
	r->color = Red;
	c->color = Yellow;
	const vtShape* rt = r->vtb;
	const vtShape* ct = c->vtb;
	int a1 = rt->GetArea((Shape*)r);
	int a2 = ct->GetArea((Shape*)c);
	rt->Dtor((Shape*)r);
	ct->Dtor((Shape*)c);
	return 0;
}
