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
#include "Log.h"

CmdTestDevice::CmdTestDevice(RitexDevice* p_device, std::string commport, int speed)
	: DeviceCommand(true), m_pDevice(p_device), m_commport(commport), m_speed(speed)
{
	// TODO Auto-generated constructor stub

}

CmdTestDevice::~CmdTestDevice() {
	// TODO Auto-generated destructor stub
}

bool CmdTestDevice::Execute() {
	Log( "IN DAEMON: deviceId=%d", m_pDevice->getDeviceId());
	m_pDevice->TestDevice(this, m_commport, m_speed);
	return true;
}

void CmdTestDevice::SetReply(DataPacket* packet, int status)
{
	if(status == ERROR_READ_NO_ERROR) {
		if(packet->GetCmd() == ACK_INFO_MODE_0) {
			unsigned short tm;
			memcpy(&tm, packet->GetDataPtr() + 33, 2);
			tm = swap16(tm);
			m_rawResult = std::string("8|Ответ получен. Наработка ") + itoa(tm) + std::string(" часов\n");
		} else {
			m_rawResult = std::string("7|Получен неизвестный ответ\n");
		}
		NotifyResultReady();

	}  else {
		DeviceCommand::SetReply(packet, status);
	}
}


