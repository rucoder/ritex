/*
 * CmdTestDevice.cpp
 *
 *  Created on: May 3, 2013
 *      Author: mmalyshe
 */

#include "CmdTestDevice.h"
#include "RitexDevice.h"
#include "Utils.h"
#include <stdio.h>
#include <string.h>

CmdTestDevice::CmdTestDevice(RitexDevice* p_device, std::string commport, int speed)
	: DeviceCommand(true), m_pDevice(p_device), m_commport(commport), m_speed(speed)
{
	// TODO Auto-generated constructor stub

}

CmdTestDevice::~CmdTestDevice() {
	// TODO Auto-generated destructor stub
}

bool CmdTestDevice::Execute() {
	syslog(LOG_ERR, "IN DAEMON: deviceId=%d", m_pDevice->getDeviceId());
	m_pDevice->TestDevice(this, m_commport, m_speed);
	return true;
}

void CmdTestDevice::SetReply(DataPacket* packet, int status, DataPacket* param2)
{
	if(status == ERROR_READ_NO_ERROR) {
		if(packet->GetCmd() == ACK_INFO_MODE_0) {
			unsigned short tm;
			memcpy(&tm, packet->GetDataPtr() + 33, 2);
			tm = swap16(tm);
			m_rawResult = "8|Ответ получен. Наработка " + itoa(tm) + " часов\n";
		} else {
			m_rawResult = "7|Получен неизвестный ответ\n";
		}
		NotifyResultReady();

	}  else {
		DeviceCommand::SetReply(packet, status);
	}
}


