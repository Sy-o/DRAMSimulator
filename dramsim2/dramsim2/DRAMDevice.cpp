#include "DRAMDevice.h"
#include "MemoryController.h"
#include "SystemConfiguration.h"
#include "Parsing.h"
#include <vector>

using namespace std;
using namespace DRAMSim;

DRAMDevice::DRAMDevice(MemoryController *mc, string faultFilePath)
:faultController(this)
{
	currentClockCycle = 0;
	ranks = new vector<Rank>();

	for (size_t i = 0; i<NUM_RANKS; i++)
	{
		Rank r = Rank();
		r.setId(i);
		r.attachMemoryController(mc);
		ranks->push_back(r);
	}
	discharger.Initialize();
	if (!faultFilePath.empty())
	{
		vector<Fault> faults = ParseCSV(faultFilePath);
		faultController.SetFaults(faults);
	}
}

DRAMDevice::~DRAMDevice()
{
	ranks->clear();
	delete(ranks);
}

void DRAMDevice::attachRanks(vector<Rank> *ranks)
{
	this->ranks = ranks;
}

void DRAMDevice::update()
{
	//updates the state of each of the objects
	// NOTE - do not change order
	for (size_t i = 0; i<NUM_RANKS; i++)
	{
		(*ranks)[i].update();
	}

	/*if (currentClockCycle == 2500)
		(*ranks)[0].banks[0].writeBit(Address(1, true), true);*/
	/*if (currentClockCycle == 2501)
		(*ranks)[0].banks[0].writeBit(0, 0, 3, true);
	if (currentClockCycle == 2502)
		(*ranks)[0].banks[0].writeBit(0, 0, 6, true);*/
}

void DRAMDevice::step()
{
	for (size_t i = 0; i<NUM_RANKS; i++)
	{
		(*ranks)[i].step();
	}
	SimulatorObject::step();
}

void DRAMDevice::receiveFromBus(BusPacket *packet)
{
	if (packet->busPacketType == DATA)
	{
		faultController.DoFaults(packet->address, packet->data->getData());
		(*ranks)[packet->address.rank].receiveFromBus(packet);
	}
	else
	{
		(*ranks)[packet->address.rank].receiveFromBus(packet);
	}
}

uint16_t DRAMDevice::read(Address addr)
{
	return (*ranks)[addr.rank].banks[addr.bank].read(addr);
}

uint16_t DRAMDevice::readBit(Address addr)
{
	return (read(addr) & (1 << addr.bit)) ? 1 : 0;
}

void DRAMDevice::write(Address addr, uint16_t data)
{	
	faultController.DoFaults(addr, data);
}

void DRAMDevice::invertBit(Address addr)
{
	uint16_t oldBitValue = readBit(addr);
	uint16_t newBitValue = oldBitValue ? 0 : 1;
	faultController.DoOperationOnBit(addr, newBitValue, oldBitValue);
}

void DRAMDevice::setRefreshWaitingFlag(int rank)
{
	(*ranks)[rank].refreshWaiting = true;
}

bool DRAMDevice::getRefreshWaitingFlag(int rank)
{
	return (*ranks)[rank].refreshWaiting;
}

void DRAMDevice::powerDown(int rank)
{
	(*ranks)[rank].powerDown();
}

void DRAMDevice::powerUp(int rank)
{
	(*ranks)[rank].powerUp();
}

void DRAMDevice::dischargeCells(int cycleCount, int rank)
{
	std::vector<int> addrList = discharger.GetRandomAddressList(cycleCount);
	(*ranks)[rank].dischargeCells(addrList);
}