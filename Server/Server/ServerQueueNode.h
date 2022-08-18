#pragma once

#include <memory>

struct Client;

class ServerQueueNode
{
public:
	ServerQueueNode(Client* client);
	std::shared_ptr<ServerQueueNode> Next;
	std::weak_ptr<ServerQueueNode> Previous;
	inline Client* GetClient() const
	{
		return mClient;
	}
private:
	Client* mClient;
};