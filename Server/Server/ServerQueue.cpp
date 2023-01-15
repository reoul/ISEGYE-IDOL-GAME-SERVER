#include "ServerQueue.h"
#include <iostream>

#include "Client.h"
#include "SettingData.h"
#include "Server.h"
#include "reoul/logger.h"
#include "PacketStruct.h"

using namespace Logger;

ServerQueue::ServerQueue()
	: mClientQueue(nullptr)
	, mSize(0)
{
}

void ServerQueue::AddClient(Client& client)
{
	if (mSize == 0)
	{
		mClientQueue = make_shared<ServerQueueNode>(&client);
	}
	else
	{
		auto node = mClientQueue;
		for (size_t i = 0; i < mSize - 1; ++i)
		{
			node = node->Next;
		}
		shared_ptr<ServerQueueNode> newClient = make_shared<ServerQueueNode>(&client);
		node->Next = newClient;
		newClient->Previous = node;
	}

	++mSize;
	Log("log", "네트워크 {0}번 클라이언트 대기열 추가 (현재 {1}명 대기중)", client.GetNetworkID(), mSize);
}

void ServerQueue::RemoveClient(Client& client)
{
	auto node = mClientQueue;
	for (size_t i = 0; i < mSize; ++i)
	{
		if (node->GetClient() == &client)
		{
			if (node == mClientQueue)
			{
				mClientQueue = node->Next;
				if (mClientQueue != nullptr)
				{
					mClientQueue->Previous.reset();
				}
			}
			else
			{
				node->Previous.lock()->Next = node->Next;
				if (node->Next != nullptr)
				{
					node->Next->Previous = node->Previous.lock();
				}
			}
			--mSize;
			Log("log", "네트워크 {0}번 클라이언트 대기열 제거 (현재 {1}명 대기중)", client.GetNetworkID(), mSize);
			break;
		}
		node = node->Next;
	}
}

Room* ServerQueue::TryCreateRoomOrNullPtr()
{
	if (mSize >= MAX_ROOM_PLAYER)
	{
		Room& room = Server::GetRoomManager().GetUnUsedRoom();

		auto node = mClientQueue;

		vector<Client*> clients;
		for (size_t i = 0; i < MAX_ROOM_PLAYER; ++i)
		{
			Client& client = *node->GetClient();
			clients.emplace_back(&client);
			node = node->Next;
		}
		room.AddClients(clients);
		Log("log", "{0}번 룸 활성화 (현재 활성화된 방 : {1})", room.GetNumber(), Server::GetRoomManager().GetUsingRoomCount());
		mClientQueue = node;
		if (node != nullptr)
		{
			node->Previous.reset();
		}

		mSize -= MAX_ROOM_PLAYER;
		return &room;
	}
	return nullptr;
}

void ServerQueue::SendMatchingQueueInfo()
{
	auto node = mClientQueue;

	if (mSize <= 0)
	{
		return;
	}

	vector<Client*> clients;
	lock_guard<mutex> lg(mLock);

	for (size_t i = 0; i < mSize; ++i)
	{
		Client& client = *node->GetClient();
		clients.emplace_back(&client);
		node = node->Next;
	}

	for (Client* client : clients)
	{
		sc_MatchingInfoPacket packet(mSize);
		Server::SendPacket(client->GetNetworkID(), &packet);
	}
}
