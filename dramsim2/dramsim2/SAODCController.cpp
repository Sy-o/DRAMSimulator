#include "SystemConfiguration.h"
#include "SAODCController.h"

using namespace DRAMSim;

SAODCController::SAODCController()
{
	RefSignature = TestSignature = 0;
	dramDevice = 0;
}

SAODCController::~SAODCController()
{

}

void SAODCController::SetDRAMDevice(DRAMDevice *dram)
{
	dramDevice = dram;
}

void SAODCController::UpdateRef(BusPacket* packet)
{
    if (packet->busPacketType == DATA)
    {
		uint16_t oldData = dramDevice->read(packet->rank, packet->bank, packet->row, packet->column);
        uint16_t newData = packet->data->getData();
       if (oldData != newData)
            UpdateRef(packet->rank, packet->bank, packet->row, packet->column, newData ^ oldData);
    }
}

void SAODCController::UpdateRef(int rank, int bank, int row, int col, uint16_t data)
{
	RefSignature ^= GetAddress(rank, bank, row, col, data);
}

void SAODCController::UpdateTestSig(int rank, int bank, int row, int col, uint16_t data)
{
	TestSignature ^= GetAddress(rank, bank, row, col, data);
}

int SAODCController::GetSignaturesSum()
{
	return TestSignature ^ RefSignature;
}

int SAODCController::GetAddress(int rank, int bank, int row, int col, uint16_t data)
{
    //get 'bit address sum'
	if (!data) return 0;
    
	int bitSum = 0;
    int bitCount  = 0; // '1' bit count
    for (int i = 0; (unsigned)i < DEVICE_WIDTH; i++)
    {
        if (data & (1 << i))
        {
            bitSum ^= i;
            bitCount++;
        }
    }

    if (bitCount % 2 == 0)
    {
        rank = bank = row = col = 0;
    }
	int address = translator.TranslateToAddr(rank, bank, row, col, bitSum);
	if (bitCount % 2 != 0)
	{
		AddParityBit(address);
	}
	return address;
}

int SAODCController::CalculateTestAndCompare()
{
    TestSignature = 0;
	for (int r = 0; r < NUM_RANKS; r++)
	{
		for (int b = 0; b < NUM_BANKS; b++)
		{
			for (int row = 0; row < NUM_ROWS; row++)
			{
				for (int col = 0; col < NUM_COLS; col++)
				{
					uint16_t data = dramDevice->read(r, b, row, col);
					UpdateTestSig(r, b, row, col, data);
				}
			}
		}
	}

	return GetSignaturesSum();
}

void SAODCController::ClearTestSig()
{
	TestSignature = 0;
}

void SAODCController::SetRefSignature(int signature)
{
	RefSignature = signature;
}

int SAODCController::GetTestSignature()
{
	return TestSignature;
}

void SAODCController::AddParityBit(int &address)
{
	address <<= 1;
	address |= 1;
}

bool SAODCController::ParityBitInZero(int signature)
{
	return (signature & 1) == 0;
}
