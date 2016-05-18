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





#include "Parsing.h"
#include <sstream>
#include <fstream>

using namespace DRAMSim;
using namespace std;

void trim(string& s)
{
    s.erase(0, s.find_first_not_of(" \n\r\t"));
    s.erase(s.find_last_not_of(" \n\r\t") + 1);
}

DataPacket *parseTraceFileLine(string &line, uint64_t &addr, enum TransactionType &transType, uint64_t &clockCycle, TraceType type)
{
    size_t previousIndex = 0;
    size_t spaceIndex = 0;
    DataPacket *dataPacket = new DataPacket();
    string addressStr = "", cmdStr = "", dataStr = "", ccStr = "";
#ifndef _SIM_
    bool useClockCycle = false;
#else
    bool useClockCycle = true;
#endif

    switch (type)
    {
    case k6:
    {
               spaceIndex = line.find_first_of(" ", 0);

               addressStr = line.substr(0, spaceIndex);
               previousIndex = spaceIndex;

               spaceIndex = line.find_first_not_of(" ", previousIndex);
               cmdStr = line.substr(spaceIndex, line.find_first_of(" ", spaceIndex) - spaceIndex);
               previousIndex = line.find_first_of(" ", spaceIndex);

               spaceIndex = line.find_first_not_of(" ", previousIndex);
               ccStr = line.substr(spaceIndex, line.find_first_of(" ", spaceIndex) - spaceIndex);

               if (cmdStr.compare("P_MEM_WR") == 0 ||
                   cmdStr.compare("BOFF") == 0)
               {
                   transType = DATA_WRITE;
               }
               else if (cmdStr.compare("P_FETCH") == 0 ||
                   cmdStr.compare("P_MEM_RD") == 0 ||
                   cmdStr.compare("P_LOCK_RD") == 0 ||
                   cmdStr.compare("P_LOCK_WR") == 0)
               {
                   transType = DATA_READ;
               }
               else
               {
                   ERROR_("== Unknown Command : " << cmdStr);
                   exit(0);
               }

               istringstream a(addressStr.substr(2));//gets rid of 0x
               a >> hex >> addr;

               //if this is set to false, clockCycle will remain at 0, and every line read from the trace
               //  will be allowed to be issued
               if (useClockCycle)
               {
                   istringstream b(ccStr);
                   b >> clockCycle;
               }
               break;
    }
    case mase:
    {
                 spaceIndex = line.find_first_of(" ", 0);

                 addressStr = line.substr(0, spaceIndex);
                 previousIndex = spaceIndex;

                 spaceIndex = line.find_first_not_of(" ", previousIndex);
                 cmdStr = line.substr(spaceIndex, line.find_first_of(" ", spaceIndex) - spaceIndex);
                 previousIndex = line.find_first_of(" ", spaceIndex);

                 spaceIndex = line.find_first_not_of(" ", previousIndex);
                 ccStr = line.substr(spaceIndex, line.find_first_of(" ", spaceIndex) - spaceIndex);

                 if (cmdStr.compare("IFETCH") == 0 ||
                     cmdStr.compare("READ") == 0)
                 {
                     transType = DATA_READ;
                 }
                 else if (cmdStr.compare("WRITE") == 0)
                 {
                     transType = DATA_WRITE;
                 }
                 else
                 {
                     ERROR_("== Unknown command in tracefile : " << cmdStr);
                 }

                 istringstream a(addressStr.substr(2));//gets rid of 0x
                 a >> hex >> addr;

                 //if this is set to false, clockCycle will remain at 0, and every line read from the trace
                 //  will be allowed to be issued
                 if (useClockCycle)
                 {
                     istringstream b(ccStr);
                     b >> clockCycle;
                 }

                 break;
    }
    case misc:
        spaceIndex = line.find_first_of(" ", spaceIndex + 1);
        if (spaceIndex == string::npos)
        {
            ERROR_("Malformed line: '" << line << "'");
        }

        addressStr = line.substr(previousIndex, spaceIndex);
        previousIndex = spaceIndex;

        //DEBUGN("---PARSING: address = '" << addressStr << "';");

        spaceIndex = line.find_first_of(" ", spaceIndex + 1);
        if (spaceIndex == string::npos)
        {
            cmdStr = line.substr(previousIndex + 1);
        }
        else
        {
            cmdStr = line.substr(previousIndex + 1, spaceIndex - previousIndex - 1);
            dataStr = line.substr(spaceIndex + 1);
            trim(dataStr);

            //DEBUGN("the data = '" << dataStr << "';");
            //DEBUG("command = '" << cmdStr << "';" );			
        }

        //convert address string -> number
        istringstream b(addressStr.substr(2)); //substr(2) chops off 0x characters
        b >> hex >> addr;

        // parse command
        if (cmdStr.compare("read") == 0)
        {
            transType = DATA_READ;
        }
        else if (cmdStr.compare("write") == 0)
        {
            transType = DATA_WRITE;
        }
        else
        {
            ERROR_("INVALID COMMAND '" << cmdStr << "'");
            exit(-1);
        }
        if (SHOW_SIM_OUTPUT)
        {
            DEBUG("ADDR='" << hex << addr << dec << "',CMD='" << (transType ? "write" : "read") << "'");
        }

        //parse data
        //if we are running in a no storage mode, don't allocate space, just return NULL
#ifndef NO_STORAGE
        if (dataStr.size() > 0 && transType == DATA_WRITE)
        {
            if (dataStr.size() > 5)
            {
                ERROR_("Data '" << dataStr << "' is too long!");
                exit(-1);
            }

            unsigned val;
            istringstream iss(dataStr);
            iss >> hex >> val;

            dataPacket = new DRAMSim::DataPacket((uint16_t)val, addr);
            PRINT("ds=" << dataStr << ", dp=" << *dataPacket);
        }
#else
#endif

        break;
    }
    return dataPacket;
}

