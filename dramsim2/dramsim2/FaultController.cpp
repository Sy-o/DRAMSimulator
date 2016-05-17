#include "FaultController.h"
#include "DRAMDevice.h"
#include <vector>
using namespace std;

FaultController::FaultController(DRAMDevice* dram)
{
	dramDevice = dram;
}

FaultController::~FaultController()
{
}

void FaultController::SetFaults(std::map<int, Fault>& faults)
{
	faultyCells = faults;
}

bool FaultController::IsFaulty(int address)
{
	auto it = faultyCells.find(address);
	return it != faultyCells.end();
}

bool FaultController::IsAgressor(int address)
{
	for (auto f : faultyCells)
	{
		if (f.second.agressorAddr == address)
			return true;
	}
	return false;
}

void FaultController::DoFaults(int r, int b, int row, int col, uint16_t data)
{
	uint16_t oldData = dramDevice->read(r, b, row, col);
	uint16_t bitMask = oldData ^ data;
	
	for (unsigned i = 0; i < DEVICE_WIDTH; i++)
	{
		if (bitMask & (1 << i))
		{
			int addr = translator.TranslateToAddr(r, b, row, col, i);
			DoOperationOnBit(addr, data & (1 << i), oldData & (1 << i));
		}
	}
}

void FaultController::DoOperationOnBit(int addr, uint16_t newVal, uint16_t oldVal)
{
	int r = 0, b = 0, row = 0, col = 0, bit = 0;
	translator.Translate(addr, r, b, row, col, bit);
	
	if (IsFaulty(addr))
	{
		auto fault = faultyCells[addr];
		switch (fault.type)
		{
		case SAF: break;//set on init		
		case TF:
		{
				   if (oldVal != fault.victimVal)
				   {
					   (*dramDevice->ranks)[r].banks[b].writeBit(row, col, bit, newVal == 1);
				   }
				   break;
		}
		case CFin:
		{
					 (*dramDevice->ranks)[r].banks[b].writeBit(row, col, bit, newVal == 1);
				     break;
		}			
		case CFid:
		{
					 (*dramDevice->ranks)[r].banks[b].writeBit(row, col, bit, newVal == 1); 
					 break;
		}
		case CFst:
		{
					 if (newVal != fault.victimVal)
						 (*dramDevice->ranks)[r].banks[b].writeBit(row, col, bit, newVal == 1);
					 else
					 {
						 int r_ = 0, b_ = 0, row_ = 0, col_ = 0, bit_ = 0;
						 translator.Translate(fault.agressorAddr, r_, b_, row_, col_, bit_);
						 uint16_t aggressorData = (*dramDevice->ranks)[r_].banks[b_].read(row_, col_);
						 if ((aggressorData & (1 << bit_)) == fault.agressorVal)
							 (*dramDevice->ranks)[r].banks[b].writeBit(row, col, bit, newVal == 1);
					 }
					 break;
		}
		default: break;
		}
	}
	else
	{
		if (IsAgressor(addr))
		{
			Fault fault;
			int victim = 0;
			GetAgressorAttributes(addr, victim, fault);
			switch (fault.type)
			{
			case CFin:
			{
						 if (newVal == fault.agressorVal && newVal != oldVal)
						 {
							 int r_=0, b_=0, row_=0, col_=0, bit_=0;
							 translator.Translate(victim, r_, b_, row_, col_, bit_);
							 (*dramDevice->ranks)[r_].banks[b_].invertBit(row_, col_, bit_);
						 }
						 break;
			}
			case CFid:
			{
						 if (newVal == fault.agressorVal && newVal != oldVal)
						 {
							 int r_ = 0, b_ = 0, row_ = 0, col_ = 0, bit_ = 0;
							 translator.Translate(victim, r_, b_, row_, col_, bit_);
							 (*dramDevice->ranks)[r_].banks[b_].writeBit(row_, col_, bit_, fault.agressorVal == 1);
						 }
						 break;
			}
			default :
				break;
			}
			(*dramDevice->ranks)[r].banks[b].writeBit(row, col, bit, newVal == 1);
		}
		else
		{
			(*dramDevice->ranks)[r].banks[b].writeBit(row, col, bit, newVal == 1);
		}
	}	
}

void FaultController::GetAgressorAttributes(int agressorAddr, int& victimAddr, Fault& fault)
{
	for (auto f : faultyCells)
	{
		if (f.second.agressorAddr == agressorAddr)
		{
			victimAddr = f.first;
			fault = f.second;
			return;
		}
	}
}