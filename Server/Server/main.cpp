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
	

	LogPrintf("{0}", sizeof(cs_StartMatchingPacket));
	LogPrintf("{0}", sizeof(sc_ConnectRoomPacket));
	LogPrintf("{0}", sizeof(sc_AddNewItemPacket));
	LogPrintf("{0}", sizeof(cs_sc_ChangeItemSlotPacket));
	LogPrintf("{0}", sizeof(sc_UpgradeItemPacket));
	LogPrintf("{0}", sizeof(cs_sc_ChangeCharacterPacket));
	LogPrintf("{0}", sizeof(sc_BattleInfoPacket));
	LogPrintf("{0}", sizeof(cs_sc_UseEmoticonPacket));
	LogPrintf("{0}", sizeof(cs_sc_NotificationPacket));
	LogPrintf("{0}", sizeof(sc_UpdateCharacterInfoPacket));
	LogPrintf("{0}", sizeof(sc_SetReadyTimePacket));
	LogPrintf("{0}", sizeof(sc_SetChoiceCharacterTimePacket));
	LogPrintf("{0}", sizeof(cs_sc_DropItemPacket));
	LogPrintf("{0}", sizeof(cs_RequestCombinationItemPacket));
	LogPrintf("{0}", sizeof(sc_SetItemTicketPacket));
	LogPrintf("{0}", sizeof(sc_ActiveItemPacket));
	LogPrintf("{0}", sizeof(sc_FadeInPacket));
	LogPrintf("{0}", sizeof(sc_FadeOutPacket));
	LogPrintf("{0}", sizeof(sc_BattleOpponentsPacket));
	LogPrintf("{0}", sizeof(sc_SetHamburgerTypePacket));
	LogPrintf("{0}", sizeof(sc_MagicStickInfoPacket));
	LogPrintf("{0}", sizeof(cs_RequestUpgradeItemPacket));
	LogPrintf("{0}", sizeof(sc_DoctorToolInfoPacket));

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
