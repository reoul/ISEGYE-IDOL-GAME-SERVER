#pragma once
#include "ItemBase.h"
#include "Items.h"
#include "reoul/logger.h"
#include "Client.h"

using namespace Logger;

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
		LogWarning("log", "["#name"] 아이템 강화 수치가 {0}입니다", upgrade);			\
		break;																			\
	}																					\
	}																					\
};																						\
static name s##name;																	\

// 낡은 채찍 : x 데미지
ITEM(Item001,
	opponents.ToDamageAvatar(5);
,
	opponents.ToDamageAvatar(7);
,
	opponents.ToDamageAvatar(10);
)

// 오베인 게르크 티 : 방어도를 X만큼 획득
ITEM(Item002,
	me.SetAvatarDefensive(me.GetAvatarDefensive() + 3);
,
	me.SetAvatarDefensive(me.GetAvatarDefensive() + 5);
,
	me.SetAvatarDefensive(me.GetAvatarDefensive() + 7);
)

// 귀상어두 가면 : 상대방 약화
ITEM(Item003,
	LogPrintf("log", "Item003 1성 효과");
,
LogPrintf("log", "Item003 2성 효과");
,
LogPrintf("log", "Item003 3성 효과");
)

// 왁엔터 사장 명패 : 
ITEM(Item004,
	LogPrintf("log", "Item004 1성 효과");
,
LogPrintf("log", "Item004 2성 효과");
,
LogPrintf("log", "Item004 3성 효과");
)

// 가지치기용 도끼 : 본인 방어력 비례 데미지 기본 데미지 + 방어력 * x
ITEM(Item005,
	LogPrintf("log", "Item005 1성 효과");
,
LogPrintf("log", "Item005 2성 효과");
,
LogPrintf("log", "Item005 3성 효과");
)

// 홍삼스틱
ITEM(Item006,
	LogPrintf("log", "Item006 1성 효과");
,
LogPrintf("log", "Item006 2성 효과");
,
LogPrintf("log", "Item006 3성 효과");
)

// 햄버거
ITEM(Item007,
	LogPrintf("log", "Item007 1성 효과");
,
	LogPrintf("log", "Item007 2성 효과");
,
	LogPrintf("log", "Item007 3성 효과");
)

static ItemBase* sItems[]{ &sItem001, &sItem002, &sItem003, &sItem004, &sItem005, &sItem006, &sItem007 };