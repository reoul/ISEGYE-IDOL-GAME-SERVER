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
	void ReturnRoom(Room& room);
	void TrySendRandomItemQueue();
	void CheckActiveRoom();
	// todo : 활성화 됬다가 사용안되는 룸 끄는 함수 구현
private:
	Room mRooms[MAX_ROOM_COUNT];
};
