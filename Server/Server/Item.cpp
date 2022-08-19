#include "Item.h"

Item::Item()
	: mType(EMPTY_ITEM)
	, mActivePercent(0)
{
}

void Item::SetActivePercent(uint8_t percent)
{
	mActivePercent = percent;
}
