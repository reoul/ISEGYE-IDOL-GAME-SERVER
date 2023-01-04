#include "Room.h"

#include <iostream>
#include <reoul/functional.h>
#include <reoul/logger.h>
#include "Client.h"
#include "Random.h"
#include "PacketStruct.h"
#include "Server.h"
#include "Items.h"

using namespace Logger;

Room::Room()
	: mSize(0)
	, mNumber(0)
	, mIsRun(false)
	, mCapacity(MAX_ROOM_PLAYER)
	, mIsFinishChoiceCharacter(false)
	, mOpenCount(0)
	, mRound(0)
	, mCurRoomStatusType(ERoomStatusType::ChoiceCharacter)
	, mCreepRound(0)
{
}

void Room::AddClients(vector<Client*>& clients)
{
	{
		lock_guard<mutex> lg(cLock);
		if (mSize == mCapacity)
		{
			return;
		}

		for (Client* client : clients)
		{
			++mSize;
			mClients.emplace_back(client);
			client->SetRoom(this);
		}
	}

	mBattleManager.SetClients(clients);
}

void Room::RemoveClient(const Client& client)
{
	{
		lock_guard<mutex> lg(cLock);
		auto it = mClients.cbegin();
		for (; it != mClients.cend(); ++it)
		{
			if (*it == &client)
			{
				break;
			}
		}

		if (it != mClients.cend())
		{
			mClients.erase(it);
			mBattleManager.RemoveClient(client.GetNetworkID());
			--mSize;
		}

		if (mSize == 0)
		{
			Init();
		}
	}
}

void Room::SendPacketToAllClients(void* pPacket) const
{
	for (auto it = mClients.begin(); it != mClients.end(); ++it)
	{
		Server::SendPacket((*it)->GetNetworkID(), pPacket);
	}
}

void Room::SendPacketToAllClients(void* pPacket, ULONG size) const
{
	for (auto it = mClients.begin(); it != mClients.end(); ++it)
	{
		Server::SendPacket((*it)->GetNetworkID(), pPacket, size);
	}
}

void Room::SendPacketToAnotherClients(const Client& client, void* pPacket) const
{
	for (auto it = mClients.begin(); it != mClients.end(); ++it)
	{
		if (&client == *it)
		{
			continue;
		}

		Server::SendPacket((*it)->GetNetworkID(), pPacket);
	}
}

void Room::SendPacketToAnotherClients(const Client& client, void* pPacket, ULONG size) const
{
	for (auto it = mClients.begin(); it != mClients.end(); ++it)
	{
		if (&client == *it)
		{
			continue;
		}

		Server::SendPacket((*it)->GetNetworkID(), pPacket, size);
	}
}

void Room::ApplyRandomBattleOpponent()
{
	lock_guard<mutex> lg(cLock);

	if (mSize < 2)
	{
		return;
	}

	mBattleOpponents = mBattleManager.GetBattleOpponent();
}

void Room::Init()
{
	mClients.clear();
	mSize = 0;
	mIsRun = false;
	mIsFinishChoiceCharacter = false;
	mRound = 0;
	mCurRoomStatusType = ERoomStatusType::ChoiceCharacter;
	mCreepRound = 0;

	{
		lock_guard<mutex> lg(Server::GetRoomManager().cLock);
		Log("log", "{0}번 룸 비활성화 (현재 활성화된 방 : {1})", mNumber, Server::GetRoomManager().GetUsingRoomCount());
	}
}

void Room::TrySendEnterInGame()
{
	size_t readyCount = 0;

	if (mClients.size() < 2)
	{
		return;
	}

	for (const Client* client : mClients)
	{
		if (client->IsChoiceCharacter())
		{
			++readyCount;
		}
	}

	if (readyCount == mClients.size() && !mIsFinishChoiceCharacter)
	{
		cs_sc_NotificationPacket packet(0, ENotificationType::ChoiceAllCharacter);
		SendPacketToAllClients(&packet);
		Log("log", "{0}번 룸 캐릭터 선택 끝남", mNumber);
		mIsFinishChoiceCharacter = true;
	}
}

/**
 * \brief 게임 진행 스레드 함수
 */
