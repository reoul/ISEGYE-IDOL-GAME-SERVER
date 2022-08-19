#include "BattleManager.h"
#include <vector>
#include <array>

BattleManager::BattleManager(const std::vector<Client*> clients)
	: mBattleInfo{}
	, mReady{}
{
	for (size_t player = 0; player < MAX_ROOM_PLAYER; ++player)
	{
		for (size_t another = 0; another < MAX_ROOM_PLAYER; ++another)
		{
			if (player == another)
			{
				continue;
			}

			mBattleInfo[player][another].pClient = clients[another];
		}
	}
}

std::array<int32_t, MAX_ROOM_PLAYER> BattleManager::GetBattleOpponent()
{
	std::array<int32_t, MAX_ROOM_PLAYER> aa{};
	aa[1] = 10;
	return aa;
}
