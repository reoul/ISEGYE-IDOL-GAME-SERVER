﻿#pragma once
#pragma warning(disable:4996)
#include <memory>

#include "Client.h"
#include "SettingData.h"
#include "Room.h"
#include "reoul/logger.h"
#include "reoul/MemoryStream.h"

using namespace std;

enum class ECharacterType : uint8_t;

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
	cs_sc_upgradeItem,
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
};

/// <summary> cs_sc_notification의 알림 타입 </summary>
enum class ENotificationType : uint8_t
{
	/// <summary> 클라이언트가 서버 접속했음을 알리는 패킷 타입 </summary>
	ConnectServer,
	/// <summary> 캐릭터를 선택했음을 알리는 패킷 타입 </summary>
	ChoiceCharacter,
	/// <summary> 캐릭터 선택 다 끝나고 인게임에 진입했을 때 </summary>
	EnterInGame,
	/// <summary> 클라이언트가 서버 해제했음을 알리는 패킷 타입 </summary>
	DisconnectServer,
	/// <summary> 서버에 계속 연결되는 상태를 알리는 패킷 타입 </summary>
	ConnectCheck,
	/// <summary> 랜덤 아이템 추가 요청을 알리는 패킷 타입 </summary>
	RequestAddRandomItem,
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
	const_wrapper<uint8_t> itemCode;

	void Write(OutputMemoryStream& memoryStream) const
	{
		Packet::Write(memoryStream);
		memoryStream.Write(networkID.get());
		memoryStream.Write(itemCode.get());
	}

	sc_AddNewItemPacket(int networkID, uint8_t itemCode)
		: Packet(sizeof(sc_AddNewItemPacket), EPacketType::sc_addNewItem)
		, networkID(networkID)
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

struct cs_sc_UpgradeItemPacket : protected Packet
{
	const_wrapper<int32_t> networkID;
	const_wrapper<uint8_t> slot;

	void Write(OutputMemoryStream& memoryStream) const
	{
		Packet::Write(memoryStream);
		memoryStream.Write(networkID.get());
		memoryStream.Write(slot.get());
	}

	cs_sc_UpgradeItemPacket() = delete;
};

struct cs_sc_ChangeCharacterPacket : protected Packet
{
	const_wrapper<int32_t> networkID;
	const_wrapper<uint8_t> characterType;

	void Write(OutputMemoryStream& memoryStream) const
	{
		Packet::Write(memoryStream);
		memoryStream.Write(networkID.get());
		memoryStream.Write(characterType.get());
	}

	cs_sc_ChangeCharacterPacket() = delete;
};

struct ItemQueueInfo
{
	int32_t networkID;
	uint8_t itemQueue[MAX_USING_ITEM * BATTLE_ITEM_QUEUE_LOOP_COUNT * 2];
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
			constexpr int itemQueueCount = MAX_USING_ITEM * BATTLE_ITEM_QUEUE_LOOP_COUNT * 2;
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
	SlotItemInfo usingInventoryInfos[MAX_USING_ITEM];
	SlotItemInfo unUsingInventoryInfos[MAX_UN_USING_ITEM];

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
		vector<Item> usingItems = client.GetUsingItems();
		vector<Item> unUsingItems = client.GetUnUsingItems();

		for (int i = 0; i < MAX_USING_ITEM; ++i)
		{
			usingInventoryInfos[i].type = usingItems[i].GetType();
			usingInventoryInfos[i].upgrade = usingItems[i].GetUpgrade();
		}

		for (int i = 0; i < MAX_UN_USING_ITEM; ++i)
		{
			unUsingInventoryInfos[i].type = unUsingItems[i].GetType();
			unUsingInventoryInfos[i].upgrade = unUsingItems[i].GetUpgrade();
		}
	}
};

#pragma pack(pop)
