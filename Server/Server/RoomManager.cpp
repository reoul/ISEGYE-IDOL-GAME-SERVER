#include "RoomManager.h"

RoomManager::RoomManager()
	: mRooms{}
{
	for (size_t i = 0; i < MAX_ROOM_COUNT; ++i)
	{
		mRooms[i].SetNumber(i);
	}
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
