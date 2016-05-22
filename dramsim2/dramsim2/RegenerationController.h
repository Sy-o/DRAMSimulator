#pragma once
#include "SimulatorObject.h"
#include "SAODCController.h"
#include "MarchTestController.h"
#include "BusPacket.h"
#include "AddressTranslator.h"
#include "DRAMDevice.h"
#include "Rank.h"

class RegenerationController : public DRAMSim::SimulatorObject
{
public:
	RegenerationController();
	~RegenerationController();

public:
	void Initialize(DRAMDevice* dram);
	void UpdateRefSignature(DRAMSim::BusPacket* packet);
	void SetMarchTest(string marchTest);
	void StartRefresh();
	void update();

private:
	int GetMarchTestType(string marchTest);
		
private:
	MarchTestController marchController;
	SAODCController saodcController;
    AddressTranslator addrTranslator;

	DRAMDevice* dramDevice;

    int lastError;

	int refreshEndCycle;
	int startMarchTestCycle;

	bool needTest;
	
};