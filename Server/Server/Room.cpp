#include "Room.h"

#include <iostream>
#include <reoul/functional.h>
#include <reoul/logger.h>
#include "Client.h"
#include "Random.h"
#include "PacketStruct.h"
#include "Server.h"
#include "Items.h"

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

	{
		lock_guard<mutex> lg(Server::GetRoomManager().cLock);
		Log("log", "{0}번 룸 비활성화 (현재 활성화된 방 : {1})", mNumber, Server::GetRoomManager().GetUsingRoomCount());
	}

	mRound = 0;
	mCurRoomStatusType = ERoomStatusType::ChoiceCharacter;
	mCreepRound = 0;
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
		constexpr size_t bufferSize = sizeof(sc_AddNewItemPacket) * 2 * 8 + sizeof(sc_SetItemTicketPacket) * 4 * 8;
		OutputMemoryStream memoryStream(bufferSize);

		for (Client* client : room.mClients)
		{
			constexpr uint8_t defaultItemCode1 = 1;	// 기본템 1
			constexpr uint8_t defaultItemCode2 = 4;	// 기본템 2

			uint8_t slot1 = client->AddItem(defaultItemCode1);
			const sc_AddNewItemPacket addItemPacket(client->GetNetworkID(), slot1, defaultItemCode1);
			addItemPacket.Write(memoryStream);

			uint8_t slot2 = client->AddItem(defaultItemCode2);
			const sc_AddNewItemPacket addItemPacket2(client->GetNetworkID(), slot2, defaultItemCode2);
			addItemPacket2.Write(memoryStream);

			client->SetNormalItemTicketCount(10);
			client->SetAdvancedItemTicketCount(10);
			client->SetTopItemTicketCount(10);
			client->SetSupremeItemTicketCount(10);

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
	// todo : 크립 추가시 해제하기
	/*ReadyStage(room, false);

	Sleep(1000);

	CreepStage(room);

	Sleep(1000);

	ReadyStage(room, false);

	Sleep(1000);

	CreepStage(room);

	Sleep(1000);

	ReadyStage(room, false);

	Sleep(1000);

	CreepStage(room);*/

	Sleep(1000);

	while (true)
	{
		for (int i = 0; i < 4; ++i)
		{
			// 대기 시간
			if (!ReadyStage(room, true))
			{
				goto loopOut;
			}

			Sleep(1000);

			// 전투
			if (!BattleStage(room))
			{
				goto loopOut;
			}

			Sleep(1000);

			// 대기 시간
			if (!ReadyStage(room, false))
			{
				goto loopOut;
			}

			Sleep(1000);

			// 크립
			if (!CreepStage(room))
			{
				break;
			}

			Sleep(1000);

			// todo : 없애야함
			++room.mRound;
		}

		// 대기 시간
		// todo : 이거 크립 추가되면 해제하기
		//if (!ReadyStage(room, false))
		//{
		//	break;
		//}

		Sleep(1000);

		//// 크립
		//if (!CreepStage(room))
		//{
		//	break;
		//}

		Sleep(1000);

		//++room.mRound;
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
inline bool Room::ReadyStage(Room& room, bool isNextStageBattle)
{
	room.mCurRoomStatusType = ERoomStatusType::ReadyStage;

	{
		sc_FadeOutPacket packet(1);
		room.SendPacketToAllClients(&packet);
	}
	Sleep(1000);

	const size_t roomOpenCount = room.GetOpenCount();


	LogPrintf("준비시간 시작");

	if (!room.mIsRun || room.GetSize() < 2 || room.GetOpenCount() != roomOpenCount)
	{
		return false;
	}

	{
		const size_t bufferSize = (isNextStageBattle ? sizeof(sc_BattleOpponentsPacket) : 0) + sizeof(cs_sc_NotificationPacket);
		OutputMemoryStream memoryStream(bufferSize);

		if (isNextStageBattle)
		{
			room.ApplyRandomBattleOpponent();
			sc_BattleOpponentsPacket packet(room.GetBattleOpponents());
			room.SendPacketToAllClients(&packet);
		}

		{
			cs_sc_NotificationPacket packet(0, ENotificationType::EnterReadyStage);
			room.SendPacketToAllClients(&packet);
		}
	}


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

	{
		sc_FadeOutPacket packet(1);
		room.SendPacketToAllClients(&packet);
	}

	Sleep(1000);

	const size_t roomOpenCount = room.GetOpenCount();

	LogPrintf("전투 스테이지 시작");

	if (!room.mIsRun || room.GetSize() < 2 || room.GetOpenCount() != roomOpenCount)
	{
		return false;
	}

	const vector<int32_t>& battleOpponents = room.GetBattleOpponents();

	// todo : 나중에 속도 빠르게 수정
	constexpr int waitTimes[2]{ 500, 500 };

	const int avatarCount = battleOpponents.size();

	unique_ptr<BattleAvatar[]> avatars = make_unique<BattleAvatar[]>(battleOpponents.size());
	for (int i = 0; i < battleOpponents.size(); ++i)
	{
		const int networkID = battleOpponents[i] >= 0 ? battleOpponents[i] : ~battleOpponents[i];
		Client& client = Server::GetClients(networkID);
		avatars[i].SetAvatar(client, battleOpponents[i] < 0);
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

					sc_SetHamburgerTypePacket packet(avatars[i].GetNetworkID(), j, hamburgerType);
					packet.Write(memoryStream);
					Logger::Log("log", "{0}번 클라이언트 햄버거 {1}번 햄버거로 설정됨", avatars[i].GetNetworkID(), static_cast<int>(hamburgerType));
				}
				else if (item.GetType() == 16)	// 박사의 만능툴
				{
					int index = i % 2 == 0 ? 1 : -1;	// 0-1   2-3   4-5  6-7 이렇게 전투하기 때문에 인덱스 번호에 따라 상대 인덱스가 다르다
					BattleAvatar& opponent = avatars[i + index];
					Item choiceItem = opponent.GetRandomCopyItem();
					sc_DoctorToolInfoPacket packet(avatars[i].GetNetworkID(), j, choiceItem.GetType(), choiceItem.GetUpgrade());
					packet.Write(memoryStream);
					item.SetType(choiceItem.GetType());
					item.SetUpgrade(choiceItem.GetUpgrade());
					Logger::Log("log", "{0}번 클라이언트 만능툴 {1}번 아이템 강화 수치 {2}으로 설정됨", avatars[i].GetNetworkID(), choiceItem.GetType(), choiceItem.GetUpgrade());
				}
			}
		}

		room.SendPacketToAllClients(memoryStream.GetBufferPtr(), memoryStream.GetLength());
	}

	if (!room.mIsRun || room.GetSize() < 2 || room.GetOpenCount() != roomOpenCount)
	{
		return false;
	}

	Sleep(1000);

	for (int i = 0; i < battleOpponents.size(); i += 2)
	{
		if (avatars[i].GetFirstAttackState() < avatars[i + 1].GetFirstAttackState())
		{
			BattleAvatar avatar1 = avatars[i];
			avatars[i] = avatars[i + 1];
			avatars[i + 1] = avatar1;
			// todo : 둘중에 한명이라도 나갔으면 처리하기
		}
	}

	for (size_t battleLoop = 0; battleLoop < 20; ++battleLoop)
	{
		for (int j = 0; j < battleOpponents.size(); ++j)
		{
			const vector<SlotInfo> itemQueue = Server::GetClients(avatars[j].GetNetworkID()).GetItemActiveQueue();
			avatars[j].SetActiveQueue(itemQueue);
		}

		{
			OutputMemoryStream memoryStream;

			int packetSize = 0;
			for (int k = 0; k < avatarCount; ++k)
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
				OutputMemoryStream memoryStream;

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

						avatars[k].SetFinish();
						avatars[k + 1].SetFinish();
					}

					if (!avatars[k].IsFinish() && !avatars[k + 1].IsFinish())
					{
						avatars[k].EffectBleeding();
					}

					if (avatars[k].GetHP() == 0 || avatars[k + 1].GetHP() == 0)
					{
						avatars[k].ToDamageCharacter(avatars[k + 1].GetDamage());
						avatars[k + 1].ToDamageCharacter(avatars[k].GetDamage());

						avatars[k].SetFinish();
						avatars[k + 1].SetFinish();
					}

					sc_ActiveItemPacket packet(avatars[k].GetNetworkID(), activeSlot);
					packet.Write(memoryStream);
					packetSize += sizeof(sc_ActiveItemPacket);
				}
				if (packetSize > 0)
				{
					room.SendPacketToAllClients(memoryStream.GetBufferPtr(), packetSize);
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
				OutputMemoryStream memoryStream;

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

						avatars[k].SetFinish();
						avatars[k - 1].SetFinish();
					}

					if (!avatars[k].IsFinish() && !avatars[k - 1].IsFinish())
					{
						avatars[k].EffectBleeding();
					}

					if (avatars[k].GetHP() == 0 || avatars[k - 1].GetHP() == 0)
					{
						avatars[k].ToDamageCharacter(avatars[k - 1].GetDamage());
						avatars[k - 1].ToDamageCharacter(avatars[k].GetDamage());

						avatars[k].SetFinish();
						avatars[k - 1].SetFinish();
					}

					sc_ActiveItemPacket packet(avatars[k].GetNetworkID(), activeSlot);
					packet.Write(memoryStream);
					packetSize += sizeof(sc_ActiveItemPacket);
				}
				if (packetSize > 0)
				{
					room.SendPacketToAllClients(memoryStream.GetBufferPtr(), packetSize);
				}
			}

			Sleep(waitTimes[battleLoop / 10]);

			for (k = 0; k < avatarCount; k += 2)
			{
				if (avatars[k].IsFinish())
				{
					continue;
				}

				avatars[k].EffectBomb();
				avatars[k].InitCycle();
				avatars[k + 1].EffectBomb();
				avatars[k + 1].InitCycle();
				if (avatars[k].GetHP() == 0 || avatars[k + 1].GetHP() == 0)
				{
					avatars[k].SetFinish();
					avatars[k + 1].SetFinish();
				}
			}

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
	}

	// 시간이 다 지났는데도 안끝난 전투가 있으면 강제 데미지 10
	for (int k = 0; k < avatarCount; k += 2)
	{
		if (avatars[k].IsFinish())
		{
			continue;
		}
		avatars[k].ToDamageCharacter(10);
		avatars[k + 1].ToDamageCharacter(10);
	}

