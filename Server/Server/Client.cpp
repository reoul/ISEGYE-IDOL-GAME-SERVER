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
	, mCharacterType(ECharacterType::Empty)
	, mRoomPtr(nullptr)
	, mName{}
	, mUsingItems{}
	, mUnUsingItems{}
	, mFirstAttackState(0)
	, mIsChoiceCharacter(false)
	, mNormalItemTicketCount(0)
	, mAdvancedItemTicketCount(0)
	, mTopItemTicketCount(0)
	, mSupremeItemTicketCount(0)
	, mRoomOpenCount(0)
{
	static_assert(MAX_USING_ITEM_COUNT == 6, "MAX_USING_ITEM_COUNT is not 6");

	static_assert(sItemProbability[0][0] + sItemProbability[0][1] + sItemProbability[0][2] +
		sItemProbability[0][3] + sItemProbability[0][4] == 100, "Sum is Not 100");
	static_assert(sItemProbability[1][0] + sItemProbability[1][1] + sItemProbability[1][2] +
		sItemProbability[1][3] + sItemProbability[1][4] == 100, "Sum is Not 100");
	static_assert(sItemProbability[2][0] + sItemProbability[2][1] + sItemProbability[2][2] +
		sItemProbability[2][3] + sItemProbability[2][4] == 100, "Sum is Not 100");
	static_assert(sItemProbability[3][0] + sItemProbability[3][1] + sItemProbability[3][2] +
		sItemProbability[3][3] + sItemProbability[3][4] == 100, "Sum is Not 100");
	static_assert(sItemProbability[4][0] + sItemProbability[4][1] + sItemProbability[4][2] +
		sItemProbability[4][3] + sItemProbability[4][4] == 100, "Sum is Not 100");
	static_assert(sItemProbability[5][0] + sItemProbability[5][1] + sItemProbability[5][2] +
		sItemProbability[5][3] + sItemProbability[5][4] == 100, "Sum is Not 100");

	// UsingItems 자리에 선공 확률 지정
	mUsingItems[0].SetActivePercent(40);
	mUsingItems[1].SetActivePercent(40);
	mUsingItems[2].SetActivePercent(33);
	mUsingItems[3].SetActivePercent(33);
	mUsingItems[4].SetActivePercent(27);
	mUsingItems[5].SetActivePercent(27);

	// 아이템 슬롯 캐싱
	for (int i = 0; i < MAX_USING_ITEM_COUNT; ++i)
	{
		mUsingItems[i].SetSlot(i);
	}

	for (int i = 0; i < MAX_UN_USING_ITEM_COUNT; ++i)
	{
		mUnUsingItems[i].SetSlot(i + 6);
	}
}

void Client::Init()
{
	mSocket = INVALID_SOCKET;
	mPrevSize = 0;
	memset(mPacketBuf, 0, MAX_PACKET_SIZE);
	mIsAlive = false;
	mCharacterType = ECharacterType::Empty;
	mName[0] = '\0';

	if (mRoomPtr != nullptr)
	{
		mRoomPtr->RemoveClient(*this);
		mRoomPtr = nullptr;
	}

	for (Item& item : mUsingItems)
	{
		item.SetEmptyItem();
	}
	for (Item& item : mUnUsingItems)
	{
		item.SetEmptyItem();
	}

	mStatus = ESocketStatus::FREE;
	mFirstAttackState = 0;
	mIsChoiceCharacter = false;
	mHp = MAX_CHARACTER_MAX_HP;
	mAvatarMaxHp = MAX_AVATAR_MAX_HP;
	mAvatarHp = MAX_AVATAR_MAX_HP;
	mAvatarDefensive = START_AVATAR_DEFENSIVE;
	mNormalItemTicketCount = 0;
	mAdvancedItemTicketCount = 0;
	mTopItemTicketCount = 0;
	mSupremeItemTicketCount = 0;
	mRoomOpenCount = 0;
}

vector<Item> Client::GetUsingItems() const
{
	vector<Item> items;
	items.reserve(MAX_USING_ITEM_COUNT);
	copy_n(mUsingItems, MAX_USING_ITEM_COUNT, back_inserter(items));
	return items;
}

vector<SlotInfo> Client::GetValidUsingItems() const
{
	vector<SlotInfo> items;
	for (size_t i = 0; i < MAX_USING_ITEM_COUNT; ++i)
	{
		if (mUsingItems[i].GetType() > EMPTY_ITEM)	// 비어있지 않은 경우
			items.emplace_back(SlotInfo(i, mUsingItems[i]));
	}
	return items;
}

/**
 * UnUsingInventory에 모든 아이템을 가져온다
 * \return 아이템 vector
 */
