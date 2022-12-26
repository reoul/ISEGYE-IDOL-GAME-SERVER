﻿#pragma once
#include <vector>

#include "Client.h"
#include "Item.h"
#include "Random.h"
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
	bool IsCounterAttack() const;
	void SetCounterAttack(int counterDamage);
	int GetCounterAttackDamage() const;
	bool IsCounterHeal() const;
	void SetCounterHeal(int counterHeal);
	int GetCounterHeal() const;
	void SetHamburgerType(EHamburgerType type);
	EHamburgerType GetHamburgerType() const;
	void ToJustHeal(int heal);
	bool IsEffectHeal() const;
	void SetEffectHeal(int effectHeal);
	int GetEffectHeal() const;
	bool IsInstallBomb() const;
	void SetInstallBomb(int bombDamage);
	int GetInstallBombDamage() const;
	void SetEffectItemCount(int count);
	int GetEffectItemCount() const;
	void UseKeyCap();
	int GetUseKeyCapCount() const;
	void SetCounterRestoreDefense(bool isCounterRestoreDefense);
	bool isCounterRestoreDefense() const;
	void SetDefendNegativeEffect(bool canDefendNegativeEffect);
	bool CanDefendNegativeEffect() const;
	void SetOffensePower(int offensePower);
	void SetWeakening(int weakening);
	void SetIgnoreNextDamage(bool isIgnoreNextDamage);
	bool IsIgnoreNextDamage() const;
	Item& GetItemBySlot(int slot);
	EHamburgerType GetRandomHamburgerType() const;
	Item& GetRandomCopyItem();
private:
	Client* mClient;
	int mNetworkID;
	int mMaxHp;						// 최대 체력
	int mHp;						// 체력
	int mDefensive;					// 방어도
	int mOffensePower;				// 공격력
	int mAdditionDefensive;			// 추가 방어도
	int mWeakening;					// 약화
	int mBleeding;					// 출혈
	int mReducedHealing;			// 치유력 감소
	Item mUsingItem[MAX_USING_ITEM_COUNT];
	int mActiveQueue[MAX_USING_ITEM_COUNT];
	bool mIsGhost;					// 유령인지
	bool mIsFinish;					// 전투가 끝났는지
	bool mIsCounterAttack;			// 반격 데미지가 있는지
	int mCounterAttackDamage;		// 반격 데미지
	bool mIsCounterHeal;			// 반격 힐이 가능한지
	int mCounterHeal;				// 반격 힐
	EHamburgerType mHamburgerType;	// 햄버거 타입
	bool mIsEffectHeal;				// 유물 사용할 때 마다 회복 가능한지
	int mEffectHeal;				// 유물 사용할 때 마다 회복
	bool mIsInstallBomb;			// 언니의 마음 폭탄이 설치되어 있는지
	int mInstallBombDamage;			// 언니의 마음 폭탄 데미지
	int mEffectItemCount;			// 아이템 발동 횟수
	int mUseKeyCapCount;			// 벌레의 키캡 발동 횟수
	bool mIsCounterRestoreDefense;	// 반격 잃은 방어력 복구
	bool mCanDefendNegativeEffect;	// 부정적인 효과 방어
	bool mIsIgnoreNextDamage;		// 다음 피해 무시
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
	mIsInstallBomb = false;
	mInstallBombDamage = 0;
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


inline bool BattleAvatar::IsCounterAttack() const
{
	return mIsCounterAttack;
}

inline void BattleAvatar::SetCounterAttack(int counterDamage)
{
	mIsCounterAttack = true;
	mCounterAttackDamage += counterDamage;
}

inline int BattleAvatar::GetCounterAttackDamage() const
{
	return mCounterAttackDamage;
}

inline bool BattleAvatar::IsCounterHeal() const
{
	return mIsCounterHeal;
}

inline void BattleAvatar::SetCounterHeal(int counterHeal)
{
	mIsCounterHeal = true;
	mCounterHeal += counterHeal;
}

inline int BattleAvatar::GetCounterHeal() const
{
	return mCounterHeal;
}

inline void BattleAvatar::SetHamburgerType(EHamburgerType type)
{
	mHamburgerType = type;
}

inline EHamburgerType BattleAvatar::GetHamburgerType() const
{
	return mHamburgerType;
}

inline void BattleAvatar::ToJustHeal(int heal)
{
	mHp += min(mMaxHp, mHp + heal);
}

inline bool BattleAvatar::IsEffectHeal() const
{
	return mIsEffectHeal;
}

inline void BattleAvatar::SetEffectHeal(int heal)
{
	mIsEffectHeal = true;
	mEffectHeal += heal;
}

inline int BattleAvatar::GetEffectHeal() const
{
	return mEffectHeal;
}

inline bool BattleAvatar::IsInstallBomb() const
{
	return mIsInstallBomb;
}

inline void BattleAvatar::SetInstallBomb(int bombDamage)
{
	mIsInstallBomb = true;
	mInstallBombDamage += bombDamage;
}

inline int BattleAvatar::GetInstallBombDamage() const
{
	return mInstallBombDamage;
}

inline void BattleAvatar::SetEffectItemCount(int count)
{
	mEffectItemCount = count;
}

inline int BattleAvatar::GetEffectItemCount() const
{
	return mEffectItemCount;
}

inline void BattleAvatar::UseKeyCap()
{
	++mUseKeyCapCount;
}

inline int BattleAvatar::GetUseKeyCapCount() const
{
	return mUseKeyCapCount;
}

inline void BattleAvatar::SetCounterRestoreDefense(bool isCounterRestoreDefense)
{
	mIsCounterRestoreDefense = isCounterRestoreDefense;
}

inline bool BattleAvatar::isCounterRestoreDefense() const
{
	return mIsCounterRestoreDefense;
}

inline void BattleAvatar::SetDefendNegativeEffect(bool canDefendNegativeEffect)
{
	mCanDefendNegativeEffect = canDefendNegativeEffect;
}

inline bool BattleAvatar::CanDefendNegativeEffect() const
{
	return mCanDefendNegativeEffect;
}

inline void BattleAvatar::SetOffensePower(int offensePower)
{
	mOffensePower = offensePower;
}

inline void BattleAvatar::SetWeakening(int weakening)
{
	mWeakening = weakening;
}

inline void BattleAvatar::SetIgnoreNextDamage(bool isIgnoreNextDamage)
{
	mIsIgnoreNextDamage = isIgnoreNextDamage;
}

inline bool BattleAvatar::IsIgnoreNextDamage() const
{
	return mIsIgnoreNextDamage;
}

inline Item& BattleAvatar::GetItemBySlot(int slot)
{
	return mUsingItem[slot];
}

inline EHamburgerType BattleAvatar::GetRandomHamburgerType() const
{
	Random<int> gen(static_cast<int>(EHamburgerType::Fillet), static_cast<int>(EHamburgerType::Rice));
	return static_cast<EHamburgerType>(gen());
}

