/*
 * CmdExternal.cpp
 *
 *  Created on: May 6, 2013
 *      Author: ruinmmal
 */

#include "CmdExternal.h"
#include <string.h>
#include <stdio.h>

CmdExternal::CmdExternal(RitexDevice* device, std::string commport, int speed, CmdLineCommand* cmd, unsigned short cmdId, unsigned char param1, unsigned short param2)
	: DeviceCommand(true, cmd), m_pDevice(device), m_cmdId(cmdId), m_param1(param1), m_param2(param2), m_commport(commport), m_speed(speed)
{
	// TODO Auto-generated constructor stub

}

CmdExternal::~CmdExternal() {
	// TODO Auto-generated destructor stub
}

bool CmdExternal::Execute()
{
	syslog(LOG_ERR,"CmdExternal::Execute");
	//printf("");
	//bool RitexDevice::ExecuteCustomCommand(DeviceCommand* pCmd, std::string com, int speed, unsigned char cmd, unsigned char p1, unsigned short p2)
	return m_pDevice->ExecuteCustomCommand(this, m_commport, m_speed, m_cmdId, m_param1, m_param2);
}

void CmdExternal::SetReply(DataPacket* packet, int status)
{

	m_finishedTime = packet->GetTimestamp();

	switch(status) {
	case ERROR_READ_NO_ERROR:
	{
		m_rawResult = new char[1024];
		bzero(m_rawResult,1024);

		switch(m_cmdId) {
		// can get 0xB or ACK_ALL_SETTINGS
		case REQ_SETTING_SET:
			switch(packet->GetCmd()) {
			case ACK_ACK:
				snprintf(m_rawResult, 1024, "7|Ошибка. Код %d\n", packet->GetDataPtr()[0]);
				break;
			case ACK_ALL_SETTINGS:
				snprintf(m_rawResult, 1024, "8|Уставка изменена\n");
				break;
			}
			break;
		// can get 0xB only
		case REQ_VD_OFF:
		case REQ_VD_ON:
		case REQ_VD_ROTATION:
			if (packet->GetCmd() != ACK_ACK) {
				snprintf(m_rawResult, 1024, "7|Получен неизвестный ответ\n");
			} else {
				unsigned char errorCode = packet->GetDataPtr()[0];
				if(errorCode == 0) {
					snprintf(m_rawResult, 1024, "8|Прием без ошибок\n");
				} else {
					snprintf(m_rawResult, 1024, "7|Ошибка. Код %d\n", errorCode);
				}
			}
			break;
		}
		m_rawResultLength = strlen(m_rawResult)  +1;
		NotifyResultReady();
	}
	break;
	default:
		//common error handling
		DeviceCommand::SetReply(packet, status);
		break;
	}
}