FinishBattle:

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
 * \brief 크립 스테이지 로직
 * \param room 해당 Room
 * \return Room 유지 여부
 */
bool Room::CreepStage(Room& room)
{
	room.mCurRoomStatusType = ERoomStatusType::CreepStage;

	{
		sc_FadeOutPacket packet(1);
		room.SendPacketToAllClients(&packet);
	}

	Sleep(1000);

	const size_t roomOpenCount = room.GetOpenCount();

	LogPrintf("크립 스테이지 시작");

	ECreepType curCreeType = room.GetCurCreepType();

	const int avatarCount = room.GetClients().size() * 2;
	
	const BattleAvatar creepMonster = room.GetCreepMonster();

	unique_ptr<BattleAvatar[]> avatars = make_unique<BattleAvatar[]>(avatarCount);
	int clientIndex = 0;
	for (int i = 0; i < avatarCount; i += 2)
	{
		avatars[i].SetAvatar(*room.GetClients()[clientIndex++], false);
		avatars[i + 1] = creepMonster;
	}

	{
		OutputMemoryStream memoryStream(512);
		cs_sc_NotificationPacket notificationPacket(0, ENotificationType::EnterCreepStage);
		notificationPacket.Write(memoryStream);
		sc_CreepStageInfoPacket creepStageInfoPacket(curCreeType);
		creepStageInfoPacket.Write(memoryStream);

		for (int i = 0; i < avatarCount; ++i)
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
					Logger::Log("log", "{0}번 클라이언트 햄버거 {1}번 햄버거로 설정됨", avatars[i].GetNetworkID(), static_cast<int>(hamburgerType));
				}
			}
		}

		room.SendPacketToAllClients(memoryStream.GetBufferPtr(), memoryStream.GetLength());
	}

	// todo : 나중에 속도 빠르게 수정
	constexpr int waitTimes[2]{ 500, 500 };

	Sleep(1000);

	if (!room.mIsRun || room.GetSize() < 2 || room.GetOpenCount() != roomOpenCount)
	{
		return false;
	}


	for (size_t battleLoop = 0; battleLoop < 20; ++battleLoop)
	{
		for (int j = 0; j < avatarCount; ++j)
		{
			const vector<SlotInfo> itemQueue = Server::GetClients(avatars[j].GetNetworkID()).GetItemActiveQueue();
			avatars[j].SetActiveQueue(itemQueue);
		}

		{
			OutputMemoryStream memoryStream;

			int packetSize = 0;
			for (int k = 0; k < avatarCount; ++k)
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
				OutputMemoryStream memoryStream;

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

						avatars[k].SetFinish();
						avatars[k + 1].SetFinish();
					}

					if (!avatars[k].IsFinish() && !avatars[k + 1].IsFinish())
					{
						avatars[k].EffectBleeding();
					}

					if (avatars[k].GetHP() == 0 || avatars[k + 1].GetHP() == 0)
					{
						avatars[k].ToDamageCharacter(avatars[k + 1].GetDamage());
						avatars[k + 1].ToDamageCharacter(avatars[k].GetDamage());

						avatars[k].SetFinish();
						avatars[k + 1].SetFinish();
					}

					sc_ActiveItemPacket packet(avatars[k].GetNetworkID(), activeSlot);
					packet.Write(memoryStream);
					packetSize += sizeof(sc_ActiveItemPacket);
				}
				if (packetSize > 0)
				{
					room.SendPacketToAllClients(memoryStream.GetBufferPtr(), packetSize);
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

						avatars[k].SetFinish();
						avatars[k - 1].SetFinish();
					}

					if (!avatars[k].IsFinish() && !avatars[k - 1].IsFinish())
					{
						avatars[k].EffectBleeding();
					}

					if (avatars[k].GetHP() == 0 || avatars[k - 1].GetHP() == 0)
					{
						avatars[k].ToDamageCharacter(avatars[k - 1].GetDamage());
						avatars[k - 1].ToDamageCharacter(avatars[k].GetDamage());

						avatars[k].SetFinish();
						avatars[k - 1].SetFinish();
					}
					
					packetSize += sizeof(cs_sc_NotificationPacket);
				}

				if (packetSize > 0)
				{
					cs_sc_NotificationPacket packet(0, ENotificationType::EffectCreepItem);
					room.SendPacketToAllClients(&packet);
				}
			}

			Sleep(waitTimes[battleLoop / 10]);

			for (k = 0; k < avatarCount; k += 2)
			{
				if (avatars[k].IsFinish())
				{
					continue;
				}

				avatars[k].EffectBomb();
				avatars[k].InitCycle();
				avatars[k + 1].EffectBomb();
				avatars[k + 1].InitCycle();
				if (avatars[k].GetHP() == 0 || avatars[k + 1].GetHP() == 0)
				{
					avatars[k].SetFinish();
					avatars[k + 1].SetFinish();
				}
			}

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
	}

FinishBattle:

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

bool Room::IsValidClientInThisRoom(Client* client) const
{
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
		break;
	case ECreepType::NegativeMan:
		for (int i = 0; i < MAX_USING_ITEM_COUNT; ++i)
		{
			Item item;
			item.SetType(39);
			item.SetUpgrade(0);
			monster.SetUsingItem(i, item);
		}
		break;
	case ECreepType::Hodd:
		for (int i = 0; i < MAX_USING_ITEM_COUNT; ++i)
		{
			Item item;
			item.SetType(40);
			item.SetUpgrade(0);
			monster.SetUsingItem(i, item);
		}
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