FaultType GetFaultTypeFromString(string type)
{
	if (type == "SAF") return SAF;
	if (type == "TF") return TF;
	if (type == "CFin") return CFin;
	if (type == "CFid") return CFid;
	if (type == "CFst") return CFst;
	return NONE;
}

vector<Fault> ParseCSV(string filePath)
{
	vector<Fault> faults;
	ifstream file(filePath);
	while (file.good())
	{
		string line;
		getline(file, line);

		Fault* fault = 0;
		int paramCount = 0;

		stringstream strstr(line);
		string value = "";

		while (getline(strstr, value, '\t'))
		{
			if (value == "SAF" || value == "TF" || value == "CFid" || value == "CFin" || value == "CFst")
			{
				if (GetFaultTypeFromString(value) != NONE)
				{
					faults.push_back(Fault());
					fault = &faults.back();
					fault->type = GetFaultTypeFromString(value);
				}
			}
			else
			{
				if (fault)
				{
					paramCount++;
					switch (fault->type)
					{
					case SAF:
					case TF:
						if (paramCount == 1)
						{
							istringstream iss(value.substr(2));
							iss >> hex >> fault->victimAddress;
						}
						if (paramCount == 2)
						{
							istringstream iss(value);
							iss >> dec >> fault->victimValue;
						}
						break;
					case CFin:
					case CFst:
						if (paramCount == 1 || paramCount == 3)
						{
							istringstream iss(value.substr(2));
							if (paramCount == 1)
								iss >> hex >> fault->victimAddress;
							else
								iss >> hex >> fault->agressorAddress;
						}
						if (paramCount == 2 || paramCount == 4)
						{
							istringstream iss(value);
							if (paramCount == 2)
								iss >> dec >> fault->victimValue;
							else
								iss >> dec >> fault->agressorValue;
						}
						break;
					case CFid:
						if (paramCount == 1 || paramCount == 3)
						{
							istringstream iss(value.substr(2));
							if (paramCount == 1)
								iss >> hex >> fault->victimAddress;
							else
								iss >> hex >> fault->agressorAddress;
						}
						if (paramCount == 4)
						{
							istringstream iss(value);
							iss >> dec >> fault->agressorValue;
						}
						break;
					}
				}
			}
		}
	}
	return faults;
}