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
	void AddClient(Client& client);
	void RemoveClient(Client& client);
	Room* TryCreateRoomOrNullPtr();
	std::mutex& GetMutex();
	void SendMatchingQueueInfo(uint16_t connectionCount);
private:
	std::mutex mLock;
	std::shared_ptr<ServerQueueNode> mClientQueue;
	size_t mSize;
};

inline std::mutex& ServerQueue::GetMutex()
{
	return mLock;
}
