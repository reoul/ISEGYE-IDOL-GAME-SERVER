#include "Server.h"
#include <MSWSock.h>

#include "ItemBase.h"
#include "Items.h"
#include "PacketStruct.h"

HANDLE Server::sIocp;
SOCKET Server::sListenSocket;
Client Server::sClients[MAX_USER];
RoomManager Server::sRoomManager;
bool Server::sIsRunningServer = true;
ServerQueue Server::sServerQueue;
size_t connectCount = 0;


void Server::Start()
{
	for (int i = 0; i < MAX_USER; ++i)
	{
		sClients[i].SetNetworkID(i);
	}

	sIocp = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, NULL, 0);
	sListenSocket = ::WSASocketW(AF_INET, SOCK_STREAM, 0, nullptr, 0, WSA_FLAG_OVERLAPPED);
	if (sListenSocket == INVALID_SOCKET)
	{
		Log("log", "Listen 소켓 생성 실패");
	}

	sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	CreateIoCompletionPort(reinterpret_cast<HANDLE>(sListenSocket), sIocp, 9957, 0);

	if (::bind(sListenSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
		Log("log", "바인드 에러");
		::closesocket(sListenSocket);
		assert(false);
	}

	if (::listen(sListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		Log("log", "Listen 에러");
		::closesocket(sListenSocket);
		assert(false);
	}

	SOCKET clientSocket;
	clientSocket = ::WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	Exover accept_over;
	accept_over.c_socket = clientSocket;
	ZeroMemory(&accept_over, sizeof(accept_over.over));
	accept_over.type = EOperationType::Accept;
	::AcceptEx(sListenSocket, clientSocket, accept_over.io_buf, NULL, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, NULL, &accept_over.over);
	Log("log", "서버 시작");

	vector<thread> workerThreads;
	for (ULONG i = 0; i < std::thread::hardware_concurrency(); ++i)
	{
		workerThreads.emplace_back(WorkerThread);
	}
	Log("log", "{0}개의 쓰레드 작동", std::thread::hardware_concurrency());

	system_clock::time_point lastConnectCheckTime = system_clock::now();
	constexpr milliseconds checkIntervalTime(CONNECT_CHECK_INTERVAL * 1000);

	while (sIsRunningServer)
	{
		Sleep(CONNECT_CHECK_INTERVAL * 1000);
		int connCount = 0;
		lastConnectCheckTime = system_clock::now();
		for (Client& client : sClients)
		{
			if (client.GetStatus() == ESocketStatus::ACTIVE)
			{
				if (!client.IsValidConnect())
				{
					Log("log", "{0}번 클라이언트 연결 불안으로 접속 해제", client.GetNetworkID());
					Disconnect(client.GetNetworkID(), true);
					continue;
				}
				++connCount;
			}
		}
		LogByRelease("Connection", "현재 유저 : {0}명", connCount);
	}

	for (auto& th : workerThreads)
	{
		th.join();
	}

	WSACleanup();
}

void Server::WorkerThread()
{
	while (true)
	{
		DWORD io_byte;
		ULONG_PTR key;
		WSAOVERLAPPED* over;
		bool sucess = ::GetQueuedCompletionStatus(sIocp, &io_byte, &key, &over, 1000);
		DWORD errorCode = ::GetLastError();

		// 서버가 끝낸 상태
		if (!sIsRunningServer)
		{
			break;
		}

		// 타임아웃인 경우
		if (sucess == FALSE && over == nullptr)
		{
			continue;
		}

		Exover* exover = reinterpret_cast<Exover*>(over);
		int userID = static_cast<int>(key);

		switch (exover->type)
		{
		case EOperationType::Recv:			//받은 패킷 처리 -> overlapped구조체 초기화 -> recv
		{
			if (0 == io_byte)
			{
				Disconnect(userID, true);

				if (EOperationType::Send == exover->type)
					delete exover;
			}
			else
			{
				Client& client = sClients[userID];
				PacketConstruct(userID, io_byte);
				ZeroMemory(&client.GetRecvOver().over, sizeof(client.GetRecvOver().over));
				DWORD flags = 0;
				::WSARecv(client.GetSocket(), &client.GetRecvOver().wsabuf, 1, NULL, &flags, &client.GetRecvOver().over, NULL);
			}
			break;
		}
		case EOperationType::Send:			//구조체 delete
			if (0 == io_byte)
			{
				Disconnect(userID, true);
			}

			LogWrite("PacketSendRecive", "네트워크 {0}번 클라이언트 {1}Byte 패킷 전송", userID, io_byte);
			delete exover;
			break;
		case EOperationType::Accept:			//CreateIoCompletionPort으로 클라소켓 iocp에 등록 -> 초기화 -> recv -> accept 다시(다중접속)
		{
			int userID = -1;
			for (int i = 0; i < MAX_USER; ++i)
			{
				lock_guard<mutex> gl{ sClients[i].GetMutex() }; //이렇게 하면 unlock이 필요 없다. 이 블록에서 빠져나갈때 unlock을 자동으로 해준다.
				if (ESocketStatus::FREE == sClients[i].GetStatus())
				{
					sClients[i].SetStatus(ESocketStatus::ALLOCATED);
					userID = i;
					break;
				}
			}
			//main에서 소켓을 worker스레드로 옮겨오기 위해 listen소켓은 전역변수로, client소켓은 멤버로 가져왔다.
			SOCKET clientSocket = exover->c_socket;

			if (userID == -1)
				::closesocket(clientSocket); // send_login_fail_packet();
			else
			{
				const HANDLE result = ::CreateIoCompletionPort(reinterpret_cast<HANDLE>(clientSocket), sIocp, userID, 0);
				if (result == NULL)
				{
					LogWarning("log", "{0}번 소켓 IOCP 등록 실패", userID);
					::closesocket(clientSocket);
				}
				else
				{
					sClients[userID].SetPrevSize(0); //이전에 받아둔 조각이 없으니 0
					sClients[userID].SetSocket(clientSocket);

					ZeroMemory(&sClients[userID].GetRecvOver().over, sizeof(sClients[userID].GetRecvOver().over));
					sClients[userID].GetRecvOver().type = EOperationType::Recv;
					sClients[userID].GetRecvOver().wsabuf.buf = sClients[userID].GetRecvOver().io_buf;
					sClients[userID].GetRecvOver().wsabuf.len = MAX_BUF_SIZE;

					// 접속 ip 계산 로직
					sockaddr* lpLocalSockaddr = nullptr, * lpRemoteSockaddr = nullptr;
					int localSockaddrLen = 0, remoteSockaddrLen = 0;
					LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExSockaddrs = nullptr;
					GetAcceptExSockaddrs(exover->io_buf, NULL, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, &lpLocalSockaddr
						, &localSockaddrLen, &lpRemoteSockaddr, &remoteSockaddrLen);

					char ipAdress[16]{};
					sprintf(ipAdress, "%d.%d.%d.%d", *reinterpret_cast<uint8_t*>(&lpRemoteSockaddr->sa_data[2]), *reinterpret_cast<uint8_t*>(&lpRemoteSockaddr->sa_data[3]), *reinterpret_cast<uint8_t*>(&lpRemoteSockaddr->sa_data[4]), *reinterpret_cast<uint8_t*>(&lpRemoteSockaddr->sa_data[5]));

					NewClientEvent(userID, ipAdress);

					sClients[userID].SetLastConnectCheckPacketTime(system_clock::now());

					sClients[userID].SetStatus(ESocketStatus::ACTIVE);

					DWORD flags = 0;
					::WSARecv(clientSocket, &sClients[userID].GetRecvOver().wsabuf, 1, NULL, &flags, &sClients[userID].GetRecvOver().over, NULL);
				}
			}
			//소켓 초기화 후 다시 accept
			clientSocket = ::WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			exover->c_socket = clientSocket; //새로 받는 소켓을 넣어준다. 안그러면 클라들이 같은 소켓을 공유한다.
			ZeroMemory(&exover->over, sizeof(exover->over)); //accept_over를 exover라는 이름으로 받았으니 exover를 재사용
			::AcceptEx(sListenSocket, clientSocket, exover->io_buf, NULL,
				sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, NULL, &exover->over);
			break;
		}
		}
	}
}

void Server::NewClientEvent(int networkID, char* ipAdress)
{
	Log("log", "네트워크 {0}번 클라이언트 서버 접속 (ip주소: {1}) (소켓 번호 : {2})", networkID, ipAdress, sClients[networkID].GetSocket() / 4);
	if ((connectCount++ % 100) == 0)
	{
		LogByRelease("ConnectionCount", "누적 접속 횟수 : {0}", connectCount);
	}
	cs_sc_NotificationPacket packet(networkID, ENotificationType::ConnectServer);
	SendPacket(networkID, &packet);
}

void Server::Disconnect(int networkID, bool isSendAnotherRoomClient)
{
	if (sClients[networkID].GetStatus() == ESocketStatus::FREE)
	{
		return;
	}

	{
		lock_guard<mutex> lg(sServerQueue.GetMutex());
		sServerQueue.RemoveClient(sClients[networkID]);
	}

	Log("log", "네트워크 {0}번 클라이언트 서버 접속 해제", networkID);

	if (isSendAnotherRoomClient)
	{
		cs_sc_NotificationPacket packet(networkID, ENotificationType::DisconnectServer);
		sClients[networkID].SendPacketInAnotherRoomClients(&packet);
	}

	{
		lock_guard<mutex> lg(sClients[networkID].GetMutex());
		sClients[networkID].SetStatus(ESocketStatus::ALLOCATED);	//처리 되기 전에 FREE하면 아직 떠나는 뒷처리가 안됐는데 새 접속을 받을 수 있음

		::closesocket(sClients[networkID].GetSocket());

		sClients[networkID].Init();
	}

}

/**
 * \brief 들어온 패킷을 가공
 * \param networkID 유저 ID
 * \param ioByteLength 패킷 데이터 길이
 */
void Server::PacketConstruct(int networkID, int ioByteLength)
{
	Client& curUser = sClients[networkID];
	Exover& recvOver = curUser.GetRecvOver();

	int restByte = ioByteLength;		//이만큼 남은걸 처리해줘야 한다
	char* p = recvOver.io_buf;			//처리해야할 데이터의 포인터가 필요하다
	int packetSize = 0;				//이게 0이라는 것은 이전에 처리하던 패킷이 없다는 것 

	// 이전에 받아둔 패킷이 있다면
	if (curUser.GetPrevSize() != 0)
	{
		packetSize = reinterpret_cast<uint16_t*>(curUser.GetPacketBuf())[0]; //재조립을 기다기는 패킷 사이즈
	}

	LogWrite("PacketSendRecive", "네트워크 {0}번 클라이언트 {1}Byte 패킷 받음", networkID, ioByteLength);

	while (restByte > 0)	//처리해야할 데이터가 남아있으면 처리해야한다.
	{
		// 이전에 처리해야할 패킷이 없다면
		if (0 == packetSize)
		{
			// 지금 들어온 패킷의 사이즈를 넣는다
			packetSize = reinterpret_cast<uint16_t*>(p)[0];
		}

		// 나머지 데이터로 패킷을 만들 수 있나 없나 확인
		// 지금 처리해야 하는 패킷의 사이즈가 남은 데이터랑 기존에 더한것보다 작다면(즉 패킷 사이즈 만큼 채울수 있는 데이터가 있다면)
		if (packetSize <= restByte + curUser.GetPrevSize())
		{
			//만들어서 처리한 데이터 크기만큼 패킷 사이즈에서 빼주기
			memcpy(curUser.GetPacketBuf() + curUser.GetPrevSize(), p, packetSize - curUser.GetPrevSize());

			p += packetSize - curUser.GetPrevSize();
			restByte -= packetSize - curUser.GetPrevSize();
			packetSize = 0;														//이 패킷은 이미 처리를 했고 다음 패킷 사이즈는 모름.

			ProcessPacket(networkID, curUser.GetPacketBuf());

			curUser.SetPrevSize(0);
		}
		else	//패킷 하나를 만들 수 없다면 버퍼에 복사해두고 포인터와 사이즈 증가
		{
			memcpy(curUser.GetPacketBuf() + curUser.GetPrevSize(), p, restByte); //남은 데이터 몽땅 받는데, 지난번에 받은 데이터가 남아있을 경우가 있으니, 그 뒤에 받아야한다.
			curUser.SetPrevSize(curUser.GetPrevSize() + restByte);
			restByte = 0;
			p += restByte;
		}
	}
}

void Server::SendDisconnect(int networkID)
{
	if (sClients[networkID].GetStatus() != ESocketStatus::ACTIVE)
	{
		return;
	}
	cs_sc_NotificationPacket packet(networkID, ENotificationType::DisconnectServer);
	sClients[networkID].SendPacketInAnotherRoomClients(&packet);
	Log("log", "네트워크 {0}번 클라이언트 접속 해제 패킷 보냄", networkID);
}

void Server::SendDisconnectDelay(int networkID)
{
	if (sClients[networkID].GetStatus() != ESocketStatus::ACTIVE)
	{
		return;
	}

	cs_sc_NotificationPacket packet(networkID, ENotificationType::DisconnectServer);
	sClients[networkID].SendPacketInAllRoomClients(&packet);

	Sleep(1000);
	Disconnect(networkID, false);

	Log("log", "네트워크 {0}번 클라이언트 접속 해제 패킷 보냄", networkID);
}

void Server::ProcessPacket(int networkID, char* buf)
{
	const EPacketType packetType = static_cast<EPacketType>(buf[2]);  //[0,1]은 size

	switch (packetType)
	{
	case EPacketType::cs_startMatching:
	{
		if (sClients[networkID].GetRoomPtr() != nullptr)	// 방이 있으면 매칭을 돌리면 안됨
		{
			return;
		}

		const cs_StartMatchingPacket* pPacket = reinterpret_cast<cs_StartMatchingPacket*>(buf);
		wcscpy(sClients[networkID].GetName(), pPacket->name);
		sClients[networkID].GetName()[MAX_USER_NAME_LENGTH - 1] = '\0';

		Room* pRoom = nullptr;
		{
			lock_guard<mutex> lg(sServerQueue.GetMutex());
			sServerQueue.AddClient(sClients[networkID]);
			pRoom = sServerQueue.TryCreateRoomOrNullPtr();
		}

		if (pRoom != nullptr) // 방을 만들 수 있다면
		{
			{
				constexpr size_t bufferSize = sizeof(sc_SetChoiceCharacterTimePacket) + sizeof(sc_ConnectRoomPacket);
				OutputMemoryStream memoryStream(bufferSize);

				const sc_SetChoiceCharacterTimePacket setChoiceCharacterTimePacket(CHOICE_CHARACTER_TIME);
				setChoiceCharacterTimePacket.Write(memoryStream);
				const sc_ConnectRoomPacket connectRoomPacket(*pRoom);
				connectRoomPacket.Write(memoryStream);

				pRoom->SendPacketToAllClients(memoryStream.GetBufferPtr(), bufferSize);
			}

			unsigned threadID;
			const HANDLE hThread = (HANDLE)_beginthreadex(nullptr, 0, &Room::ProgressThread, pRoom, 0, &threadID);
			CloseHandle(hThread);
		}
	}
	break;
	case EPacketType::cs_sc_changeCharacter:
	{
		cs_sc_ChangeCharacterPacket* pPacket = reinterpret_cast<cs_sc_ChangeCharacterPacket*>(buf);

		const Room* room = sClients[pPacket->networkID].GetRoomPtr();
		if (room == nullptr || room->GetCurRoomStatusType() != ERoomStatusType::ChoiceCharacter)
		{
			return;
		}

		Log("log", "[cs_sc_changeCharacter] 네트워크 {0}번 클라이언트 캐릭터 {1}번 교체", pPacket->networkID, static_cast<uint8_t>(pPacket->characterType.get()));

		if (sClients[networkID].GetRoomPtr() != nullptr)
		{
			lock_guard<mutex> lg(sClients[networkID].GetRoomPtr()->cLock);
			sClients[networkID].SetCharacterType(pPacket->characterType);
			sClients[networkID].SendPacketInAllRoomClients(pPacket);
		}
		else
		{
			log_assert(false);
		}
	}
	break;
	case EPacketType::cs_sc_changeItemSlot:
	{
		cs_sc_ChangeItemSlotPacket* pPacket = reinterpret_cast<cs_sc_ChangeItemSlotPacket*>(buf);

		const Room* room = sClients[pPacket->networkID].GetRoomPtr();
		if (room == nullptr)
		{
			return;
		}

		if (room->GetCurRoomStatusType() != ERoomStatusType::ReadyStage)
		{
			lock_guard<mutex> lg(sClients[networkID].GetRoomPtr()->cLock);
			sc_InventoryInfoPacket inventoryInfoPacket(sClients[pPacket->networkID]);
			sClients[networkID].SendPacketInAllRoomClients(&inventoryInfoPacket);
			return;
		}

		sClients[networkID].SwapItem(pPacket->slot1, pPacket->slot2);
		Log("log", "[cs_sc_changeItemSlot] 네트워크 {0}번 클라이언트 아이템 슬롯 {1} <-> {2} 교체", pPacket->networkID, pPacket->slot1, pPacket->slot2);

		lock_guard<mutex> lg(sClients[networkID].GetRoomPtr()->cLock);
		sc_InventoryInfoPacket inventoryInfoPacket(sClients[pPacket->networkID]);
		sClients[networkID].SendPacketInAllRoomClients(&inventoryInfoPacket);
	}
	break;
	case EPacketType::cs_sc_useEmoticon:
	{
		const Room* room = sClients[networkID].GetRoomPtr();
		if (room == nullptr)
		{
			return;
		}

		const cs_sc_UseEmoticonPacket* pPacket = reinterpret_cast<cs_sc_UseEmoticonPacket*>(buf);
		Log("log", "[cs_sc_useEmoticon] 네트워크 {0}번 클라이언트 {1}번 이모티콘 사용", pPacket->networkID, pPacket->emoticonType);
		sClients[networkID].SendPacketInAllRoomClients(buf);
	}
	break;
	case EPacketType::cs_sc_notification:
	{
		cs_sc_NotificationPacket* pPacket = reinterpret_cast<cs_sc_NotificationPacket*>(buf);
		const Room* room = sClients[pPacket->networkID].GetRoomPtr();
		switch (pPacket->notificationType.get())
		{
		case ENotificationType::ChoiceCharacter:
		{
			if (room == nullptr || room->GetCurRoomStatusType() != ERoomStatusType::ChoiceCharacter)
			{
				return;
			}

			Client& client = sClients[pPacket->networkID];
			client.SetChoiceCharacter(true);

			client.SendPacketInAllRoomClients(pPacket);
			Log("log", "[ENotificationType::ChoiceCharacter] 네트워크 {0}번 클라이언트 캐릭터 확정", pPacket->networkID);
		}
		break;
		case ENotificationType::ConnectCheck:
			sClients[pPacket->networkID].SetLastConnectCheckPacketTime(system_clock::now());
			LogWrite("PacketSendRecive", "[ENotificationType::ConnectCheck] 네트워크 {0}번 클라이언트 연결 확인", pPacket->networkID);
			break;
		case ENotificationType::UseNormalItemTicket:
		{
			if (room == nullptr || room->GetCurRoomStatusType() != ERoomStatusType::ReadyStage)
			{
				return;
			}

			Client& client = sClients[pPacket->networkID];
			const uint8_t newItemType = client.GetRandomItemTypeOfNormalItemTicket();
			if (newItemType != EMPTY_ITEM)
			{
				OutputMemoryStream memoryStream;
				uint8_t slot = client.AddItem(newItemType);

				sc_InventoryInfoPacket inventoryInfoPacket(sClients[pPacket->networkID]);
				inventoryInfoPacket.Write(memoryStream);

				client.SetNormalItemTicketCount(client.GetNormalItemTicketCount() - 1);
				sc_SetItemTicketPacket setItemTicketPacket(client.GetNetworkID(), EItemTicketType::Normal, client.GetNormalItemTicketCount());
				setItemTicketPacket.Write(memoryStream);

				room->SendPacketToAllClients(memoryStream.GetBufferPtr(), memoryStream.GetLength());

				Log("log", "[ENotificationType::UseNormalItemTicket] 네트워크 {0}번 클라이언트 일반 뽑기권 요청 / {1} 아이템 지급 / 현재 {2}개", pPacket->networkID, newItemType, client.GetNormalItemTicketCount());
			}
			else
			{
				Log("log", "[ENotificationType::UseNormalItemTicket] 네트워크 {0}번 클라이언트 일반 뽑기권 개수 부족, 현재 {1}개", pPacket->networkID, client.GetNormalItemTicketCount());
			}
		}
		break;
		case ENotificationType::UseAdvancedItemTicket:
		{
			if (room == nullptr || room->GetCurRoomStatusType() != ERoomStatusType::ReadyStage)
			{
				return;
			}

			Client& client = sClients[pPacket->networkID];
			const uint8_t newItemType = client.GetRandomItemTypeOfAdvancedItemTicket();
			if (newItemType != EMPTY_ITEM)
			{
				OutputMemoryStream memoryStream;
				uint8_t slot = client.AddItem(newItemType);

				sc_InventoryInfoPacket inventoryInfoPacket(sClients[pPacket->networkID]);
				inventoryInfoPacket.Write(memoryStream);

				client.SetAdvancedItemTicketCount(client.GetAdvancedItemTicketCount() - 1);
				sc_SetItemTicketPacket setItemTicketPacket(client.GetNetworkID(), EItemTicketType::Advanced, client.GetAdvancedItemTicketCount());
				setItemTicketPacket.Write(memoryStream);

				room->SendPacketToAllClients(memoryStream.GetBufferPtr(), memoryStream.GetLength());

				Log("log", "[ENotificationType::UseAdvancedItemTicket] 네트워크 {0}번 클라이언트 고급 뽑기권 요청 / {1} 아이템 지급 / 현재 {2}개", pPacket->networkID, newItemType, client.GetAdvancedItemTicketCount());
			}
			else
			{
				Log("log", "[ENotificationType::UseAdvancedItemTicket] 네트워크 {0}번 클라이언트 고급 뽑기권 개수 부족, 현재 {1}개", pPacket->networkID, client.GetAdvancedItemTicketCount());
			}
		}
		break;
		case ENotificationType::UseTopItemTicket:
		{
			if (room == nullptr || room->GetCurRoomStatusType() != ERoomStatusType::ReadyStage)
			{
				return;
			}

			Client& client = sClients[pPacket->networkID];
			const uint8_t newItemType = client.GetRandomItemTypeOfTopItemTicket();
			if (newItemType != EMPTY_ITEM)
			{
				OutputMemoryStream memoryStream;
				uint8_t slot = client.AddItem(newItemType);

				sc_InventoryInfoPacket inventoryInfoPacket(sClients[pPacket->networkID]);
				inventoryInfoPacket.Write(memoryStream);

				client.SetTopItemTicketCount(client.GetTopItemTicketCount() - 1);
				sc_SetItemTicketPacket setItemTicketPacket(client.GetNetworkID(), EItemTicketType::Top, client.GetTopItemTicketCount());
				setItemTicketPacket.Write(memoryStream);

				room->SendPacketToAllClients(memoryStream.GetBufferPtr(), memoryStream.GetLength());

				Log("log", "[ENotificationType::UseTopItemTicket] 네트워크 {0}번 클라이언트 최고급 뽑기권 요청 / {1} 아이템 지급 / 현재 {2}개", pPacket->networkID, newItemType, client.GetTopItemTicketCount());
			}
			else
			{
				Log("log", "[ENotificationType::UseTopItemTicket] 네트워크 {0}번 클라이언트 최고급 뽑기권 개수 부족, 현재 {1}개", pPacket->networkID, client.GetTopItemTicketCount());
			}
		}
		break;
		case ENotificationType::UseSupremeItemTicket:
		{
			if (room == nullptr || room->GetCurRoomStatusType() != ERoomStatusType::ReadyStage)
			{
				return;
			}

			Client& client = sClients[pPacket->networkID];
			const uint8_t newItemType = client.GetRandomItemTypeOfSupremeItemTicket();
			if (newItemType != EMPTY_ITEM)
			{
				OutputMemoryStream memoryStream;
				uint8_t slot = client.AddItem(newItemType);

				sc_InventoryInfoPacket inventoryInfoPacket(sClients[pPacket->networkID]);
				inventoryInfoPacket.Write(memoryStream);

				client.SetSupremeItemTicketCount(client.GetSupremeItemTicketCount() - 1);
				sc_SetItemTicketPacket setItemTicketPacket(client.GetNetworkID(), EItemTicketType::Supreme, client.GetSupremeItemTicketCount());
				setItemTicketPacket.Write(memoryStream);

				room->SendPacketToAllClients(memoryStream.GetBufferPtr(), memoryStream.GetLength());

				Log("log", "[ENotificationType::UseSupremeItemTicket] 네트워크 {0}번 클라이언트 지존 뽑기권 요청 / {1} 아이템 지급 / 현재 {2}개", pPacket->networkID, newItemType, client.GetSupremeItemTicketCount());
			}
			else
			{
				Log("log", "[ENotificationType::UseSupremeItemTicket] 네트워크 {0}번 클라이언트 지존 뽑기권 개수 부족, 현재 {1}개", pPacket->networkID, client.GetSupremeItemTicketCount());
			}
		}
		break;
		case ENotificationType::ChoiceAllCharacter:
		case ENotificationType::ConnectServer:
		case ENotificationType::DisconnectServer:
		case ENotificationType::EnterReadyStage:
		case ENotificationType::EnterCutSceneStage:
		case ENotificationType::FinishCutSceneStage:
		case ENotificationType::EnterBattleStage:
		case ENotificationType::EnterCreepStage:
		case ENotificationType::FinishChoiceCharacterTime:
		case ENotificationType::InitBattleSlot:
		case ENotificationType::SetDefaultUsingItem:
			LogWarning("log", "[ENotificationType::{0}] 받으면 안되는 패킷을 받음", static_cast<int>(pPacket->notificationType.get()));
			break;
		default:
			assert(false);
			break;
		}
	}
	break;
	case EPacketType::cs_sc_dropItem:
	{
		cs_sc_DropItemPacket* pPacket = reinterpret_cast<cs_sc_DropItemPacket*>(buf);

		const Room* room = sClients[pPacket->networkID].GetRoomPtr();
		if (room == nullptr)
		{
			return;
		}

		if (room->GetCurRoomStatusType() != ERoomStatusType::ReadyStage)
		{
			sc_InventoryInfoPacket inventoryInfoPacket(sClients[pPacket->networkID]);
			sClients[networkID].SendPacketInAllRoomClients(&inventoryInfoPacket);
			return;
		}

		Item& item = sClients[pPacket->networkID].GetItem(pPacket->index);

		if (item.GetType() == EMPTY_ITEM)
		{
			sc_InventoryInfoPacket inventoryInfoPacket(sClients[pPacket->networkID]);
			sClients[networkID].SendPacketInAllRoomClients(&inventoryInfoPacket);
			return;
		}

		item.SetEmptyItem();


		sc_InventoryInfoPacket inventoryInfoPacket(sClients[pPacket->networkID]);
		sClients[networkID].SendPacketInAllRoomClients(&inventoryInfoPacket);

		LogWrite("log", "[EPacketType::cs_sc_dropItem] 네트워크 {0}번 클라이언트 {1}번 아이템 버림}", pPacket->networkID, pPacket->index.get());
	}
	break;
	case EPacketType::cs_requestCombinationItem:
	{
		cs_RequestCombinationItemPacket* pPacket = reinterpret_cast<cs_RequestCombinationItemPacket*>(buf);
		Client& client = sClients[pPacket->networkID];

		const Room* room = sClients[pPacket->networkID].GetRoomPtr();
		if (room == nullptr)
		{
			return;
		}

		if (room->GetCurRoomStatusType() != ERoomStatusType::ReadyStage)
		{
			sc_InventoryInfoPacket inventoryInfoPacket(sClients[pPacket->networkID]);
			sClients[networkID].SendPacketInAllRoomClients(&inventoryInfoPacket);
		}

		Item& item1 = client.GetItem(pPacket->index1);
		Item& item2 = client.GetItem(pPacket->index2);
		Item& item3 = client.GetItem(pPacket->index3);

		if (item1.GetType() == EMPTY_ITEM || item2.GetType() == EMPTY_ITEM || item3.GetType() == EMPTY_ITEM)
		{
			sc_InventoryInfoPacket inventoryInfoPacket(sClients[pPacket->networkID]);
			sClients[networkID].SendPacketInAllRoomClients(&inventoryInfoPacket);
			return;
		}

		uint8_t tier1 = static_cast<uint8_t>(sItems[item1.GetType()]->TIER_TYPE);
		uint8_t tier2 = static_cast<uint8_t>(sItems[item2.GetType()]->TIER_TYPE);
		uint8_t tier3 = static_cast<uint8_t>(sItems[item3.GetType()]->TIER_TYPE);

		const EItemTierType maxTier = static_cast<EItemTierType>(max(max(tier1, tier2), tier3));

		item1.SetEmptyItem();
		item2.SetEmptyItem();
		item3.SetEmptyItem();

		uint8_t newItemType = client.GetRandomItemTypeByCombination(maxTier);
		uint8_t findEmptyItemSlot = client.FindEmptyItemSlotIndex();

		Item& newItem = client.GetItem(findEmptyItemSlot);
		newItem.SetType(newItemType);
		newItem.SetUpgrade(1);

		sc_InventoryInfoPacket inventoryInfoPacket(sClients[pPacket->networkID]);
		sClients[networkID].SendPacketInAllRoomClients(&inventoryInfoPacket);
		Log("log", "cs_requestCombinationItem 재조합 완료 {0}번 클라이언트 {1}슬롯 {2} 아이템 {3} 강화", pPacket->networkID, newItem.GetSlot(), newItem.GetType(), newItem.GetUpgrade());
	}
	break;
	case EPacketType::cs_requestUpgradeItem:
	{
		cs_RequestUpgradeItemPacket* pPacket = reinterpret_cast<cs_RequestUpgradeItemPacket*>(buf);

		Log("log", "cs_RequestUpgradeItemPacket {0}번 클라이언트 {1}/{2}", pPacket->networkID, pPacket->slot1, pPacket->slot2);

		const Room* room = sClients[pPacket->networkID].GetRoomPtr();
		if (room == nullptr)
		{
			return;
		}

		if (room->GetCurRoomStatusType() != ERoomStatusType::ReadyStage)
		{
			sc_InventoryInfoPacket inventoryInfoPacket(sClients[pPacket->networkID]);
			sClients[networkID].SendPacketInAllRoomClients(&inventoryInfoPacket);
			return;
		}

		Client& client = sClients[pPacket->networkID];
		Item& upgradeItem = client.GetItem(pPacket->slot1);
		Item& materialItem = client.GetItem(pPacket->slot2);

		if (pPacket->slot1 == pPacket->slot2)
		{
			Log("log", "cs_RequestUpgradeItemPacket pPacket->slot1 == pPacket->slot2");
			sc_InventoryInfoPacket inventoryInfoPacket(sClients[pPacket->networkID]);
			sClients[networkID].SendPacketInAllRoomClients(&inventoryInfoPacket);
			return;
		}

		if (upgradeItem.GetType() != materialItem.GetType())
		{
			Log("log", "cs_RequestUpgradeItemPacket upgradeItem.GetType() != materialItem.GetType()");
			Log("log", "{0}번 자리 : {1}번 아이템, {2}번 자리 : {3}번 아이템", upgradeItem.GetSlot(), upgradeItem.GetType(), materialItem.GetSlot(), materialItem.GetType());
			sc_InventoryInfoPacket inventoryInfoPacket(sClients[pPacket->networkID]);
			sClients[networkID].SendPacketInAllRoomClients(&inventoryInfoPacket);
			return;
		}

		if (upgradeItem.GetType() == 0)
		{
			Log("log", "cs_RequestUpgradeItemPacket upgradeItem.GetType() == 0");
			sc_InventoryInfoPacket inventoryInfoPacket(sClients[pPacket->networkID]);
			sClients[networkID].SendPacketInAllRoomClients(&inventoryInfoPacket);
			return;
		}

		if (upgradeItem.GetUpgrade() != materialItem.GetUpgrade())
		{
			Log("log", "cs_RequestUpgradeItemPacket upgradeItem.GetUpgrade() != materialItem.GetUpgrade()");
			Log("log", "{0}번 자리 : {1}강화, {2}번 자리 : {3}강화", upgradeItem.GetSlot(), upgradeItem.GetUpgrade(), materialItem.GetSlot(), materialItem.GetUpgrade());
			sc_InventoryInfoPacket inventoryInfoPacket(sClients[pPacket->networkID]);
			sClients[networkID].SendPacketInAllRoomClients(&inventoryInfoPacket);
			return;
		}

		if (upgradeItem.GetUpgrade() >= 2)
		{
			Log("log", "cs_RequestUpgradeItemPacket upgradeItem.GetUpgrade() >= 2");
			sc_InventoryInfoPacket inventoryInfoPacket(sClients[pPacket->networkID]);
			sClients[networkID].SendPacketInAllRoomClients(&inventoryInfoPacket);
			return;
		}

		uint8_t upgrade = upgradeItem.GetUpgrade() + 1;
		upgradeItem.SetUpgrade(upgrade);
		materialItem.SetEmptyItem();

		sc_InventoryInfoPacket inventoryInfoPacket(sClients[pPacket->networkID]);
		sClients[networkID].SendPacketInAllRoomClients(&inventoryInfoPacket);
		Log("log", "cs_RequestUpgradeItemPacket 업그레이드 완료 {0}번 클라이언트 {1}슬롯 {2}수치", pPacket->networkID, pPacket->slot1, upgrade);
	}
	break;
	case EPacketType::sc_addNewItem:
	case EPacketType::sc_connectRoom:
	case EPacketType::sc_battleInfo:
	case EPacketType::sc_updateCharacterInfo:
	case EPacketType::sc_setChoiceCharacterTime:
	case EPacketType::sc_setReadyTime:
	case EPacketType::sc_setItemTicket:
	case EPacketType::sc_activeItem:
	case EPacketType::sc_fadeIn:
	case EPacketType::sc_fadeOut:
	case EPacketType::sc_battleOpponents:
	case EPacketType::sc_upgradeItem:
	case EPacketType::sc_setHamburgerType:
	case EPacketType::sc_magicStickInfo:
	case EPacketType::sc_DoctorToolInfo:
	case EPacketType::sc_CreepStageInfo:
		LogWarning("log", "{0} 받으면 안되는 패킷을 받음", static_cast<int>(packetType));
		break;
	default:
		LogWarning("log", "미정의 패킷 받음");
		DebugBreak();
		//exit(-1);
		break;
	}
}

void Server::SendPacket(int networkID, void* pPacket)
{
	char* buf = static_cast<char*>(pPacket);

	const Client& client = sClients[networkID];

	//WSASend의 두번째 인자의 over는 recv용이라 쓰면 안된다. 새로 만들어야 한다.
	Exover* exover = new Exover;
	exover->type = EOperationType::Send;
	ZeroMemory(&exover->over, sizeof(exover->over));
	exover->wsabuf.buf = exover->io_buf;
	const ULONG length = reinterpret_cast<uint16_t*>(buf)[0];
	exover->wsabuf.len = length;
	memcpy(exover->io_buf, buf, length);

	::WSASend(client.GetSocket(), &exover->wsabuf, 1, NULL, 0, &exover->over, NULL);
}

void Server::SendPacket(int networkID, void* pPacket, ULONG size)
{
	char* buf = static_cast<char*>(pPacket);

	const Client& client = sClients[networkID];

	//WSASend의 두번째 인자의 over는 recv용이라 쓰면 안된다. 새로 만들어야 한다.
	Exover* exover = new Exover;
	exover->type = EOperationType::Send;
	ZeroMemory(&exover->over, sizeof(exover->over));
	exover->wsabuf.buf = exover->io_buf;
	exover->wsabuf.len = size;
	memcpy(exover->io_buf, buf, size);

	::WSASend(client.GetSocket(), &exover->wsabuf, 1, NULL, 0, &exover->over, NULL);
}
