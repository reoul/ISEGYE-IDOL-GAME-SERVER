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
	void RemoveRoom(std::shared_ptr<Room> pRoom);
private:
	Room mRooms[MAX_ROOM_COUNT];
};
