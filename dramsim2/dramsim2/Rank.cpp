/*********************************************************************************
*  Copyright (c) 2010-2011, Elliott Cooper-Balis
*                             Paul Rosenfeld
*                             Bruce Jacob
*                             University of Maryland 
*                             dramninjas [at] umd [dot] edu
*  All rights reserved.
*  
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions are met:
*  
*     * Redistributions of source code must retain the above copyright notice,
*        this list of conditions and the following disclaimer.
*  
*     * Redistributions in binary form must reproduce the above copyright notice,
*        this list of conditions and the following disclaimer in the documentation
*        and/or other materials provided with the distribution.
*  
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
*  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
*  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
*  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
*  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
*  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
*  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
*  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*********************************************************************************/



#include <algorithm>  
#include "Rank.h"
#include "MemoryController.h"

using namespace std;
using namespace DRAMSim;

Rank::Rank() :
		// store the rank #, mostly for convenience and printing
		id(-1),
		isPowerDown(false),
		memoryController(NULL),
		outgoingDataPacket(NULL),
		dataCyclesLeft(0),
		refreshWaiting(false),
		readReturnCountdown(0)
		
{

	/*DEBUG("num BANKS = " << NUM_BANKS);
	DEBUG("num ROWS = " << NUM_ROWS);
	DEBUG("num COLS = " << NUM_COLS);*/
	bankStates = vector<BankState>(NUM_BANKS, BankState());
	currentClockCycle = 0;

#ifndef NO_STORAGE
	banks = vector<Bank>(NUM_BANKS, Bank());
    for (auto& b : banks) 
        b.initialize();
#endif

}

// mutators
void Rank::setId(int id)
{
	this->id = id;
}

// attachMemoryController() must be called before any other Rank functions
// are called
void Rank::attachMemoryController(MemoryController *memoryController)
{
	this->memoryController = memoryController;
}

