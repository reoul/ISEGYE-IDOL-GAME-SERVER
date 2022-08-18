#pragma once
#include "Room.h"
#include "ServerQueue.h"

extern Room gRoom;
extern Client g_clients[MAX_USER];
extern HANDLE g_hIocp;
extern SOCKET g_hListenSocket;
extern bool g_bIsRunningServer;
extern ServerQueue g_serverQueue;
extern int32_t g_roomIndex;
extern int g_count;
