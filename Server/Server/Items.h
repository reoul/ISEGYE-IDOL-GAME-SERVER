﻿#pragma once
#include <vector>

#include "ItemBase.h"
#include "reoul/logger.h"
#include "BattleAvatar.h"
#include "Random.h"
#include "PacketStruct.h"

using namespace std;
using namespace Logger;

/**
 * \brief 아이템 생성 매크로
 * \param name 아이템 클래스 이름
 * \param oneStarUseEffect 1성 아이템 효과
 * \param twoStarUseEffect 2성 아이템 효과
 * \param threeStarUseEffect 3성 아이템 효과
 */
#define ITEM(name, code, tier, type, oneStarUseEffect, twoStarUseEffect, threeStarUseEffect, Passive)	\
class name final : public ItemBase																		\
{																										\
public:																									\
	name() : ItemBase(code, tier, type)																	\
	{																									\
	}																									\
																										\
	void Active(BattleAvatar& me, BattleAvatar& opponents, int upgrade) override						\
	{																									\
		switch (upgrade)																				\
		{																								\
		case 0:																							\
			oneStarUseEffect																			\
			break;																						\
		case 1:																							\
			twoStarUseEffect																			\
			break;																						\
		case 2:																							\
			threeStarUseEffect																			\
			break;																						\
		default:																						\
			LogWarning("log", "["#name"] 아이템 강화 수치가 {0}입니다", upgrade);						\
		}																								\
	}																									\
																										\
	void FitmentEffect(BattleAvatar& me, int upgrade) override											\
	{																									\
		Passive																							\
	}																									\
};																										\
static name s##name;																					\