unsigned Room::ProgressThread(void* pArguments)
{
	LogPrintf("진행 시작");
	Room& room = *static_cast<Room*>(pArguments);

	room.mCurRoomStatusType = ERoomStatusType::ChoiceCharacter;

	const size_t roomOpenCount = room.GetOpenCount();

	for (int i = 0; i < CHOICE_CHARACTER_TIME; ++i)
	{
		Sleep(1000);

		{
			lock_guard<mutex> lg(room.cLock);
			room.TrySendEnterInGame();
		}

		// 모두 선택 완료했을 때
		if (room.mIsFinishChoiceCharacter)
		{
			Log("log", "모두 선택 완료");
			break;
		}

		if (!room.mIsRun || room.GetSize() < 2 || room.GetOpenCount() != roomOpenCount)
		{
			Log("log", "Room 진행 종료");
			_endthreadex(0);
			return 0;
		}
	}

	room.mCurRoomStatusType = ERoomStatusType::CutSceneStage;

	// 모두 선택 했을 때
	if (room.mIsFinishChoiceCharacter)
	{
		LogWrite("Check", "캐릭터 선택 상태");

		{
			// 캐릭터 선택 동기화
			const size_t bufferSize = sizeof(cs_sc_ChangeCharacterPacket) * room.GetSize();
			OutputMemoryStream memoryStream(bufferSize);

			for (const Client* client : room.GetClients())
			{
				cs_sc_ChangeCharacterPacket packet(client->GetNetworkID(), client->GetCharacterType());
				packet.Write(memoryStream);
				LogWrite("Check", "{0} : {1}", client->GetNetworkID(), static_cast<uint8_t>(client->GetCharacterType()));
			}

			room.SendPacketToAllClients(memoryStream.GetBufferPtr(), bufferSize);

			Sleep(5000);
		}

		if (!room.mIsRun || room.GetSize() < 2 || room.GetOpenCount() != roomOpenCount)
		{
			Log("log", "Room 진행 종료");
			_endthreadex(0);
			return 0;
		}

		{
			sc_FadeInPacket packet(1);
			room.SendPacketToAllClients(&packet);
			Sleep(1000);
		}

		{
			const size_t bufferSize = sizeof(cs_sc_NotificationPacket) + sizeof(sc_SetReadyTimePacket) + sizeof(sc_FadeOutPacket);
			OutputMemoryStream memoryStream(bufferSize);
			const cs_sc_NotificationPacket notificationPacket(0, ENotificationType::EnterCutSceneStage);
			notificationPacket.Write(memoryStream);

			const sc_SetReadyTimePacket setReadyTimePacket(BATTLE_READY_TIME);
			setReadyTimePacket.Write(memoryStream);

			const sc_FadeOutPacket fadeOutPacket(1);
			fadeOutPacket.Write(memoryStream);

			room.SendPacketToAllClients(memoryStream.GetBufferPtr(), bufferSize);
		}
	}
	else  // 확정없이 준비시간이 다 지났을 때
	{
		LogWrite("Check", "캐릭터 선택 상태");

		// 캐릭터 선택 동기화
		{
			OutputMemoryStream memoryStream(512);

			size_t bufferSize = 0;
			for (Client* client : room.GetClients())
			{
				if (client->GetCharacterType() == ECharacterType::Empty)
				{
					client->SetCharacterType(ECharacterType::Woowakgood);
					cs_sc_ChangeCharacterPacket changeCharacterPacket(client->GetNetworkID(), ECharacterType::Woowakgood);
					changeCharacterPacket.Write(memoryStream);
					bufferSize += sizeof(cs_sc_ChangeCharacterPacket);
				}
				if (!client->IsChoiceCharacter())
				{
					cs_sc_NotificationPacket notificationPacket(client->GetNetworkID(), ENotificationType::ChoiceCharacter);
					notificationPacket.Write(memoryStream);
					bufferSize += sizeof(cs_sc_NotificationPacket);
				}
				LogWrite("Check", "{0} : {1}", client->GetNetworkID(), static_cast<uint8_t>(client->GetCharacterType()));
			}

			{
				// 캐릭터 선택 시간 끝났다고 알림
				cs_sc_NotificationPacket packet(0, ENotificationType::FinishChoiceCharacterTime);
				packet.Write(memoryStream);
				bufferSize += sizeof(cs_sc_NotificationPacket);
			}

			{
				sc_FadeInPacket packet(1);
				packet.Write(memoryStream);
				bufferSize += sizeof(sc_FadeInPacket);
			}

			room.SendPacketToAllClients(memoryStream.GetBufferPtr(), bufferSize);
		}

		Sleep(3000);

		{
			constexpr size_t bufferSize = sizeof(cs_sc_NotificationPacket) + sizeof(sc_SetReadyTimePacket) + sizeof(sc_FadeOutPacket);
			OutputMemoryStream memoryStream(bufferSize);

			const cs_sc_NotificationPacket notificationPacket(0, ENotificationType::EnterCutSceneStage);
			notificationPacket.Write(memoryStream);

			const sc_SetReadyTimePacket setReadyTimePacket(BATTLE_READY_TIME);
			setReadyTimePacket.Write(memoryStream);

			const sc_FadeOutPacket fadeOutPacket(1);
			fadeOutPacket.Write(memoryStream);

			room.SendPacketToAllClients(memoryStream.GetBufferPtr(), bufferSize);
		}
	}

	Sleep(1000);

	if (!room.mIsRun || room.GetSize() < 2 || room.GetOpenCount() != roomOpenCount)
	{
		Log("log", "Room 진행 종료");
		_endthreadex(0);
		return 0;
	}

	// CutScene 연출
	Sleep(9000);

	{
		sc_FadeInPacket packet(1);
		room.SendPacketToAllClients(&packet);
	}

	Sleep(1000);

	if (!room.mIsRun || room.GetSize() < 2 || room.GetOpenCount() != roomOpenCount)
	{
		LogPrintf("Room 진행 종료");
		_endthreadex(0);
		return 0;
	}

	{
		cs_sc_NotificationPacket packet(0, ENotificationType::FinishCutSceneStage);
		room.SendPacketToAllClients(&packet);
	}

	Log("log", "기본 템 지급 시작");
	// 기본 템 지급
	{
		OutputMemoryStream memoryStream(512);

		for (Client* client : room.mClients)
		{
			constexpr uint8_t defaultItemCode1 = 1;	// 기본템 1
			constexpr uint8_t defaultItemCode2 = 4;	// 기본템 2

			uint8_t slot1 = client->AddItem(defaultItemCode1);

			uint8_t slot2 = client->AddItem(defaultItemCode2);

			sc_InventoryInfoPacket inventoryInfoPacket(*client);
			inventoryInfoPacket.Write(memoryStream);

			client->SetNormalItemTicketCount(0);
			client->SetAdvancedItemTicketCount(0);
			client->SetTopItemTicketCount(0);
			client->SetSupremeItemTicketCount(0);

			const sc_SetItemTicketPacket setItemTicketPacket1(client->GetNetworkID(), EItemTicketType::Normal, client->GetNormalItemTicketCount());
			const sc_SetItemTicketPacket setItemTicketPacket2(client->GetNetworkID(), EItemTicketType::Advanced, client->GetAdvancedItemTicketCount());
			const sc_SetItemTicketPacket setItemTicketPacket3(client->GetNetworkID(), EItemTicketType::Top, client->GetTopItemTicketCount());
			const sc_SetItemTicketPacket setItemTicketPacket4(client->GetNetworkID(), EItemTicketType::Supreme, client->GetSupremeItemTicketCount());
			setItemTicketPacket1.Write(memoryStream);
			setItemTicketPacket2.Write(memoryStream);
			setItemTicketPacket3.Write(memoryStream);
			setItemTicketPacket4.Write(memoryStream);
		}

		room.SendPacketToAllClients(memoryStream.GetBufferPtr(), memoryStream.GetLength());
	}

	room.mCurRoomStatusType = ERoomStatusType::ReadyStage;
	Log("log", "기본 템 지급 완료");

	// 처음 크립 3판
	ReadyStage(room, false);

	Sleep(1000);

	CreepStage(room);

	Sleep(1000);

	ReadyStage(room, false);

	Sleep(1000);

	CreepStage(room);

	Sleep(1000);

	ReadyStage(room, false);

	Sleep(1000);

	CreepStage(room);

	Sleep(1000);

	++room.mRound;

	while (true)
	{
		for (int i = 0; i < 4; ++i)
		{
			// 대기 시간
			if (!ReadyStage(room, true))
			{
				goto loopOut;
			}

			// 전투
			if (!BattleStage(room))
			{
				goto loopOut;
			}
		}

		// 대기 시간
		if (!ReadyStage(room, false))
		{
			break;
		}

		// 크립
		if (!CreepStage(room))
		{
			break;
		}

		++room.mRound;
	}

loopOut:
	Log("log", "Room 진행 종료");
	_endthreadex(0);
	return 0;
}

/**
 * \brief 준비 스테이지 로직
 * \param room 해당 Room
 * \param isNextStageBattle 다음 스테이지가 전투 스테이지인지
 * \return Room 유지 여부
 */
bool Room::ReadyStage(Room& room, bool isNextStageBattle)
{
	room.mCurRoomStatusType = ERoomStatusType::ReadyStage;

	const size_t roomOpenCount = room.GetOpenCount();

	LogPrintf("준비시간 시작");

	if (!room.mIsRun || room.GetSize() < 2 || room.GetOpenCount() != roomOpenCount)
	{
		return false;
	}

	{
		OutputMemoryStream memoryStream(1024);

		if (isNextStageBattle)
		{
			room.ApplyRandomBattleOpponent();
			sc_BattleOpponentsPacket packet(room.GetBattleOpponents());
			packet.Write(memoryStream);
		}

		{
			cs_sc_NotificationPacket packet(0, ENotificationType::EnterReadyStage);
			packet.Write(memoryStream);
		}

		const vector<Client*> clients = room.GetClients();

		for (Client* client : clients)
		{
			sc_UpdateCharacterInfoPacket packet(*client);
			packet.Write(memoryStream);

			// 매번 3개씩 지급
			const int currNormalItemTicketCount = client->GetNormalItemTicketCount();
			client->SetNormalItemTicketCount(currNormalItemTicketCount + 3);
			sc_SetItemTicketPacket setItemTicketPacket(client->GetNetworkID(), EItemTicketType::Normal, client->GetNormalItemTicketCount());
			setItemTicketPacket.Write(memoryStream);
		}

		room.SendPacketToAllClients(memoryStream.GetBufferPtr(), memoryStream.GetLength());
	}

	LogPrintf("캐릭터 갱신 패킷 전송");

	Sleep(1000);

	{
		sc_FadeOutPacket packet(1);
		room.SendPacketToAllClients(&packet);
	}

	Sleep(1000);


	for (size_t i = 0; i < BATTLE_READY_TIME + 1; ++i)
	{
		Sleep(1000);

		if (!room.mIsRun || room.GetSize() < 2 || room.GetOpenCount() != roomOpenCount)
		{
			return false;
		}
	}

	LogPrintf("준비시간 끝");

	{
		const vector<Client*> clients = room.GetClients();
		const size_t bufferSize = sizeof(sc_UpdateCharacterInfoPacket) * clients.size() + sizeof(cs_sc_NotificationPacket) * clients.size();
		OutputMemoryStream memoryStream(bufferSize);

		// 기본 템 장착
		for (Client* client : room.GetClients())
		{
			const bool isSetDefaultUsingItem = client->TrySetDefaultUsingItem();
			if (isSetDefaultUsingItem)
			{
				cs_sc_NotificationPacket packet(client->GetNetworkID(), ENotificationType::SetDefaultUsingItem);
				packet.Write(memoryStream);
			}
		}
		LogPrintf("기본 유물 장착");

		for (const Client* client : clients)
		{
			sc_UpdateCharacterInfoPacket packet(*client);
			packet.Write(memoryStream);
		}

		room.SendPacketToAllClients(memoryStream.GetBufferPtr(), memoryStream.GetLength());
	}

	LogPrintf("캐릭터 갱신 패킷 전송");

	if (!room.mIsRun || room.GetSize() < 2 || room.GetOpenCount() != roomOpenCount)
	{
		return false;
	}

	{
		sc_FadeInPacket packet(1);
		room.SendPacketToAllClients(&packet);
	}

	Sleep(1000);

	return true;
}

