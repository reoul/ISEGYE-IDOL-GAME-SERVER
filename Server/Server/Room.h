#pragma once
#include "ServerShared.h"
#include "ServerStruct.h"

class Room
{
public:
	Room(int roomNumber, int capacity = MAX_ROOM_PLAYER);
	~Room() = default;
	Room(const Room&) = delete;
	Room& operator=(const Room&) = delete;
	void AddClient(Client& client);
	void RemoveClient(const Client& client);
	void SendAllClient(void* pPacket);
	void SendAnotherClient(const Client& client, void* pPacket);
	int GetRoomNumber() const;
	const vector<Client*>& GetClients() const;
private:
	vector<Client*> mClients;
	size_t mSize;
	const size_t mCapacity;
	int mRoomNumber;
};

inline int Room::GetRoomNumber() const
{
	return mRoomNumber;
}

inline const vector<Client*>& Room::GetClients() const
{
	return mClients;
}
