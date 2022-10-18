#pragma once
#include "ServerShared.h"
#include "BattleManager.h"
#include "ServerStruct.h"

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
	void SendPacketToAllClients(void* pPacket) const;
	void SendPacketToAnotherClients(const Client& client, void* pPacket) const;
	vector<int32_t> GetRandomItemQueue();
	void TrySendBattleInfo();
	const vector<Client*>& GetClients() const;
	vector<Client*>& GetClients();
	bool IsRun() const;
	void SetIsRun(bool isRun);
	void Init();
	size_t GetSize() const;
	size_t GetNumber() const;
	void SetNumber(size_t number);
	size_t GetBattleReadyCount() const;
	void TrySendEnterInGame();
	bool IsFinishChoiceCharacter() const;
private:
	void SendRandomItemQueue();
	vector<Client*> mClients;
	size_t mSize;
	size_t mNumber;
	bool mIsRun;
	const_wrapper<size_t> mCapacity;
	BattleManager mBattleManager;
	bool mIsFinishChoiceCharacter;
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
