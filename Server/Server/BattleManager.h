#pragma once
#include <memory>
#include <vector>

#include "SettingData.h"

struct Client;

class BattleManager
{
public:
	BattleManager() = delete;
	BattleManager(std::vector<Client*> clients);
	BattleManager(const BattleManager&) = delete;
	~BattleManager() = default;

	std::array<int32_t, MAX_ROOM_PLAYER> GetBattleOpponent();
	
private:
	struct BattleInfo
	{
		Client* pClient;
		bool isFight;	// 전투를 했는지
		bool isChoice;	// 선택을 했는지
		BattleInfo()
			: pClient(nullptr)
			, isFight(false)
			, isChoice(false)
		{
		}
	};
	BattleInfo	mBattleInfo[MAX_ROOM_PLAYER][MAX_ROOM_PLAYER - 1];
	bool		mReady[MAX_ROOM_PLAYER];
};