#include "MarchTestController.h"
#include "SystemConfiguration.h"
#include <iostream>

MarchTestController::MarchTestController()
{
	dramDevice = 0;
}

MarchTestController::~MarchTestController()
{}

void MarchTestController::Initialize(int marchTest, DRAMDevice* dram)
{
	phases.clear();
	dramDevice = dram;
	InitMarchTest(marchTest);
}

void MarchTestController::InitMarchTest(int marchTest)
{
	switch (marchTest)
	{
	case MARCH_C_MINUS:
	{
					phases.push_back({ MD_UP, { MO_RD, MO_WDC } });
					phases.push_back({ MD_UP, { MO_RDC, MO_WD } });
					phases.push_back({ MD_DOWN, { MO_RD, MO_WDC } });
					phases.push_back({ MD_DOWN, { MO_RDC, MO_WD } });
					phases.push_back({ MD_BOTH, { MO_RD } });
					break;
	}
	case MARCH_B:
	{
					phases.push_back({ MD_UP, { MO_RD, MO_WDC, MO_RDC, MO_WD, MO_RD, MO_WDC } });
					phases.push_back({ MD_UP, { MO_RDC, MO_WD, MO_WDC } });
					phases.push_back({ MD_DOWN, { MO_RDC, MO_WD, MO_WDC, MO_WD } });
					phases.push_back({ MD_DOWN, { MO_RD, MO_WDC, MO_WD } });
					break;
	}
	case MARCH_A:
	{
					phases.push_back({ MD_UP, { MO_RD, MO_WDC, MO_WD, MO_WDC } });
					phases.push_back({ MD_UP, { MO_RDC, MO_WD, MO_WDC } });
					phases.push_back({ MD_DOWN, { MO_RDC, MO_WD, MO_WDC, MO_WD } });
					phases.push_back({ MD_DOWN, { MO_RD, MO_WDC, MO_WD } });
					break;
	}
	case MARCH_X:
	{
					phases.push_back({ MD_UP, { MO_RD, MO_WDC } });
					phases.push_back({ MD_DOWN, { MO_RDC, MO_WD } });
					phases.push_back({ MD_BOTH, { MO_RD } });
					break;
	}
	case MARCH_Y:
	{
					phases.push_back({ MD_UP, { MO_RD, MO_WDC, MO_RDC } });
					phases.push_back({ MD_DOWN, { MO_RDC, MO_WD, MO_RD } });
					phases.push_back({ MD_BOTH, { MO_RD } });
					break;
	}
	case MATS:
	{
					phases.push_back({ MD_UP, { MO_RD, MO_WDC } });
					phases.push_back({ MD_DOWN, { MO_RDC } });
					break;
	}
	case MATS_PLUS:
	{
					phases.push_back({ MD_UP, { MO_RD, MO_WDC } });
					phases.push_back({ MD_DOWN, { MO_RDC, MO_WD } });
					break;
	}
	case MATS_PLUS_PLUS:
	{
					phases.push_back({ MD_UP, { MO_RD, MO_WDC } });
					phases.push_back({ MD_DOWN, { MO_RDC, MO_WD, MO_RD } });
					break;
	}
	default:
		break;
	}
}

void MarchTestController::RunTest(int refSignature)
{
	Reset();
	state.testStarted = true;
	saodc.SetRefSignature(refSignature);
}

void MarchTestController::Update()
{
	if (!state.testStarted)
		return;

    saodc.ClearTestSig();

	MarchPhase& phase = phases[state.phase];

	int addrStart = 0;
	int counter = 0;

	int bottom = 0;
	int top = NUM_BANKS * NUM_COLS * NUM_ROWS - 1;

	if (phase.direction == MD_UP || phase.direction == MD_BOTH)
	{
		addrStart = bottom;
		counter = 1;
	}
	else if (phase.direction == MD_DOWN)
	{
		addrStart = top;
		counter = -1;
	}

	for (int addr = addrStart; addr >= bottom && addr <= top; addr += counter)
	{
		uint16_t buffer = 0;
		uint16_t old = 0;
		for (auto& el : phase.elements)
		{
			RunElement(el, addr, buffer, old);
		}
	}

	int err = saodc.GetSignaturesSum();
	if (err)
	{
		state.testPassed = false;
        cout << "[MarchTest] Errors are detected while running phase!\n      Signature sum is " << err << "(" << addrTranslator.GetDescription(err>>1, true) << ")" << endl;
	}

	state.phase++;
	if (state.phase == phases.size())
	{
		state.testCompleted = true;
		state.testStarted = false;
	}
}

void MarchTestController::RunElement(int element, int address, uint16_t &buffer, uint16_t& oldValue)
{
	Address addr(address, false);
	uint16_t data = 0;

	switch (element)
	{
	case MO_RD:
		data = dramDevice->read(addr);
		buffer = data;
		break;
	case MO_RDC:
		data = dramDevice->read(addr);
		buffer = ~data;
		break;
	case MO_WD:
		dramDevice->write(addr, buffer);
		break;
	case MO_WDC:
		dramDevice->write(addr, ~buffer);
		break;
	}

	//convertData
	if (element == MO_RD || element == MO_RDC)
	{
		saodc.UpdateTestSig(addr, data ^ oldValue);
		oldValue = data;
	}
}

bool MarchTestController::TestCompleted()
{
	return state.testCompleted;
}

bool MarchTestController::TestPassed()
{
	return state.testPassed;
}

void MarchTestController::Reset()
{
	state.testStarted = false;
	state.testPassed = true;
	state.testCompleted = false;
	state.phase = 0;
	saodc.ClearTestSig();
}

