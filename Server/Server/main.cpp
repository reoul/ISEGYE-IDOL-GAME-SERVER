
#include <iostream>
#include <iomanip>
#include "ServerShared.h"

using namespace std;

bool gIsRunningServer;
vector<TCPNetworkUserInfo> gReadBlockSockets;

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

	int networkID = 0;
	TCPNetworkUserInfo newNetworkUserInfo(listenSocket, networkID++, receivingAddres);
	gReadBlockSockets.emplace_back(newNetworkUserInfo);

	listenSocket->Listen(SOMAXCONN);
	cout << "Listen 시작" << endl;

	vector<TCPNetworkUserInfo> readableSockets;
	while (gIsRunningServer)
	{
		if (!SocketUtil::Select(&gReadBlockSockets, &readableSockets, nullptr, nullptr, nullptr, nullptr))
		{
			continue;
		}

		bool flag = true;
		for (const auto& networkUserInfo : readableSockets)
		{
			if(networkUserInfo.GetNetworkID() == listenSocket->GetSocket())
			{
				SocketAddress newClientAddress;
				auto newSocket = listenSocket->Accept(newClientAddress);
				int opt_val = TRUE;
				setsockopt(newSocket->GetSocket(), IPPROTO_TCP, TCP_NODELAY, (char*)&opt_val, sizeof(opt_val));
				TCPNetworkUserInfo newNetworkUserInfo(newSocket, networkID++, receivingAddres);
				gReadBlockSockets.emplace_back(newNetworkUserInfo);
				ProcessNewClient(newNetworkUserInfo);
			}
			else
			{
				char segment[1500];
				int dataReceived = networkUserInfo.GetTcpSocketPtr()->Receive(segment, 1500);
				if (dataReceived > 0)
				{
					const int* pInt = reinterpret_cast<const int*>(segment);
					cout << networkUserInfo << "전송 받은 데이터 : " << pInt[0] << " " << pInt[1] << " " << pInt[2] << endl;
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
			while(it != gReadBlockSockets.end())
			{
				cout << setw(10) << it->GetNetworkID();
				if(cnt++ > 8)
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
	}
	SocketUtil::CleanUp();
	string tmp;
	cin >> tmp;
	return 0;
}
