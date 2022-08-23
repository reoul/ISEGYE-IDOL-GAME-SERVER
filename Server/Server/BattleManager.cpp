#include "BattleManager.h"
#include <vector>

BattleManager::BattleManager(const std::vector<Client*> clients)
{
	for (size_t player = 0; player < MAX_ROOM_PLAYER; ++player)
	{
		for (size_t another = 0; another < MAX_ROOM_PLAYER; ++another)
		{
			if (player == another)
			{
				continue;
			}

			//mBattleInfo[player][another].pClient = clients[another];
		}
	}
}

std::vector<int32_t> BattleManager::GetBattleOpponent()
{
	std::vector<int32_t> list;
	// todo : 배틀 상대 지정 함수 구현
	/*size_t cnt = 0;
	for (size_t i = 0; i < MAX_ROOM_PLAYER; ++i)
	{
		if(m)
	}
	for(size_t i = 0; i < )*/
	return list;
}
