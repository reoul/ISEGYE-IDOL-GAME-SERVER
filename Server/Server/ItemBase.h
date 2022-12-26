#pragma once

enum class EItemTierType { One, Two, Three, Four, Five };
enum class EItemType { Attack, Defense, Effect, Heal, Empty };

class BattleAvatar;

class ItemBase
{
public:
	ItemBase(int code, EItemTierType tierType, EItemType type)
		: CODE(code)
		, TIER_TYPE(tierType)
		, TYPE(type)
	{
	}
	virtual ~ItemBase() = default;
	virtual void Active(BattleAvatar& me, BattleAvatar& opponents, int upgrade) = 0;
	virtual void FitmentEffect(BattleAvatar& me) = 0;
public:
	/// <summary> 아이템 고유 코드 </summary>
	const int CODE;
	/// <summary> 아이템 티어 </summary>
	const EItemTierType TIER_TYPE;
	/// <summary> 아이템 발동 타입 </summary>
	const EItemType TYPE;
};
