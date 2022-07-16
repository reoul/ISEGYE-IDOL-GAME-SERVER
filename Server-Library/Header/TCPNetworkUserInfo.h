#pragma once
#include "ServerShared.h"
#include "ostream"

using namespace std;

class TCPNetworkUserInfo
{
	friend ostream& operator<<(ostream& os, const TCPNetworkUserInfo& info)
	{
		os << "(client IP: " << info.mSocketAddress.ToString() << " , network ID: " << info.GetNetworkID() << " )";
		return os;
	}
public:
	TCPNetworkUserInfo(const TCPSocketPtr socketPtr, int id, const SocketAddress address)
		: mTcpSocketPtr(socketPtr)
		, mSocketAddress(address)
	{
	}

	TCPSocketPtr GetTcpSocketPtr() const
	{
		return mTcpSocketPtr;
	}

	uint64_t GetNetworkID() const
	{
		return mTcpSocketPtr->GetSocket();
	}

private:
	TCPSocketPtr mTcpSocketPtr;
	SocketAddress mSocketAddress;
};
