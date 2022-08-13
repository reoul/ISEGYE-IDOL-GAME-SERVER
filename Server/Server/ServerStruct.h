#pragma once
#include <cstdint>
#include <mutex>
#include <WinSock2.h>
#include "SettingData.h"

using namespace std;

class Room;

enum class OperationType { Send, Recv, Accept };
/*
 * FREE : �ƹ��� ������� ����, �Ҵ� ����, ���� ����
 * ALLOCATED : ����Ǿ���, ��� �Ұ���, ���� ����
 *		���� login ó���� ������ ���� Ŭ���̾�Ʈ�̴� �����͸� ������ �� ��
 * ACTIVE : ��� �Ұ���, ���� ����
 */
enum C_STATUS { ST_FREE, ST_ALLOCATED, ST_ACTIVE };
enum class CharacterType : uint8_t { Woowakgood, Ine, Jingberger, Lilpa, Jururu, Gosegu, Viichan };

//Ȯ�� overlapped ����ü
struct Exover
{
	WSAOVERLAPPED	over;
	OperationType	type;					// send, recv, accpet �� �������� 
	char			io_buf[MAX_BUF_SIZE];	// ������ ��ġ ����
	WSABUF			wsabuf;					// ������ ���� �ٿ��� ���� ���� ��ü�� ����. �ѱ��� ���� �θ� Ȯ�屸��ü�� �����ϸ� ��ü�� ���� �ȴ�.
	SOCKET			c_socket;
};

//Ŭ���̾�Ʈ ���� ���� ����ü
struct Client
{
	mutex				cLock;
	SOCKET				socket;
	int32_t				networkID;						// Ŭ���̾�Ʈ ���̵�
	Exover				recvOver;						// Ȯ�� overlapped ����ü
	int32_t				prevSize;						// ������ �޾Ƴ��� ��
	char				packetBuf[MAX_PACKET_SIZE];		// ������ �� �޾Ƶα� ���� ����
	C_STATUS			status;							// �����߳� ���߳�
	shared_ptr<Room>	room;							// Ŭ���̾�Ʈ�� ���� ��
	CharacterType		characterType;					// �÷��̾� ĳ����
	wchar_t				name[MAX_USER_NAME_LENGTH]; // �÷��̾� �̸�
	Client()
		: socket(NULL)
		, networkID()
		, recvOver()
		, prevSize()
		, packetBuf{}
		, status()
		, room(nullptr)
		, characterType(CharacterType::Woowakgood)
	{
		name[0] = '\0';
	}
};
