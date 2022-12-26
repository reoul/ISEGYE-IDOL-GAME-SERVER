#pragma once
#pragma warning(disable:4996)
#include <memory>

#include "Client.h"
#include "SettingData.h"
#include "Room.h"
#include "reoul/logger.h"
#include "reoul/MemoryStream.h"

using namespace std;
using namespace Logger;

enum class ECharacterType : uint8_t;
enum class EItemTicketType : uint8_t;

/// <summary>
/// 패킷 타입<br/>
/// sc : Server to Client<br/>
/// cs : Client to Server
/// </summary>
enum class EPacketType : uint8_t
{
	/// <summary> 클라이언트에게 Room 생성과 접속했음을 알리는 패킷 타입 </summary>
	sc_connectRoom,
	/// <summary> 클라이언트가 서버에게 매칭을 시작했음을 알리는 패킷 타입 </summary>
	cs_startMatching,
	/// <summary> 아이템을 추가했음을 알리는 패킷 타입  </summary>
	sc_addNewItem,
	/// <summary> 아이템을 다른 슬롯으로 이동했음을 알리는 패킷 타입 </summary>
	cs_sc_changeItemSlot,
	/// <summary> 아이템을 업그레이드 했음을 알리는 패킷 타입 </summary>
	sc_upgradeItem,
	/// <summary> 선택 캐릭터가 교체되었음을 알리는 패킷 타입 </summary>
	cs_sc_changeCharacter,
	/// <summary> 클라이언트에게 전투 정보를 알리는 패킷 타입 </summary>
	sc_battleInfo,
	/// <summary> 무언가를 알리는 패킷 타입 </summary>
	cs_sc_notification,
	/// <summary> 이모티콘을 보냈음을 알리는 패킷 타입 </summary>
	cs_sc_useEmoticon,
	/// <summary> 캐릭터 정보를 갱신해주는 패킷 타입 </summary>
	sc_updateCharacterInfo,
	/// <summary> 캐릭터 선택 시간을 설정해주는 패킷 타입 </summary>
	sc_setChoiceCharacterTime,
	/// <summary> 준비시간을 설정해주는 패킷 타입 </summary>
	sc_setReadyTime,
	/// <summary> 아이템 드랍 패킷 타입 </summary>
	cs_sc_dropItem,
	/// <summary> 아이템 조합 요청 패킷 타입 </summary>
	cs_requestCombinationItem,
	/// <summary> 아이템 뽑기권 개수 지정 패킷 타입 </summary>
	sc_setItemTicket,
	/// <summary> 아이템 발동 패킷 타입 </summary>
	sc_activeItem,
	/// <summary> 화면이 서서히 어두어지는 FadeIn 패킷 타입 </summary>
	sc_fadeIn,
	/// <summary> 화면이 서서히 밝아지는 FadeOut 패킷 타입 </summary>
	sc_fadeOut,
	/// <summary> 전투 상대 정보 패킷 타입 </summary>
	sc_battleOpponents,
	/// <summary> 햄버거 타입 지정하는 패킷 타입 </summary>
	sc_setHamburgerType,
	/// <summary> 비밀스런 마법봉 정보 패킷 타입 </summary>
	sc_magicStickInfo,
	/// <summary> 아이템 업그레이드 요청 패킷 타입 </summary>
	cs_requestUpgradeItem,
	/// <summary> 박사의 만능툴 정보 패킷 타입 </summary>
	sc_DoctorToolInfo,
};

