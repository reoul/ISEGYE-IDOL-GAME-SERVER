#include "Item.h"

Item::Item()
	: mType(EMPTY_ITEM)
	, mActivePercent(0)
{
}

void Item::SetType(uint8_t type)
{
	mType = type;
}

void Item::SetActivePercent(uint8_t percent)
{
	mActivePercent = percent;
}
