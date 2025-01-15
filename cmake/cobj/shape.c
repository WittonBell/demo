#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "shape.h"

void shapeCtor(Shape* p, const char* name) {
	printf("shapeCtor:%s\n", name);
	p->name = strdup(name);
}

void shapeDtor(Shape* p) {
	printf("shapeDtor:%s\n", p->name);
	free(p->name);
}


typedef struct {
	Rect; // 这里继承头文件中公开的Rect定义
	// 下面定义私有变量
	int w, h;
}realRect;

// 计算矩形面积
static int RectArea(realRect* s) {
	printf("Rect GetArea\n");
	return s->w * s->h;
}

// 矩形的析构函数
static void RectRelease(realRect* s) {
	if (s) {
		printf("Rect Dtor:%s\n", s->name);
		shapeDtor((Shape*)s);
		free(s);
	}
}

// 矩形的虚函数表
// 虚函数表只有唯一的一份，这样不管构建了多少实例，
// 每个实例都只有一个指向虚函数表的指针，节约了内存空间
static const vtShape vtRect = {
	.GetArea = (ShapeGetArea)RectArea,
	.Dtor = (ShapeDtor)RectRelease,
};

Rect* newRect(int w, int h) {
	// 以realRect大小分配内存
	realRect* p = calloc(1, sizeof(realRect));
	if (NULL == p)
		return NULL;
	// 调用基类的构造函数
	shapeCtor((Shape*)p, "Rect");
	// 设置虚函数表
	p->vtb = &vtRect;
	p->h = h;
	p->w = w;
	printf("Rect Ctor\n");
	printf("Rect Size:%zd\n", sizeof(realRect));
	return (Rect*)p;
}


typedef struct {
	Circle; // 这里继承头文件中公开的Circle定义
	// 下面定义私有变量
	int r;
}realCircle;

// 计算圆形面积
static int CircleArea(realCircle* s) {
	printf("Circle GetArea\n");
	return (int)(3.14 * s->r * s->r);
}

// 圆形的析构函数
static void CircleRelease(realCircle* s) {
	if (s) {
		printf("Circle Dtor:%s\n", s->name);
		shapeDtor((Shape*)s);
		free(s);
	}
}

// 圆形的虚函数表
// 虚函数表只有唯一的一份，这样不管构建了多少实例，
// 每个实例都只有一个指向虚函数表的指针，节约了内存空间
static const vtShape vtCircle = {
	.GetArea = (ShapeGetArea)CircleArea,
	.Dtor = (ShapeDtor)CircleRelease,
};

Circle* newCircle(int r) {
	realCircle* p = calloc(1, sizeof(realCircle));
	if (NULL == p)
		return NULL;

	shapeCtor((Shape*)p, "Circle");
	p->vtb = &vtCircle;
	p->r = r;
	printf("Circle Ctor\n");
	printf("Circle Size:%zd\n", sizeof(realCircle));
	return (Circle*)p;
}