/// <summary> cs_sc_notification의 알림 타입 </summary>
enum class ENotificationType : uint8_t
{
	/// <summary> 클라이언트가 서버 접속했음을 알리는 패킷 타입 </summary>
	ConnectServer,
	/// <summary> 캐릭터를 선택했음을 알리는 패킷 타입 </summary>
	ChoiceCharacter,
	/// <summary> 캐릭터 선택 다 끝났을 때 </summary>
	ChoiceAllCharacter,
	/// <summary> 클라이언트가 서버 해제했음을 알리는 패킷 타입 </summary>
	DisconnectServer,
	/// <summary> 서버에 계속 연결되는 상태를 알리는 패킷 타입 </summary>
	ConnectCheck,
	/// <summary> 준비 스테이지에 진입했음을 알리는 패킷 타입 </summary>
	EnterReadyStage,
	/// <summary> 컷신 스테이지에 진입했음을 알리는 패킷 타입 </summary>
	EnterCutSceneStage,
	/// <summary> 컷신 스테이지에 끝났음을 알리는 패킷 타입 </summary>
	FinishCutSceneStage,
	/// <summary> 전투 스테이지에 진입했음을 알리는 패킷 타입 </summary>
	EnterBattleStage,
	/// <summary> 크립라운드 스테이지에 진입했음을 알리는 패킷 타입 </summary>
	EnterCreepStage,
	/// <summary> 캐릭터 선택이 끝났음을 알리는 패킷 타입 </summary>
	FinishChoiceCharacterTime,
	/// <summary> 전투 필드 아이템 초기화 패킷 타입 </summary>
	InitBattleSlot,
	/// <summary> 일반 아이템 뽑기권 사용을 알리는 패킷 타입 </summary>
	UseNormalItemTicket,
	/// <summary> 고급 아이템 뽑기권 사용을 알리는 패킷 타입 </summary>
	UseAdvancedItemTicket,
	/// <summary> 최고급 아이템 뽑기권 사용을 알리는 패킷 타입 </summary>
	UseTopItemTicket,
	/// <summary> 지존 아이템 뽑기권 사용을 알리는 패킷 타입 </summary>
	UseSupremeItemTicket,
};

#pragma pack(push, 1)

struct Packet
{
	const_wrapper<uint16_t> size;
	const_wrapper<EPacketType> type;

	void Write(OutputMemoryStream& memoryStream) const
	{
		memoryStream.Write(size.get());
		memoryStream.Write(type.get());
	}

	Packet(uint16_t size, EPacketType type)
		: size(size)
		, type(type)
	{
	}
};

struct cs_StartMatchingPacket : protected Packet
{
	int32_t networkID;
	wchar_t name[MAX_USER_NAME_LENGTH];

	void Write(OutputMemoryStream& memoryStream) const
	{
		Packet::Write(memoryStream);
		memoryStream.Write(networkID);
		for (wchar_t c : name)
		{
			memoryStream.Write(c);
		}
	}

	cs_StartMatchingPacket() = delete;
};

struct UserInfo
{
	int32_t networkID;
	wchar_t name[MAX_USER_NAME_LENGTH];
};

struct sc_ConnectRoomPacket : protected Packet
{
	UserInfo users[MAX_ROOM_PLAYER];

	void Write(OutputMemoryStream& memoryStream) const
	{
		Packet::Write(memoryStream);
		for (const UserInfo& userInfo : users)
		{
			memoryStream.Write(userInfo.networkID);
			for (wchar_t wc : userInfo.name)
			{
				memoryStream.Write(wc);
			}
		}
	}

	sc_ConnectRoomPacket(const Room& room)
		: Packet(sizeof(sc_ConnectRoomPacket), EPacketType::sc_connectRoom)
		, users{}
	{
		const vector<Client*> clients = room.GetClients();
		for (size_t i = 0; i < clients.size(); ++i)
		{
			users[i].networkID = clients[i]->GetNetworkID();
			wcscpy(users[i].name, clients[i]->GetName());
		}
	}
};

struct sc_AddNewItemPacket : protected Packet
{
	const_wrapper<int32_t> networkID;
	const_wrapper<uint8_t> slot;
	const_wrapper<uint8_t> itemCode;

	void Write(OutputMemoryStream& memoryStream) const
	{
		Packet::Write(memoryStream);
		memoryStream.Write(networkID.get());
		memoryStream.Write(slot.get());
		memoryStream.Write(itemCode.get());
	}

