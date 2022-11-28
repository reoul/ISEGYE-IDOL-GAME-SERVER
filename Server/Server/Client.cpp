#include "Client.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include "PacketStruct.h"
#include "Random.h"
#include "Items.h"

Client::Client()
	: mSocket(INVALID_SOCKET)
	, mNetworkID(0)
	, mRecvOver()
	, mPrevSize(0)
	, mHp(MAX_CHARACTER_MAX_HP)
	, mAvatarMaxHp(MAX_AVATAR_MAX_HP)
	, mAvatarHp(MAX_AVATAR_MAX_HP)
	, mAvatarDefensive(START_AVATAR_DEFENSIVE)
	, mPacketBuf{}
	, mIsAlive(false)
	, mStatus(ESocketStatus::FREE)
	, mCharacterType(ECharacterType::Woowakgood)
	, mRoomPtr(nullptr)
	, mName{}
	, mUsingItems{}
	, mUnUsingItems{}
	, mFirstAttackState(0)
	, mIsChoiceCharacter(false)
{
	static_assert(MAX_USING_ITEM == 6, "MAX_USING_ITEM is not 6");

	// UsingItems 자리에 선공 확률 지정
	mUsingItems[0].SetActivePercent(27);
	mUsingItems[1].SetActivePercent(27);
	mUsingItems[2].SetActivePercent(33);
	mUsingItems[3].SetActivePercent(33);
	mUsingItems[4].SetActivePercent(40);
	mUsingItems[5].SetActivePercent(40);
}

void Client::Init()
{
	mSocket = INVALID_SOCKET;
	mPrevSize = 0;
	memset(mPacketBuf, 0, MAX_PACKET_SIZE);
	mIsAlive = false;
	mCharacterType = ECharacterType::Woowakgood;
	mName[0] = '\0';

	if (mRoomPtr != nullptr)
	{
		mRoomPtr->RemoveClient(*this);
		mRoomPtr = nullptr;
	}

	for (Item& item : mUsingItems)
	{
		item.SetType(EMPTY_ITEM);
	}
	for (Item& item : mUnUsingItems)
	{
		item.SetType(EMPTY_ITEM);
	}

	mStatus = ESocketStatus::FREE;
	mFirstAttackState = 0;
	mIsChoiceCharacter = false;
	mHp = MAX_CHARACTER_MAX_HP;
	mAvatarMaxHp = MAX_AVATAR_MAX_HP;
	mAvatarHp = MAX_AVATAR_MAX_HP;
	mAvatarDefensive = START_AVATAR_DEFENSIVE;
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
		if (mUsingItems[i].GetType() != LOCK_ITEM)	// 안 잠긴 경우
			items.emplace_back(SlotInfo(i, mUsingItems[i]));
	}
	return items;
}

int Client::GetLockSlotCount() const
{
	int count = 0;
	for (const Item& item : mUsingItems)
	{
		if (item.GetType() == LOCK_ITEM)
			++count;
	}

	return count;
}

/**
 * UnUsingInventory에 모든 아이템을 가져온다
 * \return 아이템 vector
 */
vector<Item> Client::GetUnUsingItems() const
{
	vector<Item> items;
	items.reserve(MAX_UN_USING_ITEM);
	copy_n(mUnUsingItems, MAX_UN_USING_ITEM, back_inserter(items));
	return items;
}

/**
 * UnUsingInventory에서 유효한 아이템들을 가져온다
 * \return 아이템 vector
 */
vector<SlotInfo> Client::GetValidUnUsingItems() const
{
	vector<SlotInfo> items;
	for (uint8_t i = 0; i < MAX_UN_USING_ITEM; ++i)
	{
		if (mUnUsingItems[i].GetType() < LOCK_ITEM)	// 비어있지 않거나 안 잠긴 경우
			items.emplace_back(SlotInfo(i + MAX_USING_ITEM, mUnUsingItems[i]));
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

/// <summary> 랜덤한 아이템 타입 가져오기 </summary>
uint8_t Client::GetRandomItemType() const
{
	Random<int> gen(0, _countof(sItems) - 1);
	return static_cast<uint8_t>(gen());
}

/**
 * UsingItems에 아무것도 장착되어 있지 않으면 UnUsingItems에서 유효한 아이템을 가져다 장착한다
 */
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

void Client::SendPacketInAllRoomClients(void* pPacket) const
{
	if (mRoomPtr != nullptr)
	{
		mRoomPtr->SendPacketToAllClients(pPacket);
	}
}

void Client::SendPacketInAnotherRoomClients(void* pPacket) const
{
	if (mRoomPtr != nullptr)
	{
		mRoomPtr->SendPacketToAnotherClients(*this, pPacket);
	}
}

bool Client::IsValidConnect() const
{
	constexpr milliseconds intervalTime(CONNECT_CHECK_INTERVAL * 1000);
	if (mLastConnectCheckPacketTime + intervalTime < system_clock::now())	// 만약 마지막에 보낸 확인 패킷이 3초가 지났다면
	{
		return false;	// 유효하지 않는 연결 상태
	}
	return true;		// 유효한 연결 상태
}
