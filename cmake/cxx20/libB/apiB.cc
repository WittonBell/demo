// 在给别人使用模块时，可以只给出相应的模块声明文件（包括引用的头文件）
export module libB;
import stdm;

#define EXPORT export
#include "item.h"

export {

class Bag
{
public:
	Bag(IPlayer* owner);
	void AddItem(Item* p);
};

int max(int x, int y);

}
