#include <iostream>
#include <string>
#include "PacketStruct.h"
#include "Room.h"
#include "ServerQueue.h"
#include "GlobalVariable.h"
#include "Client.h"

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
			room->SendAllClient(&connectRoomPacket);
		}

		/*cs_packet_login* packet = reinterpret_cast<cs_packet_login*>(buf);
		strcpy_s(g_clients[userID].name, packet->name);
		g_clients[userID].name[MAX_ID_LEN] = NULL;
		send_login_ok_packet(userID);
		enter_game(userID);*/
	}
	break;
	case PacketType::cs_sc_addNewItem:
	{
		cs_AddNewItemPacket* pPacket = reinterpret_cast<cs_AddNewItemPacket*>(buf);
		wcout << pPacket->networkID << L" 번 유저가 새로운 아이템 " << pPacket->itemCode << L" 을 추가하였습니다" << endl;
		g_clients[pPacket->networkID].AddItem(pPacket->itemCode);
		g_clients[pPacket->networkID].GetRoomPtr()->SendAllClient(pPacket);
	}
	break;
	case PacketType::cs_sc_changeCharacter:
	{
		cs_sc_changeCharacterPacket* pPacket = reinterpret_cast<cs_sc_changeCharacterPacket*>(buf);
		wcout << pPacket->networkID << L" 번 유저가 캐릭터를 " << static_cast<int>(pPacket->characterType) << L" 으로 변경하였습니다" << endl;
		g_clients[pPacket->networkID].GetRoomPtr()->SendAnotherClient(g_clients[userID], pPacket);
	}
	break;
	case PacketType::cs_sc_changeItemSlot:
	{
		cs_sc_changeItemSlotPacket* pPacket = reinterpret_cast<cs_sc_changeItemSlotPacket*>(buf);
		wcout << pPacket->networkID << L" 번 유저가 슬롯 " << pPacket->slot1 << L" <-> " << pPacket->slot2 << L" 변경하였습니다" << endl;
		g_clients[pPacket->networkID].SwapItem(pPacket->slot1, pPacket->slot2);
		g_clients[pPacket->networkID].GetRoomPtr()->SendAllClient(pPacket);
	}
	break;
	case PacketType::cs_battleReady:
	{
		cs_battleReadyPacket* pPacket = reinterpret_cast<cs_battleReadyPacket*>(buf);
		g_clients[pPacket->networkID].GetRoomPtr()->BattleReady();
		//g_clients[pPacket->networkID].GetRoomPtr()->mBattleReadyCount++;
		//g_roomManager.mRooms[0].mBattleReadyCount++;
		cout << "출력" << endl;
	}
	break;
	default:
		cout << "unknown packet type error \n";
		DebugBreak();
		//exit(-1);
		break;
	}
}
