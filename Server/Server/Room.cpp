#include "Room.h"

#include <iostream>

#include "Client.h"
#include "ExtensionMethod.h"
#include "Random.h"
#include "PacketStruct.h"

void SendPacket(int userID, void* pPacket);

Room::Room()
	: mSize(0)
	, mNumber(0)
	, mIsRun(false)
	, mCapacity(MAX_ROOM_PLAYER)
	, mBattleReadyCount(0)
{
}

void Room::AddClient(Client& client)
{
	if (mSize == mCapacity)
	{
		return;
	}

	++mSize;
	mClients.emplace_back(&client);
}

void Room::RemoveClient(const Client& client)
{
	auto it = mClients.cbegin();
	for (; it != mClients.cend(); ++it)
	{
		if (*it == &client)
		{
			break;
		}
	}

	if (it != mClients.cend())
	{
		mClients.erase(it);
		--mSize;
	}
}

void Room::SendPacketToAllClients(void* pPacket) const
{
	for (auto it = mClients.begin(); it != mClients.end(); ++it)
	{
		SendPacket((*it)->GetNetworkID(), pPacket);
	}
}

void Room::SendPacketToAnotherClients(const Client& client, void* pPacket) const
{
	for (auto it = mClients.begin(); it != mClients.end(); ++it)
	{
		if (&client == *it)
		{
			continue;
		}

		SendPacket((*it)->GetNetworkID(), pPacket);
	}
}

vector<int32_t> Room::GetRandomItemQueue() const
{
	vector<SlotInfo> items;
	items.reserve(MAX_USING_ITEM * BATTLE_ITEM_QUEUE_LOOP_COUNT);

	vector<int32_t> itemQueue;
	itemQueue.reserve(BATTLE_ITEM_QUEUE_LENGTH);

	for (auto it = mClients.begin(); it != mClients.end(); ++it)
	{
		itemQueue.emplace_back((*it)->GetNetworkID());
		items = (*it)->GetValidUsingItems();

		const size_t length = items.size();
		log_assert(length <= MAX_USING_ITEM);

		int sum = 0;
		for (const SlotInfo& slotInfo : items)
		{
			sum += slotInfo.item.GetActivePercent();
		}

		CopySelf(items, BATTLE_ITEM_QUEUE_LOOP_COUNT - 1);

		{
			if (items.empty())
			{
				for (size_t i = 0; i < MAX_USING_ITEM * BATTLE_ITEM_QUEUE_LOOP_COUNT; ++i)
				{
					itemQueue.emplace_back(EMPTY_ITEM);
					itemQueue.emplace_back(ACTIVATE_ITEM);
				}
			}
			else
			{
				int loopSum = sum;
				size_t loopLength = length;

				while (!items.empty())
				{
					Random<int> gen(0, loopSum - 1);
					int rand = gen();

					auto iter = items.begin();
					for (size_t i = 0; i < loopLength; ++i)
					{
						const int itemActivePercent = iter->item.GetActivePercent();
						rand -= itemActivePercent;
						if (rand < 0)
						{
							--loopLength;
							loopSum -= itemActivePercent;
							itemQueue.emplace_back(iter->index);
							itemQueue.emplace_back(ACTIVATE_ITEM);
							items.erase(iter);
							break;
						}
					}

					if (loopLength == 0)
					{
						loopSum = sum;
						loopLength = length;

						const int lockSlotCnt = (*it)->GetLockSlotCount();
						for (size_t i = 0; i < MAX_USING_ITEM - length - lockSlotCnt; ++i)
						{
							itemQueue.emplace_back(EMPTY_ITEM);
							itemQueue.emplace_back(ACTIVATE_ITEM);
						}

						for (size_t i = 0; i < lockSlotCnt; ++i)
						{
							itemQueue.emplace_back(LOCK_ITEM);
							itemQueue.emplace_back(DISABLE_ITEM);
						}
					}
				}
			}
		}

		items.clear();
	}

	// 순서 디버그용
	//for (size_t i = 0; i < MAX_ROOM_PLAYER - mSize; ++i)
	//{
	//	itemQueue.emplace_back(-1);
	//	for (size_t j = 0; j < MAX_USING_ITEM * BATTLE_ITEM_QUEUE_LOOP_COUNT; ++j)
	//	{
	//		itemQueue.emplace_back(0);
	//		itemQueue.emplace_back(0);
	//	}
	//}

	log_assert(itemQueue.size() == BATTLE_ITEM_QUEUE_LENGTH);

	/*for (size_t i = 0; i < MAX_ROOM_PLAYER; ++i)
	{
		size_t index = i * 60 + i;
		for (size_t j = 0; j < BATTLE_ITEM_QUEUE_LOOP_COUNT; ++j)
		{
			size_t index2 = index + j * (MAX_USING_ITEM * 2);
			LogWriteTest("[itemQueue] {0}:{1} == {2}:{3}  {4}:{5}  {6}:{7}  {8}:{9}  {10}:{11}  {12}:{13} ",
				itemQueue[index], j, itemQueue[index2 + 1], itemQueue[index2 + 2], itemQueue[index2 + 3], itemQueue[index2 + 4],
				itemQueue[index2 + 5], itemQueue[index2 + 6], itemQueue[index2 + 7], itemQueue[index2 + 8],
				itemQueue[index2 + 9], itemQueue[index2 + 10], itemQueue[index2 + 11], itemQueue[index2 + 12]);
		}
	}*/

	return itemQueue;
}

void Room::TrySendRandomItemQueue()
{
	if(mSize == 0)
	{
		return;
	}

	if (mBattleReadyCount >= mSize)
	{
		{
			lock_guard<mutex> lg(cLock);
			mBattleReadyCount = 0;
		}
		SendRandomItemQueue();
	}
}

void Room::BattleReady()
{
	lock_guard<mutex> lg(cLock);
	++mBattleReadyCount;
	Log("{0} Room Increment Battle Ready Count (Cur:{1})", mNumber, mBattleReadyCount);
}

void Room::Init()
{
	mClients.clear();
	mSize = 0;
	mIsRun = false;
	mBattleReadyCount = 0;
	Log("{0} Room Initialization", mNumber);
}

void Room::SendRandomItemQueue() const
{
	const vector<int32_t> itemQueue = GetRandomItemQueue();
	sc_battleItemQueuePacket packet(itemQueue);
	SendPacketToAllClients(&packet);
}
