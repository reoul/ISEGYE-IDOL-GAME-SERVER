﻿#pragma once
#include <mutex>
#include <WinSock2.h>
#include <vector>

#include "ServerStruct.h"

using namespace std;

class Client
{
public:
	std::mutex				cLock;
	Client();
	SOCKET& GetSocket();
	const SOCKET& GetSocket() const;
	int32_t GetNetworkID() const;
	void SetNetworkID(int32_t networkID);
	Exover& GetRecvOver();
	int32_t GetPrevSize() const;
	void SetPrevSize(int32_t size);
	char* GetPacketBuf();
	bool IsAlive() const;
	C_STATUS GetStatus() const;
	void SetStatus(C_STATUS status);
	CharacterType GetCharacterType() const;
	shared_ptr<Room>& GetRoom();
	const shared_ptr<Room>& GetRoom() const;
	wchar_t* GetName();
	const wchar_t* GetName() const;
	vector<Item> GetUsingItems() const;
	vector<Item> GetValidUsingItems() const;
	vector<Item> GetUnUsingItems() const;
	vector<Item> GetValidUnUsingItems() const;
private:
	SOCKET				mSocket;
	int32_t				mNetworkID;						// 클라이언트 아이디
	Exover				mRecvOver;						// 확장 overlapped 구조체
	int32_t				mPrevSize;						// 이전에 받아놓은 양
	char				mPacketBuf[MAX_PACKET_SIZE];	// 조각난 거 받아두기 위한 버퍼
	bool				mIsAlive;						// 플레이 도중에 살아있는지(HP가 0이 아닌경우)
	C_STATUS			mStatus;						// 접속했나 안했나
	CharacterType		mCharacterType;					// 플레이어 캐릭터
	shared_ptr<Room>	mRoom;							// 클라이언트가 속한 룸
	wchar_t				mName[MAX_USER_NAME_LENGTH];	// 플레이어 이름
	Item				mUsingItems[MAX_USING_ITEM];
	Item				mUnUsingItems[MAX_UN_USING_ITEM];

};

inline SOCKET& Client::GetSocket()
{
	return mSocket;
}

inline const SOCKET& Client::GetSocket() const
{
	return mSocket;
}

inline int32_t Client::GetNetworkID() const
{
	return mNetworkID;
}

inline void Client::SetNetworkID(int32_t networkID)
{
	mNetworkID = networkID;
}

inline Exover& Client::GetRecvOver()
{
	return mRecvOver;
}

inline int32_t Client::GetPrevSize() const
{
	return mPrevSize;
}

inline void Client::SetPrevSize(int32_t size)
{
	mPrevSize = size;
}

inline char* Client::GetPacketBuf()
{
	return mPacketBuf;
}

inline bool Client::IsAlive() const
{
	return mIsAlive;
}

inline C_STATUS Client::GetStatus() const
{
	return mStatus;
}

inline void Client::SetStatus(C_STATUS status)
{
	mStatus = status;
}

inline CharacterType Client::GetCharacterType() const
{
	return mCharacterType;
}

inline shared_ptr<Room>& Client::GetRoom()
{
	return mRoom;
}

inline const shared_ptr<Room>& Client::GetRoom() const
{
	return mRoom;
}

inline wchar_t* Client::GetName()
{
	return mName;
}

inline const wchar_t* Client::GetName() const
{
	return mName;
}