#include "Client.h"

Client::Client()
	: mSocket(NULL)
	, mNetworkID(0)
	, mRecvOver()
	, mPrevSize(0)
	, mPacketBuf{}
	, mIsAlive(false)
	, mStatus(ST_FREE)
	, mCharacterType(CharacterType::Woowakgood)
	, mRoom(nullptr)
	, mUsingItems{}
	, mUnUsingItems{}
{
	mName[0] = '\0';
}

vector<Item> Client::GetUsingItems() const
{
	vector<Item> items;
	for (const Item& item : mUsingItems)
	{
		items.emplace_back(item);
	}
	return items;
}

vector<Item> Client::GetValidUsingItems() const
{
	vector<Item> items;
	for (const Item& item : mUsingItems)
	{
		if (item.GetType() != EMPTY_ITEM && item.GetType() != LOCK_ITEM)
		{
			items.emplace_back(item);
		}
	}
	return items;
}

vector<Item> Client::GetUnUsingItems() const
{
	vector<Item> items;
	for (const Item& item : mUnUsingItems)
	{
		items.emplace_back(item);
	}
	return items;
}

vector<Item> Client::GetValidUnUsingItems() const
{
	vector<Item> items;
	for (const Item& item : mUnUsingItems)
	{
		if (item.GetType() != EMPTY_ITEM && item.GetType() != LOCK_ITEM)
		{
			items.emplace_back(item);
		}
	}
	return items;
}
