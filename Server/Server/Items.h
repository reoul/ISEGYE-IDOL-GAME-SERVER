#pragma once
#include <vector>

#include "ItemBase.h"
#include "reoul/logger.h"
#include "BattleAvatar.h"
#include "Random.h"

using namespace std;
using namespace Logger;

/**
 * \brief 아이템 생성 매크로
 * \param name 아이템 클래스 이름
 * \param oneStarUseEffect 1성 아이템 효과
 * \param twoStarUseEffect 2성 아이템 효과
 * \param threeStarUseEffect 3성 아이템 효과
 */
#define ITEM(name, code, tier, oneStarUseEffect, twoStarUseEffect, threeStarUseEffect)	\
class name final : public ItemBase														\
{																						\
public:																					\
	name() : ItemBase(code, tier)														\
	{																					\
	}																					\
																						\
	void Active(BattleAvatar& me, BattleAvatar& opponents, int upgrade) override		\
	{																					\
		switch (upgrade)																\
		{																				\
		case 0:																			\
			oneStarUseEffect															\
			break;																		\
		case 1:																			\
			twoStarUseEffect															\
			break;																		\
		case 2:																			\
			threeStarUseEffect															\
			break;																		\
		default:																		\
			LogWarning("log", "["#name"] 아이템 강화 수치가 {0}입니다", upgrade);		\
		}																				\
	}																					\
																						\
	void FitmentEffect(BattleAvatar& me) override													\
	{																					\
	}																					\
};																						\
static name s##name;																	\

 // 빈칸 : 아무 효과 없음
ITEM(ItemEmpty, 0, EItemTierType::One,
	,
	,
	)

	// 낡은 채찍 : x 데미지
	ITEM(Item000, 1, EItemTierType::One,
		opponents.ToDamage(5, me);
,
opponents.ToDamage(7, me);
,
opponents.ToDamage(12, me);
)

// 오베인 게르크 티 : 방어도를 X만큼 획득
ITEM(Item001, 2, EItemTierType::One,
	me.ToDefensive(3);
,
me.ToDefensive(5);
,
me.ToDefensive(7);
)

// 귀상어두 가면 : 상대방 약화
ITEM(Item002, 3, EItemTierType::One,
	opponents.ToWeakening(2);
,
opponents.ToWeakening(3);
,
opponents.ToWeakening(4);
)

// 왁엔터 사장 명패 : X 데미지를 두번 줍니다. (여러번 공격)
ITEM(Item003, 4, EItemTierType::One,
	opponents.ToDamage(2, me);
opponents.ToDamage(2, me);
,
opponents.ToDamage(3, me);
opponents.ToDamage(3, me);
,
opponents.ToDamage(5, me);
opponents.ToDamage(5, me);
)

// 가지치기용 도끼 : 본인 방어력 비례 데미지 기본 데미지 + 방어력 * x
ITEM(Item004, 5, EItemTierType::Four,
	opponents.ToDamage(5 + me.GetDefensive() * 3, me);
,
opponents.ToDamage(5 + me.GetDefensive() * 4, me);
,
opponents.ToDamage(5 + me.GetDefensive() * 6, me);
)

// 홍삼스틱 : 부정 효과 제거
ITEM(Item005, 6, EItemTierType::Three,
	me.Clear();
,
me.Clear();
,
me.Clear();
)

// 햄버거
// 휠렛버거 : 효과
// 기네스버거: 공격
// 화이트 갈릭 버거 : 회복
// 밥버거 : 방어
ITEM(Item006, 7, EItemTierType::Two,
	// todo : 효과 추가
	,
	// todo : 효과 추가
	,
	// todo : 효과 추가
	)

