/*
 * DataPacket.h
 *
 *  Created on: Apr 27, 2013
 *      Author: ruinmmal
 */

#ifndef DATAPACKET_H_
#define DATAPACKET_H_

#include <time.h>

class DataPacket {
protected:
	unsigned char m_address;
	unsigned char m_cmd;
	int           m_type;
	time_t   	  m_timestamp;
	unsigned short m_size;
	unsigned char* m_pData;
public:
	DataPacket(int type, unsigned char address, unsigned char cmd, time_t timestamp);
	virtual ~DataPacket();
	unsigned char* Allocate(unsigned short size);
	unsigned char* GetDataPtr() { return m_pData; }
	unsigned short GetSize() const {return m_size; }
	unsigned char GetCmd() const { return m_cmd; }
	unsigned int GetType() const { return m_type; }
	time_t& GetTimestamp() { return m_timestamp; }
};

#endif /* DATAPACKET_H_ */
