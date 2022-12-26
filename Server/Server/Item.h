#pragma once
#include <cstdint>

constexpr int EMPTY_ITEM = (0);
constexpr int ACTIVATE_ITEM = (1);
constexpr int DISABLE_ITEM = (0);

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
	uint8_t		GetSlot() const;
	void		SetSlot(uint8_t slot);
private:
	uint8_t		mType;
	uint8_t		mActivePercent;
	uint8_t		mUpgrade;
	uint8_t		mSlot;
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

inline uint8_t Item::GetSlot() const
{
	return mSlot;
}

inline void Item::SetSlot(uint8_t slot)
{
	mSlot = slot;
}
