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
#include "Utils.h"

DeviceCommand::DeviceCommand(bool isHwCommand, CmdLineCommand* parent)
	: m_isHWCommand(isHwCommand), m_pParentCommand(parent), m_arrivalTime(0), m_finishedTime(0)
{
	m_arrivalTime = time(NULL);
}


DeviceCommand::~DeviceCommand() {
	syslog(LOG_ERR, "[DESTRUCTOR] ~DeviceCommand() : result=%s", m_rawResult.c_str());
	m_Listeners.clear();
}

void DeviceCommand::NotifyResultReady()
{
	for(std::list<ICmdResulReadytListener*>::iterator itr = m_Listeners.begin(); itr != m_Listeners.end(); itr++) {
		(*itr)->OnResultReady(this);
	}
}

void DeviceCommand::SetReply(DataPacket* packet, int status, DataPacket* param2)
{
	if(status == ERROR_READ_BAD_CRC) {
		m_rawResult = "7|Ошибка в CRC\n";
	} else if(status == ERROR_READ_TIMEOUT) {
		m_rawResult = "7|Станция не отвечает\n";
	} else {
		m_rawResult = "7|Ошибка неизвестна: код " + itoa(status) + "\n";
	}

	NotifyResultReady();
}