/**
 * \brief 전투 스테이지 로직
 * \param room 해당 Room
 * \return Room 유지 여부
 */
bool Room::BattleStage(Room& room)
{
	room.mCurRoomStatusType = ERoomStatusType::BattleStage;

	const size_t roomOpenCount = room.GetOpenCount();

	LogPrintf("전투 스테이지 시작");

	if (!room.mIsRun || room.GetSize() < 2 || room.GetOpenCount() != roomOpenCount)
	{
		return false;
	}

	const vector<int32_t>& battleOpponents = room.GetBattleOpponents();

	constexpr int waitTimes[2]{ 500, 500 };

	const int avatarCount = battleOpponents.size();

	unique_ptr<BattleAvatar[]> avatars = make_unique<BattleAvatar[]>(battleOpponents.size());
	for (int i = 0; i < battleOpponents.size(); ++i)
	{
		const int networkID = battleOpponents[i] >= 0 ? battleOpponents[i] : ~battleOpponents[i];
		Client& client = Server::GetClients(networkID);
		avatars[i].SetAvatar(client, networkID, battleOpponents[i] < 0, room.GetRound());
	}

	{
		OutputMemoryStream memoryStream;

		cs_sc_NotificationPacket notificationPacket(0, ENotificationType::EnterBattleStage);
		notificationPacket.Write(memoryStream);

		for (int i = 0; i < battleOpponents.size(); ++i)
		{
			avatars[i].FitmentEffect();
			for (int j = 0; j < MAX_USING_ITEM_COUNT; ++j)
			{
				Item& item = avatars[i].GetItemBySlot(j);
				if (item.GetType() == 7)	// 햄버거인 경우
				{
					const EHamburgerType hamburgerType = avatars[i].GetRandomHamburgerType();
					avatars[i].SetHamburgerType(hamburgerType);

					const int networkID = avatars[i].IsGhost() ? ~avatars[i].GetNetworkID() : avatars[i].GetNetworkID();
					sc_SetHamburgerTypePacket packet(networkID, j, hamburgerType);
					packet.Write(memoryStream);
					Logger::Log("BattleInfo", "[전투] {0}번 클라이언트 햄버거 {1}번 햄버거로 설정됨", networkID, static_cast<int>(hamburgerType));
				}
				else if (item.GetType() == 16)	// 박사의 만능툴
				{
					int index = i % 2 == 0 ? 1 : -1;	// 0-1   2-3   4-5  6-7 이렇게 전투하기 때문에 인덱스 번호에 따라 상대 인덱스가 다르다
					BattleAvatar& opponent = avatars[i + index];
					Item choiceItem = opponent.GetRandomCopyItem();

					const int networkID = avatars[i].IsGhost() ? ~avatars[i].GetNetworkID() : avatars[i].GetNetworkID();
					sc_DoctorToolInfoPacket packet(networkID, j, choiceItem.GetType(), choiceItem.GetUpgrade());
					packet.Write(memoryStream);
					item.SetType(choiceItem.GetType());
					item.SetUpgrade(choiceItem.GetUpgrade());
					Logger::Log("BattleInfo", "[전투] {0}번 클라이언트 만능툴 {1}번 아이템 강화 수치 {2}으로 설정됨", networkID, choiceItem.GetType(), choiceItem.GetUpgrade());
				}
			}
		}

		for (int i = 0; i < battleOpponents.size(); ++i)
		{
			sc_BattleAvatarInfoPacket packet(avatars[i]);
			packet.Write(memoryStream);
		}

		room.SendPacketToAllClients(memoryStream.GetBufferPtr(), memoryStream.GetLength());
	}

	Sleep(1000);

	{
		sc_FadeOutPacket packet(1);
		room.SendPacketToAllClients(&packet);
	}

	Sleep(1000);

	if (!room.mIsRun || room.GetSize() < 2 || room.GetOpenCount() != roomOpenCount)
	{
		return false;
	}

	for (int i = 0; i < battleOpponents.size(); i += 2)
	{
		int networkID1 = avatars[i].GetNetworkID();
		int networkID2 = avatars[i + 1].GetNetworkID();
		if (!room.IsValidClientInThisRoom(&Server::GetClients(networkID1))
			|| !room.IsValidClientInThisRoom(&Server::GetClients(networkID2)))
		{
			Logger::Log("BattleInfo", "[전투] {0}, {1} 전투 끝남", networkID1, networkID2);
			avatars[i].SetFinish();
			avatars[i + 1].SetFinish();
		}

		if (avatars[i].GetFirstAttackState() < avatars[i + 1].GetFirstAttackState())
		{
			BattleAvatar avatar1 = avatars[i];
			avatars[i] = avatars[i + 1];
			avatars[i + 1] = avatar1;
		}
	}

	for (size_t battleLoop = 0; battleLoop < 20; ++battleLoop)
	{
		for (int j = 0; j < battleOpponents.size(); ++j)
		{
			if (avatars[j].IsFinish() == false)
			{
				const vector<SlotInfo> itemQueue = Server::GetClients(avatars[j].GetNetworkID()).GetItemActiveQueue();
				avatars[j].SetActiveQueue(itemQueue);
			}
		}

		{
			OutputMemoryStream memoryStream(1024);

			int packetSize = 0;
			for (int k = 0; k < avatarCount; ++k)
			{
				if (avatars[k].IsFinish())
				{
					continue;
				}

				const int networkID = avatars[k].IsGhost() ? ~avatars[k].GetNetworkID() : avatars[k].GetNetworkID();
				cs_sc_NotificationPacket packet(networkID, ENotificationType::InitBattleSlot);
				packet.Write(memoryStream);
				packetSize += sizeof(cs_sc_NotificationPacket);
			}

			if (packetSize > 0)
			{
				room.SendPacketToAllClients(memoryStream.GetBufferPtr(), packetSize);
			}
		}

		Sleep(1000);

		for (int activeItemIndex = 0; activeItemIndex < MAX_USING_ITEM_COUNT; ++activeItemIndex)
		{
			{
				OutputMemoryStream memoryStream;

				vector<int> disconnectNetworkIdList;

				int packetSize = 0;
				for (int k = 0; k < avatarCount; k += 2)
				{
					if (avatars[k].IsFinish())
					{
						continue;
					}

					const uint8_t activeSlot = avatars[k].ActiveItem(activeItemIndex, avatars[k + 1]);

					if (avatars[k].GetHP() == 0 || avatars[k + 1].GetHP() == 0)
					{
						avatars[k].ToDamageCharacter(avatars[k + 1].GetDamage());
						avatars[k + 1].ToDamageCharacter(avatars[k].GetDamage());

						if (avatars[k].GetClient()->GetHp() == 0 && room.IsValidClientInThisRoom(avatars[k].GetClient()))
							disconnectNetworkIdList.emplace_back(avatars[k].GetNetworkID());
						if (avatars[k + 1].GetClient()->GetHp() == 0 && room.IsValidClientInThisRoom(avatars[k + 1].GetClient()))
							disconnectNetworkIdList.emplace_back(avatars[k + 1].GetNetworkID());

						avatars[k].SetFinish();
						avatars[k + 1].SetFinish();
					}

					if (sItems[avatars[k].GetItemBySlot(activeSlot).GetType()]->TYPE == EItemType::Attack)
					{
						avatars[k + 1].EffectCounter(avatars[k]);
					}

					if (!avatars[k].IsFinish() && !avatars[k + 1].IsFinish())
					{
						avatars[k].EffectBleeding();
					}

					{
						const int networkID = avatars[k].IsGhost() ? ~avatars[k].GetNetworkID() : avatars[k].GetNetworkID();
						sc_ActiveItemPacket packet(networkID, activeSlot);
						packet.Write(memoryStream);
						packetSize += sizeof(sc_ActiveItemPacket);

						sc_BattleAvatarInfoPacket packet1(avatars[k]);
						packet1.Write(memoryStream);
						packetSize += sizeof(sc_BattleAvatarInfoPacket);

						sc_BattleAvatarInfoPacket packet2(avatars[k + 1]);
						packet2.Write(memoryStream);
						packetSize += sizeof(sc_BattleAvatarInfoPacket);
					}

					if (avatars[k].GetHP() == 0 || avatars[k + 1].GetHP() == 0)
					{
						avatars[k].ToDamageCharacter(avatars[k + 1].GetDamage());
						avatars[k + 1].ToDamageCharacter(avatars[k].GetDamage());

						if (avatars[k].GetClient()->GetHp() == 0 && room.IsValidClientInThisRoom(avatars[k].GetClient()))
							disconnectNetworkIdList.emplace_back(avatars[k].GetNetworkID());
						if (avatars[k + 1].GetClient()->GetHp() == 0 && room.IsValidClientInThisRoom(avatars[k + 1].GetClient()))
							disconnectNetworkIdList.emplace_back(avatars[k + 1].GetNetworkID());

						avatars[k].SetFinish();
						avatars[k + 1].SetFinish();
					}
				}

				// 중복 제거
				if (disconnectNetworkIdList.size() > 0)
				{
					sort(disconnectNetworkIdList.begin(), disconnectNetworkIdList.end());
					disconnectNetworkIdList.erase(unique(disconnectNetworkIdList.begin(), disconnectNetworkIdList.end()), disconnectNetworkIdList.end());
				}

				for (int networkID : disconnectNetworkIdList)
				{
					cs_sc_NotificationPacket packet(networkID, ENotificationType::DisconnectServer);
					packet.Write(memoryStream);
				}

				if (memoryStream.GetLength() > 0)
				{
					room.SendPacketToAllClients(memoryStream.GetBufferPtr(), memoryStream.GetLength());
					Sleep(200);
				}

				for (int networkID : disconnectNetworkIdList)
				{
					Log("BattleInfo", "[전투] {0}번 클라이언트 체력 0으로 접속 종료", networkID);
					Server::Disconnect(networkID, false);
				}
			}

			Sleep(waitTimes[battleLoop / 10]);

			int k;
			for (k = 0; k < avatarCount; ++k)
			{
				if (avatars[k].IsFinish() == false) break;
			}
			if (k == avatarCount)
			{
				goto FinishBattle;
			}

			{
				OutputMemoryStream memoryStream(1024);

				vector<int> disconnectNetworkIdList;

				int packetSize = 0;
				for (int k = 1; k < avatarCount; k += 2)
				{
					if (avatars[k].IsFinish())
					{
						continue;
					}

					const uint8_t activeSlot = avatars[k].ActiveItem(activeItemIndex, avatars[k - 1]);

					if (avatars[k].GetHP() == 0 || avatars[k - 1].GetHP() == 0)
					{
						avatars[k].ToDamageCharacter(avatars[k - 1].GetDamage());
						avatars[k - 1].ToDamageCharacter(avatars[k].GetDamage());

						if (avatars[k].GetClient()->GetHp() == 0 && room.IsValidClientInThisRoom(avatars[k].GetClient()))
							disconnectNetworkIdList.emplace_back(avatars[k].GetNetworkID());
						if (avatars[k - 1].GetClient()->GetHp() == 0 && room.IsValidClientInThisRoom(avatars[k - 1].GetClient()))
							disconnectNetworkIdList.emplace_back(avatars[k - 1].GetNetworkID());

						avatars[k].SetFinish();
						avatars[k - 1].SetFinish();
					}

					if (sItems[avatars[k].GetItemBySlot(activeSlot).GetType()]->TYPE == EItemType::Attack)
					{
						avatars[k - 1].EffectCounter(avatars[k]);
					}

					if (!avatars[k].IsFinish() && !avatars[k - 1].IsFinish())
					{
						avatars[k].EffectBleeding();
					}

					{
						const int networkID = avatars[k].IsGhost() ? ~avatars[k].GetNetworkID() : avatars[k].GetNetworkID();
						sc_ActiveItemPacket packet(networkID, activeSlot);
						packet.Write(memoryStream);
						packetSize += sizeof(sc_ActiveItemPacket);

						sc_BattleAvatarInfoPacket packet1(avatars[k]);
						packet1.Write(memoryStream);
						packetSize += sizeof(sc_BattleAvatarInfoPacket);

						sc_BattleAvatarInfoPacket packet2(avatars[k - 1]);
						packet2.Write(memoryStream);
						packetSize += sizeof(sc_BattleAvatarInfoPacket);
					}

					if (avatars[k].GetHP() == 0 || avatars[k - 1].GetHP() == 0)
					{
						avatars[k].ToDamageCharacter(avatars[k - 1].GetDamage());
						avatars[k - 1].ToDamageCharacter(avatars[k].GetDamage());

						if (avatars[k].GetClient()->GetHp() == 0 && room.IsValidClientInThisRoom(avatars[k].GetClient()))
							disconnectNetworkIdList.emplace_back(avatars[k].GetNetworkID());
						if (avatars[k - 1].GetClient()->GetHp() == 0 && room.IsValidClientInThisRoom(avatars[k - 1].GetClient()))
							disconnectNetworkIdList.emplace_back(avatars[k - 1].GetNetworkID());

						avatars[k].SetFinish();
						avatars[k - 1].SetFinish();
					}
				}

				// 중복 제거
				if (disconnectNetworkIdList.size() > 0)
				{
					sort(disconnectNetworkIdList.begin(), disconnectNetworkIdList.end());
					disconnectNetworkIdList.erase(unique(disconnectNetworkIdList.begin(), disconnectNetworkIdList.end()), disconnectNetworkIdList.end());
				}

				for (int networkID : disconnectNetworkIdList)
				{
					cs_sc_NotificationPacket packet(networkID, ENotificationType::DisconnectServer);
					packet.Write(memoryStream);
				}

				if (memoryStream.GetLength() > 0)
				{
					room.SendPacketToAllClients(memoryStream.GetBufferPtr(), memoryStream.GetLength());
					Sleep(200);
				}

				for (int networkID : disconnectNetworkIdList)
				{
					Log("BattleInfo", "[전투] {0}번 클라이언트 체력 0으로 접속 종료", networkID);
					Server::Disconnect(networkID, false);
				}
			}

			Sleep(waitTimes[battleLoop / 10]);

			Log("BattleInfo", "----------");
			for (k = 0; k < avatarCount; ++k)
			{
				BattleAvatar& avatar = avatars[k];
				const int networkID = avatars[k].IsGhost() ? ~avatars[k].GetNetworkID() : avatars[k].GetNetworkID();
				Log("BattleInfo", "[전투] {0}번 클라이언트 maxHp:{1}, hp:{2}, isFinish:{3}, mIsGhost:{4}", networkID, avatar.GetMaxHP(), avatar.GetHP(), avatar.IsFinish(), avatar.IsGhost());
			}
			Log("BattleInfo", "----------");

			for (k = 0; k < avatarCount; ++k)
			{
				if (avatars[k].IsFinish() == false) break;
			}
			if (k == avatarCount)
			{
				goto FinishBattle;
			}

			if (!room.mIsRun || room.GetSize() < 2 || room.GetOpenCount() != roomOpenCount)
			{
				return false;
			}
		}

		{
			OutputMemoryStream memoryStream(512);

			vector<int> disconnectNetworkIdList;

			for (int k = 0; k < avatarCount; k += 2)
			{
				if (avatars[k].IsFinish())
				{
					continue;
				}

				avatars[k].EffectBomb();
				avatars[k].InitCycle();
				avatars[k + 1].EffectBomb();
				avatars[k + 1].InitCycle();

				sc_BattleAvatarInfoPacket packet1(avatars[k]);
				packet1.Write(memoryStream);

				sc_BattleAvatarInfoPacket packet2(avatars[k + 1]);
				packet2.Write(memoryStream);

				if (avatars[k].GetHP() == 0 || avatars[k + 1].GetHP() == 0)
				{
					avatars[k].ToDamageCharacter(avatars[k + 1].GetDamage());
					avatars[k + 1].ToDamageCharacter(avatars[k].GetDamage());

					if (avatars[k].GetClient()->GetHp() == 0 && room.IsValidClientInThisRoom(avatars[k].GetClient()))
						disconnectNetworkIdList.emplace_back(avatars[k].GetNetworkID());
					if (avatars[k + 1].GetClient()->GetHp() == 0 && room.IsValidClientInThisRoom(avatars[k + 1].GetClient()))
						disconnectNetworkIdList.emplace_back(avatars[k + 1].GetNetworkID());

					avatars[k].SetFinish();
					avatars[k + 1].SetFinish();
				}
			}

			// 중복 제거
			if (disconnectNetworkIdList.size() > 0)
			{
				sort(disconnectNetworkIdList.begin(), disconnectNetworkIdList.end());
				disconnectNetworkIdList.erase(unique(disconnectNetworkIdList.begin(), disconnectNetworkIdList.end()), disconnectNetworkIdList.end());
			}

			for (int networkID : disconnectNetworkIdList)
			{
				cs_sc_NotificationPacket packet(networkID, ENotificationType::DisconnectServer);
				packet.Write(memoryStream);
			}

			if (memoryStream.GetLength() > 0)
			{
				room.SendPacketToAllClients(memoryStream.GetBufferPtr(), memoryStream.GetLength());
				Sleep(200);
			}

			for (int networkID : disconnectNetworkIdList)
			{
				Log("BattleInfo", "[전투] {0}번 클라이언트 체력 0으로 접속 종료", networkID);
				Server::Disconnect(networkID, false);
			}
		}

	}

	Sleep(200);

	// 시간이 다 지났는데도 안끝난 전투가 있으면 강제 데미지 10
	{
		OutputMemoryStream memoryStream(128);
		vector<int> disconnectNetworkIdList;
		for (int k = 0; k < avatarCount; k += 2)
		{
			if (avatars[k].IsFinish())
			{
				continue;
			}
			avatars[k].ToDamageCharacter(10);
			avatars[k + 1].ToDamageCharacter(10);

			if (avatars[k].GetClient()->GetHp() == 0 && room.IsValidClientInThisRoom(avatars[k].GetClient()))
				disconnectNetworkIdList.emplace_back(avatars[k].GetNetworkID());
			if (avatars[k + 1].GetClient()->GetHp() == 0 && room.IsValidClientInThisRoom(avatars[k + 1].GetClient()))
				disconnectNetworkIdList.emplace_back(avatars[k + 1].GetNetworkID());
		}

		// 중복 제거
		if (disconnectNetworkIdList.size() > 0)
		{
			sort(disconnectNetworkIdList.begin(), disconnectNetworkIdList.end());
			disconnectNetworkIdList.erase(unique(disconnectNetworkIdList.begin(), disconnectNetworkIdList.end()), disconnectNetworkIdList.end());
		}

		for (int networkID : disconnectNetworkIdList)
		{
			cs_sc_NotificationPacket packet(networkID, ENotificationType::DisconnectServer);
			packet.Write(memoryStream);
		}

		if (memoryStream.GetLength() > 0)
		{
			room.SendPacketToAllClients(memoryStream.GetBufferPtr(), memoryStream.GetLength());
			Sleep(200);
		}

		for (int networkID : disconnectNetworkIdList)
		{
			Log("BattleInfo", "[전투] {0}번 클라이언트 체력 0으로 접속 종료", networkID);
			Server::Disconnect(networkID, false);
		}
	}


FinishBattle:

	Log("FinishBattleResultInfo", "----------");

	for (Client* pClient : room.GetClients())
	{
		Log("FinishBattleResultInfo", "[전투] {0}번 클라이언트 체력 : {1}", pClient->GetNetworkID(), pClient->GetHp());
	}

	Log("FinishBattleResultInfo", "----------");

	if (room.GetClients().size() == 1)
	{
		int lastClientNetworkID = room.GetClients()[0]->GetNetworkID();
		cs_sc_NotificationPacket packet(lastClientNetworkID, ENotificationType::DisconnectServer);
		room.SendPacketToAllClients(&packet);
		Sleep(200);
		Server::Disconnect(lastClientNetworkID, false);
	}

	Sleep(200);

	if (!room.mIsRun || room.GetSize() < 2 || room.GetOpenCount() != roomOpenCount)
	{
		return false;
	}

	Sleep(1000);

	{
		sc_FadeInPacket packet(1);
		room.SendPacketToAllClients(&packet);
	}

	Sleep(1000);

	return true;
}

