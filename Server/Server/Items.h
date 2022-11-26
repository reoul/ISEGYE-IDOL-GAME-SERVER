#pragma once
#include <iostream>

#include "ItemBase.h"
#include "Items.h"
#include "reoul/logger.h"

class Item001 final : public ItemBase
{
public:
	Item001() = default;

	// 발동 능력
	void Use(Client& me, Client& opponents, int grade) override
	{
		LogPrintf("x데미지");
		switch (grade)
		{
		case 0:
			break;
		case 1:
			break;
		case 2:
			break;
		}
	}
};

class Item002 final : public ItemBase
{
public:
	Item002() = default;

	// 발동 능력
	void Use(Client& me, Client& opponents, int grade) override
	{
		LogPrintf("방어도를 x만큼 획득");
		switch (grade)
		{
		case 0:
			break;
		case 1:
			break;
		case 2:
			break;
		}
	}
};

static Item001 sItem001;
static Item002 sItem002;

static ItemBase* sItems[10]{ &sItem001, &sItem002 };