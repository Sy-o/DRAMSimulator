#include "RegenerationController.h"
#include "SystemConfiguration.h"
#include "MarchDef.h"
#include <iostream>

RegenerationController::RegenerationController()
{
    currentClockCycle = 0;
    lastError = 0;
    dramDevice = 0;
}

RegenerationController::~RegenerationController()
{
}

void RegenerationController::Initialize(DRAMDevice* dram)
{
	dramDevice = dram;
    refreshEndCycle = 0;
	startMarchTestCycle = 0;
	saodcController.SetDRAMDevice(dram);
	marchController.Initialize(dram);
	needTest = false;
}

void RegenerationController::SetMarchTest(string marchTest)
{
	marchController.SetMarchTest(GetMarchTestType(marchTest));
}

int RegenerationController::GetMarchTestType(string marchTest)
{
	if (marchTest.compare("March C-") == 0)
		return MARCH_C_MINUS;
	if (marchTest.compare("March A") == 0)
		return MARCH_A;
	if (marchTest.compare("March B") == 0)
		return MARCH_B;
	if (marchTest.compare("March X") == 0)
		return MARCH_X;
	if (marchTest.compare("March Y") == 0)
		return MARCH_Y;
	if (marchTest.compare("MATS") == 0)
		return MATS;
	if (marchTest.compare("MATS+") == 0)
		return MATS_PLUS;
	if (marchTest.compare("MATS++") == 0)
		return MATS_PLUS_PLUS;
	return MATS;
}

void RegenerationController::StartRefresh()
{
	refreshEndCycle = currentClockCycle + tRFC - 1;
    int err = saodcController.CalculateTestAndCompare();
    if (/*err*/true)
	{
        std::cout << "[Regeneration Controller] Detected error(s) in memory.\n      Signature Sum = " << err << "(" << addrTranslator.GetDescription(err>>1, true) << ")" << std::endl;
        if (/*lastError*/ true)
        {
            needTest = true;
            startMarchTestCycle = currentClockCycle + 1;
        }
        else
        {
			if (saodcController.ParityBitInZero(err))
			{
				//multiple errors
				needTest = true;
				startMarchTestCycle = currentClockCycle + 1;
			}
			else
			{
				//if it first time - let's decide that it was 1 error. Invert bit
				lastError = err;
				std::cout << "[Regeneration Controller] Try to restore the damaged cell." << std::endl;
				Address addr(err >> 1, true);
				dramDevice->invertBit(addr);
			}
        }		
	}
    else
    {
        lastError = 0;
    }
}

void RegenerationController::update()
{
	if (currentClockCycle >= refreshEndCycle)
		return;

	if (needTest)
	{
		if (startMarchTestCycle == currentClockCycle)
		{
            std::cout << "[Regeneration Controller] March Test Started." << std::endl;
            marchController.RunTest(saodcController.GetTestSignature());
		}
		else if (startMarchTestCycle < currentClockCycle)
		{
			marchController.Update();
		}

		if (marchController.TestCompleted())
		{
			if (!marchController.TestPassed())
			{
                std::cout << "[Regeneration Controller] March Test Failed." << std::endl;
                //do smth in case of errors
			}
            else
            {
                std::cout << "[Regeneration Controller] March Test Passed." << std::endl;
            }
			marchController.Reset();
			needTest = false;
		}
	}
}

void RegenerationController::UpdateRefSignature(DRAMSim::BusPacket* packet)
{
	saodcController.UpdateRef(packet);
}

