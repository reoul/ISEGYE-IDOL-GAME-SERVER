#include "Client.h"

#include <algorithm>
#include <iterator>
#include "PacketStruct.h"

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
	for (uint8_t i = 0; i < MAX_UN_USING_ITEM; ++i)
	{
		const uint8_t type = mUnUsingItems[i].GetType();
		if (type < LOCK_ITEM)	// 비어있지 않거나 안 잠긴 경우
		{
			items.emplace_back(SlotInfo(i, mUnUsingItems[i]));
		}
	}
	return items;
}

void Client::SwapItem(const uint8_t index1, const uint8_t index2)
{
	Item& item1 = index1 < MAX_USING_ITEM ? mUsingItems[index1] : mUnUsingItems[index1 - MAX_USING_ITEM];
	Item& item2 = index2 < MAX_USING_ITEM ? mUsingItems[index2] : mUnUsingItems[index2 - MAX_USING_ITEM];

	const uint8_t type2 = item2.GetType();
	item2.SetType(item1.GetType());
	item1.SetType(type2);
}

void Client::AddItem(uint8_t type)
{
	for (Item& item : mUnUsingItems)
	{
		if (item.GetType() == EMPTY_ITEM)
		{
			item.SetType(type);
			break;
		}
	}
}

// UsingItems에 아무것도 장착되어 있지 않으면 UnUsingItems에서 가져다 장착한다
void Client::TrySetDefaultUsingItem()
{
	vector<SlotInfo> validUnUsingItems = GetValidUnUsingItems();

	if (!validUnUsingItems.empty() && GetValidUsingItems().empty())
	{
		for (uint8_t slot1 = 0; slot1 < MAX_USING_ITEM; ++slot1)
		{
			const uint8_t slot2 = validUnUsingItems[0].index;
			SwapItem(slot1, slot2);
			cs_sc_changeItemSlotPacket packet(mNetworkID, slot1, slot2);
			mRoomPtr->SendAllClient(&packet);

			validUnUsingItems.erase(validUnUsingItems.begin());
			if (validUnUsingItems.empty())
			{
				break;
			}
		}
	}
}
