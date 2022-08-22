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
	void		SetActivePercent(uint8_t);
private:
	uint8_t		mType;
	uint8_t		mActivePercent;
};

inline uint8_t Item::GetType() const
{
	return mType;
}

inline uint8_t Item::GetActivePercent() const
{
	return mActivePercent;
}
