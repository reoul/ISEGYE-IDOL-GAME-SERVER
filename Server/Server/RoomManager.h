#pragma once
#include <memory>
#include <mutex>
#include "SettingData.h"
#include "Room.h"

class RoomManager
{
public:
	std::mutex cLock;
	RoomManager();
	~RoomManager() = default;
	Room& GetUnUsedRoom();
	void TrySendBattleInfo();
	size_t GetUsingRoomCount();
private:
	Room mRooms[MAX_ROOM_COUNT];
	size_t mUsingRoomCount;
};
