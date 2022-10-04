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
#include "Client.h"
#include "reoul/logger.h"
#include "Server.h"

using namespace std;

int main()
{
	std::locale::global(std::locale("Korean"));
	SocketUtil::StaticInit();
	LogInit();
	Server::Start();
	
	return 0;
}