	sc_AddNewItemPacket(int networkID, uint8_t slot, uint8_t itemCode)
		: Packet(sizeof(sc_AddNewItemPacket), EPacketType::sc_addNewItem)
		, networkID(networkID)
		, slot(slot)
		, itemCode(itemCode)
	{
	}
};

struct cs_sc_ChangeItemSlotPacket : protected Packet
{
	const_wrapper<int32_t>  networkID;
	const_wrapper<uint8_t> slot1;
	const_wrapper<uint8_t> slot2;

	void Write(OutputMemoryStream& memoryStream) const
	{
		Packet::Write(memoryStream);
		memoryStream.Write(networkID.get());
		memoryStream.Write(slot1.get());
		memoryStream.Write(slot2.get());
	}

	cs_sc_ChangeItemSlotPacket() = delete;
};

struct sc_UpgradeItemPacket : protected Packet
{
	const_wrapper<int32_t> networkID;
	const_wrapper<uint8_t> slot;
	const_wrapper<uint8_t> upgrade;

	void Write(OutputMemoryStream& memoryStream) const
	{
		Packet::Write(memoryStream);
		memoryStream.Write(networkID.get());
		memoryStream.Write(slot.get());
		memoryStream.Write(upgrade.get());
	}

	sc_UpgradeItemPacket(int32_t networkID, uint8_t slot, uint8_t upgrade)
		: Packet(sizeof(sc_UpgradeItemPacket), EPacketType::sc_upgradeItem)
		, networkID(networkID)
		, slot(slot)
		, upgrade(upgrade)
	{
	}
};

struct cs_sc_ChangeCharacterPacket : protected Packet
{
	const_wrapper<int32_t> networkID;
	const_wrapper<ECharacterType> characterType;

	void Write(OutputMemoryStream& memoryStream) const
	{
		Packet::Write(memoryStream);
		memoryStream.Write(networkID.get());
		memoryStream.Write(characterType.get());
	}

	cs_sc_ChangeCharacterPacket(int32_t networkID, ECharacterType characterType)
		: Packet(sizeof(cs_sc_ChangeCharacterPacket), EPacketType::cs_sc_changeCharacter)
		, networkID(networkID)
		, characterType(characterType)
	{
	}
};

struct ItemQueueInfo
{
	int32_t networkID;
	uint8_t itemQueue[MAX_USING_ITEM_COUNT * BATTLE_ITEM_QUEUE_LOOP_COUNT * 2];
};

struct sc_BattleInfoPacket : protected Packet
{
	int32_t battleOpponentQueue[MAX_ROOM_PLAYER];
	ItemQueueInfo itemQueueInfo[MAX_ROOM_PLAYER];

	void Write(OutputMemoryStream& memoryStream) const
	{
		Packet::Write(memoryStream);
		for (int32_t opponent : battleOpponentQueue)
		{
			memoryStream.Write(opponent);
		}
		for (const ItemQueueInfo& itemInfo : itemQueueInfo)
		{
			memoryStream.Write(itemInfo.networkID);
			for (uint8_t item : itemInfo.itemQueue)
			{
				memoryStream.Write(item);
			}
		}
	}

	sc_BattleInfoPacket(const vector<int32_t>& battleOpponents, const vector<int32_t>& itemQueues)
		: Packet(sizeof(sc_BattleInfoPacket)
			, EPacketType::sc_battleInfo)
	{
		log_assert(battleOpponents.size() <= MAX_ROOM_PLAYER);
		copy(battleOpponents.begin(), battleOpponents.end(), battleOpponentQueue);
		if (battleOpponents.size() < MAX_ROOM_PLAYER)
		{
			battleOpponentQueue[battleOpponents.size()] = INT32_MAX;
		}

		log_assert(itemQueues.size() == BATTLE_ITEM_QUEUE_LENGTH);
		for (size_t i = 0; i < MAX_ROOM_PLAYER; ++i)
		{
			constexpr int itemQueueCount = MAX_USING_ITEM_COUNT * BATTLE_ITEM_QUEUE_LOOP_COUNT * 2;
			itemQueueInfo[i].networkID = itemQueues[i * itemQueueCount + i];

			for (size_t j = 0; j < itemQueueCount; ++j)
			{
				itemQueueInfo[i].itemQueue[j] = itemQueues[i * itemQueueCount + i + (j + 1)];
			}
		}
	}
};

