#include "Client.h"

#include <algorithm>
#include <iterator>

Client::Client()
	: mSocket(NULL)
	, mNetworkID(0)
	, mRecvOver()
	, mPrevSize(0)
	, mPacketBuf{}
	, mIsAlive(false)
	, mStatus(ST_FREE)
	, mCharacterType(CharacterType::Woowakgood)
	, mRoomPtr(nullptr)
	, mUsingItems{}
	, mUnUsingItems{}
{
	mName[0] = '\0';
}

vector<Item> Client::GetUsingItems() const
{
	vector<Item> items;
	items.reserve(MAX_USING_ITEM);
	copy_n(mUsingItems, MAX_USING_ITEM, back_inserter(items));
	return items;
}

vector<SlotInfo> Client::GetValidUsingItems() const
{
	vector<SlotInfo> items;
	for (size_t i = 0; i < MAX_USING_ITEM; ++i)
	{
		uint8_t type = mUsingItems[i].GetType();
		if (type < LOCK_ITEM)	// 비어있지 않거나 안 잠긴 경우
		{
			items.emplace_back(SlotInfo(i, mUsingItems[i]));
		}
	}
	return items;
}

int Client::GetLockSlotCount() const
{
	int count = 0;
	for (const Item& item : mUsingItems)
	{
		if (item.GetType() == LOCK_ITEM)
		{
			++count;
		}
	}

	return count;
}

vector<Item> Client::GetUnUsingItems() const
{
	vector<Item> items;
	items.reserve(MAX_UN_USING_ITEM);
	copy_n(mUnUsingItems, MAX_UN_USING_ITEM, back_inserter(items));
	return items;
}

vector<SlotInfo> Client::GetValidUnUsingItems() const
{
	vector<SlotInfo> items;
	for (size_t i = 0; i < MAX_UN_USING_ITEM; ++i)
	{
		uint8_t type = mUnUsingItems[i].GetType();
		if (type < LOCK_ITEM)	// 비어있지 않거나 안 잠긴 경우
		{
			items.emplace_back(SlotInfo(i, mUnUsingItems[i]));
		}
	}
	return items;
}