vector<Item> Client::GetUnUsingItems() const
{
	vector<Item> items;
	items.reserve(MAX_UN_USING_ITEM_COUNT);
	copy_n(mUnUsingItems, MAX_UN_USING_ITEM_COUNT, back_inserter(items));
	return items;
}

/**
 * UnUsingInventory에서 유효한 아이템들을 가져온다
 * \return 아이템 vector
 */
vector<SlotInfo> Client::GetValidUnUsingItems() const
{
	vector<SlotInfo> items;
	for (uint8_t i = 0; i < MAX_UN_USING_ITEM_COUNT; ++i)
	{
		if (mUnUsingItems[i].GetType() > EMPTY_ITEM)	// 비어있지 않은 경우
			items.emplace_back(SlotInfo(i + MAX_USING_ITEM_COUNT, mUnUsingItems[i]));
	}
	return items;
}

void Client::SwapItem(const uint8_t index1, const uint8_t index2)
{
	Item& item1 = index1 < MAX_USING_ITEM_COUNT ? mUsingItems[index1] : mUnUsingItems[index1 - MAX_USING_ITEM_COUNT];
	Item& item2 = index2 < MAX_USING_ITEM_COUNT ? mUsingItems[index2] : mUnUsingItems[index2 - MAX_USING_ITEM_COUNT];

	const uint8_t type2 = item2.GetType();
	const uint8_t upgrade2 = item2.GetUpgrade();
	item2.SetType(item1.GetType());
	item2.SetUpgrade(item1.GetUpgrade());
	item1.SetType(type2);
	item1.SetUpgrade(upgrade2);
}

uint8_t Client::AddItem(uint8_t type)
{
	for (uint8_t i = 0; i < MAX_UN_USING_ITEM_COUNT; ++i)
	{
		if (mUnUsingItems[i].GetType() == EMPTY_ITEM)
		{
			mUnUsingItems[i].SetType(type);
			return i + MAX_USING_ITEM_COUNT;
		}
	}

	for (uint8_t i = 0; i < MAX_USING_ITEM_COUNT; ++i)
	{
		if (mUsingItems[i].GetType() == EMPTY_ITEM)
		{
			mUsingItems[i].SetType(type);
			return i;
		}
	}

	return MAX_UN_USING_ITEM_COUNT + MAX_USING_ITEM_COUNT;
}

/// <summary> 일반 뽑기권에서 랜덤한 아이템 타입 가져오기 </summary>
uint8_t Client::GetRandomItemTypeOfNormalItemTicket() const
{
	if (mNormalItemTicketCount == 0)
	{
		return EMPTY_ITEM;
	}

	const int roomRound = min(mRoomPtr->GetRound(), 5);
	Random<int> gen(0, 99);
	int rand = gen();

	int tier;
	for (tier = 0; tier < MAX_ITEM_UPGRADE; ++tier)
	{
		rand -= sItemProbability[roomRound][tier];

		if (rand < 0)
		{
			break;
		}
	}

	const vector<const ItemBase*>* pItemVec = sTierItems[tier];

	Random<int> itemGen(0, pItemVec->size() - 1);
	return static_cast<uint8_t>(pItemVec->operator[](itemGen())->CODE);
}

/// <summary> 고급 뽑기권(2~3티어)에서 랜덤한 아이템 타입 가져오기 </summary>
uint8_t Client::GetRandomItemTypeOfAdvancedItemTicket() const
{
	if (mAdvancedItemTicketCount == 0)
	{
		return EMPTY_ITEM;
	}

	Random<int> tierGen(0, 1);
	const vector<const ItemBase*>& itemList = tierGen() == 0 ? _sTwoTierItems : _sThreeTierItems;

	Random<int> itemGen(0, itemList.size() - 1);
	return static_cast<uint8_t>(itemList[itemGen()]->CODE);
}

uint8_t Client::GetRandomItemTypeOfTopItemTicket() const
{
	if (mTopItemTicketCount == 0)
	{
		return EMPTY_ITEM;
	}

	Random<int> tierGen(0, 1);
	const vector<const ItemBase*>& itemList = tierGen() == 0 ? _sThreeTierItems : _sFourTierItems;

	Random<int> itemGen(0, itemList.size() - 1);
	return static_cast<uint8_t>(itemList[itemGen()]->CODE);
}

uint8_t Client::GetRandomItemTypeOfSupremeItemTicket() const
{
	if (mSupremeItemTicketCount == 0)
	{
		return EMPTY_ITEM;
	}

	Random<int> itemGen(0, _sFiveTierItems.size() - 1);
	return static_cast<uint8_t>(_sFiveTierItems[itemGen()]->CODE);
}

