#pragma once
#include "ServerShared.h"
#include "PacketStruct.h"

class Room
{
public:
	Room(int capacity = 8);
	~Room() = default;
	Room(const Room&) = delete;
	Room& operator=(const Room&) = delete;
	void AddUser(TCPNetworkUserInfo userInfo);
	void Send(const TestPacket& tp) const;
private:
	vector<TCPNetworkUserInfo> mUserInfos;
	size_t mSize;
	const size_t mCapacity;
};