 // 빈칸 : 아무 효과 없음
ITEM(ItemEmpty, 0, EItemTierType::One, EItemType::Empty,
	{ int a = 0; }
	,
	{ int b = 0; }
	,
	{ int c = 0; }
	,
	{ int fitmentEffectEmpty = 0; }
)

// 낡은 채찍 : x 데미지
ITEM(Item001, 1, EItemTierType::One, EItemType::Attack,
	opponents.ToDamage(5, me);
,
opponents.ToDamage(8, me);
,
opponents.ToDamage(12, me);
,
{ int fitmentEffectEmpty = 0; }
)

// 오베인 게르크 티 : 방어도를 X만큼 획득
ITEM(Item002, 2, EItemTierType::One, EItemType::Defense,
	me.ToDefensive(3);
,
me.ToDefensive(5);
,
me.ToDefensive(7);
,
{ int fitmentEffectEmpty = 0; }
)

// 귀상어두 가면 : 상대방 약화
ITEM(Item003, 3, EItemTierType::One, EItemType::Effect,
	opponents.ToWeakening(1);
,
opponents.ToWeakening(2);
,
opponents.ToWeakening(3);
,
{ int fitmentEffectEmpty = 0; }
)

// 왁엔터 사장 명패 : X 데미지를 두번 줍니다. (여러번 공격)
ITEM(Item004, 4, EItemTierType::One, EItemType::Attack,
	opponents.ToDamage(3, me);
opponents.ToDamage(3, me);
,
opponents.ToDamage(4, me);
opponents.ToDamage(4, me);
,
opponents.ToDamage(6, me);
opponents.ToDamage(6, me);
,
{ int fitmentEffectEmpty = 0; }
)

// 가지치기용 도끼 : 본인 방어력 비례 데미지 기본 데미지 + 방어력 * x
ITEM(Item005, 5, EItemTierType::Four, EItemType::Attack,
	opponents.ToDamage(5 + me.GetDefensive() * 3, me);
,
opponents.ToDamage(5 + me.GetDefensive() * 4, me);
,
opponents.ToDamage(5 + me.GetDefensive() * 6, me);
,
{ int fitmentEffectEmpty = 0; }
)

// 홍삼스틱 : 부정 효과 제거
ITEM(Item006, 6, EItemTierType::Three, EItemType::Heal,
	me.Clear();
,
me.Clear();
,
me.Clear();
,
me.SetMaxHP(me.GetMaxHP() + 5 + 5 * upgrade);
)

// 햄버거
// 휠렛버거 : 효과
// 기네스버거: 공격
// 화이트 갈릭 버거 : 회복
// 밥버거 : 방어
// 만약 아이템 번호를 바꾸면 햄버거 체크하는 부분 번호 수정해야함
ITEM(Item007, 7, EItemTierType::Two, EItemType::Effect,
	switch (me.GetHamburgerType())
	{
	case EHamburgerType::Fillet:
		opponents.ToWeakening(2);
		break;
	case EHamburgerType::Guinness:
		opponents.ToDamage(7, me);
		break;
	case EHamburgerType::WhiteGarlic:
		me.ToHeal(4);
		break;
	case EHamburgerType::Rice:
		me.ToDefensive(4);
		break;
	}
,
switch (me.GetHamburgerType())
{
case EHamburgerType::Fillet:
	opponents.ToWeakening(3);
	break;
case EHamburgerType::Guinness:
	opponents.ToDamage(9, me);
	break;
case EHamburgerType::WhiteGarlic:
	me.ToHeal(6);
	break;
case EHamburgerType::Rice:
	me.ToDefensive(6);
	break;
}
,
switch (me.GetHamburgerType())
{
case EHamburgerType::Fillet:
	opponents.ToWeakening(4);
	break;
case EHamburgerType::Guinness:
	opponents.ToDamage(12, me);
	break;
case EHamburgerType::WhiteGarlic:
	me.ToHeal(9);
	break;
case EHamburgerType::Rice:
	me.ToDefensive(9);
	break;
}
,
me.ToDefensive(3 + 3 * upgrade);
)

// 악질안경 : 공격력 상승
ITEM(Item008, 8, EItemTierType::One, EItemType::Effect,
	me.ToOffensePower(2);
,
me.ToOffensePower(3);
,
me.ToOffensePower(5);
,
{ int fitmentEffectEmpty = 0; }
)

// 전투 메이드복 : 방어도 + 5, 장착 후 공격 당할시 1회성 반격 X 데미지
ITEM(Item009, 9, EItemTierType::Two, EItemType::Defense,
	me.ToDefensive(4);
me.SetCounterAttack(5);
,
me.ToDefensive(4);
me.SetCounterAttack(7);
,
me.ToDefensive(4);
me.SetCounterAttack(10);
,
{ int fitmentEffectEmpty = 0; }
)

// 수녀복 : 방어도 + 5, 장착 후 공격 당할시 1회성 회복 X
ITEM(Item010, 10, EItemTierType::Two, EItemType::Defense,
	me.ToDefensive(4);
me.SetCounterHeal(6);
,
me.ToDefensive(4);
me.SetCounterHeal(8);
,
me.ToDefensive(4);
me.SetCounterHeal(11);
,
{ int fitmentEffectEmpty = 0; }
)

// 매우 큰 리본 : 추가방어력 X 제공
ITEM(Item011, 11, EItemTierType::Three, EItemType::Effect,
	me.ToAdditionDefensive(2);
,
me.ToAdditionDefensive(4);
,
me.ToAdditionDefensive(6);
,
{ int fitmentEffectEmpty = 0; }
)

// 슈크림 붕어빵 : 자신의 유물을 사용할 때 마다 체력 회복, 사이클 끝나면 효과 끝
ITEM(Item012, 12, EItemTierType::Two, EItemType::Heal,
	me.SetEffectHeal(1);
,
me.SetEffectHeal(3);
,
me.SetEffectHeal(4);
,
{ int fitmentEffectEmpty = 0; }
)

// 언니의 마음 : 사이클이 종료되면 폭발하는 폭탄을 설치, 폭발시 X 데미지
ITEM(Item013, 13, EItemTierType::Four, EItemType::Attack,
	opponents.SetInstallBomb(15);
,
opponents.SetInstallBomb(20);
,
opponents.SetInstallBomb(30);
,
{ int fitmentEffectEmpty = 0; }
)

// 좋은 거 : 체력 X만큼 깎고 3X만큼 방어도
ITEM(Item014, 14, EItemTierType::Two, EItemType::Defense,
	me.ToJustDamage(3);
me.ToDefensive(3 * 3);
,
me.ToJustDamage(4);
me.ToDefensive(3 * 4);
,
me.ToJustDamage(5);
me.ToDefensive(3 * 5);
,
{ int fitmentEffectEmpty = 0; }
)

// 다이아 검 : 이번 라운드에 아이템을 사용한 횟수(다이아검 포함) 만큼 X 데미지 (여러번 공격)
ITEM(Item015, 15, EItemTierType::Five, EItemType::Attack,
	{
		const int effectItemCount = me.GetEffectItemCount();
		for (int i = 0; i < effectItemCount; ++i)
		{
			opponents.ToDamage(10, me);
		}
	}
	,
{
	const int effectItemCount = me.GetEffectItemCount();
	for (int i = 0; i < effectItemCount; ++i)
	{
		opponents.ToDamage(20, me);
	}
}
,
{
	const int effectItemCount = me.GetEffectItemCount();
	for (int i = 0; i < effectItemCount; ++i)
	{
		opponents.ToDamage(30, me);
	}
}
,
me.SetFirstAttackState(me.GetFirstAttackState() + 2 + upgrade);
)

// 박사의 만능툴
// 번호를 바꿀시 아이템 체크하는 부분 번호 수정해야함
ITEM(Item016, 16, EItemTierType::Two, EItemType::Effect,
	opponents.ToDamage(5, me);
,
opponents.ToDamage(5, me);
,
opponents.ToDamage(5, me);
,
me.SetMaxHP(me.GetMaxHP() + 5 + 5 * upgrade);
)

// 여고생의 헤어롤 : 뽑기권 * X의 데미지를 줍니다
ITEM(Item017, 17, EItemTierType::Four, EItemType::Effect,
	{
	const Client * client = me.GetClient();
const int sumTicket = client->GetNormalItemTicketCount() + client->GetAdvancedItemTicketCount()
						+ client->GetTopItemTicketCount() + client->GetSupremeItemTicketCount();
opponents.ToDamage(sumTicket * 0.5f, me);
	}
	,
{
	const Client * client = me.GetClient();
const int sumTicket = client->GetNormalItemTicketCount() + client->GetAdvancedItemTicketCount()
						+ client->GetTopItemTicketCount() + client->GetSupremeItemTicketCount();
opponents.ToDamage(sumTicket, me);
}
,
{
	const Client * client = me.GetClient();
const int sumTicket = client->GetNormalItemTicketCount() + client->GetAdvancedItemTicketCount()
						+ client->GetTopItemTicketCount() + client->GetSupremeItemTicketCount();
opponents.ToDamage(sumTicket * 2, me);
}
,
{ int fitmentEffectEmpty = 0; }
)

// 광대의 권총 : X 데미지 (상대방에게 방어력이 있을 시 2배 데미지)
ITEM(Item018, 18, EItemTierType::Four, EItemType::Attack,
	opponents.ToDamage(10 * (opponents.GetDefensive() == 0 ? 1 : 2), me);
,
opponents.ToDamage(15 * (opponents.GetDefensive() == 0 ? 1 : 2), me);
,
opponents.ToDamage(20 * (opponents.GetDefensive() == 0 ? 1 : 2), me);
,
{ int fitmentEffectEmpty = 0; }
)

// 보컬로이드의 전기충격기 : 적 약화 1부여 + X 데미지
ITEM(Item019, 19, EItemTierType::Two, EItemType::Attack,
	opponents.ToWeakening(1);
opponents.ToDamage(6, me);
,
opponents.ToWeakening(1);
opponents.ToDamage(9, me);
,
opponents.ToWeakening(1);
opponents.ToDamage(13, me);
,
{ int fitmentEffectEmpty = 0; }
)

// 부정적인 드롭스 : 상대방에게 X+1, 나에게 X만큼 약화
ITEM(Item020, 20, EItemTierType::Two, EItemType::Effect,
	opponents.ToWeakening(2);
,
opponents.ToWeakening(3);
me.ToWeakening(1);
,
opponents.ToWeakening(4);
me.ToWeakening(2);
,
{ int fitmentEffectEmpty = 0; }
)

// 비밀스런 마법봉 : 상대에게 데미지 주거나 나의 체력 회복 (50퍼 확률)
ITEM(Item021, 21, EItemTierType::Two, EItemType::Attack,
	{
		Random<int> gen(0,1);
const bool isDamage = gen() == 1 ? true : false;

const Client* cl = me.GetClient();
sc_MagicStickInfoPacket packet(cl->GetNetworkID(), isDamage);
cl->SendPacketInAllRoomClients(&packet);

if (isDamage)
{
	opponents.ToDamage(6, me);
}
else
{
	me.ToHeal(7);
}
	}
	,
{
	Random<int> gen(0,1);
const bool isDamage = gen() == 1 ? true : false;

const Client* cl = me.GetClient();
sc_MagicStickInfoPacket packet(cl->GetNetworkID(), isDamage);
cl->SendPacketInAllRoomClients(&packet);

	if (gen() == 0)
	{
		opponents.ToDamage(9, me);
	}
	else
	{
		me.ToHeal(10);
	}
}
,
{
	Random<int> gen(0,1);
const bool isDamage = gen() == 1 ? true : false;

const Client* cl = me.GetClient();
sc_MagicStickInfoPacket packet(cl->GetNetworkID(), isDamage);
cl->SendPacketInAllRoomClients(&packet);

if (gen() == 0)
{
	opponents.ToDamage(13, me);
}
else
{
	me.ToHeal(14);
}
}
,
{ int fitmentEffectEmpty = 0; }
)

// 귀족의 상현딸 : 매턴 적 출혈 2뎀 + X 데미지
ITEM(Item022, 22, EItemTierType::Two, EItemType::Attack,
	opponents.ToBleeding(2);
opponents.ToDamage(3, me);
,
opponents.ToBleeding(2);
opponents.ToDamage(6, me);
,
opponents.ToBleeding(2);
opponents.ToDamage(11, me);
,
{ int fitmentEffectEmpty = 0; }
)

// 대학원생의 USB : 상대방에게 걸린 공격력 상승과 내게 걸린 약화를 지움
ITEM(Item023, 23, EItemTierType::Three, EItemType::Effect,
	opponents.SetOffensePower(0);
me.SetWeakening(0);
,
opponents.SetOffensePower(0);
me.SetWeakening(0);
,
opponents.SetOffensePower(0);
me.SetWeakening(0);
,
me.SetFirstAttackState(me.GetFirstAttackState() + upgrade + 1);
)

// 선량한 시민의 빠루
ITEM(Item024, 24, EItemTierType::Three, EItemType::Attack,
	{
	const int percentHp = opponents.GetMaxHP() / 5;
	const int damage = (opponents.GetMaxHP() - opponents.GetHP()) / percentHp;
		opponents.ToDamage(7 + damage, me);
	}
	,
{
	const int percentHp = opponents.GetMaxHP() / 5;
	const int damage = (opponents.GetMaxHP() - opponents.GetHP()) / percentHp;
	opponents.ToDamage(11 + damage, me);
}
,
{
	const int percentHp = opponents.GetMaxHP() / 5;
	const int damage = (opponents.GetMaxHP() - opponents.GetHP()) / percentHp;
	opponents.ToDamage(15 + damage, me);
}
,
{ int fitmentEffectEmpty = 0; }
)

// 노인의 틀니 : 발동 순서에 따라 공격력 버프
ITEM(Item025, 25, EItemTierType::Four, EItemType::Effect,
	if (me.GetEffectItemCount() < 4)
	{
		me.ToOffensePower(3);
	}
	else
	{
		me.ToOffensePower(5);
	}
,
if (me.GetEffectItemCount() < 4)
{
	me.ToOffensePower(4);
}
else
{
	me.ToOffensePower(6);
}
,
if (me.GetEffectItemCount() < 4)
{
	me.ToOffensePower(5);
}
else
{
	me.ToOffensePower(7);
}
,
{ int fitmentEffectEmpty = 0; }
)

// 마법사의 스테프 : 5 데미지 + 적 치유력 감소 X
ITEM(Item026, 26, EItemTierType::Two, EItemType::Attack,
	opponents.ToDamage(10, me);
opponents.ToReducedHealing(2);
,
opponents.ToDamage(10, me);
opponents.ToReducedHealing(4);
,
opponents.ToDamage(10, me);
opponents.ToReducedHealing(6);
,
{ int fitmentEffectEmpty = 0; }
)

// 바텐더의 나비넥타이 : 방어도 + X, 현재 방어력이 2배가 됨
ITEM(Item027, 27, EItemTierType::Five, EItemType::Defense,
	me.ToDefensive(3);
me.ToDefensive(me.GetDefensive());
,
me.ToDefensive(5);
me.ToDefensive(me.GetDefensive());
,
me.ToDefensive(10);
me.ToDefensive(me.GetDefensive());
,
me.ToDefensive(5 + 5 * upgrade);
)

// 오니의 카타나 : 매턴 적 출혈 4뎀 + X 데미지
ITEM(Item028, 28, EItemTierType::Three, EItemType::Attack,
	opponents.ToBleeding(2);
opponents.ToDamage(6, me);
,
opponents.ToBleeding(3);
opponents.ToDamage(6, me);
,
opponents.ToBleeding(5);
opponents.ToDamage(6, me);
,
{ int fitmentEffectEmpty = 0; }
)

// 심리상담사의 자격증 : X 확률로 일반 뽑기권을 생성합니다
ITEM(Item029, 29, EItemTierType::Three, EItemType::Effect,
	{
		Random<int> gen(0,99);
		if (gen() < 30)
		{
			if (!me.IsValidBattleAvatarInRoom())
			{
				return;
			}

			const Client* client = me.GetClient();
			const int ticketCount = client->GetNormalItemTicketCount() + 1;
			me.GetClient()->SetNormalItemTicketCount(ticketCount);
			sc_SetItemTicketPacket packet(client->GetNetworkID(), EItemTicketType::Normal, ticketCount);
			client->SendPacketInAllRoomClients(&packet);
		}
	}
	,
{
	Random<int> gen(0,99);
	if (gen() < 50)
	{
		if (!me.IsValidBattleAvatarInRoom())
		{
			return;
		}

		const Client* client = me.GetClient();
		const int ticketCount = client->GetNormalItemTicketCount() + 1;
		me.GetClient()->SetNormalItemTicketCount(ticketCount);
		sc_SetItemTicketPacket packet(client->GetNetworkID(), EItemTicketType::Normal, ticketCount);
		client->SendPacketInAllRoomClients(&packet);
	}
}
,
{
	if (!me.IsValidBattleAvatarInRoom())
	{
		return;
	}

	const Client* client = me.GetClient();
	const int ticketCount = client->GetNormalItemTicketCount() + 1;
	me.GetClient()->SetNormalItemTicketCount(ticketCount);
	sc_SetItemTicketPacket packet(client->GetNetworkID(), EItemTicketType::Normal, ticketCount);
	client->SendPacketInAllRoomClients(&packet);
}
,
{ int fitmentEffectEmpty = 0; }
)

// 알바생의 빗자루 : 다음 피해를 1회 무시함
ITEM(Item030, 30, EItemTierType::Two, EItemType::Defense,
	me.SetIgnoreNextDamage(true);
,
me.SetIgnoreNextDamage(true);
,
me.SetIgnoreNextDamage(true);
,
me.ToDefensive(3 + 3 * upgrade);
)

// 관심병사의 K2 : 방어력 관통 X 데미지
ITEM(Item031, 31, EItemTierType::Two, EItemType::Attack,
	opponents.ToPiercingDamage(6, me);
,
opponents.ToPiercingDamage(9, me);
,
opponents.ToPiercingDamage(13, me);
,
{ int fitmentEffectEmpty = 0; }
)

// 철학도의 칫솔 : 잃은 체력의 X % 회복
ITEM(Item032, 32, EItemTierType::Five, EItemType::Heal,
	{
		int lostHp = me.GetMaxHP() - me.GetHP();
	lostHp *= 0.25f;
	me.ToHeal(lostHp);
	}
	,
{
	int lostHp = me.GetMaxHP() - me.GetHP();
	lostHp *= 0.4f;
	me.ToHeal(lostHp);
}
,
{
	int lostHp = me.GetMaxHP() - me.GetHP();
	lostHp *= 0.5f;
	me.ToHeal(lostHp);
}
,
switch (upgrade)
{
case 0:
	me.SetMaxHP(me.GetMaxHP() + 7);
	break;
case 1:
	me.SetMaxHP(me.GetMaxHP() + 12);
	break;
case 2:
	me.SetMaxHP(me.GetMaxHP() + 20);
	break;
}

)

// 잼민이의 돈까스 : 체력 회복
ITEM(Item033, 33, EItemTierType::One, EItemType::Heal,
	me.ToHeal(3);
,
me.ToHeal(5);
,
me.ToHeal(7);
,
{ int fitmentEffectEmpty = 0; }
)

// 벌레의 키캡 : 이 아이템을 사용한 횟수만큼 X 데미지로 공격 (하나의 전투 한정)
ITEM(Item034, 34, EItemTierType::Three, EItemType::Effect,
	me.UseKeyCap();
for (int i = 0; i < me.GetUseKeyCapCount(); ++i)
{
	opponents.ToDamage(3, me);
}
,
me.UseKeyCap();
for (int i = 0; i < me.GetUseKeyCapCount(); ++i)
{
	opponents.ToDamage(4, me);
}
,
me.UseKeyCap();
for (int i = 0; i < me.GetUseKeyCapCount(); ++i)
{
	opponents.ToDamage(6, me);
}
,
{ int fitmentEffectEmpty = 0; }
)

// 500원짜리 고철 : 방어도를 X 얻습니다. 5 // 10 // 15
ITEM(Item035, 35, EItemTierType::Three, EItemType::Defense,
	me.ToDefensive(5);
,
me.ToDefensive(9);
,
me.ToDefensive(13);
,
{ int fitmentEffectEmpty = 0; }
)

// 탐험가의 벨트 : 발동 순서에 따라서 방어도를 얻습니다.
ITEM(Item036, 36, EItemTierType::Three, EItemType::Defense,
	if (me.GetEffectItemCount() < 4)
	{
		me.ToDefensive(4);
	}
	else
	{
		me.ToDefensive(7);
	}
,
if (me.GetEffectItemCount() < 4)
{
	me.ToDefensive(4);
}
else
{
	me.ToDefensive(9);
}
,
if (me.GetEffectItemCount() < 4)
{
	me.ToDefensive(4);
}
else
{
	me.ToDefensive(11);
}
,
me.SetMaxHP(me.GetMaxHP() + 5 + 5 * upgrade);
)


// 히어로의 망토 : 부정적인 효과를 1회 방어
ITEM(Item037, 37, EItemTierType::Two, EItemType::Effect,
	me.SetDefendNegativeEffect(true);
,
me.SetDefendNegativeEffect(true);
,
me.SetDefendNegativeEffect(true);
,
me.ToDefensive(3 + 3 * upgrade);
)

// 대학원생의 분노 : 데미지 10
ITEM(Item038, 38, EItemTierType::One, EItemType::Attack,
	opponents.ToDamage(1, me);
,
opponents.ToDamage(1, me);
,
opponents.ToDamage(1, me);
,
{ int fitmentEffectEmpty = 0; }
)

// 군인의 분노 : 데미지 10
ITEM(Item039, 39, EItemTierType::One, EItemType::Attack,
	opponents.ToDamage(2, me);
,
opponents.ToDamage(2, me);
,
opponents.ToDamage(2, me);
,
{ int fitmentEffectEmpty = 0; }
)

// 거지의 분노 : 데미지 10
ITEM(Item040, 40, EItemTierType::One, EItemType::Attack,
	opponents.ToDamage(3, me);
,
opponents.ToDamage(3, me);
,
opponents.ToDamage(3, me);
,
{ int fitmentEffectEmpty = 0; }
)

// 충전 : 자힐 10, 데미지 5
ITEM(Item041, 41, EItemTierType::Two, EItemType::Attack,
	opponents.ToDamage(2, me);
me.ToHeal(3);
,
opponents.ToDamage(2, me);
me.ToHeal(3);
,
opponents.ToDamage(2, me);
me.ToHeal(3);
,
{ int fitmentEffectEmpty = 0; }
)

// 예 : 데미지 15
ITEM(Item042, 42, EItemTierType::Two, EItemType::Attack,
	opponents.ToDamage(10, me);
,
opponents.ToDamage(10, me);
,
opponents.ToDamage(10, me);
,
{ int fitmentEffectEmpty = 0; }
)

// 아니요 : 방어도 10
ITEM(Item043, 43, EItemTierType::Two, EItemType::Defense,
	me.ToDefensive(8);
,
me.ToDefensive(8);
,
me.ToDefensive(8);
,
{ int fitmentEffectEmpty = 0; }
)

// 프레스기 떨구기 : 데미지 30
ITEM(Item044, 44, EItemTierType::Two, EItemType::Attack,
	opponents.ToDamage(20, me);
,
opponents.ToDamage(20, me);
,
opponents.ToDamage(20, me);
,
{ int fitmentEffectEmpty = 0; }
)

// 쿤미옌 소한 : 데미지 1, 아이템 사용시마다 데미지가 두배
ITEM(Item045, 45, EItemTierType::Two, EItemType::Attack,
	{
		const int effectItemCount = me.GetEffectItemCount();
int damage;
if (effectItemCount == 1)
{
	damage = 1;
}
else
{
	damage = pow(2, effectItemCount - 1);
}
opponents.ToDamage(damage, me);
	}
	,
{
	const int effectItemCount = me.GetEffectItemCount();
int damage;
if (effectItemCount == 1)
{
	damage = 1;
}
else
{
	damage = pow(2, effectItemCount - 1);
}
opponents.ToDamage(damage, me);
}
,
{
	const int effectItemCount = me.GetEffectItemCount();
int damage;
if (effectItemCount == 1)
{
	damage = 1;
}
else
{
	damage = pow(2, effectItemCount - 1);
}
opponents.ToDamage(damage, me);
}
,
{ int fitmentEffectEmpty = 0; }
)

/// <summary> 모든 아이템의 주소를 가지고 있는 배열 </summary>
static ItemBase* sItems[]
{
	&sItemEmpty, &sItem001, &sItem002, &sItem003, &sItem004, &sItem005, &sItem006, &sItem007, &sItem008, &sItem009,
	&sItem010, &sItem011, &sItem012, &sItem013, &sItem014, &sItem015, &sItem016, &sItem017, &sItem018, &sItem019,
	&sItem020, &sItem021, &sItem022, &sItem023, &sItem024, &sItem025, &sItem026, &sItem027, &sItem028, &sItem029,
	&sItem030, &sItem031, &sItem032, &sItem033, &sItem034, &sItem035, &sItem036, &sItem037, &sItem038, &sItem039,
	&sItem040, &sItem041, &sItem042, &sItem043, &sItem044, &sItem045
};

//각 티어마다의 아이템 벡터
static const vector<const ItemBase*> _sOneTierItems{ &sItem001, &sItem002, &sItem003, &sItem004, &sItem008, &sItem033 };
static const vector<const ItemBase*> _sTwoTierItems{ &sItem007, &sItem009, &sItem010, &sItem012, &sItem014, &sItem016, &sItem019, &sItem020, &sItem021, &sItem022, &sItem026, &sItem030, &sItem031, &sItem037 };
static const vector<const ItemBase*> _sThreeTierItems{ &sItem006, &sItem011, &sItem023, &sItem024, &sItem028, &sItem029, &sItem034, &sItem035, &sItem036 };
static const vector<const ItemBase*> _sFourTierItems{ &sItem005, &sItem013, &sItem017, &sItem018, &sItem025 };
static const vector<const ItemBase*> _sFiveTierItems{ &sItem015, &sItem027, &sItem032 };

// 각 티어 아이템 벡터를 가진 배열
static const vector<const ItemBase*>* sTierItems[MAX_ITEM_UPGRADE]{ &_sOneTierItems, &_sTwoTierItems, &_sThreeTierItems, &_sFourTierItems, &_sFiveTierItems };