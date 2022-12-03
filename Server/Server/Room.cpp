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

vector<int32_t> Room::GetRandomItemQueue()
{
	vector<SlotInfo> items;

	vector<int32_t> itemQueue;
	itemQueue.reserve(BATTLE_ITEM_QUEUE_LENGTH);

	for (auto it = mClients.begin(); it != mClients.end(); ++it)
	{
		Client& client = **it;
		itemQueue.emplace_back(client.GetNetworkID());

		const vector<Item> usingItems = client.GetUsingItems();
		for (size_t i = 0; i < MAX_USING_ITEM; ++i)
		{
			items.emplace_back(i, usingItems[i]);
		}

		LogWriteTest("log", "{0} 클라이언트 {1}개 아이템 장착중", client.GetNetworkID(), items.size());

		const size_t length = items.size();
		log_assert(length <= MAX_USING_ITEM);

		int sum = 0;
		for (const SlotInfo& slotInfo : items)
		{
			sum += slotInfo.item.GetActivePercent();
		}

		items.reserve(MAX_USING_ITEM * BATTLE_ITEM_QUEUE_LOOP_COUNT);
		CopySelf(items, BATTLE_ITEM_QUEUE_LOOP_COUNT - 1);

		if (items.empty())
		{
			for (size_t i = 0; i < MAX_USING_ITEM * BATTLE_ITEM_QUEUE_LOOP_COUNT; ++i)
			{
				itemQueue.emplace_back(EMPTY_ITEM);
				itemQueue.emplace_back(ACTIVATE_ITEM);
			}
		}
		else
		{
			int loopSum = sum;
			size_t loopLength = length;

			while (!items.empty())
			{
				Random<int> gen(0, loopSum - 1);
				int rand = gen();

				auto iter = items.begin();
				for (size_t i = 0; i < loopLength; ++i)
				{
					const int itemActivePercent = iter->item.GetActivePercent();
					rand -= itemActivePercent;
					if (rand < 0)
					{
						--loopLength;
						loopSum -= itemActivePercent;
						itemQueue.emplace_back(iter->index);
						itemQueue.emplace_back(ACTIVATE_ITEM);
						items.erase(iter);
						break;
					}
					++iter;
				}

				if (loopLength == 0)
				{
					loopSum = sum;
					loopLength = length;

					const int lockSlotCnt = client.GetLockSlotCount();
					for (size_t i = 0; i < MAX_USING_ITEM - length - lockSlotCnt; ++i)
					{
						itemQueue.emplace_back(EMPTY_ITEM);
						itemQueue.emplace_back(ACTIVATE_ITEM);
					}

					for (size_t i = 0; i < lockSlotCnt; ++i)
					{
						itemQueue.emplace_back(LOCK_ITEM);
						itemQueue.emplace_back(DISABLE_ITEM);
					}
				}
			}
		}

		items.clear();
	}

	for (size_t i = 0; i < MAX_ROOM_PLAYER - mSize; ++i)
	{
		itemQueue.emplace_back(-1);
		for (size_t j = 0; j < MAX_USING_ITEM * BATTLE_ITEM_QUEUE_LOOP_COUNT; ++j)
		{
			itemQueue.emplace_back(0);
			itemQueue.emplace_back(0);
		}
	}
	log_assert(itemQueue.size() == BATTLE_ITEM_QUEUE_LENGTH);
	for (size_t i = 0; i < MAX_ROOM_PLAYER; ++i)
	{
		size_t index = i * 60 + i;
		for (size_t j = 0; j < BATTLE_ITEM_QUEUE_LOOP_COUNT; ++j)
		{
			size_t index2 = index + j * (MAX_USING_ITEM * 2);
			LogWriteTest("log", "[아이템순서] 네트워크 {0}번 클라이언트 : {1}번째 순서 === {2}:{3}  {4}:{5}  {6}:{7}  {8}:{9}  {10}:{11}  {12}:{13} ",
				itemQueue[index], j, itemQueue[index2 + 1], itemQueue[index2 + 2], itemQueue[index2 + 3], itemQueue[index2 + 4],
				itemQueue[index2 + 5], itemQueue[index2 + 6], itemQueue[index2 + 7], itemQueue[index2 + 8],
				itemQueue[index2 + 9], itemQueue[index2 + 10], itemQueue[index2 + 11], itemQueue[index2 + 12]);
		}
	}

	return itemQueue;
}