/**
 * \brief 크립 스테이지 로직
 * \param room 해당 Room
 * \return Room 유지 여부
 */
bool Room::CreepStage(Room& room)
{
	room.mCurRoomStatusType = ERoomStatusType::CreepStage;

	const size_t roomOpenCount = room.GetOpenCount();

	LogPrintf("크립 스테이지 시작");

	ECreepType curCreeType = room.GetCurCreepType();

	vector<Client*>& clients = room.GetClients();
	const int avatarCount = clients.size() * 2;

	const BattleAvatar creepMonster = room.GetCreepMonster();
	unique_ptr<BattleAvatar[]> avatars = make_unique<BattleAvatar[]>(avatarCount);
	int clientIndex = 0;
	for (int i = 0; i < avatarCount; i += 2)
	{
		avatars[i].SetAvatar(*clients[clientIndex], clients[clientIndex]->GetNetworkID(), false, room.GetRound());
		avatars[i + 1] = creepMonster;
		avatars[i + 1].SetNetworkID(clients[clientIndex]->GetNetworkID() + 1000000);	// 크립 몬스터는 따로 인덱스 할 수 있는 번호가 없기 때문에 일정 숫자를 더해준다.
		++clientIndex;
	}
	// todo : 플레이어 강화, 이동, 재조합, 버림 할 경우 인벤토리 정보 전달
	{
		OutputMemoryStream memoryStream(1024);
		cs_sc_NotificationPacket notificationPacket(0, ENotificationType::EnterCreepStage);
		notificationPacket.Write(memoryStream);
		sc_CreepStageInfoPacket creepStageInfoPacket(curCreeType);
		creepStageInfoPacket.Write(memoryStream);

		// 홀수는 크립몬스터라
		// 만약 장착 효과가 있으면 ++i 로 변경해야함
		for (int i = 0; i < avatarCount; i += 2)
		{
			avatars[i].FitmentEffect();
			for (int j = 0; j < MAX_USING_ITEM_COUNT; ++j)
			{
				Item& item = avatars[i].GetItemBySlot(j);
				if (item.GetType() == 7)	// 햄버거인 경우
				{
					const EHamburgerType hamburgerType = avatars[i].GetRandomHamburgerType();
					avatars[i].SetHamburgerType(hamburgerType);

					sc_SetHamburgerTypePacket packet(avatars[i].GetNetworkID(), j, hamburgerType);
					packet.Write(memoryStream);
					Logger::Log("log", "[크립] {0}번 클라이언트 햄버거 {1}번 햄버거로 설정됨", avatars[i].GetNetworkID(), static_cast<int>(hamburgerType));
				}
			}
		}

		for (int i = 0; i < avatarCount; ++i)
		{
			sc_BattleAvatarInfoPacket packet(avatars[i]);
			packet.Write(memoryStream);
		}

		room.SendPacketToAllClients(memoryStream.GetBufferPtr(), memoryStream.GetLength());
	}

	Sleep(1000);

	{
		sc_FadeOutPacket packet(1);
		room.SendPacketToAllClients(&packet);
	}

	Sleep(1000);

	constexpr int waitTimes[2]{ 500, 500 };

	Sleep(1000);

	if (!room.mIsRun || room.GetSize() < 2 || room.GetOpenCount() != roomOpenCount)
	{
		return false;
	}


	for (size_t battleLoop = 0; battleLoop < 20; ++battleLoop)
	{
		for (int j = 0; j < avatarCount; j += 2)
		{
			const vector<SlotInfo> itemQueue = Server::GetClients(avatars[j].GetNetworkID()).GetItemActiveQueue();
			avatars[j].SetActiveQueue(itemQueue);
		}

		{
			OutputMemoryStream memoryStream;

			int packetSize = 0;
			for (int k = 0; k < avatarCount; k += 2)
			{
				if (avatars[k].IsFinish())
				{
					continue;
				}

				cs_sc_NotificationPacket packet(avatars[k].GetNetworkID(), ENotificationType::InitBattleSlot);
				packet.Write(memoryStream);
				packetSize += sizeof(cs_sc_NotificationPacket);
			}

			if (packetSize > 0)
			{
				room.SendPacketToAllClients(memoryStream.GetBufferPtr(), packetSize);
			}
		}

		Sleep(1000);

		for (int activeItemIndex = 0; activeItemIndex < MAX_USING_ITEM_COUNT; ++activeItemIndex)
		{
			{
				OutputMemoryStream memoryStream(1024);

				vector<int> disconnectNetworkIdList;

				int packetSize = 0;
				for (int k = 0; k < avatarCount; k += 2)
				{
					BattleAvatar& playerAvatar = avatars[k];
					BattleAvatar& creepAvatar = avatars[k + 1];
					if (playerAvatar.IsFinish())
					{
						continue;
					}

					const uint8_t activeSlot = playerAvatar.ActiveItem(activeItemIndex, creepAvatar);

					if (playerAvatar.GetHP() == 0 || creepAvatar.GetHP() == 0)
					{
						playerAvatar.ToDamageCharacter(creepAvatar.GetDamage());
						creepAvatar.ToDamageCharacter(playerAvatar.GetDamage());

						if (playerAvatar.GetClient()->GetHp() == 0 && room.IsValidClientInThisRoom(playerAvatar.GetClient()))
							disconnectNetworkIdList.emplace_back(playerAvatar.GetNetworkID());
						if (!playerAvatar.IsFinish() && creepAvatar.GetHP() == 0)
						{
							CreepRewardInfo rewardInfo = room.GetCreepRewardTicketType();
							int currItemTicketCount = playerAvatar.IncreaseItemTicket(rewardInfo.itemTicketType, rewardInfo.count);
							sc_SetItemTicketPacket packet(playerAvatar.GetNetworkID(), rewardInfo.itemTicketType, currItemTicketCount);
							packet.Write(memoryStream);
						}

						playerAvatar.SetFinish();
						creepAvatar.SetFinish();
					}

					if (!playerAvatar.IsFinish() && !creepAvatar.IsFinish())
					{
						playerAvatar.EffectBleeding();
					}

					{
						sc_ActiveItemPacket packet(playerAvatar.GetNetworkID(), activeSlot);
						packet.Write(memoryStream);
						packetSize += sizeof(sc_ActiveItemPacket);

						sc_BattleAvatarInfoPacket packet1(playerAvatar);
						packet1.Write(memoryStream);
						packetSize += sizeof(sc_BattleAvatarInfoPacket);

						sc_BattleAvatarInfoPacket packet2(creepAvatar);
						packet2.Write(memoryStream);
						packetSize += sizeof(sc_BattleAvatarInfoPacket);
					}

					if (playerAvatar.GetHP() == 0 || creepAvatar.GetHP() == 0)
					{
						playerAvatar.ToDamageCharacter(creepAvatar.GetDamage());
						creepAvatar.ToDamageCharacter(playerAvatar.GetDamage());

						if (playerAvatar.GetClient()->GetHp() == 0 && room.IsValidClientInThisRoom(playerAvatar.GetClient()))
							disconnectNetworkIdList.emplace_back(playerAvatar.GetNetworkID());
						if (!playerAvatar.IsFinish() && creepAvatar.GetHP() == 0)
						{
							CreepRewardInfo rewardInfo = room.GetCreepRewardTicketType();
							int currItemTicketCount = playerAvatar.IncreaseItemTicket(rewardInfo.itemTicketType, rewardInfo.count);
							sc_SetItemTicketPacket packet(playerAvatar.GetNetworkID(), rewardInfo.itemTicketType, currItemTicketCount);
							packet.Write(memoryStream);
						}

						playerAvatar.SetFinish();
						creepAvatar.SetFinish();
					}
				}

				// 중복 제거
				if (disconnectNetworkIdList.size() > 0)
				{
					sort(disconnectNetworkIdList.begin(), disconnectNetworkIdList.end());
					disconnectNetworkIdList.erase(unique(disconnectNetworkIdList.begin(), disconnectNetworkIdList.end()), disconnectNetworkIdList.end());
				}

				for (int networkID : disconnectNetworkIdList)
				{
					cs_sc_NotificationPacket packet(networkID, ENotificationType::DisconnectServer);
					packet.Write(memoryStream);
				}

				if (memoryStream.GetLength() > 0)
				{
					room.SendPacketToAllClients(memoryStream.GetBufferPtr(), memoryStream.GetLength());
					Sleep(200);
				}

				for (int networkID : disconnectNetworkIdList)
				{
					Log("BattleInfo", "[전투] {0}번 클라이언트 체력 0으로 접속 종료", networkID);
					Server::Disconnect(networkID, false);
				}
			}

			Sleep(waitTimes[battleLoop / 10]);

			int k;
			for (k = 0; k < avatarCount; ++k)
			{
				if (avatars[k].IsFinish() == false) break;
			}
			if (k == avatarCount)
			{
				goto FinishBattle;
			}

			{
				int packetSize = 0;
				OutputMemoryStream memoryStream(1024);

				vector<int> disconnectNetworkIdList;

				for (int k = 1; k < avatarCount; k += 2)
				{
					BattleAvatar& playerAvatar = avatars[k - 1];
					BattleAvatar& creepAvatar = avatars[k];
					if (creepAvatar.IsFinish())
					{
						continue;
					}

					const uint8_t activeSlot = creepAvatar.ActiveItem(activeItemIndex, playerAvatar);

					if (creepAvatar.GetHP() == 0 || playerAvatar.GetHP() == 0)
					{
						creepAvatar.ToDamageCharacter(playerAvatar.GetDamage());
						playerAvatar.ToDamageCharacter(creepAvatar.GetDamage());

						if (playerAvatar.GetClient()->GetHp() == 0 && room.IsValidClientInThisRoom(playerAvatar.GetClient()))
							disconnectNetworkIdList.emplace_back(playerAvatar.GetNetworkID());
						if (!playerAvatar.IsFinish() && creepAvatar.GetHP() == 0)
						{
							CreepRewardInfo rewardInfo = room.GetCreepRewardTicketType();
							int currItemTicketCount = playerAvatar.IncreaseItemTicket(rewardInfo.itemTicketType, rewardInfo.count);
							sc_SetItemTicketPacket packet(playerAvatar.GetNetworkID(), rewardInfo.itemTicketType, currItemTicketCount);
							packet.Write(memoryStream);
						}

						creepAvatar.SetFinish();
						playerAvatar.SetFinish();
					}

					if (sItems[creepAvatar.GetItemBySlot(activeSlot).GetType()]->TYPE == EItemType::Attack)
					{
						playerAvatar.EffectCounter(creepAvatar);
					}

					if (!creepAvatar.IsFinish() && !playerAvatar.IsFinish())
					{
						creepAvatar.EffectBleeding();
					}

					{
						sc_ActiveItemPacket packet(creepAvatar.GetNetworkID(), activeSlot);
						packet.Write(memoryStream);
						packetSize += sizeof(sc_ActiveItemPacket);

						sc_BattleAvatarInfoPacket packet1(creepAvatar);
						packet1.Write(memoryStream);
						packetSize += sizeof(sc_BattleAvatarInfoPacket);

						sc_BattleAvatarInfoPacket packet2(playerAvatar);
						packet2.Write(memoryStream);
						packetSize += sizeof(sc_BattleAvatarInfoPacket);
					}

					if (creepAvatar.GetHP() == 0 || playerAvatar.GetHP() == 0)
					{
						creepAvatar.ToDamageCharacter(playerAvatar.GetDamage());
						playerAvatar.ToDamageCharacter(creepAvatar.GetDamage());

						if (playerAvatar.GetClient()->GetHp() == 0 && room.IsValidClientInThisRoom(playerAvatar.GetClient()))
							disconnectNetworkIdList.emplace_back(playerAvatar.GetNetworkID());
						if (!playerAvatar.IsFinish() && creepAvatar.GetHP() == 0)
						{
							CreepRewardInfo rewardInfo = room.GetCreepRewardTicketType();
							int currItemTicketCount = playerAvatar.IncreaseItemTicket(rewardInfo.itemTicketType, rewardInfo.count);
							sc_SetItemTicketPacket packet(playerAvatar.GetNetworkID(), rewardInfo.itemTicketType, currItemTicketCount);
							packet.Write(memoryStream);
						}

						creepAvatar.SetFinish();
						playerAvatar.SetFinish();
					}
				}

				// 중복 제거
				if (disconnectNetworkIdList.size() > 0)
				{
					sort(disconnectNetworkIdList.begin(), disconnectNetworkIdList.end());
					disconnectNetworkIdList.erase(unique(disconnectNetworkIdList.begin(), disconnectNetworkIdList.end()), disconnectNetworkIdList.end());
				}

				for (int networkID : disconnectNetworkIdList)
				{
					cs_sc_NotificationPacket packet(networkID, ENotificationType::DisconnectServer);
					packet.Write(memoryStream);
				}

				if (memoryStream.GetLength() > 0)
				{
					room.SendPacketToAllClients(memoryStream.GetBufferPtr(), memoryStream.GetLength());
					Sleep(200);
				}

				for (int networkID : disconnectNetworkIdList)
				{
					Log("BattleInfo", "[크립] {0}번 클라이언트 체력 0으로 접속 종료", networkID);
					Server::Disconnect(networkID, false);
				}
			}

			Sleep(waitTimes[battleLoop / 10]);

			Log("BattleInfo", "----------");
			for (k = 0; k < avatarCount; k += 2)
			{
				BattleAvatar& avatar = avatars[k];
				Log("BattleInfo", "[크립] {0}번 클라이언트 maxHp:{1}, hp:{2}, isFinish:{3}, mIsGhost:{4}", avatar.GetNetworkID(), avatar.GetMaxHP(), avatar.GetHP(), avatar.IsFinish(), avatar.IsGhost());
				BattleAvatar& avatar2 = avatars[k + 1];
				Log("BattleInfo", "[크립] {0}번 크립 몬스터 maxHp:{1}, hp:{2}, isFinish:{3}, mIsGhost:{4}", ((k + 1) - (k + 1) / 2) - 1 , avatar2.GetMaxHP(), avatar2.GetHP(), avatar2.IsFinish(), avatar2.IsGhost());
			}
			Log("BattleInfo", "----------");

			for (k = 0; k < avatarCount; ++k)
			{
				if (avatars[k].IsFinish() == false) break;
			}
			if (k == avatarCount)
			{
				goto FinishBattle;
			}

			if (!room.mIsRun || room.GetSize() < 2 || room.GetOpenCount() != roomOpenCount)
			{
				return false;
			}
		}

		{
			OutputMemoryStream memoryStream(512);

			vector<int> disconnectNetworkIdList;

			for (int k = 0; k < avatarCount; k += 2)
			{
				BattleAvatar& playerAvatar = avatars[k];
				BattleAvatar& creepAvatar = avatars[k + 1];

				if (playerAvatar.IsFinish())
				{
					continue;
				}

				playerAvatar.EffectBomb();
				playerAvatar.InitCycle();
				creepAvatar.EffectBomb();
				creepAvatar.InitCycle();

				sc_BattleAvatarInfoPacket packet1(playerAvatar);
				packet1.Write(memoryStream);

				sc_BattleAvatarInfoPacket packet2(creepAvatar);
				packet2.Write(memoryStream);

				if (playerAvatar.GetHP() == 0 || creepAvatar.GetHP() == 0)
				{
					playerAvatar.ToDamageCharacter(creepAvatar.GetDamage());
					creepAvatar.ToDamageCharacter(playerAvatar.GetDamage());

					if (playerAvatar.GetClient()->GetHp() == 0 && room.IsValidClientInThisRoom(playerAvatar.GetClient()))
						disconnectNetworkIdList.emplace_back(playerAvatar.GetNetworkID());
					if (!playerAvatar.IsFinish() && creepAvatar.GetHP() == 0)
					{
						CreepRewardInfo rewardInfo = room.GetCreepRewardTicketType();
						int currItemTicketCount = playerAvatar.IncreaseItemTicket(rewardInfo.itemTicketType, rewardInfo.count);
						sc_SetItemTicketPacket packet(playerAvatar.GetNetworkID(), rewardInfo.itemTicketType, currItemTicketCount);
						packet.Write(memoryStream);
					}

					playerAvatar.SetFinish();
					creepAvatar.SetFinish();
				}
			}

			// 중복 제거
			if (disconnectNetworkIdList.size() > 0)
			{
				sort(disconnectNetworkIdList.begin(), disconnectNetworkIdList.end());
				disconnectNetworkIdList.erase(unique(disconnectNetworkIdList.begin(), disconnectNetworkIdList.end()), disconnectNetworkIdList.end());
			}

			for (int networkID : disconnectNetworkIdList)
			{
				cs_sc_NotificationPacket packet(networkID, ENotificationType::DisconnectServer);
				packet.Write(memoryStream);
			}

			if (memoryStream.GetLength() > 0)
			{
				room.SendPacketToAllClients(memoryStream.GetBufferPtr(), memoryStream.GetLength());
				Sleep(200);
			}

			for (int networkID : disconnectNetworkIdList)
			{
				Log("BattleInfo", "[전투] {0}번 클라이언트 체력 0으로 접속 종료", networkID);
				Server::Disconnect(networkID, false);
			}
		}
	}

	Sleep(200);

	// 시간이 다 지났는데도 안끝난 전투가 있으면 강제 데미지 10
	{
		OutputMemoryStream memoryStream(128);
		vector<int> disconnectNetworkIdList;
		for (int k = 0; k < avatarCount; k += 2)
		{
			if (avatars[k].IsFinish())
			{
				continue;
			}

			avatars[k].ToDamageCharacter(10);

			if (avatars[k].GetClient()->GetHp() == 0 && room.IsValidClientInThisRoom(avatars[k].GetClient()))
				disconnectNetworkIdList.emplace_back(avatars[k].GetNetworkID());
		}

		// 중복 제거
		if (disconnectNetworkIdList.size() > 0)
		{
			sort(disconnectNetworkIdList.begin(), disconnectNetworkIdList.end());
			disconnectNetworkIdList.erase(unique(disconnectNetworkIdList.begin(), disconnectNetworkIdList.end()), disconnectNetworkIdList.end());
		}

		for (int networkID : disconnectNetworkIdList)
		{
			cs_sc_NotificationPacket packet(networkID, ENotificationType::DisconnectServer);
			packet.Write(memoryStream);
		}

		if (memoryStream.GetLength() > 0)
		{
			room.SendPacketToAllClients(memoryStream.GetBufferPtr(), memoryStream.GetLength());
			Sleep(200);
		}

		for (int networkID : disconnectNetworkIdList)
		{
			Log("BattleInfo", "[크립] {0}번 클라이언트 체력 0으로 접속 종료", networkID);
			Server::Disconnect(networkID, false);
		}
	}

FinishBattle:

	Log("FinishBattleResultInfo", "----------");

	for (Client* pClient : room.GetClients())
	{
		Log("FinishBattleResultInfo", "[크립] {0}번 클라이언트 체력 : {1}", pClient->GetNetworkID(), pClient->GetHp());
	}

	Log("FinishBattleResultInfo", "----------");

	if (room.GetClients().size() == 1)
	{
		int lastClientNetworkID = room.GetClients()[0]->GetNetworkID();
		cs_sc_NotificationPacket packet(lastClientNetworkID, ENotificationType::DisconnectServer);
		room.SendPacketToAllClients(&packet);
		Sleep(100);
		Server::Disconnect(lastClientNetworkID, false);
	}

	Sleep(200);

	if (!room.mIsRun || room.GetSize() < 2 || room.GetOpenCount() != roomOpenCount)
	{
		return false;
	}

	Sleep(1000);

	{
		sc_FadeInPacket packet(1);
		room.SendPacketToAllClients(&packet);
	}

	Sleep(1000);

	return true;
}

