#include "AddressTranslator.h"
#include "SystemConfiguration.h"
#include <sstream>

using namespace DRAMSim;

AddressTranslator::AddressTranslator()
{
	RankWidth = dramsim_log2(NUM_RANKS);
	BankWidth = dramsim_log2(NUM_BANKS);
    RowWidth = dramsim_log2(NUM_ROWS);
    ColWidth = dramsim_log2(NUM_COLS);
    CellWidth = dramsim_log2(DEVICE_WIDTH);
}

AddressTranslator::~AddressTranslator()
{}

int AddressTranslator::GetValue(int width, int& address)
{
    int temp = address;
    temp >>= width;
    temp <<= width;
    int n = temp ^ address;
    address >>= width;
    return n;
}

void AddressTranslator::SetValue(int width, int value, int& address)
{
	address <<= width;
	address |= value;	
}

void AddressTranslator::Translate(int address, int& rank, int& bank, int& row, int& col)
{
    col = GetValue(ColWidth, address);
    row = GetValue(RowWidth, address);
    bank = GetValue(BankWidth, address);
	rank = GetValue(RankWidth, address);
}

void AddressTranslator::Translate(int address, int& rank, int& bank, int& row, int& col, int& bit)
{
    bit = GetValue(CellWidth, address);
    col = GetValue(ColWidth, address);
    row = GetValue(RowWidth, address);
    bank = GetValue(BankWidth, address);
	rank = GetValue(RankWidth, address);
}

int AddressTranslator::TranslateToAddr(int rank, int bank, int row, int col, int bit)
{
	int address = 0;
	SetValue(RankWidth, rank, address);
	SetValue(BankWidth, bank, address);
	SetValue(RowWidth, row, address);
	SetValue(ColWidth, col, address);
	if (bit >= 0)
		SetValue(CellWidth, bit, address);
	return address;
}

std::string AddressTranslator::GetDescription(int address, bool includeBit)
{
    std::ostringstream stream;
    int r = 0, b = 0, row = 0, col = 0, bit = 0;
    if (includeBit)
        Translate(address, r, b, row, col, bit);
    else
        Translate(address, r, b, row, col);

    stream << "r:" << r << ", b:" << b << ", row:" << row << ", col:" << col;
    if (includeBit)
        stream << ", bit:" << bit;
   return stream.str();    
}
