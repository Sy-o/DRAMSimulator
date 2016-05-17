#pragma once
#include "SimulatorObject.h"
#include "BusPacket.h"
#include "Rank.h"
#include "Discharger.h"
#include "FaultController.h"

using namespace DRAMSim;

namespace DRAMSim
{
	class MemoryController;
	class DRAMDevice : public SimulatorObject
	{
		friend FaultController;
	public:
		DRAMDevice(MemoryController *mc);
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
		uint16_t read(int r, int b, int row, int col);
		void write(int r, int b, int row, int col, uint16_t data);
		void invertBit(int r, int b, int row, int col, int bit);

	private:
		vector<Rank> *ranks;
		Discharger discharger;
		FaultController faultController;
	};
}