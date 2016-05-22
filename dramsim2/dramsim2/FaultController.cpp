#include "FaultController.h"
#include "DRAMDevice.h"
#include <algorithm>
using namespace std;

FaultController::FaultController(DRAMDevice* dram) : dramDevice(dram)
{
}

FaultController::~FaultController()
{
}

void FaultController::SetFaults(std::vector<Fault>& faults)
{
	faultyCells = faults;
	auto it = find_if(faultyCells.begin(), faultyCells.end(), [](Fault f){return f.type == SAF; });
	if (it != faultyCells.end())
	{
		InitSAFs();
	}
}

void FaultController::InitSAFs()
{
	for (auto& f : faultyCells)
	{
		if (f.type == SAF)
		{
			Address addr(f.victimAddress, true);
			(*dramDevice->ranks)[addr.rank].banks[addr.bank].writeBit(addr, f.victimValue == 1);
		}
	}
}

bool FaultController::IsFaulty(int address)
{
	auto it = find_if(faultyCells.begin(), faultyCells.end(), 
		[address](Fault f){return f.victimAddress == address && f.type != AF; });
	return it != faultyCells.end();
}

bool FaultController::IsAgressor(int address)
{
	auto it = find_if(faultyCells.begin(), faultyCells.end(), 
		[address](Fault f){return f.agressorAddress == address; });
	return it != faultyCells.end();
}

void FaultController::DoFaults(Address addr, uint16_t data)
{
	uint16_t oldData = dramDevice->read(addr);
	uint16_t bitMask = oldData ^ data;
	
	for (unsigned i = 0; i < DEVICE_WIDTH; i++)
	{
		if (bitMask & (1 << i))
		{
			addr.bit = i;
			DoOperationOnBit(addr, data & (1 << i) ? 1 : 0, oldData & (1 << i) ? 1 : 0);
		}
	}
}

void FaultController::DoOperationOnBit(Address address, uint16_t newVal, uint16_t oldVal)
{
	int addr = address.GetPhysical();
	if (IsFaulty(addr))
	{
		Fault fault = GetCellAttributes(addr, false);
		switch (fault.type)
		{
		case SAF: break;
		case TF:
		{
				   if (oldVal != fault.victimValue)
				   {
					   WriteBit(address, newVal);
				   }
				   break;
		}
		case CFin:
		{
					 WriteBit(address, newVal);
				     break;
		}			
		case CFid:
		{
					 WriteBit(address, newVal);
					 break;
		}
		case CFst:
		{
					 if (newVal != fault.victimValue)
						 WriteBit(address, newVal);
					 else
					 {
						 Address agrAdr(fault.agressorAddress, true);
						 if (dramDevice->readBit(agrAdr) == fault.agressorValue)
							 WriteBit(address, newVal);
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
			Fault fault = GetCellAttributes(addr, true);			
			switch (fault.type)
			{
			case CFin:
			{
						 if (newVal == fault.agressorValue && newVal != oldVal)
						 {
							 Address vicAdr(fault.victimAddress, true);
							 (*dramDevice->ranks)[vicAdr.rank].banks[vicAdr.bank].invertBit(vicAdr);
						 }
						 break;
			}
			case CFid:
			{
						 if (newVal == fault.agressorValue && newVal != oldVal)
						 {
							 Address vicAdr(fault.victimAddress, true);
							 WriteBit(vicAdr, fault.victimValue);
						 }
						 break;
			}
			case AF:
			{
					   Address vicAdr(fault.victimAddress, true);
					   WriteBit(vicAdr, newVal);
					   break;
			}
			default :
				break;
			}
			WriteBit(address, newVal);
		}
		else
		{
			WriteBit(address, newVal);
		}
	}	
}

Fault FaultController::GetCellAttributes(int address, bool isAgressor)
{
	vector<Fault>::iterator it;
	if (isAgressor)
	{
		it = find_if(faultyCells.begin(), faultyCells.end(), [address](Fault f){return f.agressorAddress == address; });
	}
	else
	{
		it = find_if(faultyCells.begin(), faultyCells.end(), [address](Fault f){return f.victimAddress == address && f.type != AF; });
	}
	if (it != faultyCells.end())
		return *it;
	else return Fault();
}

void FaultController::WriteBit(Address address, int val)
{
	(*dramDevice->ranks)[address.rank].banks[address.bank].writeBit(address, val == 1);
}