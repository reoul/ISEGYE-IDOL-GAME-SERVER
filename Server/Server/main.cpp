
#include <iostream>
#include <iomanip>
#include <mutex>
#include <thread>

#include "ServerShared.h"
#include "Room.h"
#include "SettingData.h"
#include "ServerStruct.h"
#include <MSWSock.h>

using namespace std;

Room gRoom(ROOM_MAX_PLAYER);
CLIENT g_clients[MAX_USER];
HANDLE g_hIocp;
SOCKET g_hListenSocket;
bool g_isRunningServer = true;

void ProcessNewClient(TCPNetworkUserInfo networkUserInfo)
{
	cout << "새로운 클라이언트 " << networkUserInfo << " 가 연결 되었습니다." << endl;
}

#pragma pack(push, 1)
struct sc_packet_login_ok
{
	char size;
	char type;
	int id;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct cs_packet_login
{
	char size;
	char type;
	int id;
};
#pragma pack(pop)

//int main()
//{
//	SocketUtil::StaticInit();
//
//	gIsRunningServer = true;
//	auto thread = CreateThread(NULL, 0, ClientThread, 0, 0, NULL);
//	if (thread != NULL)
//	{
//		CloseHandle(thread);
//	}
//	else
//	{
//		SocketUtil::CleanUp();
//		return 1;
//	}
//
//	while (gIsRunningServer)
//	{
//		string str;
//		cin >> str;
//		if (str == "quit")
//		{
//			gIsRunningServer = false;
//		}
//		if (str == "list")
//		{
//			size_t connectCount = gReadBlockSockets.size() - 1;
//			cout << "접속 인원 수 : " << connectCount << endl;
//
//			size_t row = 0;
//			size_t cnt = 0;
//			auto it = gReadBlockSockets.begin() + 1;
//			cout << setw(3) << row;
//			while (it != gReadBlockSockets.end())
//			{
//				cout << setw(10) << it->GetNetworkID();
//				if (cnt++ > 8)
//				{
//					cnt = 0;
//					cout << "\n";
//					cout << setw(3) << ++row;
//				}
//				++it;
//			}
//			cout << endl;
//			cout << "---------------------------------------------------------------" << endl;
//		}
//		if (str == "send")
//		{
//			TestPacket tp(12, 456, 789);
//			gRoom.Send(tp);
//		}
//	}
//	SocketUtil::CleanUp();
//	string tmp;
//	cin >> tmp;
//	return 0;
//}

void SendPacket(int userID, void* p)
{
	char* buf = reinterpret_cast<char*>(p);

	CLIENT& user = g_clients[userID];

	//WSASend의 두번째 인자의 over는 recv용이라 쓰면 안된다. 새로 만들어야 한다.
	EXOVER* exover = new EXOVER;
	exover->type = OperationType::Send;
	ZeroMemory(&exover->over, sizeof(exover->over));
	exover->wsabuf.buf = exover->io_buf;
	exover->wsabuf.len = buf[0];
	memcpy(exover->io_buf, buf, buf[0]);

	WSASend(user.socket, &exover->wsabuf, 1, NULL, 0, &exover->over, NULL);
}

void ProcessPacket(int userID, char* buf)
{
	switch (buf[1]) //[0]은 size
	{
		/*case C2S_LOGIN:
		{
			cs_packet_login* packet = reinterpret_cast<cs_packet_login*>(buf);
			strcpy_s(g_clients[userID].name, packet->name);
			g_clients[userID].name[MAX_ID_LEN] = NULL;
			send_login_ok_packet(userID);
			enter_game(userID);
			break;
		}
		case C2S_MOVE:
		{	cs_packet_move* packet = reinterpret_cast<cs_packet_move*>(buf);
		do_move(userID, packet->direction);
		break;
		}
		default:
			cout << "unknown packet type error \n";
			DebugBreak();
			exit(-1);*/
	}
}

struct cs_test_data
{
	int a;
	int b;
	int c;
};

/**
 * \brief 들어온 패킷을 가공
 * \param userID 유저 ID
 * \param ioByteLength 패킷 데이터 길이
 */
void PacketConstruct(int userID, int ioByteLength)
{
	CLIENT& curUser = g_clients[userID];
	EXOVER& recvOver = curUser.recvOver;

	int restByte = ioByteLength;		//이만큼 남은걸 처리해줘야 한다
	char* p = recvOver.io_buf;			//처리해야할 데이터의 포인터가 필요하다
	int packetSize = 0;				//이게 0이라는 것은 이전에 처리하던 패킷이 없다는 것 

	// 이전에 받아논 패킷이 있다면
	if (curUser.prevSize != 0)
	{
		packetSize = curUser.packetBuf[0]; //재조립을 기다기는 패킷 사이즈
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

	while (restByte > 0)	//처리해야할 데이터가 남아있으면 처리해야한다.
	{
		// 이전에 처리해야할 패킷이 없다면
		if (0 == packetSize)
		{
			// 지금 들어온 패킷의 사이즈를 넣는다
			packetSize = p[0];
		}

		// 나머지 데이터로 패킷을 만들 수 있나 없나 확인
		// 지금 처리해야 하는 패킷의 사이즈가 남은 데이터랑 기존에 더한것보다 작다면(즉 패킷 사이즈 만큼 채울수 있는 데이터가 있다면)
		if (packetSize <= restByte + curUser.prevSize)
		{
			//만들어서 처리한 데이터 크기만큼 패킷 사이즈에서 빼주기
			memcpy(curUser.packetBuf + curUser.prevSize, p, packetSize - curUser.prevSize);

			p += packetSize - curUser.prevSize;
			restByte -= packetSize - curUser.prevSize;
			packetSize = 0;														//이 패킷은 이미 처리를 했고 다음 패킷 사이즈는 모름.

			ProcessPacket(userID, curUser.packetBuf);

			curUser.prevSize = 0;
		}
		else	//패킷 하나를 만들 수 없다면 버퍼에 복사해두고 포인터와 사이즈 증가
		{
			memcpy(curUser.packetBuf + curUser.prevSize, p, restByte); //남은 데이터 몽땅 받는데, 지난번에 받은 데이터가 남아있을 경우가 있으니, 그 뒤에 받아야한다.
			curUser.prevSize += restByte;
			restByte = 0;
			p += restByte;
		}
	}
}

void Disconnect(int userID)
{
	g_clients[userID].cLock.lock();
	g_clients[userID].status = ST_ALLOCATED;	//처리 되기 전에 FREE하면 아직 떠나는 뒷처리가 안됐는데 새 접속을 받을 수 있음

	//send_leave_packet(userID, userID); //나는 나에게 보내기
	closesocket(g_clients[userID].socket);

	/*for (auto& cl : g_clients)
	{
		if (cl.id == userID) continue;
		cl.cLock.lock();
		if (ST_ACTIVE == cl.status)
			send_leave_packet(cl.id, userID);
		cl.cLock.unlock();
	}*/
	g_clients[userID].status = ST_FREE;	//다 처리했으면 FREE
	g_clients[userID].cLock.unlock();
}

void WorkerThread()
{
	while (true)
	{
		DWORD io_byte;
		ULONG_PTR key;
		WSAOVERLAPPED* over;
		bool ret = GetQueuedCompletionStatus(g_hIocp, &io_byte, &key, &over, 1000);
		DWORD errorCode = GetLastError();

		// 서버가 끝낸 상태
		if (!g_isRunningServer)
		{
			break;
		}
		// 타임아웃인 경우
		if (ret == FALSE && over == NULL)
		{
			continue;
		}

		EXOVER* exover = reinterpret_cast<EXOVER*>(over);
		int user_id = static_cast<int>(key);

		cout << user_id << endl;

		switch (exover->type)
		{
		case OperationType::Recv:			//받은 패킷 처리 -> overlapped구조체 초기화 -> recv
		{
			if (0 == io_byte)
			{
				cout << "연결 해제" << endl;
				Disconnect(user_id);
				if (OperationType::Send == exover->type)
					delete exover;
			}
			else
			{
				CLIENT& cl = g_clients[user_id]; //타이핑 줄이기 위해
				//cout << *reinterpret_cast<int*>(exover->io_buf) << "의 데이터가 입력되었습니다. 크기는 " << io_byte << endl;
				/*if (*reinterpret_cast<int*>(exover->io_buf) % 5 == 0)
				{
					cout << *reinterpret_cast<int*>(exover->io_buf) << "의 데이터가 입력되었습니다. 크기는 " << io_byte << endl;
				}*/
				PacketConstruct(user_id, io_byte);
				ZeroMemory(&cl.recvOver.over, 0, sizeof(cl.m_recv_over.over));
				DWORD flags = 0;
				WSARecv(cl.socket, &cl.recvOver.wsabuf, 1, NULL, &flags, &cl.recvOver.over, NULL);
			}
			break;
		}
		case OperationType::Send:			//구조체 delete
			if (0 == io_byte)
				Disconnect(user_id);

			delete exover;
			break;
		case OperationType::Accept:			//CreateIoCompletionPort으로 클라소켓 iocp에 등록 -> 초기화 -> recv -> accept 다시(다중접속)
		{
			int userID = -1;
			for (int i = 0; i < MAX_USER; ++i)
			{
				lock_guard<mutex> gl{ g_clients[i].cLock }; //이렇게 하면 unlock이 필요 없다. 이 블록에서 빠져나갈때 unlock을 자동으로 해준다.
				if (ST_FREE == g_clients[i].status)
				{
					g_clients[i].status = ST_ALLOCATED;
					userID = i;
					break;
				}
			}
			cout << userID << "번으로 할당 되었습니다" << endl;

			//main에서 소켓을 worker스레드로 옮겨오기 위해 listen소켓은 전역변수로, client소켓은 멤버로 가져왔다.
			SOCKET clientSocket = exover->c_socket;

			if (-1 == userID)
				closesocket(clientSocket); // send_login_fail_packet();
			else
			{
				HANDLE ret = CreateIoCompletionPort(reinterpret_cast<HANDLE>(clientSocket), g_hIocp, userID, 0);
				if (ret == NULL)
				{
					cout << userID << " IOCP에 등록을 실패하였습니다." << endl;
					closesocket(clientSocket);
				}
				else
				{
					//g_clients[user_id].id = user_id; 멀쓰에서 하는게 아니고 초기화 할때 한번 해줘야 함 처음에 한번.
					g_clients[userID].prevSize = 0; //이전에 받아둔 조각이 없으니 0
					g_clients[userID].socket = clientSocket;

					ZeroMemory(&g_clients[userID].recvOver.over, 0, sizeof(g_clients[user_id].m_recv_over.over));
					g_clients[userID].recvOver.type = OperationType::Recv;
					g_clients[userID].recvOver.wsabuf.buf = g_clients[userID].recvOver.io_buf;
					g_clients[userID].recvOver.wsabuf.len = MAX_BUF_SIZE;

					cout << user_id << endl;
					cout << userID << " 연결에 성공하였습니다" << endl;

					DWORD flags = 0;
					WSARecv(clientSocket, &g_clients[userID].recvOver.wsabuf, 1, NULL, &flags, &g_clients[userID].recvOver.over, NULL);
				}
			}
			//소켓 초기화 후 다시 accept
			clientSocket = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			exover->c_socket = clientSocket; //새로 받는 소켓을 넣어준다. 안그러면 클라들이 같은 소켓을 공유한다.
			ZeroMemory(&exover->over, sizeof(exover->over)); //accept_over를 exover라는 이름으로 받았으니 exover를 재사용
			AcceptEx(g_hListenSocket, clientSocket, exover->io_buf, NULL,
				sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, NULL, &exover->over);
			break;
		}
		}
	}
}

int main()
{
	SocketUtil::StaticInit();

	g_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	g_hListenSocket = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (g_hListenSocket == INVALID_SOCKET)
	{
		cout << "ListenSocket Create Fail" << endl;
	}

	sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_hListenSocket), g_hIocp, 9957, 0);

	if (bind(g_hListenSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
		cout << "bind error!" << endl;
		closesocket(g_hListenSocket);
	}

	if (listen(g_hListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		cout << "listen error!" << endl;
		closesocket(g_hListenSocket);
	}

	SOCKET clientSocket;
	clientSocket = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	EXOVER accept_over;
	accept_over.c_socket = clientSocket;
	ZeroMemory(&accept_over, sizeof(accept_over.over));
	accept_over.type = OperationType::Accept;
	AcceptEx(g_hListenSocket, clientSocket, accept_over.io_buf, NULL, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, NULL, &accept_over.over);

	cout << "start server" << endl;
	vector<thread> worker_threads;
	for (int i = 0; i < 4; ++i)
	{
		worker_threads.emplace_back(WorkerThread);
	}

	string str;
	while (true)
	{
		cin >> str;
		if (str == "exit")
		{
			g_isRunningServer = false;
			break;
		}
	}

	for (auto& th : worker_threads)
	{
		th.join();
	}
	return 0;
}