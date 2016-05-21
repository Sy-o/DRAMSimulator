#include <string>
#include "TestingSystem.h"

#include <deque>
#include <iostream>

using namespace std;
using namespace DRAMSim;

TestingSystem::TestingSystem(int cycles)
{
    memory = 0;
    numCycles = cycles;
    Initialize();
}

TestingSystem::~TestingSystem()
{
    delete memory;
}

void TestingSystem::Initialize()
{
    string systemIniFilename = "system.ini";
    string deviceIniFilename = "ini/DDR2_micron_1Mb.ini";
    unsigned megsOfMemory = 1; //1 Mb

    memory = new MemorySystem(0, deviceIniFilename, systemIniFilename, "", "SimulationResults.txt", megsOfMemory, "faults.csv");

    ReadDataCB *read_cb = new Callback<TestingSystem, void, uint64_t, uint16_t, size_t>(this, &TestingSystem::read_complete);
    TransactionCompleteCB *write_cb = new Callback<TestingSystem, void, unsigned, uint64_t, uint64_t>(this, &TestingSystem::write_complete);
    memory->RegisterCallbacks(read_cb, write_cb, 0);
}

int TestingSystem::AlignAddress(int address)
{
    return address << 5;
}

void TestingSystem::start()
{
    deque<Trans> transactions;
   
	transactions.push_back(Trans(false, 2));
	transactions.push_back(Trans(true, 2, 127));
	transactions.push_back(Trans(false, 2));
	transactions.push_back(Trans(true, 4, 4));
	transactions.push_back(Trans(false, 4));
	transactions.push_back(Trans(false, 2));
    for (size_t i = 0; i < numCycles; i++)
    {
		//if (!(i % 500))
			//transactions.push_back(Trans(false, 12458));
        if (transactions.size())
        {
            Trans t = transactions.front();
            if (memory->addTransaction(t.Write, t.Address, t.Data))
                transactions.pop_front();
        }
        memory->update();
    }
    memory->printStats();
}

void TestingSystem::read_complete(uint64_t address, uint16_t data, size_t)
{
   //cout << "[Testing System] Read data [" << dec << data << "] from adress 0x" << address << endl;
}

void TestingSystem::write_complete(unsigned id, uint64_t address, uint64_t cycle)
{
   // cout << "[Testing System] Write data at address 0x" << dec << address << " on cycle " << cycle << endl;
}