bool Room::IsValidClientInThisRoom(Client* client) const
{
	if (client->GetSocket() == INVALID_SOCKET) { return false; }
	if (client->GetRoomPtr() != this) { return false; }
	if (client->GetRoomOpenCount() != mOpenCount) { return false; }
	return true;
}

BattleAvatar Room::GetCreepMonster()
{
	BattleAvatar monster;
	monster.SetIsGhost(true);
	monster.SetMaxHP(100);
	const ECreepType curCreepType = GetCurCreepType();
	switch (curCreepType)
	{
	case ECreepType::Shrimp:
		for (int i = 0; i < MAX_USING_ITEM_COUNT; ++i)
		{
			Item item;
			item.SetType(38);
			item.SetUpgrade(0);
			monster.SetUsingItem(i, item);
		}
		monster.SetMaxHP(30);
		break;
	case ECreepType::NegativeMan:
		for (int i = 0; i < MAX_USING_ITEM_COUNT; ++i)
		{
			Item item;
			item.SetType(39);
			item.SetUpgrade(0);
			monster.SetUsingItem(i, item);
		}
		monster.SetMaxHP(30);
		break;
	case ECreepType::Hodd:
		for (int i = 0; i < MAX_USING_ITEM_COUNT; ++i)
		{
			Item item;
			item.SetType(40);
			item.SetUpgrade(0);
			monster.SetUsingItem(i, item);
		}
		monster.SetMaxHP(30);
		break;
	case ECreepType::Wakpago:
		for (int i = 0; i < MAX_USING_ITEM_COUNT; ++i)
		{
			Item item;
			item.SetType(41);
			item.SetUpgrade(0);
			monster.SetUsingItem(i, item);
		}
		break;
	case ECreepType::ShortAnswer:
		for (int i = 0; i < MAX_USING_ITEM_COUNT; i += 2)
		{
			Item item;
			item.SetType(42);
			item.SetUpgrade(0);
			monster.SetUsingItem(i, item);
			item.SetType(43);
			item.SetUpgrade(0);
			monster.SetUsingItem(i + 1, item);
		}
		break;
	case ECreepType::Chunsik:
		for (int i = 0; i < MAX_USING_ITEM_COUNT; ++i)
		{
			Item item;
			item.SetType(44);
			item.SetUpgrade(0);
			monster.SetUsingItem(i, item);
		}
		break;
	case ECreepType::KwonMin:
		for (int i = 0; i < MAX_USING_ITEM_COUNT; ++i)
		{
			Item item;
			item.SetType(45);
			item.SetUpgrade(0);
			monster.SetUsingItem(i, item);
		}
		break;
	}
	++mCreepRound;
	return monster;
}

