#pragma once
#pragma warning(disable:4996)
#include <memory>

#include "Client.h"
#include "SettingData.h"
#include "Room.h"
#include "reoul/logger.h"

using namespace std;

enum class ECharacterType : uint8_t;

enum class EPacketType : uint8_t
{
	sc_connectServer,
	sc_connectRoom,
	sc_disconnect,
	cs_startMatching,
	cs_sc_addNewItem,
	cs_sc_changeItemSlot,
	cs_sc_upgradeItem,
	cs_sc_changeCharacter,
	sc_battleInfo,
	cs_battleReady,
};

#pragma pack(push, 1)

struct UserInfo
{
	int32_t networkID;
	wchar_t name[MAX_USER_NAME_LENGTH];
};

struct cs_test_data
{
	int a;
	int b;
	int c;
};

struct sc_connectServerPacket
{
	const uint16_t size;
	const EPacketType type;
	int32_t networkID;
	sc_connectServerPacket(int network_id)
		: size(sizeof(sc_connectServerPacket))
		, type(EPacketType::sc_connectServer)
		, networkID(network_id)
	{
	}
};

struct cs_startMatchingPacket
{
	const uint16_t size;
	const EPacketType type;
	const int32_t networkID;
	const wchar_t name[MAX_USER_NAME_LENGTH];
};

struct sc_connectRoomPacket
{
	const uint16_t size;
	const EPacketType type;
	UserInfo users[MAX_ROOM_PLAYER];
	sc_connectRoomPacket(const Room& room)
		: size(sizeof(sc_connectRoomPacket))
		, type(EPacketType::sc_connectRoom)
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

struct cs_sc_AddNewItemPacket
{
	const uint16_t size;
	const EPacketType type;
	const int32_t networkID;
	const uint8_t itemCode;
};

struct sc_disconnectPacket
{
	const uint16_t size;
	const EPacketType type;
	int32_t networkID;
	sc_disconnectPacket(int32_t network_id)
		: size(sizeof(sc_disconnectPacket))
		, type(EPacketType::sc_disconnect)
		, networkID(network_id)
	{
	}
};

struct cs_sc_changeItemSlotPacket
{
	const uint16_t size;
	const EPacketType type;
	const int32_t networkID;
	const uint8_t slot1;
	const uint8_t slot2;
	cs_sc_changeItemSlotPacket(int32_t networkID, uint8_t slot1, uint8_t slot2)
		: size(sizeof(cs_sc_changeItemSlotPacket))
		, type(EPacketType::cs_sc_changeItemSlot)
		, networkID(networkID)
		, slot1(slot1)
		, slot2(slot2)
	{
	}
};

struct cs_sc_upgradeItemPacket
{
	const uint16_t size;
	const EPacketType type;
	const int32_t networkID;
	const uint8_t slot;
	cs_sc_upgradeItemPacket(int32_t networkID, uint8_t slot)
		: size(sizeof(cs_sc_upgradeItemPacket))
		, type(EPacketType::cs_sc_upgradeItem)
		, networkID(networkID)
		, slot(slot)
	{
	}
};

struct cs_sc_battleInfoPacket
{
	const uint16_t size;
	const EPacketType type;
	const int32_t networkID1;
	const int32_t networkID2;
	cs_sc_battleInfoPacket(int32_t networkID1, int16_t networkID2)
		: size(sizeof(cs_sc_battleInfoPacket))
		, type(EPacketType::cs_sc_upgradeItem)
		, networkID1(networkID1)
		, networkID2(networkID2)
	{
	}
};

struct cs_sc_changeCharacterPacket
{
	const uint16_t size;
	const EPacketType type;
	const int32_t networkID;
	const ECharacterType characterType;
	cs_sc_changeCharacterPacket(int32_t networkID, ECharacterType characterType)
		: size(sizeof(cs_sc_changeCharacterPacket))
		, type(EPacketType::cs_sc_changeCharacter)
		, networkID(networkID)
		, characterType(characterType)
	{
	}
};

struct ItemQueueInfo
{
	int32_t networkID;
	uint8_t itemQueue[MAX_USING_ITEM * BATTLE_ITEM_QUEUE_LOOP_COUNT * 2];
};

struct sc_battleInfoPacket
{
	const uint16_t size;
	const EPacketType type;
	int32_t battleOpponentQueue[MAX_ROOM_PLAYER];
	ItemQueueInfo itemQueueInfo[MAX_ROOM_PLAYER];
	sc_battleInfoPacket(const vector<int32_t>& battleOpponents, const vector<int32_t>& itemQueues)
		: size(sizeof(sc_battleInfoPacket))
		, type(EPacketType::sc_battleInfo)
	{
		log_assert(battleOpponents.size() <= MAX_ROOM_PLAYER);
		copy(battleOpponents.begin(), battleOpponents.end(), battleOpponentQueue);
		if(battleOpponents.size() < MAX_ROOM_PLAYER)
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

// 배틀 상대
struct sc_battleOpponentQueuePacket
{
	const uint16_t size;
	const EPacketType type;
	int32_t battleOpponentQueue[MAX_ROOM_PLAYER];
	sc_battleOpponentQueuePacket(const int32_t(&playerQueue)[MAX_ROOM_PLAYER])
		: size(sizeof(sc_battleOpponentQueuePacket))
		, type(EPacketType::sc_battleInfo)
	{
		::copy_n(playerQueue, _countof(playerQueue), battleOpponentQueue);
	}
};

struct cs_battleReadyPacket
{
	const uint16_t size;
	const EPacketType type;
	const int32_t networkID;
	const int16_t firstAttackState;
};

#pragma pack(pop)
