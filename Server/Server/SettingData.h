#pragma once

constexpr int MAX_ROOM_PLAYER = 8;	// Room 최대 플레이어 수
constexpr int MAX_BUF_SIZE = 1300;	// 버퍼 최대 크기
constexpr int MAX_PACKET_SIZE = 1300;	// 패킷 최대 크기
constexpr int MAX_USER = 20000;	// 최대 유저
constexpr int SERVER_PORT = 51341;	// 서버 포트
constexpr int MAX_USER_NAME_LENGTH = 11;	// 유저 닉네임 최대 길이
constexpr int MAX_USING_ITEM_COUNT = 6;	// 최대 필드 유물 개수
constexpr int MAX_UN_USING_ITEM_COUNT = 10; // 최대 인벤토리 개수
constexpr int BATTLE_ITEM_QUEUE_LOOP_COUNT = 5;		// 전투 아이템 순서 반복 횟수
constexpr int CHOICE_CHARACTER_TIME = 60;	// 캐릭터 선택 시간
constexpr int BATTLE_READY_TIME = 60;	// 전투 준비 시간
constexpr int CONNECT_CHECK_INTERVAL = 30;	// 접속 체크 간격
constexpr int BATTLE_ITEM_QUEUE_LENGTH = MAX_ROOM_PLAYER * MAX_USING_ITEM_COUNT * BATTLE_ITEM_QUEUE_LOOP_COUNT * 2 + MAX_ROOM_PLAYER;
constexpr int MAX_ROOM_COUNT = MAX_USER / MAX_ROOM_PLAYER;	// 최대 Room 개수
constexpr int MAX_ITEM_UPGRADE = 5;	// 아이템 최대 강화 수치
constexpr int DEFAULT_ITEM_TICKET_COUNT = 0;


// 전투
constexpr int MAX_CHARACTER_MAX_HP = 100;		// 만약 변경한다면 잃은 체력 비례 계산 수정해야함 (ex : 선량한 시민의 빠루)
constexpr int MAX_AVATAR_MAX_HP = 100;
constexpr int START_AVATAR_DEFENSIVE = 0;