#pragma once
#include <mutex>
#include <WinSock2.h>
#include <vector>
#include <chrono>

#include "Room.h"
#include "ServerStruct.h"
#include "reoul/logger.h"


class Room;

using namespace std;
using namespace chrono;
using namespace Logger;

enum class EItemTierType;

class Client
{
public:
	Client();

	void						Init();
	const SOCKET&				GetSocket() const;
	void						SetSocket(SOCKET socket);
	int32_t						GetNetworkID() const;
	void						SetNetworkID(int32_t networkID);
	Exover&						GetRecvOver();
	int32_t						GetPrevSize() const;
	void						SetPrevSize(int32_t size);
	char*						GetPacketBuf();
	bool						IsAlive() const;
	ESocketStatus				GetStatus() const;
	void						SetStatus(ESocketStatus status);
	ECharacterType				GetCharacterType() const;
	void						SetCharacterType(ECharacterType characterType);
	Room*						GetRoomPtr();
	const Room*					GetRoomPtr() const;
	void						SetRoom(Room* room);
	wchar_t*					GetName();
	const wchar_t*				GetName() const;
	vector<Item>				GetUsingItems() const;
	vector<SlotInfo>			GetValidUsingItems() const;
	vector<Item>				GetUnUsingItems() const;
	vector<SlotInfo>			GetValidUnUsingItems() const;
	void						SwapItem(uint8_t index1, uint8_t index2);
	uint8_t						AddItem(uint8_t type);
	uint8_t						GetRandomItemTypeOfNormalItemTicket() const;
	uint8_t						GetRandomItemTypeOfAdvancedItemTicket() const;
	uint8_t						GetRandomItemTypeOfTopItemTicket() const;
	uint8_t						GetRandomItemTypeOfSupremeItemTicket() const;
	bool						TrySetDefaultUsingItem();
	void						SendPacketInAllRoomClients(void* pPacket) const;
	void						SendPacketInAnotherRoomClients(void* pPacket) const;
	std::mutex&					GetMutex();
	int16_t						GetFirstAttackState() const;
	void						SetFirstAttackState(int16_t firstAttack);
	bool						IsChoiceCharacter() const;
	void						SetChoiceCharacter(bool isChoice);
	void						SetLastConnectCheckPacketTime(system_clock::time_point time);
	bool						IsValidConnect() const;
	void						ToDamage(int damage);
	void						ToDamageAvatar(int damage);
	void						ToHealAvatar(int heal);
	int32_t						GetHp() const;
	void						SetHp(int32_t hp);
	int32_t						GetAvatarHp() const;
	void						SetAvatarHp(int32_t hp);
	int32_t						GetAvatarMaxHp() const;
	void						SetAvatarMaxHp(int32_t maxHp);
	int32_t						GetAvatarDefensive() const;
	void						SetAvatarDefensive(int32_t defensive);
	Item&						GetItem(uint8_t index);
	void						SetItem(uint8_t index, uint8_t type);
	uint8_t						FindEmptyItemSlotIndex() const;
	int32_t						GetNormalItemTicketCount() const;
	void						SetNormalItemTicketCount(int32_t count);
	int32_t						GetAdvancedItemTicketCount() const;
	void						SetAdvancedItemTicketCount(int32_t count);
	int32_t						GetTopItemTicketCount() const;
	void						SetTopItemTicketCount(int32_t count);
	int32_t						GetSupremeItemTicketCount() const;
	void						SetSupremeItemTicketCount(int32_t count);
	uint8_t						GetRandomItemTypeByCombination(EItemTierType topTier);
	vector<SlotInfo>			GetItemActiveQueue() const;
	Item						GetBattleItem(int index) const;
	void						SetBattleItem(int index, Item item);
	void						SetRoomOpenCount(size_t openCount);
	size_t						GetRoomOpenCount() const;
private:
	std::mutex					mLock;
	SOCKET						mSocket;
	int32_t						mNetworkID;								// 클라이언트 아이디
	Exover						mRecvOver;								// 확장 overlapped 구조체
	int32_t						mPrevSize;								// 이전에 받아놓은 양
	int32_t						mHp;									// 플레이어 체력
	int32_t						mAvatarMaxHp;							// 전투 아바타 최대 체력
	int32_t						mAvatarHp;								// 전투 아바타 체력
	int32_t						mAvatarDefensive;						// 전투 아바타 방어도
	char						mPacketBuf[MAX_PACKET_SIZE];			// 조각난 거 받아두기 위한 버퍼
	bool						mIsAlive;								// 플레이 도중에 살아있는지(HP가 0이 아닌경우)
	ESocketStatus				mStatus;								// 접속했나 안했나
	ECharacterType				mCharacterType;							// 플레이어 캐릭터
	Room*						mRoomPtr;								// 클라이언트가 속한 룸
	wchar_t						mName[MAX_USER_NAME_LENGTH];			// 플레이어 이름
	Item						mUsingItems[MAX_USING_ITEM_COUNT];			// 전투시에 사용되는 인벤토리
	Item						mUnUsingItems[MAX_UN_USING_ITEM_COUNT];		// 보유 유물 보관 인벤토리
	int16_t						mFirstAttackState;						// 선공 스텟
	bool						mIsChoiceCharacter;						// 캐릭터를 선택했는지
	system_clock::time_point	mLastConnectCheckPacketTime;			// 마지막으로 연결 체크 패킷 받은 시간
	int							mNormalItemTicketCount;					// 일반 아이템 뽑기권 개수
	int							mAdvancedItemTicketCount;				// 고급 아이템 뽑기권 개수
	int							mTopItemTicketCount;					// 최고급 아이템 뽑기권 개수
	int							mSupremeItemTicketCount;				// 지존 아이템 뽑기권 개수
	Item						mBattleItems[MAX_USING_ITEM_COUNT];			// 전투에서 사용되는 아이템
	size_t						mRoomOpenCount;							// Room 유효성 체크 변수
	static constexpr int		sItemProbability[6][MAX_ITEM_UPGRADE]	// 라운드에 따른 아이템 확률
	{
		//1티어		2티어	3티어	4티어	5티어
		{100,	0,		0,		0,		0},		// 1라운드
		{75,	25,		0,		0,		0},		// 2라운드 
		{55,	30,		15,		0,		0},		// 3라운드 
		{25,	40,		30,		5,		0},		// 4라운드 
		{16,	20,		35,		25,		4},		// 5라운드 
		{9,		15,		36,		30,		10}		// 6라운드부터
	};
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

inline ESocketStatus Client::GetStatus() const
{
	return mStatus;
}

inline void Client::SetStatus(ESocketStatus status)
{
	mStatus = status;
}

inline ECharacterType Client::GetCharacterType() const
{
	return mCharacterType;
}

inline void Client::SetCharacterType(ECharacterType characterType)
{
	mCharacterType = characterType;
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
	mRoomOpenCount = room->GetOpenCount();
}

inline wchar_t* Client::GetName()
{
	return mName;
}

inline const wchar_t* Client::GetName() const
{
	return mName;
}

inline std::mutex& Client::GetMutex()
{
	return mLock;
}

inline int16_t Client::GetFirstAttackState() const
{
	return mFirstAttackState;
}

inline void Client::SetFirstAttackState(int16_t firstAttack)
{
	mFirstAttackState = firstAttack;
}

inline bool Client::IsChoiceCharacter() const
{
	return mIsChoiceCharacter;
}

inline void Client::SetChoiceCharacter(bool isChoice)
{
	mIsChoiceCharacter = isChoice;
}

inline void Client::SetLastConnectCheckPacketTime(system_clock::time_point time)
{
	mLastConnectCheckPacketTime = time;
}

inline int32_t Client::GetHp() const
{
	return mHp;
}

inline void	Client::SetHp(int32_t hp)
{
	mHp = hp;
}

inline int32_t Client::GetAvatarHp() const
{
	return mAvatarHp;
}

inline void Client::SetAvatarHp(int32_t hp)
{
	mAvatarHp = hp;
}

inline void Client::ToDamage(int damage)
{
	mHp -= damage;
	mHp = mHp > 0 ? mHp : 0;
}

inline void Client::ToDamageAvatar(int damage)
{
	mAvatarHp -= damage;
	mAvatarHp = mAvatarHp > 0 ? mAvatarHp : 0;
}

inline void Client::ToHealAvatar(int heal)
{
	mAvatarHp += heal;
}

inline int32_t Client::GetAvatarMaxHp() const
{
	return mAvatarMaxHp;
}

inline void Client::SetAvatarMaxHp(int32_t maxHp)
{
	mAvatarMaxHp = maxHp;
}

inline int32_t Client::GetAvatarDefensive() const
{
	return mAvatarDefensive;
}

inline void	Client::SetAvatarDefensive(int32_t defensive)
{
	mAvatarDefensive = defensive;
}

inline Item& Client::GetItem(uint8_t index)
{
	return index < MAX_USING_ITEM_COUNT ? mUsingItems[index] : mUnUsingItems[index - MAX_USING_ITEM_COUNT];
}

inline int32_t Client::GetNormalItemTicketCount() const
{
	return mNormalItemTicketCount;
}

inline void Client::SetNormalItemTicketCount(int32_t count)
{
	mNormalItemTicketCount = count;
}

inline int32_t Client::GetAdvancedItemTicketCount() const
{
	return mAdvancedItemTicketCount;
}

inline void Client::SetAdvancedItemTicketCount(int32_t count)
{
	mAdvancedItemTicketCount = count;
}

inline int32_t Client::GetTopItemTicketCount() const
{
	return mTopItemTicketCount;
}

inline void Client::SetTopItemTicketCount(int32_t count)
{
	mTopItemTicketCount = count;
}

inline int32_t Client::GetSupremeItemTicketCount() const
{
	return mSupremeItemTicketCount;
}

inline void Client::SetSupremeItemTicketCount(int32_t count)
{
	mSupremeItemTicketCount = count;
}

inline Item Client::GetBattleItem(int index) const
{
	//log_assert(index < MAX_USING_ITEM_COUNT);
	return mBattleItems[index];
}

inline void Client::SetBattleItem(int index, Item item)
{
	//log_assert(index < MAX_USING_ITEM_COUNT);
	mBattleItems[index] = item;
}

inline void Client::SetRoomOpenCount(size_t openCount)
{
	mRoomOpenCount = openCount;
}

inline size_t Client::GetRoomOpenCount() const
{
	return mRoomOpenCount;
}
