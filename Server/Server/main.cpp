#pragma warning(disable:4996)

#include <iostream>
#include <iomanip>
#include <mutex>
#include <thread>

#include "ServerShared.h"
#include "ServerQueue.h"
#include "SettingData.h"
#include "ServerStruct.h"
#include "PacketStruct.h"
#include <MSWSock.h>
#include "GlobalVariable.h"
#include "Random.h"
#include "Client.h"

using namespace std;

void ServerInit();
void SendPacket(int userID, void* pPacket);
void ProcessPacket(int userID, char* buf);
void PacketConstruct(int userID, int ioByteLength);
void SendDisconnect(int userID);
void Disconnect(int userID);
void WorkerThread();
void NewClientEvent(int32_t userID);

int main()
{
	setlocale(LC_ALL, "KOREAN");
	SocketUtil::StaticInit();

	ServerInit();

	SOCKET clientSocket;
	clientSocket = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	Exover accept_over;
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
		g_roomManager.TrySendRandomItemQueue();

		if (!g_bIsRunningServer)
		{
			break;
		}
	}

	for (auto& th : worker_threads)
	{
		th.join();
	}
	return 0;
}

void ServerInit()
{
	for (int i = 0; i < MAX_USER; ++i)
	{
		g_clients[i].SetNetworkID(i);
	}

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
}

void SendPacket(int userID, void* pPacket)
{
	char* buf = reinterpret_cast<char*>(pPacket);

	const Client& client = g_clients[userID];

	//WSASend의 두번째 인자의 over는 recv용이라 쓰면 안된다. 새로 만들어야 한다.
	Exover* exover = new Exover;
	exover->type = OperationType::Send;
	ZeroMemory(&exover->over, sizeof(exover->over));
	exover->wsabuf.buf = exover->io_buf;
	const size_t length = reinterpret_cast<uint16_t*>(buf)[0];
	exover->wsabuf.len = length;
	memcpy(exover->io_buf, buf, length);

	WSASend(client.GetSocket(), &exover->wsabuf, 1, NULL, 0, &exover->over, NULL);
}

/**
 * \brief 들어온 패킷을 가공
 * \param userID 유저 ID
 * \param ioByteLength 패킷 데이터 길이
 */
