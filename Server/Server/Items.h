#pragma once
#include "ItemBase.h"
#include "Items.h"
#include "SettingData.h"
#include "reoul/logger.h"

/**
 * \brief 아이템 생성 매크로
 * \param name 아이템 클래스 이름
 * \param oneStarUseEffect 1성 아이템 효과
 * \param twoStarUseEffect 2성 아이템 효과
 * \param threeStarUseEffect 3성 아이템 효과
 */
#define ITEM(name, oneStarUseEffect, twoStarUseEffect, threeStarUseEffect)				\
class name final : public ItemBase														\
{																						\
public:																					\
	name() = default;																	\
	void Use(Client& me, Client& opponents, int upgrade) override						\
	{																					\
	switch (upgrade)																	\
	{																					\
	case 0:																				\
		oneStarUseEffect																\
		break;																			\
	case 1:																				\
		twoStarUseEffect																\
		break;																			\
	case 2:																				\
		threeStarUseEffect																\
		break;																			\
	default:																			\
		LogWarning("["#name"] 아이템 강화 수치가 {0}입니다", upgrade);					\
		break;																			\
	}																					\
	}																					\
};																						\
static name s##name;																	\

ITEM(Item001,
	LogPrintf("Item001 1성 효과");
,
LogPrintf("Item001 2성 효과");
,
LogPrintf("Item001 3성 효과");
)

ITEM(Item002,
	LogPrintf("Item002 1성 효과");
,
LogPrintf("Item002 2성 효과");
,
LogPrintf("Item002 3성 효과");
)

ITEM(Item003,
	LogPrintf("Item003 1성 효과");
,
LogPrintf("Item003 2성 효과");
,
LogPrintf("Item003 3성 효과");
)

ITEM(Item004,
	LogPrintf("Item004 1성 효과");
,
LogPrintf("Item004 2성 효과");
,
LogPrintf("Item004 3성 효과");
)

ITEM(Item005,
	LogPrintf("Item005 1성 효과");
,
LogPrintf("Item005 2성 효과");
,
LogPrintf("Item005 3성 효과");
)

ITEM(Item006,
	LogPrintf("Item006 1성 효과");
,
LogPrintf("Item006 2성 효과");
,
LogPrintf("Item006 3성 효과");
)

ITEM(Item007,
	LogPrintf("Item007 1성 효과");
,
	LogPrintf("Item007 2성 효과");
,
	LogPrintf("Item007 3성 효과");
)

static ItemBase* sItems[]{ &sItem001, &sItem002, &sItem003, &sItem004, &sItem005, &sItem006, &sItem007 };