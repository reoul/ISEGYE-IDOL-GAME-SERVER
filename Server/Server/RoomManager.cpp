#include "RoomManager.h"

RoomManager::RoomManager()
	: mRooms{}
	, mUsingRoomCount(0)
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
		if (!room.IsRun() && room.IsFinishThread())
		{
			pRet = &room;
			room.SetIsRun(true);
			break;
		}
	}

	pRet->AddOpenCount();
	
	return *pRet;
}

size_t RoomManager::GetUsingRoomCount()
{
	size_t cnt = 0;
	for (const Room& room : mRooms)
	{
		if (!room.IsFinishThread())
		{
			++cnt;
		}
	}
	return cnt;
}
