#include "Address.h"
#include "SystemConfiguration.h"

AddressTranslator Address::translator;

Address::Address(int r, int b, int row, int col, int bit)
: rank(r), bank(b), row(row), col(col), bit(bit)
{
}

Address::Address(int address, bool full)
{
	if (full)
		translator.Translate(address, rank, bank, row, col, bit);
	else
		translator.Translate(address, rank, bank, row, col);	
}

void Address::Clear()
{
	rank = bank = row = col = bit = 0;
}

int Address::GetPhysical()
{
	return translator.TranslateToAddr(rank, bank, row, col, bit);
}

void Address::InitTranslator()
{
	translator.Init();
}