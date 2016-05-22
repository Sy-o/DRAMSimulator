#pragma once
#include "FaultDef.h"
#include "Address.h"
#include <stdint.h>
#include <vector>

namespace DRAMSim
{
	class DRAMDevice;
}

class FaultController
{

public:
	FaultController(DRAMSim::DRAMDevice* dram);
	~FaultController();

public:
	void SetFaults(std::vector<Fault>& faults);
	bool IsFaulty(int address);
	bool IsAgressor(int address);
	void DoFaults(Address addr, uint16_t data);
	void DoOperationOnBit(Address addr, uint16_t newVal, uint16_t oldVal);

	Fault GetCellAttributes(int address, bool isAgressor);

private:
	void WriteBit(Address address, int val);
	void InitSAFs();

private: 
	DRAMSim::DRAMDevice* dramDevice;
	AddressTranslator translator;
	std::vector<Fault> faultyCells;
};