#include "RoomManager.h"

RoomManager::RoomManager()
	: mRooms{}
{
}

Room& RoomManager::GetUnUsedRoom()
{
	for (Room& room : mRooms)
	{
		if(!room.IsRun())
		{
			return room;
		}
	}

	return mRooms[MAX_ROOM_COUNT - 1];
}
