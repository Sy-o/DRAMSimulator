#pragma once
#include <string>

class AddressTranslator
{
public:
    AddressTranslator();
    ~AddressTranslator();

    void Translate(int address, int& rank, int& bank, int& row,  int& col);
    void Translate(int address, int& rank, int& bank, int& row, int& col, int& bit);

	int TranslateToAddr(int rank, int bank, int row, int col, int bit = -1);

    std::string GetDescription(int address, bool includeBit);


private:
    int GetValue(int width, int& address);
	void SetValue(int width, int value, int& address);

private:
	int RankWidth;
    int BankWidth;
    int RowWidth;
    int ColWidth;
    int CellWidth;
};