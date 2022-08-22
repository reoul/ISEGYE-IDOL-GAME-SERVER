#pragma once
#include <memory>
#include <vector>
#include <queue>

#include "SettingData.h"

struct Client;

class BattleManager
{
public:
	BattleManager() = delete;
	BattleManager(std::vector<Client*> clients);
	BattleManager(const BattleManager&) = delete;
	~BattleManager() = default;

	std::vector<int32_t> GetBattleOpponent();
	
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
	//std::queue<BattleInfo>
};