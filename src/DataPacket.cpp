/*
 * DataPacket.cpp
 *
 *  Created on: Apr 27, 2013
 *      Author: ruinmmal
 */

#include "DataPacket.h"
#include <assert.h>

DataPacket::DataPacket(int type, unsigned char address, unsigned char cmd, time_t timestamp)
	: m_address(address), m_cmd(cmd), m_type(type), m_timestamp(timestamp), m_size(0), m_pData(NULL)
{
}

unsigned char* DataPacket::Allocate(unsigned short size) {
	assert(size > 0);
	if (m_pData) {
		delete [] m_pData;
		m_size = 0;
	}
	m_pData = new unsigned char[size];
	if(m_pData) {
		m_size = size;
	}
	return m_pData;
}

DataPacket::~DataPacket() {
	if (m_pData) {
		delete [] m_pData;
	}
}

