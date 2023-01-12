#include "BattleAvatar.h"

#include <reoul/logger.h>

#include "Client.h"
#include "Items.h"
#include "Server.h"
#include "ServerStruct.h"
#include "Server.h"

using namespace Logger;

BattleAvatar::BattleAvatar()
	: mClient(nullptr)
	, mRoom(nullptr)
	, mNetworkID(0)
	, mMaxHp(0)
	, mHp(0)
	, mDefensive(0)
	, mOffensePower(0)
	, mAdditionDefensive(0)
	, mWeakening(0)
	, mBleeding(0)
	, mReducedHealing(0)
	, mActiveQueue{}
	, mIsGhost(false)
	, mIsFinish(false)
	, mIsCounterAttack(false)
	, mCounterAttackDamage(0)
	, mIsCounterHeal(false)
	, mCounterHeal(0)
	, mHamburgerType(EHamburgerType::Fillet)
	, mIsEffectHeal(false)
	, mEffectHeal(0)
	, mIsInstallBomb(false)
	, mInstallBombDamage(0)
	, mEffectItemCount(0)
	, mUseKeyCapCount(0)
	, mCanDefendNegativeEffect(false)
	, mIsIgnoreNextDamage(false)
	, mIsCharacterDamage(false)
	, mFirstAttackState(0)
{
	for (int i = 0; i < MAX_USING_ITEM_COUNT; ++i)
	{
		mActiveQueue[i] = i;
	}
}

void BattleAvatar::SetAvatar(Client& client, Room* room, int networkID, bool isGhost, int round)
{
	mClient = &client;
	mRoom = room;
	mNetworkID = networkID;
	mMaxHp = 50 + 10 * min(5, round);
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
	mFirstAttackState = client.GetFirstAttackState();

	const vector<Item> items = client.GetUsingItems();
	for (int i = 0; i < MAX_USING_ITEM_COUNT; ++i)
	{
		mUsingItem[i] = items[i];
		mActiveQueue[i] = i;
	}
}

uint8_t BattleAvatar::ActiveItem(int index, BattleAvatar& enemy)
{
	const Item item = mUsingItem[mActiveQueue[index]];
	++mEffectItemCount;
	sItems[item.GetType()]->Active(*this, enemy, item.GetUpgrade());

	if (mIsEffectHeal && mHp > 0)
	{
		ToHeal(mEffectHeal);
	}

	return static_cast<uint8_t>(mActiveQueue[index]);
}

void BattleAvatar::FitmentEffect()
{
	for (Item& item : mUsingItem)
	{
		sItems[item.GetType()]->FitmentEffect(*this, item.GetUpgrade());
	}
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
	mBleeding = 0;
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

void BattleAvatar::ToPiercingDamage(int damage, const BattleAvatar& opponent)
{
	if (mIsIgnoreNextDamage)
	{
		mIsIgnoreNextDamage = false;
		return;
	}

	damage += opponent.mOffensePower;
	damage = max(0, damage - opponent.mWeakening);
	mHp = max(0, mHp - damage);
}

Item BattleAvatar::GetRandomCopyItem()
{
	vector<Item> validItems;
	for (int i = 0; i < MAX_USING_ITEM_COUNT; ++i)
	{
		Item& item = mUsingItem[i];
		// �ܹ���, �ڻ��� ������, ��ĭ ����
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

void BattleAvatar::EffectBomb()
{
	if (mIsIgnoreNextDamage)
	{
		mIsIgnoreNextDamage = false;
		return;
	}

	const int oldDefensive = mDefensive;
	mDefensive = max(0, mDefensive - mInstallBombDamage);

	mInstallBombDamage = max(0, mInstallBombDamage - oldDefensive);
	mHp = max(0, mHp - mInstallBombDamage);

	mInstallBombDamage = 0;
	mIsInstallBomb = false;
}

void BattleAvatar::EffectCounter(BattleAvatar& opponent)
{
	if (mIsFinish)
	{
		return;
	}

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

void BattleAvatar::ToDamageCharacter(int damage, bool isNotFinishBattle)
{
	if (mIsGhost)
	{
		return;
	}

	if (mIsCharacterDamage)
	{
		return;
	}

	if (!isNotFinishBattle && mHp > 0)
	{
		return;
	}

	if (!IsValidBattleAvatarInRoom())
	{
		return;
	}

	mClient->ToDamage(damage);

	mIsCharacterDamage = true;
}

void BattleAvatar::ApplyBattleAvatarInfoPacket(sc_BattleAvatarInfoPacket& packet) const
{
	if (mNetworkID > 100000)
	{
		packet.networkID = mNetworkID;
	}
	else
	{
		packet.networkID = mIsGhost ? ~mNetworkID : mNetworkID;
	}

	if (IsValidBattleAvatarInRoom())
	{
		packet.playerHp = mClient->GetHp();
	}
	else
	{
		packet.playerHp = 50;
	}
	packet.maxHp = mMaxHp;
	packet.hp = mHp;
	packet.firstAttackState = mFirstAttackState;
	packet.offensePower = mOffensePower;
	packet.defensive = mDefensive;
	packet.additionDefensive = mAdditionDefensive;
	packet.weakening = mWeakening;
	packet.bleeding = mBleeding;
	packet.reducedHealing = mReducedHealing;
	packet.isEffectHeal = mIsEffectHeal;
	packet.effectHeal = mEffectHeal;
	packet.isInstallBomb = mIsInstallBomb;
	packet.installBombDamage = mInstallBombDamage;
	packet.isIgnoreNextDamage = mIsIgnoreNextDamage;
	packet.canDefendNegativeEffect = mCanDefendNegativeEffect;
	packet.isCounterAttack = mIsCounterAttack;
	packet.counterAttackDamage = mCounterAttackDamage;
	packet.isCounterHeal = mIsCounterHeal;
	packet.counterHeal = mCounterHeal;

}

int BattleAvatar::IncreaseItemTicket(EItemTicketType ticketType, int count)
{
	if (mIsGhost)
	{
		return 0;
	}

	if (!IsValidBattleAvatarInRoom())
	{
		return 0;
	}

	switch (ticketType)
	{
	case EItemTicketType::Normal:
		mClient->SetNormalItemTicketCount(mClient->GetNormalItemTicketCount() + count);
		return mClient->GetNormalItemTicketCount();
	case EItemTicketType::Advanced:
		mClient->SetAdvancedItemTicketCount(mClient->GetAdvancedItemTicketCount() + count);
		return mClient->GetAdvancedItemTicketCount();
	case EItemTicketType::Top:
		mClient->SetTopItemTicketCount(mClient->GetTopItemTicketCount() + count);
		return mClient->GetTopItemTicketCount();
	case EItemTicketType::Supreme:
		mClient->SetSupremeItemTicketCount(mClient->GetSupremeItemTicketCount() + count);
		return mClient->GetSupremeItemTicketCount();
	}
}

bool BattleAvatar::IsValidBattleAvatarInRoom() const
{
	if (mRoom == nullptr)
	{
		return false;
	}

	if (mRoom->IsValidClientInThisRoom(mClient))
	{
		return true;
	}

	return false;
}
