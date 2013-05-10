/*
 * DataPacket.cpp
 *
 *  Created on: Apr 27, 2013
 *      Author: ruinmmal
 */

#include "DataPacket.h"
#include "Utils.h"
#include "RitexDevice.h"
#include <assert.h>
#include <string.h>

DataPacket::DataPacket(int type, unsigned char address, unsigned short cmd, time_t timestamp)
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
		m_pData = NULL;
	}
}


unsigned char* DataPacket::CreateRawPacket(int& rawSize)
{
	unsigned char* pRawPacket = new unsigned char[m_size + RAW_PACKET_ENVELOPE_SIZE];
	//fill in values
	pRawPacket[RAW_PACKET_ADDRESS_OFFSET] = m_address;
	pRawPacket[RAW_PACKET_LENGTH_MSB_OFFSET] = MSB(m_size + 1);
	pRawPacket[RAW_PACKET_LENGTH_LSB_OFFSET] = LSB(m_size + 1);
	pRawPacket[RAW_PACKET_CMD_OFFET] = GET_CMD(m_cmd);
	if(m_size > 0) {
		memcpy(&pRawPacket[RAW_PACKET_DATA_OFFSET], m_pData, m_size);
	}
	unsigned short crc = Utils::Crc16(0, pRawPacket, m_size + RAW_PACKET_HEADER_SIZE);

	syslog(LOG_ERR, "PACKET CREATED CRC=0x%X", crc);


	pRawPacket[RAW_PACKET_CRC16_MSB_OFFSET] = MSB(crc);
	pRawPacket[RAW_PACKET_CRC16_LSB_OFFSET] = LSB(crc);

	rawSize = m_size + RAW_PACKET_ENVELOPE_SIZE;
#if 1
	for(int i = 0; i < rawSize; i++)
		syslog(LOG_ERR, "0x%X", pRawPacket[i]);


#endif

	return pRawPacket;
}



