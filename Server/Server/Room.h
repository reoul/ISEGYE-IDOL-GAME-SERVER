#pragma once
#include "ServerShared.h"
#include "ServerStruct.h"

class Room
{
public:
	Room(int roomNumber, int capacity = ROOM_MAX_PLAYER);
	~Room() = default;
	Room(const Room&) = delete;
	Room& operator=(const Room&) = delete;
	void AddUser(Client* client);
	void SendAllPlayer(void* p);
	void SendAnotherPlayer(Client* client, void* p);
	inline int GetRoomNumber() const
	{
		return mRoomNumber;
	}
	inline const vector<Client*>& GetClients() const
	{
		return mClients;
	}
private:
	vector<Client*> mClients;
	size_t mSize;
	const size_t mCapacity;
	int mRoomNumber;
	int mCount;
};
