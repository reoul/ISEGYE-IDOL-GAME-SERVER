#pragma once
#include <cstdint>
#include <mutex>
#include "SettingData.h"

enum class OperationType { Send, Recv, Accept };
/*
 * FREE : �ƹ��� ������� ����, �Ҵ� ����, ���� ����
 * ALLOCATED : ����Ǿ���, ��� �Ұ���, ���� ����
 *		���� login ó���� ������ ���� Ŭ���̾�Ʈ�̴� �����͸� ������ �� ��
 * ACTIVE : ��� �Ұ���, ���� ����
 */
enum C_STATUS { ST_FREE, ST_ALLOCATED, ST_ACTIVE };

//Ȯ�� overlapped ����ü
struct EXOVER
{
	WSAOVERLAPPED	over;
	OperationType	type;					// send, recv, accpet �� �������� 
	char			io_buf[MAX_BUF_SIZE];	// ������ ��ġ ����
	WSABUF			wsabuf;					// ������ ���� �ٿ��� ���� ���� ��ü�� ����. �ѱ��� ���� �θ� Ȯ�屸��ü�� �����ϸ� ��ü�� ���� �ȴ�.
	SOCKET			c_socket;
};

//Ŭ���̾�Ʈ ���� ���� ����ü
struct CLIENT
{
	mutex		cLock;
	SOCKET		socket;
	int			id;							// Ŭ���̾�Ʈ ���̵�
	EXOVER		recvOver;					// Ȯ�� overlapped ����ü
	int			prevSize;					// ������ �޾Ƴ��� ��
	char		packetBuf[MAX_PACKET_SIZE];	// ������ �� �޾Ƶα� ���� ����
	C_STATUS	status;						// �����߳� ���߳�
};

struct TestPacket
{
	uint32_t type;
	uint32_t networkID;
	int data;

	TestPacket(uint32_t t, uint32_t n, int d)
		: type(t)
		, networkID(n)
		, data(d)
	{
	}
};
