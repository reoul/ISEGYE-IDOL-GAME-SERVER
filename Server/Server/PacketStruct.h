#pragma once
#include <cstdint>

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