void Room::SetBattleInfo()
{
	lock_guard<mutex> lg(cLock);

	if (mSize < 2)
	{
		return;
	}

	mBattleOpponents = mBattleManager.GetBattleOpponent();
	mItemQueues = GetRandomItemQueue();
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
	Room* pRoom = static_cast<Room*>(pArguments);
	const size_t roomOpenCount = pRoom->GetOpenCount();

	for (int i = 0; i < CHOICE_CHARACTER_TIME; ++i)
	{
		Sleep(1000);

		{
			lock_guard<mutex> lg(pRoom->cLock);
			pRoom->TrySendEnterInGame();
		}

		// 모두 선택 완료했을 때
		if (pRoom->mIsFinishChoiceCharacter)
		{
			Log("log", "모두 선택 완료");
			break;
		}

		if (!pRoom->mIsRun || pRoom->GetSize() < 2 || pRoom->GetOpenCount() != roomOpenCount)
		{
			Log("log", "Room 진행 종료");
			_endthreadex(0);
			return 0;
		}
	}

	// 모두 선택 했을 때
	if (pRoom->mIsFinishChoiceCharacter)
	{
		LogWrite("Check", "캐릭터 선택 상태");

		{
			// 캐릭터 선택 동기화
			const size_t bufferSize = sizeof(cs_sc_ChangeCharacterPacket) * pRoom->GetSize();
			OutputMemoryStream memoryStream(bufferSize);

			for (const Client* client : pRoom->GetClients())
			{
				cs_sc_ChangeCharacterPacket packet(client->GetNetworkID(), client->GetCharacterType());
				packet.Write(memoryStream);
				LogWrite("Check", "{0} : {1}", client->GetNetworkID(), static_cast<uint8_t>(client->GetCharacterType()));
			}

			pRoom->SendPacketToAllClients(memoryStream.GetBufferPtr(), bufferSize);

			Sleep(5000);
		}

		if (!pRoom->mIsRun || pRoom->GetSize() < 2 || pRoom->GetOpenCount() != roomOpenCount)
		{
			Log("log", "Room 진행 종료");
			_endthreadex(0);
			return 0;
		}
		
		{
			const size_t bufferSize = sizeof(cs_sc_NotificationPacket) + sizeof(sc_SetReadyTimePacket);
			OutputMemoryStream memoryStream(bufferSize);
			const cs_sc_NotificationPacket notificationPacket(0, ENotificationType::EnterCutSceneStage);
			notificationPacket.Write(memoryStream);

			const sc_SetReadyTimePacket setReadyTimePacket(BATTLE_READY_TIME);
			setReadyTimePacket.Write(memoryStream);

			pRoom->SendPacketToAllClients(memoryStream.GetBufferPtr(), bufferSize);
		}
	}
	else
	{
		LogWrite("Check", "캐릭터 선택 상태");

		// 캐릭터 선택 동기화
		const size_t bufferSize = sizeof(cs_sc_ChangeCharacterPacket) * pRoom->GetSize() + sizeof(cs_sc_NotificationPacket) + sizeof(sc_SetReadyTimePacket);
		OutputMemoryStream memoryStream(bufferSize);

		for (const Client* client : pRoom->GetClients())
		{
			cs_sc_ChangeCharacterPacket packet(client->GetNetworkID(), client->GetCharacterType());
			packet.Write(memoryStream);
			LogWrite("Check", "{0} : {1}", client->GetNetworkID(), static_cast<uint8_t>(client->GetCharacterType()));
		}

		const cs_sc_NotificationPacket notificationPacket(0, ENotificationType::EnterCutSceneStage);
		notificationPacket.Write(memoryStream);

		const sc_SetReadyTimePacket setReadyTimePacket(BATTLE_READY_TIME);
		setReadyTimePacket.Write(memoryStream);

		pRoom->SendPacketToAllClients(memoryStream.GetBufferPtr(), bufferSize);
	}

	if (!pRoom->mIsRun || pRoom->GetSize() < 2 || pRoom->GetOpenCount() != roomOpenCount)
	{
		Log("log", "Room 진행 종료");
		_endthreadex(0);
		return 0;
	}

	Sleep(10000);

	if (!pRoom->mIsRun || pRoom->GetSize() < 2 || pRoom->GetOpenCount() != roomOpenCount)
	{
		LogPrintf("Room 진행 종료");
		_endthreadex(0);
		return 0;
	}

	{
		cs_sc_NotificationPacket packet(0, ENotificationType::FinishCutSceneStage);
		pRoom->SendPacketToAllClients(&packet);
	}

	Log("log", "기본 템 지급 시작");
	// 기본 템 지급
	{
		constexpr size_t bufferSize = sizeof(sc_AddNewItemPacket) * 2 * 8;
		OutputMemoryStream memoryStream(bufferSize);

		for (Client* client : pRoom->mClients)
		{
			constexpr uint8_t defaultItemCode1 = 1;	// 기본템 1
			constexpr uint8_t defaultItemCode2 = 6;	// 기본템 2

			client->AddItem(defaultItemCode1);
			const sc_AddNewItemPacket addItemPacket(client->GetNetworkID(), defaultItemCode1);
			addItemPacket.Write(memoryStream);

			client->AddItem(defaultItemCode2);
			const sc_AddNewItemPacket addItemPacket2(client->GetNetworkID(), defaultItemCode2);
			addItemPacket2.Write(memoryStream);
		}

		pRoom->SendPacketToAllClients(memoryStream.GetBufferPtr(), memoryStream.GetLength());
	}

	Log("log", "기본 템 지급 완료");

	while (true)
	{
		// 대기 시간
		if (!ReadyStage(*pRoom))
		{
			break;
		}

		// 전투
		if (!BattleStage(*pRoom))
		{
			break;
		}

		// 대기 시간
		if (!ReadyStage(*pRoom))
		{
			break;
		}

		// 크립라운드
		if (!CreepStage(*pRoom))
		{
			break;
		}
	}

	Log("log", "Room 진행 종료");
	_endthreadex(0);
	return 0;
}

