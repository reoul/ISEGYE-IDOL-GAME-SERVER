#include "Room.h"

#include <iostream>

#include "Client.h"
#include "ExtensionMethod.h"
#include "Random.h"
#include "PacketStruct.h"

void SendPacket(int userID, void* pPacket);

Room::Room()
	: mSize(0)
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

void Room::SendAllClient(void* pPacket) const
{
	for (auto it = mClients.begin(); it != mClients.end(); ++it)
	{
		SendPacket((*it)->GetNetworkID(), pPacket);
	}
}

void Room::SendAnotherClient(const Client& client, void* pPacket) const
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
		assert(length <= MAX_USING_ITEM);
		
		int sum = 0;
		for (const SlotInfo& slotInfo : items)
		{
			sum += slotInfo.item.GetActivePercent();
		}

		CopySelf(items, BATTLE_ITEM_QUEUE_LOOP_COUNT - 1);
		
		{
			if(items.empty())
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
					// todo : 관련 로직 바꾸기
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

	for (size_t i = 0; i < MAX_ROOM_PLAYER - mSize; ++i)
	{
		itemQueue.emplace_back(-1);
		for (size_t j = 0; j < MAX_USING_ITEM * BATTLE_ITEM_QUEUE_LOOP_COUNT; ++j)
		{
			itemQueue.emplace_back(0);
			itemQueue.emplace_back(0);
		}
	}

	cout << itemQueue.size() << endl;
	assert(itemQueue.size() == BATTLE_ITEM_QUEUE_LENGTH);

	return itemQueue;
}

void Room::TrySendRandomItemQueue()
{
	if(mSize == 0)
	{
		return;
	}

	if(mBattleReadyCount >= mSize)
	{
		cout << "통과 " << mBattleReadyCount << " " << mSize << endl;
		{
			mBattleReadyCount = 0;
			lock_guard<mutex> lg(cLock);
		}
		SendRandomItemQueue();
	}
}

void Room::BattleReady()
{
	lock_guard<mutex> lg(cLock);
	++mBattleReadyCount;
	//cout << "아무거나" << endl;
	cout << mBattleReadyCount << endl;
}

void Room::SendRandomItemQueue() const
{
	const vector<int32_t> itemQueue = GetRandomItemQueue();
	cout << "999999" << endl;
	sc_battleItemQueuePacket packet(itemQueue);
	wcout << L"랜덤 아이템 순서 보냄" << endl;
	SendAllClient(&packet);
}
