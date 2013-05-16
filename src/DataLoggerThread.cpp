/*
 * DataLoggerThread.cpp
 *
 *  Created on: Apr 25, 2013
 *      Author: ruinmmal
 */

#include "DataLoggerThread.h"
#include "Utils.h"
#include <syslog.h>
#include <assert.h>

DataLoggerThread::DataLoggerThread(std::string db_name)
	: LoggerThread<DBDataPacket*>(db_name, "DataLoggerThread")
{
	// int,int,text (date-time),float
	m_sSqlQuery = "INSERT INTO tblparamdata (ChannelId, ParamId, RegisterDate, Value)  VALUES(?,?,?,?)";
}

bool DataLoggerThread::BindParams(DBDataPacket* event)
{
	//::Log("Inserting data: chId=%d, paramId=%d, td=%lu %s val=%g",event->getChannelId(), event->getParamId(),
	// event->getRegisterDate(), TimeToString(event->getRegisterDate()).c_str(), event->getValue());
	return  Bind(1, event->getChannelId()) &&
			Bind( 2, event->getParamId()) &&
			Bind( 3, TimeToString(event->getRegisterDate())) &&
			Bind( 4, event->getValue());
}



