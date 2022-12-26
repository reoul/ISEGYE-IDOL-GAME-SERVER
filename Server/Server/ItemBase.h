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
	/// <summary> ������ ���� �ڵ� </summary>
	const int CODE;
	/// <summary> ������ Ƽ�� </summary>
	const EItemTierType TIER_TYPE;
	/// <summary> ������ �ߵ� Ÿ�� </summary>
	const EItemType TYPE;
};
