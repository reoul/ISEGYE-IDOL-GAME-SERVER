#pragma once

class Client;

class ItemBase
{
public:
	ItemBase() = default;
	virtual ~ItemBase() = default;
	virtual void Use(Client& me, Client& opponents, int grade) = 0;
};
