#pragma once
#include "RoomManager.h"
#include "ServerQueue.h"

extern Client g_clients[MAX_USER];
extern HANDLE g_hIocp;
extern SOCKET g_hListenSocket;
extern bool g_bIsRunningServer;
extern ServerQueue g_serverQueue;
extern int32_t g_roomIndex;
extern int g_count;
extern RoomManager g_roomManager;
