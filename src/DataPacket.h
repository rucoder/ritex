/*
 * DataPacket.h
 *
 *  Created on: Apr 27, 2013
 *      Author: ruinmmal
 */

#ifndef DATAPACKET_H_
#define DATAPACKET_H_

#include <time.h>

#define RAW_PACKET_HEADER_SIZE (1 + 2 + 1) //addr + length + cmd
#define RAW_PACKET_ENVELOPE_SIZE (RAW_PACKET_HEADER_SIZE + 2) //addr + length + cmd + CRC16

#define RAW_PACKET_ADDRESS_OFFSET 0
#define RAW_PACKET_LENGTH_MSB_OFFSET 1
#define RAW_PACKET_LENGTH_LSB_OFFSET 2
#define RAW_PACKET_CMD_OFFET 3
#define RAW_PACKET_DATA_OFFSET RAW_PACKET_HEADER_SIZE
#define RAW_PACKET_CRC16_MSB_OFFSET (m_size + RAW_PACKET_HEADER_SIZE)
#define RAW_PACKET_CRC16_LSB_OFFSET (m_size + RAW_PACKET_HEADER_SIZE + 1)

class DataPacket {
protected:
	unsigned char m_address;
	unsigned short m_cmd;
	int           m_type;
	time_t   	  m_timestamp;
	unsigned short m_size;
	unsigned char* m_pData;
public:
	DataPacket(int type, unsigned char address, unsigned short cmd, time_t timestamp);
	virtual ~DataPacket();
	unsigned char* Allocate(unsigned short size);
	unsigned char* GetDataPtr() const { return m_pData; }
	unsigned short GetSize() const {return m_size; }
	unsigned short GetCmd() const { return m_cmd; }
	unsigned short SetCmdMode(unsigned short mode) {m_cmd |= (mode & 0xff) << 8; return m_cmd;}
	unsigned int GetType() const { return m_type; }
	unsigned int GetAddress() const { return m_address; }
	time_t& GetTimestamp() { return m_timestamp; }
	unsigned char* CreateRawPacket(int &rawSize);
};

#endif /* DATAPACKET_H_ */
