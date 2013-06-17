/*
 * DBEventCommon.h
 *
 *  Created on: Apr 24, 2013
 *      Author: ruinmmal
 */

#ifndef DBEVENTCOMMON_H_
#define DBEVENTCOMMON_H_

#include <time.h>
#include <string>

class DBEventCommon {
	int m_typeId;
	int m_channelId;
	time_t m_registerTimeDate;
	std::string m_sArgument1;
	std::string m_sArgument2;
	std::string m_sArgument3;
	std::string m_sArgument4;
public:
	DBEventCommon();

	DBEventCommon(const DBEventCommon& event)
		: m_typeId(event.m_typeId), m_channelId(event.m_channelId) {
		m_registerTimeDate = event.m_registerTimeDate;
		m_sArgument1 = std::string(event.m_sArgument1);
		m_sArgument2 = std::string(event.m_sArgument2);
		m_sArgument3 = std::string(event.m_sArgument3);
		m_sArgument4 = std::string(event.m_sArgument4);
	}

	virtual ~DBEventCommon();

	int getChannelId() const {
		return m_channelId;
	}

	void setChannelId(int channelId) {
		m_channelId = channelId;
	}

	time_t getRegisterTimeDate() const {
		return m_registerTimeDate;
	}

	void setRegisterTimeDate(time_t registerTimeDate) {
		m_registerTimeDate = registerTimeDate;
	}

	const std::string& getArgument1() const {
		return m_sArgument1;
	}

	void setArgument1(const std::string& sArgument1) {
		m_sArgument1 = sArgument1;
	}

	const std::string& getArgument2() const {
		return m_sArgument2;
	}

	void setArgument2(const std::string& sArgument2) {
		m_sArgument2 = sArgument2;
	}

	const std::string& getArgument3() const {
		return m_sArgument3;
	}

	void setArgument3(const std::string& sArgument3) {
		m_sArgument3 = sArgument3;
	}

	const std::string& getArgument4() const {
		return m_sArgument4;
	}

	void setArgument4(const std::string& sArgument4) {
		m_sArgument4 = sArgument4;
	}

	int getTypeId() const {
		return m_typeId;
	}

	void setTypeId(int typeId) {
		m_typeId = typeId;
	}
};

#endif /* DBEVENTCOMMON_H_ */
