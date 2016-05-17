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








//Bank.cpp
//
//Class file for bank object
//

#include "Bank.h"
#include "BusPacket.h"
#include <assert.h>
#include <bitset>

using namespace std;
using namespace DRAMSim;

Bank::Bank()
{
}

Bank::~Bank()
{
}

void Bank::initialize()
{
    //Initialize all memory at the beginning of time
    for (int r = 0; r < NUM_ROWS; r++)
    {
        rowEntries[r].reset(new vector<uint16_t>(NUM_COLS, 0));
    }
}

void Bank::write(const BusPacket *busPacket)
{
    write(busPacket->row, busPacket->column, busPacket->data->getData());
    cout << "[DPKT] Rank receiving <-- : " <<*(busPacket->data) << endl;
}

void Bank::write(int row, int col, int data)
{
    (*rowEntries[row])[col] = (uint16_t)data;
}

void Bank::writeBit(int row, int col, int bit, bool set)
{
	auto row_ = rowEntries[row].get();

    if (set)
        (*row_)[col] |= 1 << bit;
	else
	{
		if ((*row_)[col] & (1 << bit))
			(*row_)[col] ^= 1 << bit;
	}
}

void Bank::invertBit(int row, int col, int bit)
{
    auto row_ = rowEntries[row].get();

    if ((*row_)[col] & (1 << bit)) //bit is '1'
    {
        (*row_)[col] ^= 1 << bit;
    }     
    else
    {
        (*row_)[col] |= 1 << bit;
    }
}

uint16_t Bank::read(int row, int col)
{
    return (*rowEntries[row])[col];
}

void Bank::read(BusPacket *busPacket)
{
    uint16_t value = read(busPacket->row, busPacket->column);
    busPacket->data.reset(new DataPacket(value, busPacket->physicalAddress));   
    busPacket->busPacketType = DATA;
    cout << "[DPKT] Rank returning --> : " <<*(busPacket->data) << endl;	
}

void Bank::print(int id)
{
    int zeros = 0, ones = 0, unknown = 0;
    double total = NUM_ROWS * NUM_COLS;

    for (auto& r : rowEntries)
    {
        for (auto& c : *(r.second))
        {
            if (!c) zeros++;
            else ones++;           
        }
    }

    unknown = (NUM_ROWS - rowEntries.size()) * NUM_COLS;
    cout << "Bank " << dec << id << ": zeros " << (100 * zeros / total) << "%; ones " << (100 * ones / total) << "%; unknown " << (100 * unknown / total) << "%" << endl;;
}

void Bank::printShort()
{
    for (int r = 0; r < 128; r++)
    {
        for (int c = 0; c < 4; c++)
            cout << std::bitset<16>((*rowEntries[r])[c]) << " ";
        cout << endl;
    }
}