/**
 * \brief 준비 스테이지 로직
 * \param room 해당 Room
 * \return Room 유지 여부
 */
inline bool Room::ReadyStage(Room& room)
{
	const size_t roomOpenCount = room.GetOpenCount();

	LogPrintf("준비시간 시작");

	if (!room.mIsRun || room.GetSize() < 2 || room.GetOpenCount() != roomOpenCount)
	{
		return false;
	}

	{
		cs_sc_NotificationPacket packet(0, ENotificationType::EnterReadyStage);
		room.SendPacketToAllClients(&packet);
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

	// 기본 템 장착
	for (Client* client : room.GetClients())
	{
		client->TrySetDefaultUsingItem();
	}

	LogPrintf("기본 유물 장착");

	{
		const vector<Client*> clients = room.GetClients();
		const size_t bufferSize = sizeof(sc_UpdateCharacterInfoPacket) * clients.size();
		OutputMemoryStream memoryStream(bufferSize);

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

	return true;
}

/**
 * \brief 전투 스테이지 로직
 * \param room 해당 Room
 * \return Room 유지 여부
 */
bool Room::BattleStage(Room& room)
{
	const size_t roomOpenCount = room.GetOpenCount();

	LogPrintf("전투 스테이지 시작");

	room.SetBattleInfo();	// 전투 정보 전송

	if (!room.mIsRun || room.GetSize() < 2 || room.GetOpenCount() != roomOpenCount)
	{
		return false;
	}

	{
		constexpr size_t bufferSize = sizeof(sc_BattleInfoPacket) + sizeof(cs_sc_NotificationPacket);
		OutputMemoryStream memoryStream(bufferSize);

		lock_guard<mutex> lg(room.cLock);

		const sc_BattleInfoPacket battleInfoPacket(room.GetBattleOpponents(), room.GetItemQueues());
		battleInfoPacket.Write(memoryStream);

		const cs_sc_NotificationPacket notificationPacket(0, ENotificationType::EnterBattleStage);
		notificationPacket.Write(memoryStream);

		room.SendPacketToAllClients(memoryStream.GetBufferPtr(), bufferSize);
	}

	if (!room.mIsRun || room.GetSize() < 2 || room.GetOpenCount() != roomOpenCount)
	{
		return false;
	}

	const vector<int32_t>& battleOpponents = room.GetBattleOpponents();
	const vector<int32_t>& itemQueues = room.GetItemQueues();

	constexpr int waitTimes[2]{ 1000, 500 };

	for (size_t i = 0; i < 30; ++i)
	{
		// 아이템 사용했다는 패킷 보내고
		for (size_t j = 0; j < MAX_USING_ITEM; ++j)
		{
			for (size_t k = 0; k < battleOpponents.size(); k += 2)
			{
				Client& client1 = Server::GetClients(battleOpponents[k]);
				Client& client2 = Server::GetClients(battleOpponents[k + 1]);
				// 아이템 사용
				sItems[itemQueues[0]]->Use(client1, client2, 0);
			}
			Sleep(waitTimes[0]);

			if (!room.mIsRun || room.GetSize() < 2 || room.GetOpenCount() != roomOpenCount)
			{
				return false;
			}
		}
	}

	if (!room.mIsRun || room.GetSize() < 2 || room.GetOpenCount() != roomOpenCount)
	{
		return false;
	}

	return true;
}

/**
 * \brief 크립 스테이지 로직
 * \param room 해당 Room
 * \return Room 유지 여부
 */
bool Room::CreepStage(Room& room)
{
	const size_t roomOpenCount = room.GetOpenCount();

	LogPrintf("크립 스테이지 시작");

	{
		cs_sc_NotificationPacket packet(0, ENotificationType::EnterCreepStage);
		room.SendPacketToAllClients(&packet);
	}

	if (!room.mIsRun || room.GetSize() < 2 || room.GetOpenCount() != roomOpenCount)
	{
		return false;
	}
	
	return true;
}
