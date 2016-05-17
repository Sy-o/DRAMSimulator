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




#pragma once

#ifndef _DATA_PACKET_H_
#define _DATA_PACKET_H_

#include <string.h> //memcpy 
#include <stdint.h>
#include <stdlib.h> //free
#include <iostream>

using std::ostream; 
using std::dec;
using std::hex;

namespace DRAMSim {

	class DataPacket {
	/*
	 * A very thin wrapper around a data that is sent and received in DRAMSim2
	 *
	 */
	private:
		// Disable copying for a datapacket
		DataPacket(const DataPacket &other);
		DataPacket &operator =(const DataPacket &other); 

	public: 
		/**
		 * Constructor to be used if we are using NO_STORAGE
		 *
		 */
		DataPacket()
		{
			_data = 0;
			_unalignedAddr = 0;
			_hasData = false;
		}

		/**
		 * @param data pointer to a buffer of data; DRAMSim will take ownership of the buffer and will be responsible for freeing it 
		 * @param numBytes number of bytes in the data buffer
		 * @param unalignedAddr DRAMSim will typically kill the bottom bits to align them to the DRAM bus width, but if an access is unaligned (and smaller than the transaction size, the raw address will need to be known to properly execute the read/write)
		 *
		 */
		DataPacket(int data, uint64_t unalignedAddr) :
			_data(data), _unalignedAddr(unalignedAddr)
		{
			_hasData = true;
		}

        DataPacket(uint64_t unalignedAddr) :
            _data(0), _unalignedAddr(unalignedAddr)
        {
            _hasData = false;
        }

		virtual ~DataPacket()
		{
		}

		// accessors
		int getData() const
		{
			return _data;
		}
		uint64_t getAddr() const
		{
			return _unalignedAddr; 
		}
		void setData(int data)
		{
			_data = data;
			_hasData = true;
		}
		bool hasNoData() const
		{
			return !_hasData;
		}

		friend ostream &operator<<(ostream &os, const DataPacket &dp);

	private:
		bool _hasData;
		int _data; 
		uint64_t _unalignedAddr;
	};


} // namespace DRAMSim

#endif // _DATA_PACKET_H_
