#include "RoomManager.h"

RoomManager::RoomManager()
	: mRooms{}
{
}

Room& RoomManager::GetUnUsedRoom()
{
	lock_guard<mutex> lg(cLock);
	Room* pRet = &mRooms[MAX_ROOM_COUNT - 1];
	for (Room& room : mRooms)
	{
		if (!room.IsRun())
		{
			pRet = &room;
			room.SetIsRun(true);
			break;
		}
	}
	
	return *pRet;
}

void RoomManager::ReturnRoom(Room& room)
{
	room.Init();
}

void RoomManager::TrySendRandomItemQueue()
{
	for (Room& room : mRooms)
	{
		if (room.IsRun())
		{
			room.TrySendRandomItemQueue();
		}
	}
}

void RoomManager::CheckActiveRoom()
{
	for (Room& room : mRooms)
	{
		if (room.IsRun() && room.GetSize() == 0)
		{
			room.Init();
		}
	}
}
