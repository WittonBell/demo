#include <iostream>
using namespace std;

typedef enum {
	Black,
	Red,
	White,
	Yellow,
}Color;

class CShape {
private:
	const char* m_name;
	Color color;

public:
	CShape(const char* name) {
		m_name = name;
		color = Black;
		cout << "CShape Ctor:" << name << endl;
	}
	virtual int GetArea() = 0;
	virtual ~CShape() {
		cout << "CShape Dtor" << endl;
	}
};

class CRect : public CShape {
private:
	int _w, _h;
public:
	CRect(int w, int h) : CShape("Rect"), _w(w), _h(h) {
		cout << "CRect Ctor" << endl;
	}

	virtual ~CRect() {
		cout << "CRect Dtor" << endl;
	}

	virtual int GetArea() {
		cout << "CRect GetArea" << endl;
		return _w * _h;
	}
};

class CCircle : public CShape {
private:
	int _r;
public:
	CCircle(int r): CShape("Circle"), _r(r) {
		cout << "CCircle Ctor" << endl;
	}

	virtual ~CCircle() {
		cout << "CCircle Dtor" << endl;
	}

	virtual int GetArea() {
		cout << "CCircle GetArea" << endl;
		return 3.14 * _r * _r;
	}
};

extern "C" void testCC() {
	cout << "CRect Size:" << sizeof(CRect) << endl;
	cout << "CCircle Size:" << sizeof(CCircle) << endl;
	CRect* r = new CRect(10, 20);
	int a1 = r->GetArea();
	CCircle* c = new CCircle(10);
	int a2 = c->GetArea();
	delete r;
	delete c;
}