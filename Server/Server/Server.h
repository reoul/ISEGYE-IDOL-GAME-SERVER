#pragma once

#include <WinSock2.h>
#include <reoul/logger.h>

#include "Client.h"
#include "RoomManager.h"
#include "SettingData.h"
#include "ServerQueue.h"

class Server
{
public:
	Server() = default;
	Server(const Server&) = delete;
	Server& operator=(const Server&) = delete;
	static void Start();
	static RoomManager& GetRoomManager();
	static void SendPacket(int networkID, void* pPacket);
	static void SendPacket(int networkID, void* pPacket, ULONG size);
	static Client& GetClients(int networkID);
	static void SendDisconnect(int networkID);
	static void SendDisconnectDelay(int networkID);	// 해당 플레이어 연결 해제와 소속 Room에게 접속 해제 보냄
	static void Disconnect(int networkID, bool isSendAnotherRoomClient);
private:
	static void WorkerThread();
	static void NewClientEvent(int networkID, char* ipAdress);
	static void PacketConstruct(int networkID, int ioByteLength);
	static void ProcessPacket(int networkID, char* buf);
	
	static HANDLE sIocp;
	static SOCKET sListenSocket;
	static Client sClients[MAX_USER];
	static RoomManager sRoomManager;
	static bool sIsRunningServer;
	static ServerQueue sServerQueue;
};

inline RoomManager& Server::GetRoomManager()
{
	return sRoomManager;
}

inline Client& Server::GetClients(int networkID)
{
	return sClients[networkID];
}
