#pragma once

#include <vector>
#include "DRAMDevice.h"
#include "AddressTranslator.h"
#include "SAODCController.h"
#include "MarchDef.h"
#include <vector>
#include <map>

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
	void Initialize(DRAMDevice* dram);
	void SetMarchTest(int marchTest);

	void RunTest(int refSignature);
	void Update();
	bool TestCompleted();
	bool TestPassed();
	void Reset();

private:
	void RunElement(int element, int address, uint16_t &buffer, int& readOperationNum, std::map<int, SAODCController>& signatures);
	int PhasePassed(std::map<int, SAODCController>& signatures);

private:
	AddressTranslator addrTranslator;
	SAODCController saodc;
	State state;
	std::vector<MarchPhase> phases;
	DRAMDevice* dramDevice;
	bool testPass;
};