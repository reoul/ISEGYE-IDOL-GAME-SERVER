
#include <iostream>
#include <iomanip>
#include "ServerShared.h"
#include "Room.h"
#include "SettingData.h"
#include "PacketStruct.h"

using namespace std;

bool gIsRunningServer;
vector<TCPNetworkUserInfo> gReadBlockSockets;
queue<TCPNetworkUserInfo> gRoomQueue;
Room gRoom(ROOM_MAX_PLAYER);

void ProcessNewClient(TCPNetworkUserInfo networkUserInfo)
{
	cout << "새로운 클라이언트 " << networkUserInfo << " 가 연결 되었습니다." << endl;
}

DWORD WINAPI ClientThread(LPVOID arg)
{
	TCPSocketPtr listenSocket = SocketUtil::CreateTCPSocket(INET);
	SocketAddress receivingAddres(INADDR_ANY, 51341);
	if (listenSocket->Bind(receivingAddres) != NO_ERROR)
	{
		return 0;
	}
	cout << "바인딩 성공" << endl;

	TCPNetworkUserInfo newNetworkUserInfo(listenSocket, receivingAddres);
	gReadBlockSockets.emplace_back(newNetworkUserInfo);

	listenSocket->Listen(SOMAXCONN);
	cout << "Listen 시작" << endl;

	vector<TCPNetworkUserInfo> readableSockets;
	vector<TCPNetworkUserInfo> writeableSockets;
	while (gIsRunningServer)
	{
		if (!SocketUtil::Select(&gReadBlockSockets, &readableSockets, &gReadBlockSockets, &writeableSockets, nullptr, nullptr))
		{
			continue;
		}

		bool flag = true;
		for (const auto& networkUserInfo : readableSockets)
		{
			if (networkUserInfo.GetNetworkID() == listenSocket->GetSocket())
			{
				SocketAddress newClientAddress;
				auto newSocket = listenSocket->Accept(newClientAddress);
				int opt_val = TRUE;
				setsockopt(newSocket->GetSocket(), IPPROTO_TCP, TCP_NODELAY, (char*)&opt_val, sizeof(opt_val));
				TCPNetworkUserInfo newNetworkUserInfo(newSocket, receivingAddres);
				gReadBlockSockets.emplace_back(newNetworkUserInfo);
				ProcessNewClient(newNetworkUserInfo);
				gRoomQueue.push(newNetworkUserInfo);
				uint32_t userID = static_cast<uint32_t>(networkUserInfo.GetTcpSocketPtr()->GetSocket());
				newNetworkUserInfo.GetTcpSocketPtr()->Send(&userID, sizeof(userID));

				{
					auto userInfo = gRoomQueue.front();
					gRoomQueue.pop();
					gRoom.AddUser(userInfo);
				}
				if (gRoomQueue.size() >= ROOM_MAX_PLAYER)
				{
					for (int i = 0; i < ROOM_MAX_PLAYER; ++i)
					{
						auto userInfo = gRoomQueue.front();
						gRoomQueue.pop();
						gRoom.AddUser(userInfo);
					}
				}
			}
			else
			{
				char segment[1500];
				int dataReceived = networkUserInfo.GetTcpSocketPtr()->Receive(segment, 1500);
				if (dataReceived > 0)
				{
					auto pac = reinterpret_cast<TestPacket*>(segment);
					pac->networkID = static_cast<uint32_t>(networkUserInfo.GetNetworkID());
					cout << networkUserInfo << "전송 받은 데이터 : " << pac->type << " " << pac->networkID << " " << pac->data << endl;
					gRoom.Send(*pac);
				}
				else
				{
					for (auto it = gReadBlockSockets.begin(); it != gReadBlockSockets.end(); ++it)
					{
						if (it->GetNetworkID() == networkUserInfo.GetNetworkID())
						{
							gReadBlockSockets.erase(it);
							cout << "연결 해제 : " << networkUserInfo << endl;
							break;
						}
					}
				}
			}
		}
	}
	gReadBlockSockets.clear();
	return 0;
}

int main()
{
	SocketUtil::StaticInit();

	gIsRunningServer = true;
	auto thread = CreateThread(NULL, 0, ClientThread, 0, 0, NULL);
	if (thread != NULL)
	{
		CloseHandle(thread);
	}
	else
	{
		SocketUtil::CleanUp();
		return 1;
	}

	while (gIsRunningServer)
	{
		string str;
		cin >> str;
		if (str == "quit")
		{
			gIsRunningServer = false;
		}
		if (str == "list")
		{
			size_t connectCount = gReadBlockSockets.size() - 1;
			cout << "접속 인원 수 : " << connectCount << endl;

			size_t row = 0;
			size_t cnt = 0;
			auto it = gReadBlockSockets.begin() + 1;
			cout << setw(3) << row;
			while (it != gReadBlockSockets.end())
			{
				cout << setw(10) << it->GetNetworkID();
				if (cnt++ > 8)
				{
					cnt = 0;
					cout << "\n";
					cout << setw(3) << ++row;
				}
				++it;
			}
			cout << endl;
			cout << "---------------------------------------------------------------" << endl;
		}
		if (str == "send")
		{
			TestPacket tp(12, 456, 789);
			gRoom.Send(tp);
		}
	}
	SocketUtil::CleanUp();
	string tmp;
	cin >> tmp;
	return 0;
}
