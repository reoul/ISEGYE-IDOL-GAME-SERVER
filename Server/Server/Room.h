#pragma once
#include "ServerShared.h"
#include "ServerStruct.h"

class Client;

class Room
{
public:
	Room();
	~Room() = default;
	Room(const Room&) = delete;
	Room& operator=(const Room&) = delete;
	void AddClient(Client& client);
	void RemoveClient(const Client& client);
	void SendAllClient(void* pPacket) const;
	void SendAnotherClient(const Client& client, void* pPacket) const;
	vector<int32_t> GetRandomItemQueue() const;
	void SendRandomItemQueue() const;
	const vector<Client*>& GetClients() const;
	bool IsRun();
	void SetIsRun(bool isRun);
private:
	vector<Client*> mClients;
	size_t mSize;
	bool mIsRun;
	const size_t mCapacity;
};

inline const vector<Client*>& Room::GetClients() const
{
	return mClients;
}

inline bool Room::IsRun()
{
	return mIsRun;
}

inline void Room::SetIsRun(bool isRun)
{
	mIsRun = isRun;
}