	// 악질안경 : 공격력 상승
	ITEM(Item007, 8, EItemTierType::One,
		me.ToOffensePower(1);
,
me.ToOffensePower(3);
,
me.ToOffensePower(5);
)

// 전투 메이드복 : 방어도 + 5, 장착 후 공격 당할시 1회성 반격 X 데미지
ITEM(Item008, 9, EItemTierType::Two,
	me.ToDefensive(5);
// todo : 반격 추가
,
me.ToDefensive(5);
// todo : 반격 추가
,
me.ToDefensive(5);
// todo : 반격 추가
)

// 수녀복 : 방어도 + 5, 장착 후 공격 당할시 1회성 회복 X
ITEM(Item009, 10, EItemTierType::Two,
	me.ToDefensive(5);
// todo : 회복 추가
,
me.ToDefensive(5);
// todo : 회복 추가
,
me.ToDefensive(5);
// todo : 회복 추가
)

// 매우 큰 리본 : 추가방어력 X 제공
ITEM(Item010, 11, EItemTierType::Two,
	me.ToAdditionDefensive(2);
,
me.ToAdditionDefensive(4);
,
me.ToAdditionDefensive(8);
)

// 슈크림 붕어빵 : 자신의 유물을 사용할 때 마다 체력 회복, 사이클 끝나면 효과 끝
ITEM(Item011, 12, EItemTierType::Two,
	// todo : 효과 추가
	,
	// todo : 효과 추가
	,
	// todo : 효과 추가
	)


	// 언니의 마음 : 사이클이 종료되면 폭발하는 폭탄을 설치, 폭발시 X 데미지
	ITEM(Item012, 13, EItemTierType::Three,
		// todo : 효과 추가
		,
		// todo : 효과 추가
		,
		// todo : 효과 추가
		)

	// 좋은 거 : 체력 X만큼 깎고 3X만큼 방어도
	ITEM(Item013, 14, EItemTierType::Two,
		me.ToJustDamage(3);
me.ToDefensive(3 * 3);
,
me.ToJustDamage(4);
me.ToDefensive(3 * 4);
,
me.ToJustDamage(5);
me.ToDefensive(3 * 5);
)

// 다이아 검
ITEM(Item014, 15, EItemTierType::Five,
	// todo : 기능 추가
	,
	// todo : 기능 추가
	,
	// todo : 기능 추가
	)

	// 박사의 만능툴
	ITEM(Item015, 16, EItemTierType::Three,
		// todo : 기능 추가
		,
		// todo : 기능 추가
		,
		// todo : 기능 추가
		)

	// 여고생의 헤어롤 : 뽑기권 * X의 데미지를 줍니다
	ITEM(Item016, 17, EItemTierType::Three,
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
opponents.ToDamage(sumTicket, me);
}
,
{
	const Client * client = me.GetClient();
const int sumTicket = client->GetNormalItemTicketCount() + client->GetAdvancedItemTicketCount()
						+ client->GetTopItemTicketCount() + client->GetSupremeItemTicketCount();
opponents.ToDamage(sumTicket * 2, me);
}
)

// 광대의 권총 : X 데미지 (상대방에게 방어력이 있을 시 2배 데미지)
ITEM(Item017, 18, EItemTierType::Four,
	opponents.ToDamage(5 * (opponents.GetDefensive() == 0 ? 1 : 2), me);
,
opponents.ToDamage(10 * (opponents.GetDefensive() == 0 ? 1 : 2), me);
,
opponents.ToDamage(15 * (opponents.GetDefensive() == 0 ? 1 : 2), me);
)

// 보컬로이드의 전기충격기 : 적 약화 1부여 + X 데미지
ITEM(Item018, 19, EItemTierType::Two,
	opponents.ToWeakening(1);
opponents.ToDamage(6, me);
,
opponents.ToWeakening(1);
opponents.ToDamage(9, me);
,
opponents.ToWeakening(1);
opponents.ToDamage(12, me);
)

