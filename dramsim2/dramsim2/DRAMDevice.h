#pragma once
#include "SimulatorObject.h"
#include "BusPacket.h"
#include "Rank.h"
#include "Discharger.h"
#include "FaultController.h"
#include "Address.h"

using namespace DRAMSim;

namespace DRAMSim
{
	class MemoryController;
	class DRAMDevice : public SimulatorObject
	{
		friend FaultController;
	public:
		DRAMDevice(MemoryController *mc, std::string faultFilePath = "");
		~DRAMDevice();

	public:
		void receiveFromBus(BusPacket *packet);
		void attachRanks(vector<Rank> *ranks);
		void update() override;
		void step() override;

		void setRefreshWaitingFlag(int rank);
		bool getRefreshWaitingFlag(int rank);
		void powerDown(int rank);
		void powerUp(int rank);

		void dischargeCells(int cycleCount, int rank);

	public:
		uint16_t read(Address addr);
		void write(Address addr, uint16_t data);
		void invertBit(Address addr);

	private:
		uint16_t readBit(Address addr);

	private:
		vector<Rank> *ranks;
		Discharger discharger;
		FaultController faultController;
	};
}