module;
#include <iostream>
export module libA:test;
import stdm;
import libB;

export class Test
{
public:
	Test()
	{
		printf("Test()\n");
	}
	void foo()
	{
		printf("测试foo\n");
	}
};


export template<typename T>
class T1
{
public:
	T1()
	{
		printf("T1()\n");
	}

	T m_a;
};


export class Player : public IPlayer
{
public:
	Player()
	{
		bag = new Bag(this);
	}

	void SendMsg()
	{

	}
private:
	Bag* bag;
};