struct cs_sc_UseEmoticonPacket : protected Packet
{
	const_wrapper<int32_t> networkID;
	const_wrapper<uint8_t> emoticonType;

	void Write(OutputMemoryStream& memoryStream) const
	{
		Packet::Write(memoryStream);
		memoryStream.Write(networkID.get());
		memoryStream.Write(emoticonType.get());
	}

	cs_sc_UseEmoticonPacket() = delete;
};

struct cs_sc_NotificationPacket : protected Packet
{
	const_wrapper<int32_t> networkID;
	const_wrapper<ENotificationType> notificationType;

	void Write(OutputMemoryStream& memoryStream) const
	{
		Packet::Write(memoryStream);
		memoryStream.Write(networkID.get());
		memoryStream.Write(notificationType.get());
	}

	cs_sc_NotificationPacket(int networkID, ENotificationType notificationType)
		: Packet(sizeof(cs_sc_NotificationPacket), EPacketType::cs_sc_notification)
		, networkID(networkID)
		, notificationType(notificationType)
	{
	}
};

struct SlotItemInfo
{
	uint8_t type;
	uint8_t upgrade;
};

struct sc_UpdateCharacterInfoPacket : protected Packet
{
	const_wrapper<int32_t> networkID;
	const_wrapper<int16_t> hp;
	const_wrapper<int16_t> avatarHp;
	SlotItemInfo usingInventoryInfos[MAX_USING_ITEM_COUNT];
	SlotItemInfo unUsingInventoryInfos[MAX_UN_USING_ITEM_COUNT];

	void Write(OutputMemoryStream& memoryStream) const
	{
		Packet::Write(memoryStream);
		memoryStream.Write(networkID.get());
		memoryStream.Write(hp.get());
		memoryStream.Write(avatarHp.get());

		for (SlotItemInfo usingInventoryInfo : usingInventoryInfos)
		{
			memoryStream.Write(usingInventoryInfo.type);
			memoryStream.Write(usingInventoryInfo.upgrade);
		}

		for (SlotItemInfo unUsingInventoryInfo : unUsingInventoryInfos)
		{
			memoryStream.Write(unUsingInventoryInfo.type);
			memoryStream.Write(unUsingInventoryInfo.upgrade);
		}
	}

	sc_UpdateCharacterInfoPacket(const Client& client)
		: Packet(sizeof(sc_UpdateCharacterInfoPacket), EPacketType::sc_updateCharacterInfo)
		, networkID(client.GetNetworkID())
		, hp(static_cast<int16_t>(client.GetHp()))
		, avatarHp(static_cast<int16_t>(client.GetAvatarHp()))
	{
		const vector<Item> usingItems = client.GetUsingItems();
		const vector<Item> unUsingItems = client.GetUnUsingItems();

		for (int i = 0; i < MAX_USING_ITEM_COUNT; ++i)
		{
			usingInventoryInfos[i].type = usingItems[i].GetType();
			usingInventoryInfos[i].upgrade = usingItems[i].GetUpgrade();
		}

		for (int i = 0; i < MAX_UN_USING_ITEM_COUNT; ++i)
		{
			unUsingInventoryInfos[i].type = unUsingItems[i].GetType();
			unUsingInventoryInfos[i].upgrade = unUsingItems[i].GetUpgrade();
		}
	}
};

struct sc_SetReadyTimePacket : protected Packet
{
	const_wrapper<uint8_t> time;

