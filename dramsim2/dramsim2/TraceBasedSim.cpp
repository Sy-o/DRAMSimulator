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

#include <iostream>
#include <fstream>

#include <map>
#include <list>

#include "SystemConfiguration.h"
#include "MemorySystem.h"
#include "Transaction.h"
#include "DataPacket.h"
#include "Parsing.h"

using namespace DRAMSim;
using namespace std;

//#define RETURN_TRANSACTIONS 1

#ifndef _SIM_
int SHOW_SIM_OUTPUT = 1;
ofstream visDataOut; //mostly used in MemoryController

#ifdef RETURN_TRANSACTIONS
class TransactionReceiver
{
	private: 
		map<uint64_t, list<uint64_t> > pendingReadRequests; 
		map<uint64_t, list<uint64_t> > pendingWriteRequests; 

	public: 
		void add_pending(const Transaction &t, uint64_t cycle)
		{
			// C++ lists are ordered, so the list will always push to the back and
			// remove at the front to ensure ordering
			if (t.transactionType == DATA_READ)
			{
				pendingReadRequests[t.address].push_back(cycle); 
			}
			else if (t.transactionType == DATA_WRITE)
			{
				pendingWriteRequests[t.address].push_back(cycle); 
			}
			else
			{
				ERROR_("This should never happen"); 
				exit(-1);
			}
		}

		void read_complete(unsigned id, uint64_t address, uint64_t done_cycle)
		{
			map<uint64_t, list<uint64_t> >::iterator it;
			it = pendingReadRequests.find(address); 
			if (it == pendingReadRequests.end())
			{
				ERROR_("Cant find a pending read for this one"); 
				exit(-1);
			}
			else
			{
				if (it->second.size() == 0)
				{
					ERROR_("Nothing here, either"); 
					exit(-1); 
				}
			}

			uint64_t added_cycle = pendingReadRequests[address].front();
			uint64_t latency = done_cycle - added_cycle;

			pendingReadRequests[address].pop_front();
			cout << "Read Callback:  0x"<< std::hex << address << std::dec << " latency="<<latency<<"cycles ("<< done_cycle<< "->"<<added_cycle<<")"<<endl;
		}
		void write_complete(unsigned id, uint64_t address, uint64_t done_cycle)
		{
			map<uint64_t, list<uint64_t> >::iterator it;
			it = pendingWriteRequests.find(address); 
			if (it == pendingWriteRequests.end())
			{
				ERROR_("Cant find a pending read for this one"); 
				exit(-1);
			}
			else
			{
				if (it->second.size() == 0)
				{
					ERROR_("Nothing here, either"); 
					exit(-1); 
				}
			}

			uint64_t added_cycle = pendingWriteRequests[address].front();
			uint64_t latency = done_cycle - added_cycle;

			pendingWriteRequests[address].pop_front();
			cout << "Write Callback: 0x"<< std::hex << address << std::dec << " latency="<<latency<<"cycles ("<< done_cycle<< "->"<<added_cycle<<")"<<endl;
		}
};
#endif


#endif

#ifndef _SIM_

void alignTransactionAddress(Transaction &trans)
{
	// zero out the low order bits which correspond to the size of a transaction

	unsigned throwAwayBits = dramsim_log2((BL*JEDEC_DATA_BUS_BITS/8));

	trans.address >>= throwAwayBits;
	trans.address <<= throwAwayBits;
}

int TraceBasedStart(int argc, char **argv)
{
    // int c;
    string traceFileName = "traces/misc_short_3.trc";
    TraceType traceType;
    string systemIniFilename = "system.ini";
    string deviceIniFilename = "ini/DDR2_micron_1Mb.ini";
    string pwdString = "";
    unsigned megsOfMemory = 1; //1 Mb

    string overrideKey = "";
    string overrideVal = "";
    string tmp = "";
    
    unsigned numCycles = 4000;
    
    // get the trace filename
    string temp = traceFileName.substr(traceFileName.find_last_of("/") + 1);

    //get the prefix of the trace name
    temp = temp.substr(0, temp.find_first_of("_"));
    if (temp == "mase")
    {
        traceType = mase;
    }
    else if (temp == "k6")
    {
        traceType = k6;
    }
    else if (temp == "misc")
    {
        traceType = misc;
    }
    else
    {
        ERROR_("== Unknown Tracefile Type : " << temp);
        exit(0);
    }


    // no default value for the default model name
    if (deviceIniFilename.length() == 0)
    {
        ERROR_("Please provide a device ini file");
       // usage();
        exit(-1);
    }


    //ignore the pwd argument if the argument is an absolute path
    if (pwdString.length() > 0 && traceFileName[0] != '/')
    {
        traceFileName = pwdString + "/" + traceFileName;
    }

    DEBUG("== Loading trace file '" << traceFileName << "' == ");

    ifstream traceFile;
    string line;


    MemorySystem *memorySystem = new MemorySystem(0, deviceIniFilename, systemIniFilename, pwdString, traceFileName, megsOfMemory);

#ifdef RETURN_TRANSACTIONS
    TransactionReceiver transactionReceiver; 
    /* create and register our callback functions */
    Callback_t *read_cb = new Callback<TransactionReceiver, void, unsigned, uint16_t, uint64_t>(&transactionReceiver, &TransactionReceiver::read_complete);
    Callback_t *write_cb = new Callback<TransactionReceiver, void, unsigned, uint64_t, uint64_t>(&transactionReceiver, &TransactionReceiver::write_complete);
    memorySystem->RegisterCallbacks(read_cb, write_cb, NULL);
#endif


    uint64_t addr;
    uint64_t clockCycle = 0;
    enum TransactionType transType;

    DataPacket *data = NULL;
    int lineNumber = 0;
    Transaction trans;
    bool pendingTrans = false;

    traceFile.open(traceFileName.c_str());

    if (!traceFile.is_open())
    {
        cout << "== Error - Could not open trace file" << endl;
        exit(0);
    }

    for (size_t i = 0; i<numCycles; i++)
    {
        if (!pendingTrans)
        {
            if (!traceFile.eof())
            {
                getline(traceFile, line);

                if (line.size() > 0)
                {
                    data = parseTraceFileLine(line, addr, transType, clockCycle, traceType);
                    trans = Transaction(transType, addr, data);
                    alignTransactionAddress(trans);

                    if (i >= clockCycle)
                    {
                        if (!(*memorySystem).addTransaction(trans))
                        {
                            pendingTrans = true;
                        }
                        else
                        {
#ifdef RETURN_TRANSACTIONS
                            transactionReceiver.add_pending(trans, i); 
#endif
                        }
                    }
                    else
                    {
                        pendingTrans = true;
                    }
                }
                else
                {
                    DEBUG("WARNING: Skipping line " << lineNumber << " ('" << line << "') in tracefile");
                }
                lineNumber++;
            }
            else
            {
                //we're out of trace, set pending=false and let the thing spin without adding transactions
                pendingTrans = false;
            }
        }

        else if (pendingTrans && i >= clockCycle)
        {
            pendingTrans = !(*memorySystem).addTransaction(trans);
            if (!pendingTrans)
            {
#ifdef RETURN_TRANSACTIONS
                transactionReceiver.add_pending(trans, i); 
#endif
            }
        }

        (*memorySystem).update();
    }

    traceFile.close();
    (*memorySystem).printStats(true);
    // make valgrind happy
    delete(memorySystem);
    system("pause");
}
#endif
