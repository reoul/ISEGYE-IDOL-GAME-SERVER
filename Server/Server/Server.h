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
private:
	static void WorkerThread();
	static void Disconnect(int networkID);
	static void NewClientEvent(int networkID, char* ipAdress);
	static void PacketConstruct(int networkID, int ioByteLength);
	static void SendDisconnect(int networkID);
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