	void Write(OutputMemoryStream& memoryStream) const
	{
		Packet::Write(memoryStream);
		memoryStream.Write(time.get());
	}

	sc_SetReadyTimePacket(uint8_t time)
		: Packet(sizeof(sc_SetReadyTimePacket), EPacketType::sc_setReadyTime)
		, time(time)
	{
	}
};

struct sc_SetChoiceCharacterTimePacket : protected Packet
{
	const_wrapper<uint8_t> time;

	void Write(OutputMemoryStream& memoryStream) const
	{
		Packet::Write(memoryStream);
		memoryStream.Write(time.get());
	}

	sc_SetChoiceCharacterTimePacket(uint8_t time)
		: Packet(sizeof(sc_SetChoiceCharacterTimePacket), EPacketType::sc_setChoiceCharacterTime)
		, time(time)
	{
	}
};

struct cs_sc_DropItemPacket : protected Packet
{
	const_wrapper<int32_t> networkID;
	const_wrapper<uint8_t> index;

	void Write(OutputMemoryStream& memoryStream) const
	{
		Packet::Write(memoryStream);
		memoryStream.Write(networkID.get());
		memoryStream.Write(index.get());
	}

	cs_sc_DropItemPacket(int32_t networkID, uint8_t index)
		: Packet(sizeof(cs_sc_DropItemPacket), EPacketType::cs_sc_dropItem)
		, networkID(networkID)
		, index(index)
	{
	}
};

struct cs_RequestCombinationItemPacket : private Packet
{
	const_wrapper<int32_t> networkID;
	const_wrapper<uint8_t> index1;
	const_wrapper<uint8_t> index2;
	const_wrapper<uint8_t> index3;

	cs_RequestCombinationItemPacket() = delete;
};

struct sc_SetItemTicketPacket : private Packet
{
	const_wrapper<int32_t> networkID;
	const_wrapper<EItemTicketType> itemTicketType;
	const_wrapper<uint8_t> count;

	void Write(OutputMemoryStream& memoryStream) const
	{
		Packet::Write(memoryStream);
		memoryStream.Write(networkID.get());
		memoryStream.Write(itemTicketType.get());
		memoryStream.Write(count.get());
	}

	/**
	 * \param networkID 네트워크 아이디
	 * \param itemTicketType 뽑기권 타입
	 * \param count 개수
	 */
	sc_SetItemTicketPacket(int32_t networkID, EItemTicketType itemTicketType, uint8_t count)
		: Packet(sizeof(sc_SetItemTicketPacket), EPacketType::sc_setItemTicket)
		, networkID(networkID)
		, itemTicketType(itemTicketType)
		, count(count)
	{
	}
};

struct sc_ActiveItemPacket : private Packet
{
	const_wrapper<int32_t> networkID;
	const_wrapper<uint8_t> slot;

	void Write(OutputMemoryStream& memoryStream) const
	{
		Packet::Write(memoryStream);
		memoryStream.Write(networkID.get());
		memoryStream.Write(slot.get());
	}

	/**
	 * \param networkID 네트워크 아이디
	 * \param slot 발동 슬롯 번호
	 * \param active 발동 여부
	 */
	sc_ActiveItemPacket(int32_t networkID, uint8_t slot)
		: Packet(sizeof(sc_ActiveItemPacket), EPacketType::sc_setItemTicket)
		, networkID(networkID)
		, slot(slot)
	{
	}
};

struct sc_FadeInPacket : private Packet
{
	const_wrapper<uint8_t> seconds;

	/**
	 * \param seconds Fade 시간
	 */
	sc_FadeInPacket(uint8_t seconds)
		: Packet(sizeof(sc_FadeInPacket), EPacketType::sc_fadeIn)
		, seconds(seconds)
	{
	}
};

struct sc_FadeOutPacket : private Packet
{
	const_wrapper<uint8_t> seconds;