void PacketConstruct(int userID, int ioByteLength)
{
	Client& curUser = g_clients[userID];
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

	cout << ioByteLength << "들어옴 " << endl;

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

			ProcessPacket(userID, curUser.GetPacketBuf());

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

void SendDisconnect(int userID)
{
	if (g_clients[userID].GetStatus() != ST_ACTIVE)
	{
		return;
	}

	sc_disconnectPacket packet(userID);
	SendPacket(userID, &packet);
}

void Disconnect(int userID)
{
	if (g_clients[userID].GetStatus() == ST_FREE)
	{
		return;
	}
	g_serverQueue.Lock();
	g_serverQueue.RemoveClient(&g_clients[userID]);
	g_serverQueue.UnLock();

	g_clients[userID].cLock.lock();
	g_clients[userID].SetStatus(ST_ALLOCATED);	//처리 되기 전에 FREE하면 아직 떠나는 뒷처리가 안됐는데 새 접속을 받을 수 있음

	closesocket(g_clients[userID].GetSocket());
	g_clients[userID].GetSocket() = INVALID_SOCKET;
	if (g_clients[userID].GetRoomPtr() != nullptr)
	{
		g_clients[userID].GetRoomPtr()->RemoveClient(g_clients[userID]);
		g_clients[userID].SetRoom(nullptr);
	}
	wchar_t name[MAX_USER_NAME_LENGTH];
	wcscpy(name, g_clients[userID].GetName());
	g_clients[userID].GetName()[0] = '\0';
	g_clients[userID].SetStatus(ST_FREE);	//다 처리했으면 FREE
	g_clients[userID].cLock.unlock();
	wcout << L"[" << userID << L"] 유저가 접속 해제하였습니다" << endl;
}

void WorkerThread()
{
	while (true)
	{
		DWORD io_byte;
		ULONG_PTR key;
		WSAOVERLAPPED* over;
		bool sucess = GetQueuedCompletionStatus(g_hIocp, &io_byte, &key, &over, 1000);
		DWORD errorCode = GetLastError();

		// 서버가 끝낸 상태
		if (!g_bIsRunningServer)
		{
			break;
		}
		// 타임아웃인 경우
		if (sucess == FALSE && over == nullptr)
		{
			continue;
		}

		Exover* exover = reinterpret_cast<Exover*>(over);
		int user_id = static_cast<int>(key);

		switch (exover->type)
		{
		case OperationType::Recv:			//받은 패킷 처리 -> overlapped구조체 초기화 -> recv
		{
			if (0 == io_byte)
			{
				Disconnect(user_id);

				if (OperationType::Send == exover->type)
					delete exover;
			}
			else
			{
				Client& client = g_clients[user_id];
				PacketConstruct(user_id, io_byte);
				ZeroMemory(&client.GetRecvOver().over, sizeof(client.GetRecvOver().over));
				DWORD flags = 0;
				WSARecv(client.GetSocket(), &client.GetRecvOver().wsabuf, 1, NULL, &flags, &client.GetRecvOver().over, NULL);
			}
			break;
		}
		case OperationType::Send:			//구조체 delete
			if (0 == io_byte)
			{
				Disconnect(user_id);
			}
			cout << io_byte << " byte 데이터 전송" << endl;
			delete exover;
			break;
		case OperationType::Accept:			//CreateIoCompletionPort으로 클라소켓 iocp에 등록 -> 초기화 -> recv -> accept 다시(다중접속)
		{
			int userID = -1;
			for (int i = 0; i < MAX_USER; ++i)
			{
				lock_guard<mutex> gl{ g_clients[i].cLock }; //이렇게 하면 unlock이 필요 없다. 이 블록에서 빠져나갈때 unlock을 자동으로 해준다.
				if (ST_FREE == g_clients[i].GetStatus())
				{
					g_clients[i].SetStatus(ST_ALLOCATED);
					userID = i;
					break;
				}
			}

			//main에서 소켓을 worker스레드로 옮겨오기 위해 listen소켓은 전역변수로, client소켓은 멤버로 가져왔다.
			SOCKET clientSocket = exover->c_socket;

			if (userID == -1)
				closesocket(clientSocket); // send_login_fail_packet();
			else
			{
				const HANDLE result = CreateIoCompletionPort(reinterpret_cast<HANDLE>(clientSocket), g_hIocp, userID, 0);
				if (result == NULL)
				{
					cout << userID << " IOCP에 등록을 실패하였습니다." << endl;
					closesocket(clientSocket);
				}
				else
				{
					//g_clients[user_id].networkID = user_id; 멀쓰에서 하는게 아니고 초기화 할때 한번 해줘야 함 처음에 한번.
					g_clients[userID].SetPrevSize(0); //이전에 받아둔 조각이 없으니 0
					g_clients[userID].GetSocket() = clientSocket;

					ZeroMemory(&g_clients[userID].GetRecvOver().over, sizeof(g_clients[user_id].GetRecvOver().over));
					g_clients[userID].GetRecvOver().type = OperationType::Recv;
					g_clients[userID].GetRecvOver().wsabuf.buf = g_clients[userID].GetRecvOver().io_buf;
					g_clients[userID].GetRecvOver().wsabuf.len = MAX_BUF_SIZE;

					NewClientEvent(userID);

					g_clients[userID].SetStatus(ST_ACTIVE);

					DWORD flags = 0;
					WSARecv(clientSocket, &g_clients[userID].GetRecvOver().wsabuf, 1, NULL, &flags, &g_clients[userID].GetRecvOver().over, NULL);
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

void NewClientEvent(int32_t userID)
{
	cout << userID << " 번 유저가 접속하였습니다" << endl;
	sc_connectServerPacket connectServerPacket(userID);
	SendPacket(userID, &connectServerPacket);
}

