#pragma once

constexpr int MAX_ROOM_PLAYER = 8;
constexpr int MAX_BUF_SIZE = 1300;
constexpr int MAX_PACKET_SIZE = 1300;
constexpr int MAX_USER = 5000;
constexpr int SERVER_PORT = 51341;
constexpr int MAX_USER_NAME_LENGTH = 11;
constexpr int MAX_USING_ITEM = 6;
constexpr int MAX_UN_USING_ITEM = 10;
constexpr int ITEM_COUNT = 7;
constexpr int BATTLE_ITEM_QUEUE_LOOP_COUNT = 5;
constexpr int CHOOSE_CHARACTER_TIME = 60;
constexpr int BATTLE_READY_TIME = 60;
constexpr int CONNECT_CHECK_INTERVAL = 30;
constexpr int BATTLE_ITEM_QUEUE_LENGTH = MAX_ROOM_PLAYER * MAX_USING_ITEM * BATTLE_ITEM_QUEUE_LOOP_COUNT * 2 + MAX_ROOM_PLAYER;
constexpr int MAX_ROOM_COUNT = MAX_USER / MAX_ROOM_PLAYER;


// 전투
constexpr int MAX_CHARACTER_MAX_HP = 100;
constexpr int MAX_AVATAR_MAX_HP = 100;
constexpr int START_AVATAR_DEFENSIVE = 0;