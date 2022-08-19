#include "Room.h"

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
		if(*it == &client)
		{
			break;
		}
	}

	if(it != mClients.cend())
	{
		mClients.erase(it);
		--mSize;
	}
}

void Room::SendAllClient(void* pPacket)
{
	for (auto it = mClients.begin(); it != mClients.end(); ++it)
	{
		SendPacket((*it)->networkID, pPacket);
	}
}

void Room::SendAnotherClient(const Client& client, void* pPacket)
{
	for (auto it = mClients.begin(); it != mClients.end(); ++it)
	{
		if(&client == *it)
		{
			continue;
		}

		SendPacket((*it)->networkID, pPacket);
	}
}
