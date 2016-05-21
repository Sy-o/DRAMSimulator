#pragma once
#include "AddressTranslator.h"

struct Address
{
public:
	Address(int r = 0, int b = 0, int row = 0, int col = 0, int bit = 0);
	Address(int address, bool full);

	void Clear();
	int GetPhysical();

	friend bool operator==(const Address& a, const Address& b)
	{
		return a.rank == b.rank &&
			   a.bank == b.bank &&
			   a.row == b.row &&
			   a.col == b.col &&
			   a.bit == b.bit;	
	}

public:
	int rank;
	int bank;
	int row;
	int col;
	int bit;

public:
	static void InitTranslator();
	static AddressTranslator translator;
};