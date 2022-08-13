#include "ServerQueue.h"
#include <iostream>
#include "SettingData.h"

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
		node->Next = make_shared<ServerQueueNode>(client);
	}

	++mSize;
	wcout << L"[" << client->name << L"] 대기열 등록 완료" << endl;
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
				node->Next->Previous = node->Previous.lock();
			}
			wcout << L"[" << client->name << L"] 대기열 삭제 완료" << endl;
			--mSize;
			break;
		}
		node = node->Next;
	}
}

shared_ptr<Room> ServerQueue::TryCreateRoomOrNullPtr()
{
	if (mSize >= ROOM_MAX_PLAYER)
	{
		shared_ptr<Room> room = make_shared<Room>(++g_roomIndex, ROOM_MAX_PLAYER);

		auto node = mClientQueue;
		for (size_t i = 0; i < ROOM_MAX_PLAYER; ++i)
		{
			auto pClient = node->GetClient();
			pClient->room = room;
			room->AddUser(pClient);
			node = node->Next;
		}
		wcout << g_roomIndex << L"번 Room이 생성됨" << endl;
		mClientQueue = node;
		if (node != nullptr)
		{
			node->Previous.reset();
		}

		mSize -= ROOM_MAX_PLAYER;
		return room;
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
