#pragma once
#include "BusPacket.h"
#include "DRAMDevice.h"
#include "Address.h"
#include "Rank.h"

class SAODCController
{
public:
    SAODCController();
    ~SAODCController();

public:
	void SetDRAMDevice(DRAMDevice *dram);
    void UpdateRef(DRAMSim::BusPacket* packet);

	void UpdateRef(Address addr, uint16_t data);
	void UpdateTestSig(Address addr, uint16_t data);

	int GetSignaturesSum();

	bool ParityBitInZero(int signature);
    int CalculateTestAndCompare();

	void ClearTestSig();
	void SetRefSignature(int signature);
	int GetTestSignature();

private:
    // function calculates address sum of each changed bit in word
    int GetAddress(Address addr, uint16_t data);
	void AddParityBit(int &address);

private:
    int RefSignature;
    int TestSignature;
	DRAMDevice *dramDevice;
};