void Rank::receiveFromBus(BusPacket *packet)
{
	if (DEBUG_BUS)
	{
		PRINTN(" -- R" << this->id << " Receiving On Bus    : ");
		packet->print();
	}
	if (VERIFICATION_OUTPUT)
	{
		packet->print(currentClockCycle,false);
	}

	switch (packet->busPacketType)
	{
	case READ:
		//make sure a read is allowed
		if (bankStates[packet->address.bank].currentBankState != RowActive ||
			currentClockCycle < bankStates[packet->address.bank].nextRead ||
			packet->address.row != bankStates[packet->address.bank].openRowAddress)
		{
			packet->print();
			ERROR_("== Error - Rank " << id << " received a READ when not allowed");
			exit(0);
		}

		//update state table
		bankStates[packet->address.bank].nextPrecharge = max(bankStates[packet->address.bank].nextPrecharge, currentClockCycle + READ_TO_PRE_DELAY);
		for (size_t i=0;i<NUM_BANKS;i++)
		{
			bankStates[i].nextRead = max(bankStates[i].nextRead, currentClockCycle + max(tCCD, BL/2));
			bankStates[i].nextWrite = max(bankStates[i].nextWrite, currentClockCycle + READ_TO_WRITE_DELAY);
		}

		//get the read data and put it in the storage which delays until the appropriate time (RL)
#ifndef NO_STORAGE
		DEBUG("~   current cloclCycle: " << dec << currentClockCycle << "   ~");
		banks[packet->address.bank].read(packet);
#else
		packet->data = new DataPacket(); 
		packet->busPacketType = DATA;
#endif
		readReturnPacket.push_back(packet);
		readReturnCountdown.push_back(RL);
		break;
	case READ_P:
		//make sure a read is allowed
		if (bankStates[packet->address.bank].currentBankState != RowActive ||
			currentClockCycle < bankStates[packet->address.bank].nextRead ||
			packet->address.row != bankStates[packet->address.bank].openRowAddress)
		{
			ERROR_("== Error - Rank " << id << " received a READ_P when not allowed");
			exit(-1);
		}

		//update state table
		bankStates[packet->address.bank].currentBankState = Idle;
		bankStates[packet->address.bank].nextActivate = max(bankStates[packet->address.bank].nextActivate, currentClockCycle + READ_AUTOPRE_DELAY);
		for (size_t i=0;i<NUM_BANKS;i++)
		{
			//will set next read/write for all banks - including current (which shouldnt matter since its now idle)
			bankStates[i].nextRead = max(bankStates[i].nextRead, currentClockCycle + max(BL/2, tCCD));
			bankStates[i].nextWrite = max(bankStates[i].nextWrite, currentClockCycle + READ_TO_WRITE_DELAY);
		}

		//get the read data and put it in the storage which delays until the appropriate time (RL)
#ifndef NO_STORAGE
		DEBUG("~   current cloclCycle: " << dec << currentClockCycle << "   ~");
		banks[packet->address.bank].read(packet);
#else
		packet->data = new DataPacket(); 
		packet->busPacketType = DATA;
#endif

		readReturnPacket.push_back(packet);
		readReturnCountdown.push_back(RL);
		break;
	case WRITE:
		//make sure a write is allowed
		if (bankStates[packet->address.bank].currentBankState != RowActive ||
			currentClockCycle < bankStates[packet->address.bank].nextWrite ||
			packet->address.row != bankStates[packet->address.bank].openRowAddress)
		{
			ERROR_("== Error - Rank " << id << " received a WRITE when not allowed");
			bankStates[packet->address.bank].print();
			exit(0);
		}

		//update state table
		bankStates[packet->address.bank].nextPrecharge = max(bankStates[packet->address.bank].nextPrecharge, currentClockCycle + WRITE_TO_PRE_DELAY);
		for (size_t i=0;i<NUM_BANKS;i++)
		{
			bankStates[i].nextRead = max(bankStates[i].nextRead, currentClockCycle + WRITE_TO_READ_DELAY_B);
			bankStates[i].nextWrite = max(bankStates[i].nextWrite, currentClockCycle + max(BL/2, tCCD));
		}

		//take note of where data is going when it arrives
		incomingWriteBank = packet->address.bank;
		incomingWriteRow = packet->address.row;
		incomingWriteColumn = packet->address.col;
		delete(packet);
		break;
	case WRITE_P:
		//make sure a write is allowed
		if (bankStates[packet->address.bank].currentBankState != RowActive ||
			currentClockCycle < bankStates[packet->address.bank].nextWrite ||
			packet->address.row != bankStates[packet->address.bank].openRowAddress)
		{
			ERROR_("== Error - Rank " << id << " received a WRITE_P when not allowed");
			exit(0);
		}

		//update state table
		bankStates[packet->address.bank].currentBankState = Idle;
		bankStates[packet->address.bank].nextActivate = max(bankStates[packet->address.bank].nextActivate, currentClockCycle + WRITE_AUTOPRE_DELAY);
		for (size_t i=0;i<NUM_BANKS;i++)
		{
			bankStates[i].nextWrite = max(bankStates[i].nextWrite, currentClockCycle + max(tCCD, BL/2));
			bankStates[i].nextRead = max(bankStates[i].nextRead, currentClockCycle + WRITE_TO_READ_DELAY_B);
		}

		//take note of where data is going when it arrives
		incomingWriteBank = packet->address.bank;
		incomingWriteRow = packet->address.row;
		incomingWriteColumn = packet->address.col;
		delete(packet);
		break;
	case ACTIVATE:
		//make sure activate is allowed
		if (bankStates[packet->address.bank].currentBankState != Idle ||
			currentClockCycle < bankStates[packet->address.bank].nextActivate)
		{
			ERROR_("== Error - Rank " << id << " received an ACT when not allowed");
			packet->print();
			bankStates[packet->address.bank].print();
			exit(0);
		}

		bankStates[packet->address.bank].currentBankState = RowActive;
		bankStates[packet->address.bank].nextActivate = currentClockCycle + tRC;
		bankStates[packet->address.bank].openRowAddress = packet->address.row;

		//if AL is greater than one, then posted-cas is enabled - handle accordingly
		if (AL>0)
		{
			bankStates[packet->address.bank].nextWrite = currentClockCycle + (tRCD - AL);
			bankStates[packet->address.bank].nextRead = currentClockCycle + (tRCD - AL);
		}
		else
		{
			bankStates[packet->address.bank].nextWrite = currentClockCycle + (tRCD - AL);
			bankStates[packet->address.bank].nextRead = currentClockCycle + (tRCD - AL);
		}

		bankStates[packet->address.bank].nextPrecharge = currentClockCycle + tRAS;
		for (size_t i=0;i<NUM_BANKS;i++)
		{
			if (i != packet->address.bank)
			{
				bankStates[i].nextActivate = max(bankStates[i].nextActivate, currentClockCycle + tRRD);
			}
		}
		delete(packet); 
		break;
	case PRECHARGE:
		//make sure precharge is allowed
		if (bankStates[packet->address.bank].currentBankState != RowActive ||
			currentClockCycle < bankStates[packet->address.bank].nextPrecharge)
		{
			ERROR_("== Error - Rank " << id << " received a PRE when not allowed");
			exit(0);
		}

		bankStates[packet->address.bank].currentBankState = Idle;
		bankStates[packet->address.bank].nextActivate = max(bankStates[packet->address.bank].nextActivate, currentClockCycle + tRP);
		delete(packet); 
		break;
	case REFRESH:
		DEBUG("I received a REFRESH command on cycle " << dec << currentClockCycle);
		refreshWaiting = false;
		for (size_t i=0;i<NUM_BANKS;i++)
		{
			if (bankStates[i].currentBankState != Idle)
			{
				ERROR_("== Error - Rank " << id << " received a REF when not allowed");
				exit(0);
			}
			bankStates[i].nextActivate = currentClockCycle + tRFC;
		}
		delete(packet); 
		break;
	case DATA:
		// TODO: replace this check with something that works?
		/*
		if(packet->bank != incomingWriteBank ||
			 packet->row != incomingWriteRow ||
			 packet->column != incomingWriteColumn)
			{
				cout << "== Error - Rank " << id << " received a DATA packet to the wrong place" << endl;
				packet->print();
				bankStates[packet->bank].print();
				exit(0);
			}
		*/
#ifndef NO_STORAGE
		DEBUG("~   current cloclCycle: " << dec << currentClockCycle << "   ~");
		banks[packet->address.bank].write(packet);
#endif
		// end of the line for the write packet
		delete(packet);
		break;
	default:
		ERROR_("== Error - Unknown BusPacketType trying to be sent to Bank");
		exit(0);
		break;
	}
}

