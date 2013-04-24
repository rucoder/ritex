/*
 * CmdLoggerThread.cpp
 *
 *  Created on: Apr 25, 2013
 *      Author: ruinmmal
 */

#include "CmdLoggerThread.h"

CmdLoggerThread::CmdLoggerThread(std::string db_name)
	: LoggerThread<DBEventCommand*>(db_name)
{
	m_sSqlQuery = "insert  into tblcommand (CommandId, Source, State, Type, Message, Priority, " \
			"DeviceId,ArrivalDate,EndExecDate,Result) values(?,?,?,?,?,?,?,?,?,?)";
}

bool CmdLoggerThread::Insert(DBEventCommand* event)
{
	return true;
}