	/**
	 * \param seconds Fade 시간
	 */
	sc_FadeOutPacket(uint8_t seconds)
		: Packet(sizeof(sc_FadeOutPacket), EPacketType::sc_fadeOut)
		, seconds(seconds)
	{
	}
};

struct sc_BattleOpponentsPacket : private Packet
{
	int32_t battleOpponentQueue[MAX_ROOM_PLAYER];

	void Write(OutputMemoryStream& memoryStream) const
	{
		Packet::Write(memoryStream);
		for (int32_t opponent : battleOpponentQueue)
		{
			memoryStream.Write(opponent);
		}
	}

	sc_BattleOpponentsPacket(const vector<int32_t>& battleOpponents)
		: Packet(sizeof(sc_BattleOpponentsPacket), EPacketType::sc_battleOpponents)
	{
		log_assert(battleOpponents.size() <= MAX_ROOM_PLAYER);

		copy(battleOpponents.begin(), battleOpponents.end(), battleOpponentQueue);
		if (battleOpponents.size() < MAX_ROOM_PLAYER)
		{
			battleOpponentQueue[battleOpponents.size()] = INT32_MAX;
		}
	}
};

struct sc_SetHamburgerTypePacket : private Packet
{
	const_wrapper<int32_t> networkID;
	const_wrapper<uint8_t> slot;
	const_wrapper<EHamburgerType> burgerType;

	void Write(OutputMemoryStream& memoryStream) const
	{
		Packet::Write(memoryStream);
		memoryStream.Write(networkID.get());
		memoryStream.Write(slot.get());
		memoryStream.Write(burgerType.get());
	}

	sc_SetHamburgerTypePacket(int32_t networkID, uint8_t slot, EHamburgerType type)
		: Packet(sizeof(sc_SetHamburgerTypePacket), EPacketType::sc_setHamburgerType)
		, networkID(networkID)
		, slot(slot)
		, burgerType(type)
	{
	}
};

/// <summary> 비밀스런 마법봉 정보 패킷 타입 </summary>
struct sc_MagicStickInfoPacket : private Packet
{
	const_wrapper<int32_t> networkID;
	const_wrapper<bool> isDamage;

	void Write(OutputMemoryStream& memoryStream) const
	{
		Packet::Write(memoryStream);
		memoryStream.Write(networkID.get());
		memoryStream.Write(isDamage.get());
	}

	sc_MagicStickInfoPacket(int32_t networkID, bool isDamage)
		: Packet(sizeof(sc_MagicStickInfoPacket), EPacketType::sc_magicStickInfo)
		, networkID(networkID)
		, isDamage(isDamage)
	{
	}
};

/// <summary> 업그레이드 요청 패킷 타입 </summary>
struct cs_RequestUpgradeItemPacket : private Packet
{
	const_wrapper<int32_t> networkID;
	const_wrapper<uint8_t> slot1;
	const_wrapper<uint8_t> slot2;

	cs_RequestUpgradeItemPacket() = delete;
};

/// <summary> 박사의 만능툴 정보 패킷 타입 </summary>
struct sc_DoctorToolInfoPacket : private Packet
{
	const_wrapper<int32_t> networkID;
	const_wrapper<uint8_t> slot;
	const_wrapper<uint8_t> itemType;
	const_wrapper<uint8_t> upgrade;

	void Write(OutputMemoryStream& memoryStream) const
	{
		Packet::Write(memoryStream);
		memoryStream.Write(networkID.get());
		memoryStream.Write(slot.get());
		memoryStream.Write(itemType.get());
		memoryStream.Write(upgrade.get());
	}

	sc_DoctorToolInfoPacket(int32_t networkID, uint8_t slot, uint8_t itemType, uint8_t upgrade)
		: Packet(sizeof(sc_DoctorToolInfoPacket), EPacketType::sc_DoctorToolInfo)
		, networkID(networkID)
		, slot(slot)
		, itemType(itemType)
		, upgrade(upgrade)
	{
	}
};

#pragma pack(pop)
