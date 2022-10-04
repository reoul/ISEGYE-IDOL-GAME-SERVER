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
	switch (static_cast<EPacketType>(buf[2])) //[0,1]은 size
	{
	case EPacketType::cs_startMatching:
	{
		const cs_startMatchingPacket* pPacket = reinterpret_cast<cs_startMatchingPacket*>(buf);
		wcscpy(g_clients[userID].GetName(), pPacket->name);
		g_clients[userID].GetName()[MAX_USER_NAME_LENGTH - 1] = '\0';

		Room* room = nullptr;
		{
			lock_guard<mutex> lg(g_serverQueue.GetMutex());
			g_serverQueue.AddClient(&g_clients[userID]);
			room = g_serverQueue.TryCreateRoomOrNullPtr();
		}

		if (room != nullptr) // 방을 만들 수 있다면
		{
			sc_connectRoomPacket connectRoomPacket(*room);
			room->SendPacketToAllClients(&connectRoomPacket);
		}
	}
	break;
	case EPacketType::cs_sc_addNewItem:
	{
		cs_sc_AddNewItemPacket* pPacket = reinterpret_cast<cs_sc_AddNewItemPacket*>(buf);
		Client& client = g_clients[userID];
		client.AddItem(pPacket->itemCode);
		Log("[cs_sc_addNewItem] 네트워크 {0}번 클라이언트 {1}번 아이템 추가", pPacket->networkID, pPacket->itemCode);
		
		if (g_clients[userID].GetRoomPtr() != nullptr)
		{
			lock_guard<mutex> lg(client.GetRoomPtr()->cLock);
			client.SendPacketInAnotherRoomClients(pPacket);
		}
		else
		{
			log_assert(false);
		}
	}
	break;
	case EPacketType::cs_sc_changeCharacter:
	{
		cs_sc_changeCharacterPacket* pPacket = reinterpret_cast<cs_sc_changeCharacterPacket*>(buf);
		Log("[cs_sc_changeCharacter] 네트워크 {0}번 클라이언트 캐릭터 {1}번 교체", pPacket->networkID, static_cast<int>(pPacket->characterType));

		if (g_clients[userID].GetRoomPtr() != nullptr)
		{
			lock_guard<mutex> lg(g_clients[userID].GetRoomPtr()->cLock);
			g_clients[userID].SendPacketInAnotherRoomClients(pPacket);
		}
		else
		{
			log_assert(false);
		}
	}
	break;
	case EPacketType::cs_sc_changeItemSlot:
	{
		cs_sc_changeItemSlotPacket* pPacket = reinterpret_cast<cs_sc_changeItemSlotPacket*>(buf);
		g_clients[userID].SwapItem(pPacket->slot1, pPacket->slot2);
		Log("[cs_sc_changeItemSlot] 네트워크 {0}번 클라이언트 아이템 슬롯 {1} <-> {2} 교체", pPacket->networkID, pPacket->slot1, pPacket->slot2);
		
		if (g_clients[userID].GetRoomPtr() != nullptr)
		{
			lock_guard<mutex> lg(g_clients[userID].GetRoomPtr()->cLock);
			g_clients[userID].SendPacketInAllRoomClients(pPacket);
		}
		else
		{
			log_assert(false);
		}
	}
	break;
	case EPacketType::cs_battleReady:
	{
		const cs_battleReadyPacket* pPacket = reinterpret_cast<cs_battleReadyPacket*>(buf);
		Client& client = g_clients[userID];
		client.TrySetDefaultUsingItem();
		client.SetFirstAttackState(pPacket->firstAttackState);
		client.SetBattleReady(true);

		Log("[cs_battleReady] 네트워크 {0}번 클라이언트 전투 준비 완료", pPacket->networkID);
		if (client.GetRoomPtr() != nullptr)
		{
			Room& room = *client.GetRoomPtr();
			Log("{0}번 룸 전투 준비 카운트 증가 (현재 {1}/{2})", room.GetNumber(), room.GetBattleReadyCount(), room.GetSize()); // 락 안걸어서 준비 카운팅이 겹칠수 있음
		}
		else
		{
			log_assert(false);
		}
	}
	break;
	default:
		LogWarning("미정의 패킷 받음");
		DebugBreak();
		//exit(-1);
		break;
	}
}
