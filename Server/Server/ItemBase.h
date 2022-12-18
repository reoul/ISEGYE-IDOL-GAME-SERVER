#pragma once

enum class EItemTierType { One, Two, Three, Four, Five };

class BattleAvatar;

class ItemBase
{
public:
	ItemBase(int code, EItemTierType tierType)
		: CODE(code)
		, TIER_TYPE(tierType)
	{
	}
	virtual ~ItemBase() = default;
	virtual void Active(BattleAvatar& me, BattleAvatar& opponents, int upgrade) = 0;
	virtual void FitmentEffect(BattleAvatar& me) = 0;
public:
	const int CODE;
	const EItemTierType TIER_TYPE;
};
