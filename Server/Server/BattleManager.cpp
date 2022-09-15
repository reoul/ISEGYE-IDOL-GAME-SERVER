#include "BattleManager.h"
#include <vector>
#include <algorithm>
#include "Random.h"
#include "reoul/logger.h"
#include "SettingData.h"
#include "Client.h"

void BattleManager::SetClients(std::vector<Client*>& clients)
{
	log_assert(clients.size() == MAX_ROOM_PLAYER);
	mMatchingClients.clear();
	mUnMatchingClients.clear();

	for (size_t player = 0; player < MAX_ROOM_PLAYER; ++player)
	{
		mUnMatchingClients.emplace_back(clients[player], clients);
	}
}

std::vector<int32_t> BattleManager::GetBattleOpponent()
{
	std::vector<int32_t> list;
	log_assert(mUnMatchingClients.size() >= 2);

	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(mUnMatchingClients.begin(), mUnMatchingClients.end(), g);

	Random<int> gen(0, 1);

	while (!mUnMatchingClients.empty())
	{
		auto clientIt = mUnMatchingClients.begin();

		auto minClientIt = mUnMatchingClients.end();
		size_t minCount = INT64_MAX;
		for (auto it = clientIt + 1; it != mUnMatchingClients.end(); ++it)
		{
			const size_t battleCount = it->GetBattleCount(clientIt->GetClientPtr());
			if (battleCount < minCount)
			{
				minClientIt = it;
				minCount = battleCount;
			}
		}

		const Client* pClient = clientIt->GetClientPtr();
		const Client* pMinClient;
		if (minClientIt != mUnMatchingClients.end())
		{
			pMinClient = minClientIt->GetClientPtr();
		}

		if (minClientIt != mUnMatchingClients.end())
		{
			// todo : 선공 정하는 로직 추가
			int networkID1 = pClient->GetNetworkID();
			int networkID2 = pMinClient->GetNetworkID();

			const int16_t clientFirstAttackState = pClient->GetFirstAttackState();
			const int16_t minClientFirstAttackState = pMinClient->GetFirstAttackState();
			if(pClient->GetFirstAttackState() == pMinClient->GetFirstAttackState())
			{
				const int rand = gen();
				list.push_back(rand ? networkID1 : networkID2);
				list.push_back(rand ? networkID2 : networkID1);
			}
			else
			{
				list.push_back(clientFirstAttackState > minClientFirstAttackState ? networkID1 : networkID2);
				list.push_back(clientFirstAttackState > minClientFirstAttackState ? networkID2 : networkID1);
			}

			clientIt->AddBattleCount(pMinClient);
			minClientIt->AddBattleCount(pClient);

			mMatchingClients.push_back(*minClientIt);
			mUnMatchingClients.erase(minClientIt);
		}
		else
		{
			auto minClientIt2 = mMatchingClients.begin();
			minCount = INT64_MAX;
			for (auto it = mMatchingClients.begin(); it != mMatchingClients.end(); ++it)
			{
				const size_t battleCount = it->GetBattleCount(clientIt->GetClientPtr());
				if (battleCount < minCount)
				{
					minClientIt2 = it;
					minCount = battleCount;
				}
			}

			// todo : 선공 정하는 로직 추가
			int networkID1 = pClient->GetNetworkID();
			int networkID2 = ~pMinClient->GetNetworkID();

			const int16_t clientFirstAttackState = pClient->GetFirstAttackState();
			const int16_t minClientFirstAttackState = pMinClient->GetFirstAttackState();
			if (pClient->GetFirstAttackState() == pMinClient->GetFirstAttackState())
			{
				const int rand = gen();
				list.push_back(rand ? networkID1 : networkID2);
				list.push_back(rand ? networkID2 : networkID1);
			}
			else
			{
				list.push_back(clientFirstAttackState > minClientFirstAttackState ? networkID1 : networkID2);
				list.push_back(clientFirstAttackState > minClientFirstAttackState ? networkID2 : networkID1);
			}
		}

		mMatchingClients.push_back(*clientIt);
		mUnMatchingClients.erase(clientIt);
	}

	for (auto& battleInfo : mMatchingClients)
	{
		mUnMatchingClients.emplace_back(battleInfo);
	}
	mMatchingClients.clear();

	LogWriteTest("배틀 상대 목록 {0}명", list.size());
	for (size_t i = 0; i < list.size() / 2; ++i)
	{
		LogWriteTest("선공 {0} - {1}", list[i * 2], list[i * 2 + 1]);
	}

	log_assert(list.size() <= MAX_ROOM_PLAYER);
	return list;
}

void BattleManager::RemoveClient(int32_t networkID)
{
	for (auto it = mUnMatchingClients.begin(); it != mUnMatchingClients.end();)
	{
		if (it->GetClientPtr()->GetNetworkID() != networkID)
		{
			it->RemoveClient(networkID);
			++it;
		}
		else
		{
			it = mUnMatchingClients.erase(it);
		}
	}
}

BattleManager::BattleHistory::BattleHistory(Client* client)
	:pClient(client)
	, battleCount(0)
{
}

BattleManager::BattleInfo::BattleInfo(Client* client, const std::vector<Client*>& clients) : mClient(client)
{
	for (Client* c : clients)
	{
		if (client->GetNetworkID() == c->GetNetworkID())
		{
			continue;
		}

		mBattleInfos.emplace_back(c);
	}
}

size_t BattleManager::BattleInfo::GetBattleCount(const Client* c) const
{
	for (BattleHistory battleHistory : mBattleInfos)
	{
		if (c == battleHistory.pClient)
		{
			return battleHistory.battleCount;
		}
	}
	return 0;
}

void BattleManager::BattleInfo::AddBattleCount(const Client* client)
{
	for (BattleHistory& battleHistory : mBattleInfos)
	{
		if (battleHistory.pClient == client)
		{
			++battleHistory.battleCount;
		}
	}
}

void BattleManager::BattleInfo::RemoveClient(int32_t networkID)
{
	for (auto it = mBattleInfos.begin(); it != mBattleInfos.end(); ++it)
	{
		if (it->pClient.get()->GetNetworkID() == networkID)
		{
			mBattleInfos.erase(it);
			break;
		}
	}
}