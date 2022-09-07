#include <iostream>
#include <string>
#include "PacketStruct.h"
#include "Room.h"
#include "ServerQueue.h"
#include "GlobalVariable.h"
#include "Client.h"
#include "reoul/logger.h"

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
		Log("[cs_sc_addNewItem] 네트워크 {0}번 클라이언트 {1}번 아이템 추가", pPacket->networkID, pPacket->itemCode);
		client.SendPacketInAnotherRoomClients(pPacket);
	}
	break;
	case PacketType::cs_sc_changeCharacter:
	{
		cs_sc_changeCharacterPacket* pPacket = reinterpret_cast<cs_sc_changeCharacterPacket*>(buf);
		Log("[cs_sc_changeCharacter] 네트워크 {0}번 클라이언트 캐릭터 {1}번 교체", pPacket->networkID, static_cast<int>(pPacket->characterType));
		g_clients[pPacket->networkID].SendPacketInAnotherRoomClients(pPacket);
	}
	break;
	case PacketType::cs_sc_changeItemSlot:
	{
		cs_sc_changeItemSlotPacket* pPacket = reinterpret_cast<cs_sc_changeItemSlotPacket*>(buf);
		g_clients[pPacket->networkID].SwapItem(pPacket->slot1, pPacket->slot2);
		Log("[cs_sc_changeItemSlot] 네트워크 {0}번 클라이언트 아이템 슬롯 {1} <-> {2} 교체", pPacket->networkID, pPacket->slot1, pPacket->slot2);
		g_clients[pPacket->networkID].SendPacketInAllRoomClients(pPacket);
	}
	break;
	case PacketType::cs_battleReady:
	{
		cs_battleReadyPacket* pPacket = reinterpret_cast<cs_battleReadyPacket*>(buf);
		g_clients[pPacket->networkID].TrySetDefaultUsingItem();
		g_clients[pPacket->networkID].GetRoomPtr()->BattleReady();
		Log("[cs_battleReady] 네트워크 {0}번 클라이언트 전투 준비 완료", pPacket->networkID);
	}
	break;
	default:
		LogWarning("미정의 패킷 받음");
		DebugBreak();
		//exit(-1);
		break;
	}
}
