#include "Room.h"

#include <iostream>
#include <reoul/functional.h>
#include <reoul/logger.h>
#include "Client.h"
#include "Random.h"
#include "PacketStruct.h"
#include "GlobalVariable.h"

void SendPacket(int userID, void* pPacket);

Room::Room()
	: mSize(0)
	, mNumber(0)
	, mIsRun(false)
	, mCapacity(MAX_ROOM_PLAYER)
{
}

void Room::AddClients(vector<Client*>& clients)
{
	{
		lock_guard<mutex> lg(cLock);
		if (mSize == mCapacity)
		{
			return;
		}

		for (Client* client : clients)
		{
			++mSize;
			mClients.emplace_back(client);
			client->SetRoom(this);
		}
	}

	mBattleManager.SetClients(clients);
}

void Room::RemoveClient(const Client& client)
{
	{
		lock_guard<mutex> lg(cLock);
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
			mBattleManager.RemoveClient(client.GetNetworkID());
			--mSize;
		}

		if (mSize == 0)
		{
			Init();
		}
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

vector<int32_t> Room::GetRandomItemQueue()
{
	vector<SlotInfo> items;

	vector<int32_t> itemQueue;
	itemQueue.reserve(BATTLE_ITEM_QUEUE_LENGTH);

	for (auto it = mClients.begin(); it != mClients.end(); ++it)
	{
		itemQueue.emplace_back((*it)->GetNetworkID());
		items = (*it)->GetValidUsingItems();
		LogWriteTest("{0} 클라이언트 {1}개 아이템 장착중", (*it)->GetNetworkID(), items.size());

		const size_t length = items.size();
		log_assert(length <= MAX_USING_ITEM);

		int sum = 0;
		for (const SlotInfo& slotInfo : items)
		{
			sum += slotInfo.item.GetActivePercent();
		}

		items.reserve(MAX_USING_ITEM * BATTLE_ITEM_QUEUE_LOOP_COUNT);
		CopySelf(items, BATTLE_ITEM_QUEUE_LOOP_COUNT - 1);

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
					++iter;
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
	log_assert(itemQueue.size() == BATTLE_ITEM_QUEUE_LENGTH);
	for (size_t i = 0; i < MAX_ROOM_PLAYER; ++i)
	{
		size_t index = i * 60 + i;
		for (size_t j = 0; j < BATTLE_ITEM_QUEUE_LOOP_COUNT; ++j)
		{
			size_t index2 = index + j * (MAX_USING_ITEM * 2);
			LogWriteTest("[아이템순서] 네트워크 {0}번 클라이언트 : {1}번째 순서 === {2}:{3}  {4}:{5}  {6}:{7}  {8}:{9}  {10}:{11}  {12}:{13} ",
				itemQueue[index], j, itemQueue[index2 + 1], itemQueue[index2 + 2], itemQueue[index2 + 3], itemQueue[index2 + 4],
				itemQueue[index2 + 5], itemQueue[index2 + 6], itemQueue[index2 + 7], itemQueue[index2 + 8],
				itemQueue[index2 + 9], itemQueue[index2 + 10], itemQueue[index2 + 11], itemQueue[index2 + 12]);
		}
	}

	return itemQueue;
}

void Room::TrySendBattleInfo()
{
	lock_guard<mutex> lg(cLock);
	if (GetBattleReadyCount() < mSize)
	{
		return;
	}

	if (mSize < 2)
	{
		return;
	}

	SendRandomItemQueue();

	for (Client* pClient : mClients)
	{
		pClient->SetBattleReady(false);
	}
}

void Room::Init()
{
	mClients.clear();
	mSize = 0;
	mIsRun = false;

	{
		lock_guard<mutex> lg(g_roomManager.cLock);
		Log("{0}번 룸 비활성화 (현재 활성화된 방 : {1})", mNumber, g_roomManager.GetUsingRoomCount());
	}
}

size_t Room::GetBattleReadyCount() const
{
	size_t cnt = 0;
	for (const Client* client : mClients)
	{
		if (client->IsBattleReady())
		{
			++cnt;
		}
	}

	return cnt;
}

void Room::SendRandomItemQueue()
{
	vector<int32_t> battleOpponent = mBattleManager.GetBattleOpponent();
	vector<int32_t> itemQueue = GetRandomItemQueue();

	sc_battleInfoPacket packet(battleOpponent, itemQueue);
	SendPacketToAllClients(&packet);
}
