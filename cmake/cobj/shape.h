#pragma once

typedef struct _Shape Shape;
typedef int (*ShapeGetArea)(Shape*);
typedef void (*ShapeDtor)(Shape*);

// ����Shape���麯������ʵ�ֶ�̬
typedef struct {
	ShapeGetArea GetArea;
	ShapeDtor Dtor;
} vtShape;

typedef enum {
	Black,
	Red,
	White,
	Yellow,
}Color;

struct _Shape {
	const vtShape* vtb; // ָ���麯����
	char* name;
	Color color;
};

// Shape �Ĺ��캯��
void shapeCtor(Shape* shape, const char* name);
// Shape ����������
void shapeDtor(Shape* shape);

typedef struct {
	Shape; // �̳�Shape
}Rect;

typedef struct {
	Shape; // �̳�Shape
}Circle;

Rect* newRect(int w, int h);
Circle* newCircle(int r);

