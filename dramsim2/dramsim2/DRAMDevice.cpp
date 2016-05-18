#include "DRAMDevice.h"
#include "MemoryController.h"
#include "SystemConfiguration.h"

using namespace std;
using namespace DRAMSim;

DRAMDevice::DRAMDevice(MemoryController *mc)
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
		(*ranks)[0].banks[0].writeBit(0, 0, 1, true);
	if (currentClockCycle == 2501)
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
	//if (packet->busPacketType == DATA)
	//{
	//	//do some faults
	//	delete packet;
	//}
	//else
	{
		(*ranks)[packet->address.rank].receiveFromBus(packet);
	}
}

uint16_t DRAMDevice::read(Address addr)
{
	return (*ranks)[addr.rank].banks[addr.bank].read(addr);
}

void DRAMDevice::write(Address addr, uint16_t data)
{
	(*ranks)[addr.rank].banks[addr.bank].write(addr, data);
}

void DRAMDevice::invertBit(Address addr)
{
	(*ranks)[addr.rank].banks[addr.bank].invertBit(addr);
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