// 부정적인 드롭스
ITEM(Item019, 20, EItemTierType::Four,
	// 기능 바뀌면 도입
	,
	// 기능 바뀌면 도입
	,
	// 기능 바뀌면 도입
	)

	// 비밀스런 마법봉 : 상대에게 데미지 주거나 나의 체력 회복 (50퍼 확률)
	ITEM(Item020, 21, EItemTierType::Two,
		{
			Random<int> gen(0,1);
// todo : 패킷 날리기
if (gen() == 0)
{
	opponents.ToDamage(4, me);
}
else
{
	me.ToHeal(7);
}
		}
		,
		{
			Random<int> gen(0,1);
		// todo : 패킷 날리기
		if (gen() == 0)
		{
			opponents.ToDamage(6, me);
		}
		else
		{
			me.ToHeal(10);
		}
		}
			,
		{
			Random<int> gen(0,1);
		// todo : 패킷 날리기
		if (gen() == 0)
		{
			opponents.ToDamage(9, me);
		}
		else
		{
			me.ToHeal(14);
		}
		}
			)

	// 귀족의 상현딸 : 매턴 적 출혈 2뎀 + X 데미지
			ITEM(Item021, 22, EItemTierType::Two,
				opponents.ToBleeding(2);
		opponents.ToDamage(3, me);
		,
			opponents.ToBleeding(2);
		opponents.ToDamage(4, me);
		,
			opponents.ToBleeding(2);
		opponents.ToDamage(5, me);
		)

			// 대학원생의 USB : 다음 발동하는 내 아이템 효과는 2번 발동합니다.
			ITEM(Item022, 23, EItemTierType::Four,
				// todo : 브란 만들기
				,
				// todo : 브란 만들기
				,
				// todo : 브란 만들기
				)

			// 선량한 시민의 빠루
			ITEM(Item023, 24, EItemTierType::Three,
				// todo : 기능 추가
				,
				// todo : 기능 추가
				,
				// todo : 기능 추가
				)

			// 노인의 틀니 : 발동 순서에 따라 공격력 버프
			ITEM(Item024, 25, EItemTierType::Four,
				// todo : 기능 추가
				,
				// todo : 기능 추가
				,
				// todo : 기능 추가
				)

			// 마법사의 스테프 : 5 데미지 + 적 치유력 감소 X
			ITEM(Item025, 26, EItemTierType::Two,
				opponents.ToDamage(5, me);
		opponents.ToReducedHealing(2);
		,
			opponents.ToDamage(5, me);
		opponents.ToReducedHealing(4);
		,
			opponents.ToDamage(5, me);
		opponents.ToReducedHealing(6);
		)

			// 바텐더의 나비넥타이 : 방어도 + X, 현재 방어력이 2배가 됨
			ITEM(Item026, 27, EItemTierType::Five,
				me.ToDefensive(3);
		me.ToDefensive(me.GetDefensive());
		,
			me.ToDefensive(5);
		me.ToDefensive(me.GetDefensive());
		,
			me.ToDefensive(20);
		me.ToDefensive(me.GetDefensive());
		)

			// 오니의 카타나 : 매턴 적 출혈 4뎀 + X 데미지
			ITEM(Item027, 28, EItemTierType::Three,
				opponents.ToBleeding(4);
		opponents.ToDamage(5, me);
		,
			opponents.ToBleeding(4);
		opponents.ToDamage(6, me);
		,
			opponents.ToBleeding(4);
		opponents.ToDamage(10, me);
		)

			// 심리상담사의 자격증 : X 확률로 일반 뽑기권을 생성합니다
			ITEM(Item028, 29, EItemTierType::Three,
				{
					Random<int> gen(0,99);
					if (gen() < 30)
					{
						me.GetClient()->SetNormalItemTicketCount(me.GetClient()->GetNormalItemTicketCount() + 1);
						// todo : 획득 패킷 전송
					}
				}
				,
				{
					Random<int> gen(0,99);
					if (gen() < 50)
					{
						me.GetClient()->SetNormalItemTicketCount(me.GetClient()->GetNormalItemTicketCount() + 1);
						// todo : 획득 패킷 전송
					}
				}
					,
				{
					Random<int> gen(0,99);
					if (gen() < 100)
					{
						me.GetClient()->SetNormalItemTicketCount(me.GetClient()->GetNormalItemTicketCount() + 1);
						// todo : 획득 패킷 전송
					}
				}
					)

			// 알바생의 빗자루 : 다음 피해를 1회 무시함
					ITEM(Item029, 30, EItemTierType::Two,
						// todo : 기능 추가
						,
						// todo : 기능 추가
						,
						// todo : 기능 추가
						)

					// 관심병사의 K2 : 방어력 관통 X 데미지
					ITEM(Item030, 31, EItemTierType::Two,
						opponents.ToPiercingDamage(4, me);
				,
					opponents.ToPiercingDamage(6, me);
				,
					opponents.ToPiercingDamage(9, me);
				)

					// 철학도의 칫솔 : 잃은 체력의 X % 회복
					ITEM(Item031, 32, EItemTierType::Five,
						{
							int lostHp = me.GetMaxHP() - me.GetHP();
				lostHp *= 0.2f;
				me.ToHeal(lostHp);
						}
						,
						{
							int lostHp = me.GetMaxHP() - me.GetHP();
				lostHp *= 0.3f;
				me.ToHeal(lostHp);
						}
							,
						{
							int lostHp = me.GetMaxHP() - me.GetHP();
				lostHp *= 0.5f;
				me.ToHeal(lostHp);
						}
							)

					// 잼민이의 돈까스 : 체력 회복
							ITEM(Item032, 33, EItemTierType::One,
								me.ToHeal(3);
						,
							me.ToHeal(5);
						,
							me.ToHeal(7);
						)

							// 벌레의 키캡 : 이 아이템을 사용한 횟수만큼 X 데미지로 공격 (하나의 전투 한정)
							ITEM(Item033, 34, EItemTierType::Three,
								// todo : battleAvatar에 변수 추가
								,
								// todo : battleAvatar에 변수 추가
								,
								// todo : battleAvatar에 변수 추가
								)

							// 500원짜리 고철 : 방어도를 X 얻습니다, 공격력을 1 얻습니다
							ITEM(Item034, 35, EItemTierType::Three,
								me.ToDefensive(5);
						me.ToOffensePower(1);
						,
							me.ToDefensive(10);
						me.ToOffensePower(1);
						,
							me.ToDefensive(15);
						me.ToOffensePower(1);
						)

							// 탐험가의 벨트 : 공격 당할시 1회성으로 닳은 방어도는 복구됨
							ITEM(Item035, 36, EItemTierType::Three,
								// todo : 기능 구현
								,
								// todo : 기능 구현
								,
								// todo : 기능 구현
								)


							// 히어로의 망토 : 부정적인 효과를 1회 방어
							ITEM(Item036, 37, EItemTierType::Two,
								// todo : 기능 구현
								,
								// todo : 기능 구현
								,
								// todo : 기능 구현
								)

							/// <summary> 모든 아이템의 주소를 가지고 있는 배열 </summary>
							static ItemBase* sItems[]
						{
							&sItemEmpty ,&sItem000, &sItem001, &sItem002, &sItem003, &sItem004, &sItem005, &sItem006, &sItem007, &sItem008, &sItem009,
							&sItem010, &sItem011, &sItem012, &sItem013, &sItem014, &sItem015, &sItem016, &sItem017, &sItem018, &sItem019,
							&sItem020, &sItem021, &sItem022, &sItem023, &sItem024, &sItem025, &sItem026, &sItem027, &sItem028, &sItem029,
							&sItem030, &sItem031, &sItem032, &sItem033, &sItem034, &sItem035, &sItem036
						};

						// 각 티어마다의 아이템 벡터
						static const vector<const ItemBase*> _sOneTierItems{ &sItem000, &sItem001, &sItem002, &sItem003, &sItem007, &sItem032 };
						static const vector<const ItemBase*> _sTwoTierItems{ &sItem006, &sItem008, &sItem009, &sItem010, &sItem011, &sItem013, &sItem018, &sItem020, &sItem021, &sItem025, &sItem029, &sItem030, &sItem036 };
						static const vector<const ItemBase*> _sThreeTierItems{ &sItem005, &sItem012, &sItem015, &sItem016, &sItem023, &sItem027, &sItem028, &sItem033, &sItem034, &sItem035 };
						static const vector<const ItemBase*> _sFourTierItems{ &sItem004, &sItem017, &sItem019, &sItem022, &sItem024 };
						static const vector<const ItemBase*> _sFiveTierItems{ &sItem014, &sItem026, &sItem031 };

						// 각 티어 아이템 벡터를 가진 배열
						static const vector<const ItemBase*>* sTierItems[MAX_ITEM_UPGRADE]{ &_sOneTierItems, &_sTwoTierItems, &_sThreeTierItems, &_sFourTierItems, &_sFiveTierItems };