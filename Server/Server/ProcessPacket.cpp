﻿#include <iostream>
#include <string>
#include "PacketStruct.h"
#include "Room.h"
#include "ServerQueue.h"
#include "GlobalVariable.h"
#include "Client.h"
#include "Log.h"

void ProcessPacket(int userID, char* buf)
{
	switch (static_cast<PacketType>(buf[2])) //[0,1]은 size
	{
	case PacketType::cs_startMatching:
	{
		cs_startMatchingPacket* pPacket = reinterpret_cast<cs_startMatchingPacket*>(buf);
		wcscpy(g_clients[pPacket->networkID].GetName(), pPacket->name);
		g_clients[pPacket->networkID].GetName()[MAX_USER_NAME_LENGTH - 1] = '\0';

		g_serverQueue.Lock();
		g_serverQueue.AddClient(&g_clients[pPacket->networkID]);
		Room* room = g_serverQueue.TryCreateRoomOrNullPtr();
		g_serverQueue.UnLock();

		if (room != nullptr) // 방을 만들 수 있다면
		{
			sc_connectRoomPacket connectRoomPacket(*room);
			room->SendPacketToAllClients(&connectRoomPacket);
		}
	}
	break;
	case PacketType::cs_sc_addNewItem:
	{
		cs_sc_AddNewItemPacket* pPacket = reinterpret_cast<cs_sc_AddNewItemPacket*>(buf);
		Client& client = g_clients[pPacket->networkID];
		client.AddItem(pPacket->itemCode);
		Log("(cs_sc_addNewItem) Client{0} add {1} item", pPacket->networkID, pPacket->itemCode);
		client.SendPacketInAnotherRoomClients(pPacket);
	}
	break;
	case PacketType::cs_sc_changeCharacter:
	{
		cs_sc_changeCharacterPacket* pPacket = reinterpret_cast<cs_sc_changeCharacterPacket*>(buf);
		Log("(cs_sc_changeCharacter) Client{0} change character {1}", pPacket->networkID, static_cast<int>(pPacket->characterType));
		g_clients[pPacket->networkID].SendPacketInAnotherRoomClients(pPacket);
	}
	break;
	case PacketType::cs_sc_changeItemSlot:
	{
		cs_sc_changeItemSlotPacket* pPacket = reinterpret_cast<cs_sc_changeItemSlotPacket*>(buf);
		g_clients[pPacket->networkID].SwapItem(pPacket->slot1, pPacket->slot2);
		Log("Client{0} change slot {1} <-> {2}", pPacket->networkID, pPacket->slot1, pPacket->slot2);
		g_clients[pPacket->networkID].SendPacketInAllRoomClients(pPacket);
	}
	break;
	case PacketType::cs_battleReady:
	{
		cs_battleReadyPacket* pPacket = reinterpret_cast<cs_battleReadyPacket*>(buf);
		g_clients[pPacket->networkID].TrySetDefaultUsingItem();
		g_clients[pPacket->networkID].GetRoomPtr()->BattleReady();
		Log("Client{0} send ready battle packet", pPacket->networkID);
	}
	break;
	default:
		LogWarning("Recv Undefined Packet");
		DebugBreak();
		//exit(-1);
		break;
	}
}
