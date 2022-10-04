#pragma once

#include <memory>

class Client;

class ServerQueueNode
{
public:
	ServerQueueNode(Client* client);
	std::shared_ptr<ServerQueueNode> Next;
	std::weak_ptr<ServerQueueNode> Previous;
	Client* GetClient() const;
private:
	Client* mClient;
};

inline Client* ServerQueueNode::GetClient() const
{
	return mClient;
}
