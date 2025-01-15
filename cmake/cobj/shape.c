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
	Rect; // ����̳�ͷ�ļ��й�����Rect����
	// ���涨��˽�б���
	int w, h;
}realRect;

// ����������
static int RectArea(realRect* s) {
	printf("Rect GetArea\n");
	return s->w * s->h;
}

// ���ε���������
static void RectRelease(realRect* s) {
	if (s) {
		printf("Rect Dtor:%s\n", s->name);
		shapeDtor((Shape*)s);
		free(s);
	}
}

// ���ε��麯����
// �麯����ֻ��Ψһ��һ�ݣ��������ܹ����˶���ʵ����
// ÿ��ʵ����ֻ��һ��ָ���麯�����ָ�룬��Լ���ڴ�ռ�
static const vtShape vtRect = {
	.GetArea = (ShapeGetArea)RectArea,
	.Dtor = (ShapeDtor)RectRelease,
};

Rect* newRect(int w, int h) {
	// ��realRect��С�����ڴ�
	realRect* p = calloc(1, sizeof(realRect));
	if (NULL == p)
		return NULL;
	// ���û���Ĺ��캯��
	shapeCtor((Shape*)p, "Rect");
	// �����麯����
	p->vtb = &vtRect;
	p->h = h;
	p->w = w;
	printf("Rect Ctor\n");
	printf("Rect Size:%zd\n", sizeof(realRect));
	return (Rect*)p;
}


typedef struct {
	Circle; // ����̳�ͷ�ļ��й�����Circle����
	// ���涨��˽�б���
	int r;
}realCircle;

// ����Բ�����
static int CircleArea(realCircle* s) {
	printf("Circle GetArea\n");
	return (int)(3.14 * s->r * s->r);
}

// Բ�ε���������
static void CircleRelease(realCircle* s) {
	if (s) {
		printf("Circle Dtor:%s\n", s->name);
		shapeDtor((Shape*)s);
		free(s);
	}
}

// Բ�ε��麯����
// �麯����ֻ��Ψһ��һ�ݣ��������ܹ����˶���ʵ����
// ÿ��ʵ����ֻ��һ��ָ���麯�����ָ�룬��Լ���ڴ�ռ�
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
