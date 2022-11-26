#pragma warning(disable:4996)

#include "ServerShared.h"
#include "reoul/logger.h"
#include "Server.h"
#include "Items.h"

using namespace std;

int main()
{
	std::locale::global(std::locale("Korean"));
	SocketUtil::StaticInit();
	LogInit();
	Client aa;
	Client bb;
	sItems[0]->Use(aa, bb, 0);
	sItems[1]->Use(aa, bb, 0);
	Server::Start();
	
	return 0;
}
