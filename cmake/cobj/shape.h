#pragma once

typedef struct _Shape Shape;
typedef int (*ShapeGetArea)(Shape*);
typedef void (*ShapeDtor)(Shape*);

// 定义Shape的虚函数，以实现多态
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
	const vtShape* vtb; // 指向虚函数表
	char* name;
	Color color;
};

// Shape 的构造函数
void shapeCtor(Shape* shape, const char* name);
// Shape 的析构函数
void shapeDtor(Shape* shape);

typedef struct {
	Shape; // 继承Shape
}Rect;

typedef struct {
	Shape; // 继承Shape
}Circle;

Rect* newRect(int w, int h);
Circle* newCircle(int r);

