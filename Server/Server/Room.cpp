#include "Room.h"

#include <iostream>

void SendPacket(int userID, void* p);

Room::Room(int roomNumber, int capacity)
	: mSize(0)
	, mCapacity(capacity)
	, mRoomNumber(roomNumber)
	, mCount(0)
{
}

void Room::AddUser(Client* client)
{
	if (mSize++ == mCapacity)
	{
		return;
	}

	mClients.emplace_back(client);
}

void Room::SendAllPlayer(void* p)
{
	for (auto it = mClients.begin(); it != mClients.end(); ++it)
	{
		SendPacket((*it)->networkID, p);
	}
}

void Room::SendAnotherPlayer(Client* client, void* p)
{
	for (auto it = mClients.begin(); it != mClients.end(); ++it)
	{
		if(client == *it)
		{
			continue;
		}

		SendPacket((*it)->networkID, p);
	}
}
