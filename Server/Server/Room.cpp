#include "Room.h"

Room::Room(int capacity)
	: mSize(0)
	, mCapacity(capacity)
{
}

void Room::AddUser(TCPNetworkUserInfo userInfo)
{
	if(mSize++ == mCapacity)
	{
		return;
	}

	mUserInfos.push_back(userInfo);
}

void Room::Send(const TestPacket& tp) const
{
	for (size_t i = 0; i < mSize; ++i)
	{
		mUserInfos[i].GetTcpSocketPtr()->Send(&tp, sizeof(tp));
	}
}
