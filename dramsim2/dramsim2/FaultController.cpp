#include "FaultController.h"
#include "DRAMDevice.h"
#include <algorithm>
using namespace std;

FaultController::FaultController(DRAMDevice* dram)
{
	dramDevice = dram;
}

FaultController::~FaultController()
{
}

void FaultController::SetFaults(std::vector<Fault>& faults)
{
	faultyCells = faults;
}

bool FaultController::IsFaulty(int address)
{
	auto it = find_if(faultyCells.begin(), faultyCells.end(), [address](Fault f){return f.victimAddress == address; });
	return it != faultyCells.end();
}

bool FaultController::IsAgressor(int address)
{
	auto it = find_if(faultyCells.begin(), faultyCells.end(), [address](Fault f){return f.agressorAddress == address; });
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
			DoOperationOnBit(addr.GetPhysical(), data & (1 << i), oldData & (1 << i));
		}
	}
}

void FaultController::DoOperationOnBit(int addr, uint16_t newVal, uint16_t oldVal)
{
	Address address(addr, true);
	if (IsFaulty(addr))
	{
		Fault fault = GetCellAttributes(addr, false);
		switch (fault.type)
		{
		case SAF: break;//set on init		
		case TF:
		{
				   if (oldVal != fault.victimValue)
				   {
					   (*dramDevice->ranks)[address.rank].banks[address.bank].writeBit(address, newVal == 1);
				   }
				   break;
		}
		case CFin:
		{
					 (*dramDevice->ranks)[address.rank].banks[address.bank].writeBit(address, newVal == 1);
				     break;
		}			
		case CFid:
		{
					 (*dramDevice->ranks)[address.rank].banks[address.bank].writeBit(address, newVal == 1);
					 break;
		}
		case CFst:
		{
					 if (newVal != fault.victimValue)
						 (*dramDevice->ranks)[address.rank].banks[address.bank].writeBit(address, newVal == 1);
					 else
					 {
						 Address agrAdr(fault.agressorAddress, true);
						 uint16_t aggressorData = (*dramDevice->ranks)[agrAdr.rank].banks[agrAdr.bank].read(agrAdr);
						 if ((aggressorData & (1 << agrAdr.bit)) == fault.agressorValue)
							 (*dramDevice->ranks)[address.rank].banks[address.bank].writeBit(address, newVal == 1);
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
							 (*dramDevice->ranks)[vicAdr.rank].banks[vicAdr.bank].writeBit(vicAdr, fault.agressorValue == 1);
						 }
						 break;
			}
			default :
				break;
			}
			(*dramDevice->ranks)[address.rank].banks[address.bank].writeBit(address, newVal == 1);
		}
		else
		{
			(*dramDevice->ranks)[address.rank].banks[address.bank].writeBit(address, newVal == 1);
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
		it = find_if(faultyCells.begin(), faultyCells.end(), [address](Fault f){return f.victimAddress == address; });
	}
	if (it != faultyCells.end())
		return *it;
	else return Fault();
}