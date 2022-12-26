#include "BattleAvatar.h"

#include <reoul/logger.h>

#include "Client.h"
#include "Items.h"
#include "ServerStruct.h"
using namespace Logger;

void BattleAvatar::SetAvatar(Client& client, bool isGhost)
{
	mClient = &client;
	mNetworkID = client.GetNetworkID();
	mMaxHp = 100;
	mHp = mMaxHp;
	mDefensive = 0;
	mOffensePower = 0;
	mAdditionDefensive = 0;
	mWeakening = 0;
	mBleeding = 0;
	mReducedHealing = 0;
	mIsGhost = isGhost;
	mIsFinish = false;
	mIsCounterAttack = false;
	mCounterAttackDamage = 0;
	mIsCounterHeal = false;
	mCounterHeal = 0;
	mHamburgerType = EHamburgerType::Fillet;
	mIsEffectHeal = false;
	mEffectHeal = 0;
	mIsInstallBomb = false;
	mInstallBombDamage = 0;
	mEffectItemCount = 0;
	mUseKeyCapCount = 0;
	mIsCounterRestoreDefense = false;
	mCanDefendNegativeEffect = false;

	const vector<Item> items = client.GetUsingItems();
	for (int i = 0; i < MAX_USING_ITEM_COUNT; ++i)
	{
		mUsingItem[i] = items[i];
	}
}

uint8_t BattleAvatar::ActiveItem(int index, BattleAvatar& enemy)
{
	const Item item = mUsingItem[mActiveQueue[index]];
	++mEffectItemCount;
	sItems[item.GetType()]->Active(*this, enemy, item.GetUpgrade());
	return static_cast<uint8_t>(mActiveQueue[index]);
}

void BattleAvatar::SetActiveQueue(std::vector<SlotInfo> activeQueue)
{
	log_assert(activeQueue.size() == MAX_USING_ITEM_COUNT);

	for (int i = 0; i < MAX_USING_ITEM_COUNT; ++i)
	{
		mActiveQueue[i] = activeQueue[i].index;
	}
}

void BattleAvatar::ToDamage(int damage, const BattleAvatar& opponent)
{
	damage += opponent.mOffensePower;

	damage = max(0, damage - opponent.mWeakening);

	const int oldDefensive = mDefensive;
	mDefensive = max(0, mDefensive - damage);

	damage = max(0, damage - oldDefensive);
	mHp = max(0, mHp - damage);
}

void BattleAvatar::InitCycle()
{
	mOffensePower = 0;
	mAdditionDefensive = 0;
	mWeakening = 0;
	mReducedHealing = 0;
	mIsCounterAttack = false;
	mCounterAttackDamage = 0;
	mIsCounterHeal = false;
	mCounterHeal = 0;
	mIsEffectHeal = false;
	mEffectHeal = 0;
	mIsInstallBomb = false;
	mInstallBombDamage = 0;
	mEffectItemCount = 0;
	mIsCounterRestoreDefense = false;
	mCanDefendNegativeEffect = false;
	mIsIgnoreNextDamage = false;
}

Item& BattleAvatar::GetRandomCopyItem()
{
	Random<int> gen(0, MAX_USING_ITEM_COUNT - 1);

	int index;
	do
	{
		index = gen();
	}
	while (mUsingItem[index].GetType() == 7 || mUsingItem[index].GetType() == 16 || mUsingItem[index].GetType() == 0);
	// todo : ºóÄ­ º¹»ç ¾ÈÇÏ°Ô ¼öÁ¤
	return mUsingItem[index];
}
