#include "Client.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include "PacketStruct.h"

Client::Client()
	: mSocket(INVALID_SOCKET)
	, mNetworkID(0)
	, mRecvOver()
	, mPrevSize(0)
	, mPacketBuf{}
	, mIsAlive(false)
	, mStatus(ESocketStatus::FREE)
	, mCharacterType(ECharacterType::Woowakgood)
	, mRoomPtr(nullptr)
	, mName{}
	, mUsingItems{}
	, mUnUsingItems{}
	, mFirstAttackState(0)
{
	static_assert(MAX_USING_ITEM == 6, "MAX_USING_ITEM is not 6");

	// UsingItems 자리에 선공 확률 지정
	mUsingItems[0].SetActivePercent(30);
	mUsingItems[1].SetActivePercent(30);
	mUsingItems[2].SetActivePercent(20);
	mUsingItems[3].SetActivePercent(20);
	mUsingItems[4].SetActivePercent(10);
	mUsingItems[5].SetActivePercent(10);
}

void Client::Init()
{
	mSocket = INVALID_SOCKET;
	mPrevSize = 0;
	memset(mPacketBuf, 0, MAX_PACKET_SIZE);
	mIsAlive = false;
	mCharacterType = ECharacterType::Woowakgood;
	if (mRoomPtr != nullptr)
	{
		mRoomPtr->RemoveClient(*this);
		mRoomPtr = nullptr;
	}
	mName[0] = '\0';
	for (Item& item : mUsingItems)
	{
		item.SetType(EMPTY_ITEM);
	}
	for (Item& item : mUnUsingItems)
	{
		item.SetType(EMPTY_ITEM);
	}
	mStatus = ESocketStatus::FREE;
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
			items.emplace_back(SlotInfo(i + MAX_USING_ITEM, mUnUsingItems[i]));
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
	vector<uint8_t> usingItemTypes;
	usingItemTypes.reserve(MAX_USING_ITEM);
	if (!validUnUsingItems.empty() && GetValidUsingItems().empty())
	{
		uint8_t slot1 = 0;
		for (auto it = validUnUsingItems.begin(); it != validUnUsingItems.end();)
		{
			// 중복 검사
			bool isUsing = false;
			for (auto usingItemVectorIt = usingItemTypes.begin(); usingItemVectorIt != usingItemTypes.end(); ++usingItemVectorIt)
			{
				if (*usingItemVectorIt == it->item.GetType())
				{
					isUsing = true;
					break;
				}
			}

			// 중복 없을 시 장착
			if (!isUsing)
			{
				const uint8_t slot2 = it->index;
				SwapItem(slot1++, slot2);
				it = validUnUsingItems.erase(it);
				continue;
			}
			++it;
		}
	}
}

void Client::AddDefaultItem()
{
	AddItem(1);
	AddItem(6);
}

void Client::SendPacketInAllRoomClients(void* pPacket) const
{
	mRoomPtr->SendPacketToAllClients(pPacket);
}

void Client::SendPacketInAnotherRoomClients(void* pPacket) const
{
	mRoomPtr->SendPacketToAnotherClients(*this, pPacket);
}