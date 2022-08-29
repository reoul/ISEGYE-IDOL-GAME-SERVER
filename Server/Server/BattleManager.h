#pragma once
#include <vector>
#include <reoul/functional.h>

class Client;



class BattleManager
{
public:
	BattleManager() = default;
	BattleManager(const BattleManager&) = default;
	~BattleManager() = default;

	void SetClients(std::vector<Client*>& clients);
	std::vector<int32_t> GetBattleOpponent();
	void RemoveClient(int32_t networkID);
private:
	struct BattleHistory
	{
		const_wrapper<Client*> pClient;
		size_t battleCount;
		BattleHistory(Client* client);
	};
	class BattleInfo
	{
	public:
		BattleInfo(Client* client, const std::vector<Client*>& clients);
		const Client* GetClientPtr() const;
		size_t GetBattleCount(const Client* c) const;
		void AddBattleCount(const Client* client);
		void RemoveClient(int32_t networkID);
	private:
		const_wrapper<Client*> mClient;
		std::vector<BattleHistory> mBattleInfos;
	};

	std::vector<BattleInfo> mUnMatchingClients;
	std::vector<BattleInfo> mMatchingClients;
};

inline const Client* BattleManager::BattleInfo::GetClientPtr() const
{
	return mClient;
}