#include "Server.h"
#include <MSWSock.h>
#include "PacketStruct.h"

HANDLE Server::sIocp;
SOCKET Server::sListenSocket;
Client Server::sClients[MAX_USER];
RoomManager Server::sRoomManager;
bool Server::sIsRunningServer = true;
ServerQueue Server::sServerQueue;



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
		Log("Listen 소켓 생성 실패");
	}

	sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	CreateIoCompletionPort(reinterpret_cast<HANDLE>(sListenSocket), sIocp, 9957, 0);

	if (::bind(sListenSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
		Log("바인드 에러");
		::closesocket(sListenSocket);
		assert(false);
	}

	if (::listen(sListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		Log("Listen 에러");
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
	Log("서버 시작");

	vector<thread> workerThreads;
	for (ULONG i = 0; i < std::thread::hardware_concurrency(); ++i)
	{
		workerThreads.emplace_back(WorkerThread);
	}
	Log("{0}개의 쓰레드 작동", std::thread::hardware_concurrency());

	while (sIsRunningServer)
	{
		sRoomManager.TrySendBattleInfo();
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
				Disconnect(userID);

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
				Disconnect(userID);
			}

			LogWrite("네트워크 {0}번 클라이언트 {1}Byte 패킷 전송", userID, io_byte);
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
					LogWarning("{0}번 소켓 IOCP 등록 실패", userID);
					::closesocket(clientSocket);
				}
				else
				{
					sClients[userID].SetPrevSize(0); //이전에 받아둔 조각이 없으니 0
					sClients[userID].SetSocket(clientSocket);

					// keepalive 설정
					struct tcp_keepalive
					{
						u_long onoff;
						u_long keepalivetime;
						u_long keepaliveinterval;
					};

					DWORD dwError = 0L;
					tcp_keepalive sKA_Settings = { 0 }, sReturned = { 0 };
					sKA_Settings.onoff = 1;
					sKA_Settings.keepalivetime = 5500;        // Keep Alive in 5.5 sec.
					sKA_Settings.keepaliveinterval = 500;        // Resend if No-Reply

					DWORD dwBytes;
					// SIO_KEEPALIVE_VALS 대신에 _WSAIOW(IOC_VENDOR, 4) 사용
					// mstcpip.h 인클루드하면 error 발생
					if (WSAIoctl(clientSocket, _WSAIOW(IOC_VENDOR, 4), &sKA_Settings,
						sizeof(sKA_Settings), &sReturned, sizeof(sReturned), &dwBytes, NULL, NULL) != 0)
					{
						dwError = WSAGetLastError();
						log_assert(false);
						//TRACE(_T("SIO_KEEPALIVE_VALS result : %dn"), WSAGetLastError());
					}

					ZeroMemory(&sClients[userID].GetRecvOver().over, sizeof(sClients[userID].GetRecvOver().over));
					sClients[userID].GetRecvOver().type = EOperationType::Recv;
					sClients[userID].GetRecvOver().wsabuf.buf = sClients[userID].GetRecvOver().io_buf;
					sClients[userID].GetRecvOver().wsabuf.len = MAX_BUF_SIZE;

					NewClientEvent(userID);

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

void Server::NewClientEvent(int32_t networkID)
{
	Log("네트워크 {0}번 클라이언트 서버 접속", networkID);
	sc_connectServerPacket connectServerPacket(networkID);
	SendPacket(networkID, &connectServerPacket);
}

void Server::Disconnect(int networkID)
{
	if (sClients[networkID].GetStatus() == ESocketStatus::FREE)
	{
		return;
	}

	{
		lock_guard<mutex> lg(sServerQueue.GetMutex());
		sServerQueue.RemoveClient(&sClients[networkID]);
	}

	Log("네트워크 {0}번 클라이언트 서버 접속 해제", networkID);

	{
		lock_guard<mutex> lg(sClients[networkID].GetMutex());
		sClients[networkID].SetStatus(ESocketStatus::ALLOCATED);	//처리 되기 전에 FREE하면 아직 떠나는 뒷처리가 안됐는데 새 접속을 받을 수 있음

		::closesocket(sClients[networkID].GetSocket());

		wchar_t name[MAX_USER_NAME_LENGTH];
		wcscpy(name, sClients[networkID].GetName());
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

	/*cout << rest_byte << " ";
	cs_test_data* data = reinterpret_cast<cs_test_data*>(p);
	while (rest_byte > 0)
	{
		cout << data->a << " " << data->b << " " << data->c << " || ";
		rest_byte -= sizeof(cs_test_data);
		++data;
	}

	cout << endl;*/

	LogWrite("네트워크 {0}번 클라이언트 {1}Byte 패킷 받음", networkID, ioByteLength);

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
	sc_disconnectPacket packet(networkID);
	SendPacket(networkID, &packet);
	Log("네트워크 {0}번 클라이언트 접속 해제 패킷 보냄", networkID);
}

void Server::ProcessPacket(int networkID, char* buf)
{
	switch (static_cast<EPacketType>(buf[2])) //[0,1]은 size
	{
	case EPacketType::cs_startMatching:
	{
		const cs_startMatchingPacket* pPacket = reinterpret_cast<cs_startMatchingPacket*>(buf);
		wcscpy(sClients[networkID].GetName(), pPacket->name);
		sClients[networkID].GetName()[MAX_USER_NAME_LENGTH - 1] = '\0';

		Room* room = nullptr;
		{
			lock_guard<mutex> lg(sServerQueue.GetMutex());
			sServerQueue.AddClient(&sClients[networkID]);
			room = sServerQueue.TryCreateRoomOrNullPtr();
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
		Client& client = sClients[networkID];
		client.AddItem(pPacket->itemCode);
		Log("[cs_sc_addNewItem] 네트워크 {0}번 클라이언트 {1}번 아이템 추가", pPacket->networkID, pPacket->itemCode);

		if (sClients[networkID].GetRoomPtr() != nullptr)
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

		if (sClients[networkID].GetRoomPtr() != nullptr)
		{
			lock_guard<mutex> lg(sClients[networkID].GetRoomPtr()->cLock);
			sClients[networkID].SendPacketInAnotherRoomClients(pPacket);
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
		sClients[networkID].SwapItem(pPacket->slot1, pPacket->slot2);
		Log("[cs_sc_changeItemSlot] 네트워크 {0}번 클라이언트 아이템 슬롯 {1} <-> {2} 교체", pPacket->networkID, pPacket->slot1, pPacket->slot2);

		if (sClients[networkID].GetRoomPtr() != nullptr)
		{
			lock_guard<mutex> lg(sClients[networkID].GetRoomPtr()->cLock);
			sClients[networkID].SendPacketInAllRoomClients(pPacket);
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
		Client& client = sClients[networkID];
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

void Server::SendPacket(int networkID, void* pPacket)
{
	char* buf = reinterpret_cast<char*>(pPacket);

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
