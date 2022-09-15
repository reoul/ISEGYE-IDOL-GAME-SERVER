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
		if (!room.IsRun())
		{
			pRet = &room;
			room.SetIsRun(true);
			break;
		}
	}
	
	return *pRet;
}

void RoomManager::TrySendBattleInfo()
{
	for (Room& room : mRooms)
	{
		if (room.IsRun())
		{
			room.TrySendBattleInfo();
		}
	}
}

size_t RoomManager::GetUsingRoomCount()
{
	lock_guard<mutex> lg(cLock);
	size_t cnt = 0;
	for (const Room& room : mRooms)
	{
		if (room.IsRun())
		{
			++cnt;
		}
	}
	return cnt;
}
