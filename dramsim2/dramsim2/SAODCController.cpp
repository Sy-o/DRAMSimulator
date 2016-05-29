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
		uint16_t oldData = dramDevice->read(packet->address);
        uint16_t newData = packet->data->getData();
       if (oldData != newData)
            UpdateRef(packet->address, newData ^ oldData);
    }
}

void SAODCController::UpdateRef(Address addr, uint16_t data)
{
	RefSignature ^= GetAddress(addr, data);
}

void SAODCController::UpdateTestSig(Address addr, uint16_t data)
{
	TestSignature ^= GetAddress(addr, data);
}

int SAODCController::GetSignaturesSum()
{
	return TestSignature ^ RefSignature;
}

int SAODCController::GetAddress(Address addr, uint16_t data)
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
		addr.Clear();
    
	addr.bit = bitSum;
	int address = addr.GetPhysical();

	if (bitCount % 2 != 0)
		AddParityBit(address);
	
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
					Address addr(r, b, row, col);
					uint16_t data = dramDevice->read(addr);
					UpdateTestSig(addr, data);
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

int SAODCController::GetRefSignature()
{
	return RefSignature;
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
