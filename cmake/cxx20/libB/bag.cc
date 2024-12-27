module libB;
import stdm;

Bag::Bag(IPlayer* owner)
{
	owner->SendMsg();
}

void Bag::AddItem(Item* p)
{
	p->GetNum();
}

Item::Item()
{
}

int Item::GetNum()
{
	return 0;
}

int max(int x, int y)
{
	return x > y ? x : y;
}
