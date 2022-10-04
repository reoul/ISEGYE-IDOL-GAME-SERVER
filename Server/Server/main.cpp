#pragma warning(disable:4996)

#include "ServerShared.h"
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
