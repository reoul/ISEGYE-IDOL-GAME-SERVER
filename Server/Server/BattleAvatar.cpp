#include "BattleAvatar.h"

#include <reoul/logger.h>

#include "Client.h"
#include "Items.h"
#include "Server.h"
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
	mCanDefendNegativeEffect = false;
	mIsIgnoreNextDamage = false;
	mIsCharacterDamage = false;

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

	if (mIsEffectHeal)
	{
		ToHeal(mEffectHeal);
	}

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
	if (mIsIgnoreNextDamage)
	{
		mIsIgnoreNextDamage = false;
		return;
	}

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
	mCanDefendNegativeEffect = false;
	mIsIgnoreNextDamage = false;
}

Item BattleAvatar::GetRandomCopyItem()
{
	vector<Item> validItems;
	for (int i = 0; i < MAX_USING_ITEM_COUNT; ++i)
	{
		Item& item = mUsingItem[i];
		// ÇÜ¹ö°Å, ¹Ú»çÀÇ ¸¸´ÉÅø, ºóÄ­ Á¦¿Ü
		if (item.GetType() != 7 && item.GetType() != 16 && item.GetType() != 0)
		{
			validItems.emplace_back(item);
		}
	}
	Item retItem;
	if (!validItems.empty())
	{
		Random<int> gen(0, validItems.size() - 1);
		retItem = validItems[gen()];
	}
	else
	{
		Item item;
		item.SetType(16);
		item.SetUpgrade(0);
		retItem = item;
	}

	return retItem;
}

void BattleAvatar::EffectCounter(BattleAvatar& opponent)
{
	if (mIsCounterAttack)
	{
		mIsCounterAttack = false;
		opponent.ToDamage(mCounterAttackDamage, *this);
		mCounterAttackDamage = 0;
	}

	if (mIsCounterHeal)
	{
		mIsCounterHeal = false;
		ToHeal(mCounterHeal);
		mCounterHeal = 0;
	}
}

int BattleAvatar::GetDamage() const
{
	int sum = 0;
	for (Item item : mUsingItem)
	{
		if (item.GetType() != EMPTY_ITEM)
		{
			sum += static_cast<int>(sItems[item.GetType()]->TIER_TYPE) + 1;
		}
	}
	return sum;
}

void BattleAvatar::ToDamageCharacter(int damage)
{
	if (mIsGhost)
	{
		return;
	}

	if (mIsCharacterDamage)
	{
		return;
	}

	if (mHp > 0)
	{
		return;
	}


	mClient->ToDamage(damage);

	if (mClient->GetHp() == 0)
	{
		Server::SendDisconnectDelay(mNetworkID);
	}

	mIsCharacterDamage = true;
}
