#pragma once
#include <vector>

#include "Client.h"
#include "Item.h"
#include "SettingData.h"


struct SlotInfo;

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

class BattleAvatar
{
public:
	BattleAvatar() = default;
	void SetAvatar(Client& client, bool isGhost);
	uint8_t ActiveItem(int index, BattleAvatar& enemy);
	void SetActiveQueue(std::vector<SlotInfo> activeQueue);
	void ToDamage(int damage, const BattleAvatar& opponent);
	void ToDefensive(int defensive);
	void ToOffensePower(int offensePower);
	void ToAdditionDefensive(int additionDefensive);
	void ToWeakening(int weakening);
	void ToBleeding(int bleeding);
	void ToReducedHealing(int reducedHealing);
	void ToHeal(int heal);
	void InitCycle();
	int GetNetworkID() const;
	bool IsFinish() const;
	void SetFinish();
	int GetHP() const;
	int GetWeakening() const;
	int GetDefensive() const;
	void Clear();
	void ToJustDamage(int damage);
	Client* GetClient();
	void ToPiercingDamage(int damage, const BattleAvatar& opponent);
	int GetMaxHP() const;
private:
	Client* mClient;
	int mNetworkID;
	int mMaxHp;					// 최대 체력
	int mHp;					// 체력
	int mDefensive;				// 방어도
	int mOffensePower;			// 공격력
	int mAdditionDefensive;		// 추가 방어도
	int mWeakening;				// 약화
	int mBleeding;				// 출혈
	int mReducedHealing;		// 치유력 감소
	Item mUsingItem[MAX_USING_ITEM_COUNT];
	int mActiveQueue[MAX_USING_ITEM_COUNT];
	bool mIsGhost;				// 유령인지
	bool mIsFinish;				// 전투가 끝났는지
	// todo : 햄버거 타입 추가해서 아이템에서 타입 읽어와 적용하기
};

inline void BattleAvatar::ToDefensive(int defensive)
{
	mDefensive = max(0, mDefensive + defensive + mAdditionDefensive);
}

inline void BattleAvatar::ToOffensePower(int offensePower)
{
	mOffensePower = max(0, mOffensePower + offensePower);
}

inline void BattleAvatar::ToAdditionDefensive(int additionDefensive)
{
	mAdditionDefensive = max(0, mAdditionDefensive + additionDefensive);
}

inline void BattleAvatar::ToWeakening(int weakening)
{
	mWeakening = max(0, mWeakening + weakening);
}

inline void BattleAvatar::ToBleeding(int bleeding)
{
	mBleeding = max(0, mBleeding + bleeding);
}

inline void BattleAvatar::ToReducedHealing(int reducedHealing)
{
	mReducedHealing = max(0, mReducedHealing + reducedHealing);
}

inline void BattleAvatar::ToHeal(int heal)
{
	heal = max(0, heal - mReducedHealing);
	mHp += min(mMaxHp, mHp + heal);
}

inline int BattleAvatar::GetNetworkID() const
{
	return mNetworkID;
}

inline bool BattleAvatar::IsFinish() const
{
	return mIsFinish;
}

inline void BattleAvatar::SetFinish()
{
	mIsFinish = true;
}

inline int BattleAvatar::GetHP() const
{
	return mHp;
}

inline int BattleAvatar::GetWeakening() const
{
	return mWeakening;
}

inline int BattleAvatar::GetDefensive() const
{
	return mDefensive;
}

/**
 * \brief 부정효과 제거
 */
inline void BattleAvatar::Clear()
{
	mBleeding = 0;
	mWeakening = 0;
	mReducedHealing = 0;
}

inline void BattleAvatar::ToJustDamage(int damage)
{
	mHp = max(0, mHp - damage);
}

inline Client* BattleAvatar::GetClient()
{
	return mClient;
}

inline void BattleAvatar::ToPiercingDamage(int damage, const BattleAvatar& opponent)
{
	damage += opponent.mOffensePower;
	damage = max(0, damage - opponent.mWeakening);
	mHp = max(0, mHp - damage);
}

inline int BattleAvatar::GetMaxHP() const
{
	return mMaxHp;
}

