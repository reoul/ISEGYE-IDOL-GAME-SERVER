#include "Room.h"

#include "Client.h"
#include "Random.h"

void SendPacket(int userID, void* pPacket);

Room::Room(int roomNumber, int capacity)
	: mSize(0)
	, mCapacity(capacity)
	, mRoomNumber(roomNumber)
{
}

void Room::AddClient(Client& client)
{
	if (mSize++ == mCapacity)
	{
		return;
	}

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

struct SlotInfo
{
	int itemIndex;
	int activePercent;
	SlotInfo(int index, int percent)
		: itemIndex(index)
		, activePercent(percent)
	{
	}
};

void Room::SendRandomItemQueue() const
{
	int sum = 0;
	const int LOOP_NUM = 5;
	vector<SlotInfo> items;
	items.reserve(MAX_USING_ITEM * LOOP_NUM);

	for (auto it = mClients.begin(); it != mClients.end(); ++it)
	{
		for (size_t i = 0; i < MAX_USING_ITEM; ++i)
		{
			/*const Item& item = (*it)->GetUsingItems()[i];
			const uint8_t itemType = item.GetType();
			if (itemType != EMPTY_ITEM && itemType != LOCK_ITEM)
			{
				sum += item.GetActivePercent();
				items.emplace_back((SlotInfo(i, item.GetActivePercent())));
			}*/
		}

		const int length = items.size();

		vector<SlotInfo> tmpItems;
		tmpItems.reserve(24);
		for (size_t i = 0; i < LOOP_NUM - 1; ++i)		// 총 5번 루프를 돌게 복사
		{
			copy(items.begin(), items.end(), back_inserter(tmpItems));
		}
		copy(tmpItems.begin(), tmpItems.end(), back_inserter(items));

		{
			int loopSum = sum;
			int loopLength = length;
			while (!items.empty())
			{
				Random<int> gen(0, loopSum - 1);
				int rand = gen();
				auto iterator = items.begin();
				for (int i = 0; i < loopLength; ++i)
				{
					rand -= iterator->activePercent;
					if (rand < 0)
					{
						--loopLength;
						loopSum -= iterator->activePercent;
						items.erase(iterator);
						break;
					}
				}

				if(loopLength == 0)
				{
					
				}
			}
		}


		items.clear();
		sum = 0;
	}
}
