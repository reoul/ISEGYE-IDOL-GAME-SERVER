#pragma once
#include "ServerShared.h"
#include "BattleManager.h"
#include "ServerStruct.h"

enum class ERoomStatusType
{
	/// <summary> 캐릭터 선택창 </summary>
	ChoiceCharacter,
	/// <summary> 컷신 스테이지 </summary>
	CutSceneStage,
	/// <summary> 준비 스테이지 </summary>
	ReadyStage,
	/// <summary> 전투 스테이지 </summary>
	BattleStage,
	/// <summary> 크립 스테이지 </summary>
	CreepStage,
};

class BattleAvatar;

struct CreepRewardInfo
{
public:
	EItemTicketType itemTicketType;
	int count;
	CreepRewardInfo(EItemTicketType ticketType, int count)
		: itemTicketType(ticketType)
		, count(count)
	{
	}
};

class Room
{
public:
	mutex cLock;
	Room();
	~Room() = default;
	Room(const Room&) = delete;
	Room& operator=(const Room&) = delete;
	void AddClients(vector<Client*>& clients);
	void RemoveClient(const Client& client);
	void SendPacketToAllClients(void* pPacket);
	void SendPacketToAllClients(void* pPacket, ULONG size);
	void SendPacketToAnotherClients(const Client& client, void* pPacket);
	void SendPacketToAnotherClients(const Client& client, void* pPacket, ULONG size);
	void ApplyRandomBattleOpponent();
	const vector<Client*>& GetClients() const;
	vector<Client*>& GetClients();
	bool IsRun() const;
	void SetIsRun(bool isRun);
	void Init();
	size_t GetSize() const;
	size_t GetNumber() const;
	void SetNumber(size_t number);
	void TrySendEnterInGame();
	bool IsFinishChoiceCharacter() const;
	static unsigned __stdcall ProgressThread(void* pArguments);
	vector<int32_t>& GetBattleOpponents();
	vector<int32_t>& GetItemQueues();
	static bool ReadyStage(Room& room, bool isNextStageBattle);
	static bool BattleStage(Room& room);
	static bool CreepStage(Room& room);
	size_t GetOpenCount() const;
	void AddOpenCount();
	int GetRound() const;
	ERoomStatusType GetCurRoomStatusType() const;
	bool IsValidClientInThisRoom(Client* client) const;
	BattleAvatar GetCreepMonster();
	ECreepType GetCurCreepType() const;
	CreepRewardInfo GetCreepRewardTicketType() const;
	bool IsFinishThread() const;
private:
	vector<Client*> mClients;
	size_t mSize;
	size_t mNumber;
	bool mIsRun;
	const_wrapper<size_t> mCapacity;
	BattleManager mBattleManager;
	bool mIsFinishChoiceCharacter;
	vector<int32_t> mBattleOpponents;
	vector<int32_t> mItemQueues;
	size_t mOpenCount;	// 룸 열린 횟수, Room 진행이 스레드로 돌아서 해제되면서 바로 열리면 스레드가 계속 진행되므로 구별 변수
	int mRound;	// 진행 라운드
	ERoomStatusType mCurRoomStatusType;	// Room 진행 상황
	size_t mCreepRound;	// 현재 크립라운드 고멤 누구 나올지 결정해주는 변수
	bool mIsFinishThread;	// Room 진행 스레드 종료 되었는지
};

inline const vector<Client*>& Room::GetClients() const
{
	return mClients;
}

inline vector<Client*>& Room::GetClients()
{
	return mClients;
}

inline bool Room::IsRun() const
{
	return mIsRun;
}

inline void Room::SetIsRun(bool isRun)
{
	mIsRun = isRun;
}

inline size_t Room::GetSize() const
{
	return mSize;
}

inline size_t Room::GetNumber() const
{
	return mNumber;
}

inline void Room::SetNumber(size_t number)
{
	mNumber = number;
}

inline bool Room::IsFinishChoiceCharacter() const
{
	return mIsFinishChoiceCharacter;
}

inline vector<int32_t>& Room::GetBattleOpponents()
{
	return mBattleOpponents;
}

inline vector<int32_t>& Room::GetItemQueues()
{
	return mItemQueues;
}

inline size_t Room::GetOpenCount() const
{
	return mOpenCount;
}

inline void Room::AddOpenCount()
{
	++mOpenCount;
}

inline int Room::GetRound() const
{
	return mRound;
}

inline ERoomStatusType Room::GetCurRoomStatusType() const
{
	return mCurRoomStatusType;
}

inline bool Room::IsFinishThread() const
{
	return mIsFinishThread;
}
