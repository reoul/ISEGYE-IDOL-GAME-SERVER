#include "GlobalVariable.h"
#include "Client.h"

Client g_clients[MAX_USER];
HANDLE g_hIocp;
SOCKET g_hListenSocket;
bool g_bIsRunningServer = true;
ServerQueue g_serverQueue;
int32_t g_roomIndex = 0;
int g_count = 0;
RoomManager g_roomManager;
