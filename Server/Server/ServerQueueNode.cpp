#include "ServerQueueNode.h"

ServerQueueNode::ServerQueueNode(Client* client)
	: Next(nullptr)
	, mClient(client)
{
}