ECreepType Room::GetCurCreepType() const
{
	if (mCreepRound >= static_cast<int>(ECreepType::KwonMin))
	{
		return ECreepType::KwonMin;
	}

	return static_cast<ECreepType>(mCreepRound);
}

CreepRewardInfo Room::GetCreepRewardTicketType() const
{
	EItemTicketType ticketType = EItemTicketType::Normal;
	int count = 1;
	const ECreepType curCreepType = GetCurCreepType();
	switch (curCreepType)
	{
	case ECreepType::Shrimp:
	case ECreepType::NegativeMan:
	case ECreepType::Hodd:
		ticketType = EItemTicketType::Advanced;
		count = 1;
		break;
	case ECreepType::Wakpago:
	case ECreepType::ShortAnswer:
		ticketType = EItemTicketType::Advanced;
		count = 3;
		break;
	case ECreepType::Chunsik:
		ticketType = EItemTicketType::Top;
		count = 2;
		break;
	case ECreepType::KwonMin:
		Random<int> gen(0, 99);
		if (gen() < 20)
		{
			ticketType = EItemTicketType::Supreme;
			count = 1;
		}
		else
		{
			ticketType = EItemTicketType::Top;
			count = 3;
		}
		break;
	}
	return CreepRewardInfo(ticketType, count);
}
