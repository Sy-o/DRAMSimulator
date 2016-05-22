#pragma once

#include <vector>
#include "DRAMDevice.h"
#include "AddressTranslator.h"
#include "SAODCController.h"
#include "MarchDef.h"
#include <vector>

class MarchTestController
{
	struct State
	{
		bool testStarted;
		bool testCompleted;
		int phase;
		bool testPassed;

		State() { Clear(); }

		void Clear()
		{
			testStarted = false;
			testCompleted = false;
			phase = 0;
			testPassed = false;
		}
	};

	struct MarchPhase
	{
		MarchDirection direction;
		std::vector<MarchOperation> elements;
	};

public:
	MarchTestController();
	~MarchTestController();

public:
	void Initialize(int marchTest, DRAMDevice* dram);
	void RunTest(int refSignature);
	void Update();
	bool TestCompleted();
	bool TestPassed();
	void Reset();

private:
	void RunElement(int element, int address, uint16_t &buffer, uint16_t& oldValue);
	void InitMarchTest(int marchTest);

private:
	AddressTranslator addrTranslator;
	SAODCController saodc;
	State state;
	std::vector<MarchPhase> phases;
	DRAMDevice* dramDevice;
	bool testPass;
};