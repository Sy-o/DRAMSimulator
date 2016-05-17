#pragma once
#include "FaultDef.h"
#include "AddressTranslator.h"
#include <stdint.h>
#include <map>

namespace DRAMSim
{
	class DRAMDevice;
}

class FaultController
{
	struct Fault
	{
		FaultType type;
		uint16_t victimVal;
		uint16_t agressorVal;
		int agressorAddr;
	};

public:
	FaultController(DRAMSim::DRAMDevice* dram);
	~FaultController();

public:
	void SetFaults(std::map<int, Fault>& faults);
	bool IsFaulty(int address);
	bool IsAgressor(int address);
	void DoFaults(int r, int b, int row, int col, uint16_t data);
	void DoOperationOnBit(int addr, uint16_t newVal, uint16_t oldVal);

	void GetAgressorAttributes(int agressorAddr, int& victimAddr, Fault& fault);


private: 
	DRAMSim::DRAMDevice* dramDevice;
	AddressTranslator translator;
	std::map<int, Fault> faultyCells;

};