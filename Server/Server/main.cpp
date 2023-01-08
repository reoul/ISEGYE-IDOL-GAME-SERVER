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

#ifndef HIDE_LOG
	Logger::AddLogger("log", "/log.txt");
	Logger::AddLogger("test", "/testLog.txt");
	Logger::AddLogger("SetDefaultUsingItem", "/SetDefaultUsingItem.txt");
	Logger::AddLogger("Check", "/Check.txt");
	Logger::AddLogger("BattleOpponents", "/BattleOpponents.txt");
	Logger::AddLogger("BattleInfo", "/BattleInfo.txt");
	Logger::AddLogger("FinishBattleResultInfo", "/FinishBattleResultInfo.txt");
	Logger::AddLogger("PacketSendRecive", "/PacketSendRecive.txt");
	Logger::AddLogger("PacketWrite", "/PacketWrite.txt");
#endif // HIDE_LOG
	Logger::AddLogger("Connection", "/Connection.txt");
	Logger::AddLogger("ConnectionCount", "/ConnectionCount.txt");

	Server::Start();
	
	return 0;
}
