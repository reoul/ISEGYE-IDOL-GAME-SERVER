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

	Logger::LogInit();
	Logger::AddLogger("log", "/log.txt");
	Logger::AddLogger("test", "/testLog.txt");
	Logger::AddLogger("SetDefaultUsingItem", "/SetDefaultUsingItem.txt");
	Logger::AddLogger("Check", "/Check.txt");
	Logger::AddLogger("BattleOpponents", "/BattleOpponents.txt");
	Logger::AddLogger("BattleInfo", "/BattleInfo.txt");
	Logger::AddLogger("FinishBattleResultInfo", "/FinishBattleResultInfo.txt");
	Logger::AddLogger("PacketSendRecive", "/PacketSendRecive.txt");
	Logger::AddLogger("Connection", "/Connection.txt");

	//Client aa;
	//Client bb;
	//sItems[0]->Use(aa, bb, 0);
	//sItems[5]->Use(aa, bb, 0);
	//sItems[5]->Use(aa, bb, 1);
	//sItems[5]->Use(aa, bb, 2);
	//sItems[5]->Use(aa, bb, 3);
	//LogPrintf("{0}", _countof(sItems));
	Server::Start();
	
	return 0;
}
