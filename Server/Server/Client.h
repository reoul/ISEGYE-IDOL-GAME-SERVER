#pragma once
#include <mutex>
#include <WinSock2.h>
#include <vector>

#include "Room.h"
#include "ServerStruct.h"

using namespace std;

class Client
{
public:
	std::mutex				cLock;
	Client();

	const SOCKET&			GetSocket() const;
	void					SetSocket(SOCKET socket);
	int32_t					GetNetworkID() const;
	void					SetNetworkID(int32_t networkID);
	Exover&					GetRecvOver();
	int32_t					GetPrevSize() const;
	void					SetPrevSize(int32_t size);
	char*					GetPacketBuf();
	bool					IsAlive() const;
	C_STATUS				GetStatus() const;
	void					SetStatus(C_STATUS status);
	CharacterType			GetCharacterType() const;
	Room*					GetRoomPtr();
	const Room*				GetRoomPtr() const;
	void					SetRoom(Room* room);
	wchar_t*				GetName();
	const wchar_t*			GetName() const;
	vector<Item>			GetUsingItems() const;
	vector<SlotInfo>		GetValidUsingItems() const;
	int						GetLockSlotCount() const;
	vector<Item>			GetUnUsingItems() const;
	vector<SlotInfo>		GetValidUnUsingItems() const;
	void					SwapItem(uint8_t index1, uint8_t index2);
	void					AddItem(uint8_t type);
	void					TrySetDefaultUsingItem();
	void					AddDefaultItem();
	void					SendPacketInAllRoomClients(void* pPacket) const;
	void					SendPacketInAnotherRoomClients(void* pPacket) const;
private:
	SOCKET					mSocket;
	int32_t					mNetworkID;						// 클라이언트 아이디
	Exover					mRecvOver;						// 확장 overlapped 구조체
	int32_t					mPrevSize;						// 이전에 받아놓은 양
	char					mPacketBuf[MAX_PACKET_SIZE];	// 조각난 거 받아두기 위한 버퍼
	bool					mIsAlive;						// 플레이 도중에 살아있는지(HP가 0이 아닌경우)
	C_STATUS				mStatus;						// 접속했나 안했나
	CharacterType			mCharacterType;					// 플레이어 캐릭터
	Room*					mRoomPtr;						// 클라이언트가 속한 룸
	wchar_t					mName[MAX_USER_NAME_LENGTH];	// 플레이어 이름
	Item					mUsingItems[MAX_USING_ITEM];
	Item					mUnUsingItems[MAX_UN_USING_ITEM];
};

inline const SOCKET& Client::GetSocket() const
{
	return mSocket;
}

inline void Client::SetSocket(SOCKET socket)
{
	mSocket = socket;
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

inline Room* Client::GetRoomPtr()
{
	return mRoomPtr;
}

inline const Room* Client::GetRoomPtr() const
{
	return mRoomPtr;
}

inline void Client::SetRoom(Room* room)
{
	mRoomPtr = room;
}


inline wchar_t* Client::GetName()
{
	return mName;
}

inline const wchar_t* Client::GetName() const
{
	return mName;
}

inline void Client::SendPacketInAllRoomClients(void* pPacket) const
{
	mRoomPtr->SendPacketToAllClients(pPacket);
}

inline void Client::SendPacketInAnotherRoomClients(void* pPacket) const
{
	mRoomPtr->SendPacketToAnotherClients(*this, pPacket);
}
