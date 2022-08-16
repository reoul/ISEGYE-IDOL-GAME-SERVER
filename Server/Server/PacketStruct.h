#pragma once
#pragma warning(disable:4996)
#include <memory>
#include "SettingData.h"
#include "Room.h"

using namespace std;

enum class CharacterType : uint8_t;

enum class PacketType : uint8_t
{
	sc_connectServer,
	sc_connectRoom,
	sc_disconnect,
	cs_startMatching,
	cs_sc_addNewItem,
	cs_sc_changeItemSlot,
	cs_sc_upgradeItem,
	cs_sc_changeCharacter,
	cs_sc_battleItemQueue,
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
	const PacketType type;
	int32_t networkID;
	sc_connectServerPacket(int network_id)
		: size(sizeof(sc_connectServerPacket))
		, type(PacketType::sc_connectServer)
		, networkID(network_id)
	{
	}
};

struct cs_startMatchingPacket
{
	const uint16_t size;
	const PacketType type;
	const int32_t networkID;
	const wchar_t name[MAX_USER_NAME_LENGTH];
};

struct sc_connectRoomPacket
{
	const uint16_t size;
	const PacketType type;
	int32_t roomNumber;
	UserInfo users[ROOM_MAX_PLAYER];
	sc_connectRoomPacket(const shared_ptr<Room>& room)
		: size(sizeof(sc_connectRoomPacket))
		, type(PacketType::sc_connectRoom)
		, roomNumber(room->GetRoomNumber())
		, users{}
	{
		const vector<Client*> clients = room->GetClients();
		for (size_t i = 0; i < clients.size(); ++i)
		{
			users[i].networkID = clients[i]->networkID;
			wcscpy(users[i].name, clients[i]->name);
		}
	}
};

struct cs_AddNewItemPacket
{
	const uint16_t size;
	const PacketType type;
	const int32_t networkID;
	const int32_t itemCode;
};

struct sc_disconnectPacket
{
	const uint16_t size;
	const PacketType type;
	int32_t networkID;
	sc_disconnectPacket(int32_t network_id)
		: size(sizeof(sc_disconnectPacket))
		, type(PacketType::sc_disconnect)
		, networkID(network_id)
	{
	}
};

struct cs_sc_changeItemSlotPacket
{
	const uint16_t size;
	const PacketType type;
	const int32_t networkID;
	const int16_t slot1;
	const int16_t slot2;
	cs_sc_changeItemSlotPacket(int32_t networkID, int16_t slot1, int16_t slot2)
		: size(sizeof(cs_sc_changeItemSlotPacket))
		, type(PacketType::cs_sc_changeItemSlot)
		, networkID(networkID)
		, slot1(slot1)
		, slot2(slot2)
	{
	}
};

struct cs_sc_upgradeItemPacket
{
	const uint16_t size;
	const PacketType type;
	const int32_t networkID;
	const int16_t slot;
	cs_sc_upgradeItemPacket(int32_t networkID, int16_t slot)
		: size(sizeof(cs_sc_upgradeItemPacket))
		, type(PacketType::cs_sc_upgradeItem)
		, networkID(networkID)
		, slot(slot)
	{
	}
};

struct cs_sc_battleInfoPacket
{
	const uint16_t size;
	const PacketType type;
	const int32_t networkID1;
	const int32_t networkID2;
	cs_sc_battleInfoPacket(int32_t networkID1, int16_t networkID2)
		: size(sizeof(cs_sc_battleInfoPacket))
		, type(PacketType::cs_sc_upgradeItem)
		, networkID1(networkID1)
		, networkID2(networkID2)
	{
	}
};

struct cs_sc_changeCharacterPacket
{
	const uint16_t size;
	const PacketType type;
	const int32_t networkID;
	const CharacterType characterType;
	cs_sc_changeCharacterPacket(int32_t networkID, CharacterType characterType)
		: size(sizeof(cs_sc_changeCharacterPacket))
		, type(PacketType::cs_sc_changeCharacter)
		, networkID(networkID)
		, characterType(characterType)
	{
	}
};

struct cs_sc_battleItemQueuePacket
{
	const uint16_t size;
	const PacketType type;
	const int32_t networkID;
	const CharacterType characterType;
	const int8_t itemQueue[30];
	cs_sc_battleItemQueuePacket(int32_t networkID, CharacterType characterType)
		: size(sizeof(cs_sc_battleItemQueuePacket))
		, type(PacketType::cs_sc_battleItemQueue)
		, networkID(networkID)
		, characterType(characterType)
		, itemQueue{}
	{
	}
};

#pragma pack(pop)
