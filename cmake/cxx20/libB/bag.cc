export module libC;
import stdm;

export class Bag
{
public:
	Bag(IPlayer* owner)
	{
		owner->SendMsg();
	}

	void AddItem()
	{

	}
};
