#pragma once
#include <cstdint>

#define LOCK_ITEM (254)
#define EMPTY_ITEM (255)
#define ACTIVATE_ITEM (1)
#define DISABLE_ITEM (0)

class Item
{
public:
	Item();
	Item(const Item&) = default;
	~Item() = default;

	uint8_t		GetType() const;
	void		SetType(uint8_t type);
	uint8_t		GetActivePercent() const;
	void		SetActivePercent(uint8_t percent);
	uint8_t		GetUpgrade() const;
	void		SetUpgrade(uint8_t upgrade);
private:
	uint8_t		mType;
	uint8_t		mActivePercent;
	uint8_t		mUpgrade;
};

inline uint8_t Item::GetType() const
{
	return mType;
}

inline void Item::SetType(uint8_t type)
{
	mType = type;
}

inline uint8_t Item::GetActivePercent() const
{
	return mActivePercent;
}

inline void Item::SetActivePercent(uint8_t percent)
{
	mActivePercent = percent;
}

inline uint8_t Item::GetUpgrade() const
{
	return mUpgrade;
}

inline void Item::SetUpgrade(uint8_t upgrade)
{
	mUpgrade = upgrade;
}
