#pragma once
#include "ServerShared.h"
#include "ServerStruct.h"

class Client;

class Room
{
public:
	mutex cLock;
	Room();
	~Room() = default;
	Room(const Room&) = delete;
	Room& operator=(const Room&) = delete;
	void AddClient(Client& client);
	void RemoveClient(const Client& client);
	void SendAllClient(void* pPacket) const;
	void SendAnotherClient(const Client& client, void* pPacket) const;
	vector<int32_t> GetRandomItemQueue() const;
	void TrySendRandomItemQueue();
	const vector<Client*>& GetClients() const;
	bool IsRun() const;
	void SetIsRun(bool isRun);
	void BattleReady();
private:
	void SendRandomItemQueue() const;
	vector<Client*> mClients;
	size_t mSize;
	bool mIsRun;
	const size_t mCapacity;
	int mBattleReadyCount;
};

inline const vector<Client*>& Room::GetClients() const
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
