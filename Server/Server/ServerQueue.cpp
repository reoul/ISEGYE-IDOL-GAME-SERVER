#include "ServerQueue.h"
#include <iostream>

#include "Client.h"
#include "SettingData.h"
#include "GlobalVariable.h"

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
	wcout << L"[" << client->GetName() << L"] 대기열 등록 완료 " << mSize << L"명" << endl;
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
				if(node->Next != nullptr)
				{
					node->Next->Previous = node->Previous.lock();
				}
			}
			wcout << L"[" << client->GetName() << L"] 대기열 삭제 완료 " << mSize - 1 << L"명" << endl;
			--mSize;
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
			node = node->Next;
		}
		wcout << g_roomIndex << L"번 Room이 생성됨" << endl;
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