/**
 * UsingItems에 아무것도 장착되어 있지 않으면 UnUsingItems에서 유효한 아이템을 가져다 장착한다
 */
bool Client::TrySetDefaultUsingItem()
{
	bool isSetDefaultUsingItem = false;

	vector<SlotInfo> validUnUsingItems = GetValidUnUsingItems();
	vector<uint8_t> usingItemTypes;
	usingItemTypes.reserve(MAX_USING_ITEM_COUNT);
	if (!validUnUsingItems.empty() && GetValidUsingItems().empty())
	{
		uint8_t slot1 = 0;
		for (auto it = validUnUsingItems.begin(); it != validUnUsingItems.end(); ++it)
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
				isSetDefaultUsingItem = true;
				const uint8_t slot2 = it->index;
				usingItemTypes.push_back(it->item.GetType());
				SwapItem(slot1++, slot2);
				if (usingItemTypes.size() == MAX_USING_ITEM_COUNT)
				{
					break;
				}
			}
		}
	}

	Logger::LogWrite("SetDefaultUsingItem", "{0}번 클라이언트 기본템 장착 결과", mNetworkID);
	Logger::LogWrite("SetDefaultUsingItem", "  UsingItems : {0} {1} {2} {3} {4} {5}",
		mUsingItems[0].GetType(), mUsingItems[1].GetType(), mUsingItems[2].GetType(), mUsingItems[3].GetType(), mUsingItems[4].GetType(), mUsingItems[5].GetType());
	Logger::LogWrite("SetDefaultUsingItem", "UnUsingItems : {0} {1} {2} {3} {4} {5} {6} {7} {8} {9}",
		mUnUsingItems[0].GetType(), mUnUsingItems[1].GetType(), mUnUsingItems[2].GetType(), mUnUsingItems[3].GetType(), mUnUsingItems[4].GetType(),
		mUnUsingItems[5].GetType(), mUnUsingItems[6].GetType(), mUnUsingItems[7].GetType(), mUnUsingItems[8].GetType(), mUnUsingItems[9].GetType());

	return isSetDefaultUsingItem;
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

void Client::SetItem(uint8_t index, uint8_t type)
{
	if (index > MAX_USING_ITEM_COUNT + MAX_UN_USING_ITEM_COUNT)
	{
		return;
	}

	Item& item = index < MAX_USING_ITEM_COUNT ? mUsingItems[index] : mUnUsingItems[index - MAX_USING_ITEM_COUNT];
	item.SetType(type);
}

uint8_t Client::FindEmptyItemSlotIndex() const
{
	for (uint8_t i = 0; i < MAX_UN_USING_ITEM_COUNT; ++i)
	{
		if (mUnUsingItems[i].GetType() == EMPTY_ITEM)
		{
			return i + MAX_USING_ITEM_COUNT;
		}
	}

	for (uint8_t i = 0; i < MAX_USING_ITEM_COUNT; ++i)
	{
		if (mUsingItems[i].GetType() == EMPTY_ITEM)
		{
			return i;
		}
	}
	return MAX_UN_USING_ITEM_COUNT + MAX_USING_ITEM_COUNT;
}

uint8_t Client::GetRandomItemTypeByCombination(EItemTierType topTier)
{
	const int top = static_cast<int>(topTier);
	const int maxTier = min(top + 1, 4);

	Random<int> gen(top, maxTier);

	const vector<const ItemBase*>* pItemVec = sTierItems[gen()];

	Random<int> itemGen(0, pItemVec->size() - 1);
	return static_cast<uint8_t>(pItemVec->operator[](itemGen())->CODE);
}

vector<SlotInfo> Client::GetItemActiveQueue() const
{
	vector<SlotInfo> activeQueue;
	bool isIncludeList[MAX_USING_ITEM_COUNT]{};	// 발동 순서에 포함 시켰는지 여부

	for (int i = 0; i < MAX_USING_ITEM_COUNT; ++i)
	{
		int sum = 0;
		for (int j = 0; j < MAX_USING_ITEM_COUNT; ++j)
		{
			if (isIncludeList[j] == false)
			{
				sum += mUsingItems[j].GetActivePercent();
			}
		}

		Random<int> gen(0, sum - 1);
		int rand = gen();

		for (int j = 0; j < MAX_USING_ITEM_COUNT; ++j)
		{
			if (isIncludeList[j] == false)
			{
				rand -= mUsingItems[j].GetActivePercent();
				if (rand < 0)
				{
					activeQueue.emplace_back(j, mUsingItems[j]);
					isIncludeList[j] = true;
					break;
				}
			}
		}
	}

	log_assert(activeQueue.size() == MAX_USING_ITEM_COUNT);

	return activeQueue;
}
