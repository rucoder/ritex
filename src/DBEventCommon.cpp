/*
 * DBEventCommon.cpp
 *
 *  Created on: Apr 24, 2013
 *      Author: ruinmmal
 */

#include "DBEventCommon.h"

DBEventCommon::DBEventCommon()
	: m_typeId(0), m_channelId(-1), m_registerTimeDate(0)
{
	m_sArgument1 = std::string("X");
	m_sArgument2 = std::string("X");
	m_sArgument3 = std::string("X");
	m_sArgument4 = std::string("X");
}

DBEventCommon::~DBEventCommon() {
	// TODO Auto-generated destructor stub
}

