/*
 * DeviceCommand.cpp
 *
 *  Created on: Apr 22, 2013
 *      Author: ruinmmal
 */

#include <stdlib.h>
#include "DeviceCommand.h"
#include "RitexDevice.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

DeviceCommand::DeviceCommand(bool isHwCommand, CmdLineCommand* parent)
	: m_isHWCommand(isHwCommand), m_rawResult(NULL), m_rawResultLength(0), m_pParentCommand(parent)
{
	m_arrivalTime = time(NULL);
}


DeviceCommand::~DeviceCommand() {
	// TODO Auto-generated destructor stub
}

void DeviceCommand::NotifyResultReady()
{
	for(std::list<ICmdResulReadytListener*>::iterator itr = m_Listeners.begin(); itr != m_Listeners.end(); itr++) {
		(*itr)->OnResultReady(this);
	}
}

void DeviceCommand::SetReply(DataPacket* packet, int status)
{
	m_rawResult = new char[1024];
	bzero(m_rawResult,1024);

	if(status == ERROR_READ_BAD_CRC) {
		snprintf(m_rawResult, 1024, "7|Ошибка в CRC\n");
	} else if(status == ERROR_READ_TIMEOUT) {
		snprintf(m_rawResult, 1024, "7|Станция не отвечает\n");
	} else {
		snprintf(m_rawResult, 1024, "7|Ошибка неизвестна: код %d\n", status);
	}
	m_rawResultLength = strlen(m_rawResult)  +1;
	NotifyResultReady();
}