int Rank::getId() const
{
	return this->id;
}

void Rank::update()
{

	// An outgoing packet is one that is currently sending on the bus
	// do the book keeping for the packet's time left on the bus
	if (outgoingDataPacket != NULL)
	{
		dataCyclesLeft--;
		if (dataCyclesLeft == 0)
		{
			//if the packet is done on the bus, call receiveFromBus and free up the bus
			memoryController->receiveFromBus(outgoingDataPacket);
			outgoingDataPacket = NULL;
		}
	}

	// decrement the counter for all packets waiting to be sent back
	for (size_t i=0;i<readReturnCountdown.size();i++)
	{
		readReturnCountdown[i]--;
	}


	if (readReturnCountdown.size() > 0 && readReturnCountdown[0]==0)
	{
		// RL time has passed since the read was issued; this packet is
		// ready to go out on the bus

		outgoingDataPacket = readReturnPacket[0];
		dataCyclesLeft = BL/2;

		// remove the packet from the ranks
		readReturnPacket.erase(readReturnPacket.begin());
		readReturnCountdown.erase(readReturnCountdown.begin());

		if (DEBUG_BUS)
		{
			PRINTN(" -- R" << this->id << " Issuing On Data Bus : ");
			outgoingDataPacket->print();
			PRINT("");
		}

	}
}

//power down the rank
void Rank::powerDown()
{
	//perform checks
	for (size_t i=0;i<NUM_BANKS;i++)
	{
		if (bankStates[i].currentBankState != Idle)
		{
			ERROR_("== Error - Trying to power down rank " << id << " while not all banks are idle");
			exit(0);
		}

		bankStates[i].nextPowerUp = currentClockCycle + tCKE;
		bankStates[i].currentBankState = PowerDown;
	}

	isPowerDown = true;
}

//power up the rank
void Rank::powerUp()
{
	if (!isPowerDown)
	{
		ERROR_("== Error - Trying to power up rank " << id << " while it is not already powered down");
		exit(0);
	}

	isPowerDown = false;

	for (size_t i=0;i<NUM_BANKS;i++)
	{
		if (bankStates[i].nextPowerUp > currentClockCycle)
		{
			ERROR_("== Error - Trying to power up rank " << id << " before we're allowed to");
			ERROR_(bankStates[i].nextPowerUp << "    " << currentClockCycle);
			exit(0);
		}
		bankStates[i].nextActivate = currentClockCycle + tXP;
		bankStates[i].currentBankState = Idle;
	}
}

void Rank::dischargeCells(std::vector<int> & addrList)
{
    for (auto a : addrList)
    {
		Address addr(a, true);
        banks[addr.bank].writeBit(addr, false);
    }    
}

void Rank::PrintBanks()
{
    DEBUG(" ============================== TOTAL BANKS CELLS ==============================");
    DEBUG("           [ " << dec << currentClockCycle << " cycle ]          ");
    for (int i = 0; (size_t)i < banks.size(); i++)
        banks[i].print(i);
}

void Rank::PrintShort()
{
    DEBUG(" ============================== SHORT BANKS CELLS ==============================");
    DEBUG("           [ " << dec << currentClockCycle << " cycle ]          ");
    banks[5].printShort();
}
