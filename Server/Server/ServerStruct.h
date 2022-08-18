#pragma once
#include <cstdint>
#include <mutex>
#include <WinSock2.h>
#include "SettingData.h"

using namespace std;

class Room;

enum class OperationType { Send, Recv, Accept };
/*
 * FREE : 아무도 사용하지 않음, 할당 가능, 접근 금지
 * ALLOCATED : 예약되었음, 사용 불가능, 접근 금지
 *		아직 login 처리가 끝나지 않은 클라이언트이니 데이터를 보내지 말 것
 * ACTIVE : 사용 불가능, 접근 가능
 */
enum C_STATUS : uint8_t { ST_FREE, ST_ALLOCATED, ST_ACTIVE };
enum class CharacterType : uint8_t { Woowakgood, Ine, Jingberger, Lilpa, Jururu, Gosegu, Viichan };

//확장 overlapped 구조체
struct Exover
{
	WSAOVERLAPPED	over;
	OperationType	type;					// send, recv, accpet 중 무엇인지 
	char			io_buf[MAX_BUF_SIZE];	// 버퍼의 위치 관리
	WSABUF			wsabuf;					// 포인터 넣을 바에야 차라리 버퍼 자체를 넣자. 한군데 같이 두면 확장구조체를 재사용하면 전체가 재사용 된다.
	SOCKET			c_socket;
};

//클라이언트 정보 저장 구조체
struct Client
{
	mutex				cLock;
	SOCKET				socket;
	int32_t				networkID;						// 클라이언트 아이디
	Exover				recvOver;						// 확장 overlapped 구조체
	int32_t				prevSize;						// 이전에 받아놓은 양
	char				packetBuf[MAX_PACKET_SIZE];		// 조각난 거 받아두기 위한 버퍼
	bool				isAlive;						// 플레이 도중에 살아있는지(HP가 0이 아닌경우)
	C_STATUS			status;							// 접속했나 안했나
	shared_ptr<Room>	room;							// 클라이언트가 속한 룸
	CharacterType		characterType;					// 플레이어 캐릭터
	wchar_t				name[MAX_USER_NAME_LENGTH]; // 플레이어 이름
	Client()
		: socket(NULL)
		, networkID(0)
		, recvOver()
		, prevSize(0)
		, packetBuf{}
		, isAlive(false)
		, status(ST_FREE)
		, room(nullptr)
		, characterType(CharacterType::Woowakgood)
	{
		name[0] = '\0';
	}
};
