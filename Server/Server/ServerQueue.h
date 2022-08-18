#pragma once
#include <mutex>
#include <queue>
#include "ServerStruct.h"
#include "Room.h"
#include "ServerQueueNode.h"

extern int g_roomIndex;

class ServerQueue
{
public:
	ServerQueue();
	~ServerQueue() = default;
	ServerQueue(const ServerQueue&) = delete;
	ServerQueue& operator=(const ServerQueue&) = delete;
	void AddClient(Client* client);
	void RemoveClient(Client* client);
	shared_ptr<Room> TryCreateRoomOrNullPtr();
	void Lock();
	void UnLock();
private:
	std::mutex mLock;
	std::shared_ptr<ServerQueueNode> mClientQueue;
	size_t mSize;
};
