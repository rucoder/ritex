/*
 * DBDataPacket.h
 *
 *  Created on: Apr 25, 2013
 *      Author: ruinmmal
 */

#ifndef DBDATAPACKET_H_
#define DBDATAPACKET_H_

#include <time.h>

class DBDataPacket {
protected:
	int m_channelId;
	int m_paramId;
	time_t m_registerDate;
	float m_value;
public:
	DBDataPacket();
	virtual ~DBDataPacket();

	int getChannelId() const {
		return m_channelId;
	}

	void setChannelId(int channelId) {
		m_channelId = channelId;
	}

	int getParamId() const {
		return m_paramId;
	}

	void setParamId(int paramId) {
		m_paramId = paramId;
	}

	time_t getRegisterDate() const {
		return m_registerDate;
	}

	void setRegisterDate(time_t registerDate) {
		m_registerDate = registerDate;
	}

	float getValue() const {
		return m_value;
	}

	void setValue(float value) {
		m_value = value;
	}
};

#endif /* DBDATAPACKET_H_ */
