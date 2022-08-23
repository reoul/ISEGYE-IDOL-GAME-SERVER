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
	void SendPacketToAllClients(void* pPacket) const;
	void SendPacketToAnotherClients(const Client& client, void* pPacket) const;
	vector<int32_t> GetRandomItemQueue() const;
	void TrySendRandomItemQueue();
	const vector<Client*>& GetClients() const;
	vector<Client*>& GetClients();
	bool IsRun() const;
	void SetIsRun(bool isRun);
	void BattleReady();
	void Init();
	size_t GetSize() const;
	size_t GetNumber() const;
	void SetNumber(size_t number);
private:
	void SendRandomItemQueue() const;
	vector<Client*> mClients;
	size_t mSize;
	size_t mNumber;
	bool mIsRun;
	const size_t mCapacity;
	int mBattleReadyCount;
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
