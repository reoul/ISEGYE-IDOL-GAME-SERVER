#include "ServerQueue.h"
#include <iostream>

#include "Client.h"
#include "SettingData.h"
#include "GlobalVariable.h"
#include "Log.h"

ServerQueue::ServerQueue()
	: mClientQueue(nullptr)
	, mSize(0)
{
}

void ServerQueue::AddClient(Client* client)
{
	if (mSize == 0)
	{
		mClientQueue = make_shared<ServerQueueNode>(client);
	}
	else
	{
		auto node = mClientQueue;
		for (size_t i = 0; i < mSize - 1; ++i)
		{
			node = node->Next;
		}
		shared_ptr<ServerQueueNode> newClient = make_shared<ServerQueueNode>(client);
		node->Next = newClient;
		newClient->Previous = node;
	}

	++mSize;
	Log(L"[서버] %s(%d) 유저 대기열 등록 완료 (%d명 대기중)", client->GetName(), client->GetNetworkID(), mSize);
}

void ServerQueue::RemoveClient(Client* client)
{
	auto node = mClientQueue;
	for (size_t i = 0; i < mSize; ++i)
	{
		if (node->GetClient() == client)
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
			Log(L"[서버] %s(%d) 유저 대기열 삭제 완료 (%d명 대기중)", client->GetName(), client->GetNetworkID(), mSize);
			break;
		}
		node = node->Next;
	}
}

Room* ServerQueue::TryCreateRoomOrNullPtr()
{
	if (mSize >= MAX_ROOM_PLAYER)
	{
		Room& room = g_roomManager.GetUnUsedRoom();

		auto node = mClientQueue;
		for (size_t i = 0; i < MAX_ROOM_PLAYER; ++i)
		{
			Client& client = *node->GetClient();
			client.SetRoom(&room);
			room.AddClient(client);
			client.AddDefaultItem();
			node = node->Next;
		}
		Log(L"[서버] %d번 Room 생성", g_roomIndex++);
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

void ServerQueue::Lock()
{
	mLock.lock();
}

void ServerQueue::UnLock()
{
	mLock.unlock();
}
