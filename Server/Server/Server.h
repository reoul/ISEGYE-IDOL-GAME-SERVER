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
	void Start();
	static RoomManager& GetRoomManager();
	static void SendPacket(int networkID, void* pPacket);
	
private:
	static void WorkerThread();
	static void Disconnect(int networkID);
	static void NewClientEvent(int32_t networkID);
	static void PacketConstruct(int networkID, int ioByteLength);
	static void SendDisconnect(int networkID);
	static void ProcessPacket(int networkID, char* buf);
	
	static HANDLE sIocp;
	static SOCKET sListenSocket;
	static Client mClients[MAX_USER];
	static RoomManager mRoomManager;
	static bool mIsRunningServer;
	static ServerQueue mServerQueue;
};

inline RoomManager& Server::GetRoomManager()
{
	return mRoomManager;
}
