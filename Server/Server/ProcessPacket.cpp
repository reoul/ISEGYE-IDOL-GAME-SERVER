#include <iostream>
#include <string>
#include "PacketStruct.h"
#include "Room.h"
#include "ServerQueue.h"
#include "GlobalVariable.h"

void ProcessPacket(int userID, char* buf)
{
	switch (static_cast<PacketType>(buf[2])) //[0,1]은 size
	{
	case PacketType::cs_startMatching:
	{
		cs_startMatchingPacket* packet = reinterpret_cast<cs_startMatchingPacket*>(buf);
		wcscpy(g_clients[packet->networkID].name, packet->name);

		g_serverQueue.Lock();
		g_serverQueue.AddClient(&g_clients[packet->networkID]);
		shared_ptr<Room> room = g_serverQueue.TryCreateRoomOrNullPtr();
		g_serverQueue.UnLock();

		if (room != nullptr) // 방을 만들 수 있다면
		{
			sc_connectRoomPacket connectRoomPacket(room);
			room->SendAllPlayer(&connectRoomPacket);
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
		auto packet = reinterpret_cast<cs_AddNewItemPacket*>(buf);
		wcout << packet->networkID << L" 번 유저가 새로운 아이템 " << packet->itemCode << L" 을 추가하였습니다" << endl;
		g_clients[packet->networkID].room->SendAllPlayer(packet);
	}
	break;
	case PacketType::cs_sc_changeCharacter:
	{
		auto packet = reinterpret_cast<cs_sc_changeCharacterPacket*>(buf);
		cout << packet->networkID << " 번 유저가 캐릭터를 " << static_cast<int>(packet->characterType) << " 으로 변경하였습니다" << endl;
		g_clients[packet->networkID].room->SendAnotherPlayer(&g_clients[userID], packet);
	}
	break;
	case PacketType::cs_sc_changeItemSlot:
	{
		auto* packet = reinterpret_cast<cs_sc_changeItemSlotPacket*>(buf);
		cout << packet->networkID << " 번 유저가 슬롯 " << packet->slot1 << " <-> " << packet->slot2 << " 변경하였습니다" << endl;
		g_clients[packet->networkID].room->SendAllPlayer(packet);
	}
	break;
	default:
		cout << "unknown packet type error \n";
		DebugBreak();
		//exit(-1);
		break;
	}
}
