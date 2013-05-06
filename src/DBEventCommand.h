/*
 * DBEventCommand.h
 *
 *  Created on: Apr 24, 2013
 *      Author: ruinmmal
 */

#ifndef DBEVENTCOMMAND_H_
#define DBEVENTCOMMAND_H_

#include <string>

//CREATE TABLE "tblCommand"
//(
//	"CommandId"	integer		NOT NULL,
//	"Source"	text(10)	NOT NULL,
//	"State"		int		NOT NULL,
//	"Type"		int		NOT NULL,
//	"Message"	text		NOT NULL,
//	"Priority"	int		NOT NULL,
//	"DeviceId"	int		NOT NULL,
//	"ArrivalDate"	text(25)	NOT NULL,
//	"EndExecDate"	text(25)	NULL,
//	"Result"	text		NULL
//)

class DBEventCommand {
public:
	int m_cmdId;
	std::string m_src;
	int m_state;
	int m_type;
	std::string m_msg;
	int m_prio;
	int m_devId;
	std::string m_arrivalTime;
	std::string m_finishedTime;
	std::string m_result;
public:
	DBEventCommand();
	virtual ~DBEventCommand();
};

#endif /* DBEVENTCOMMAND_H_ */
