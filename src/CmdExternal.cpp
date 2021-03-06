/*
 * CmdExternal.cpp
 *
 *  Created on: May 6, 2013
 *      Author: ruinmmal
 */

#include "CmdExternal.h"
#include <string.h>
#include <stdio.h>
#include "Utils.h"
#include "Log.h"
#include "ErrorsMsg.h"

CmdExternal::CmdExternal(RitexDevice* device, std::string commport, int speed, CmdLineCommand* cmd, unsigned short cmdId, unsigned char param1, unsigned short param2)
	: DeviceCommand(true, cmd), m_pDevice(device), m_cmdId(cmdId), m_param1(param1), m_param2(param2), m_commport(commport), m_speed(speed)
{

}

CmdExternal::~CmdExternal() {
}

bool CmdExternal::Execute()
{
	Log("CmdExternal::Execute");
	//bool RitexDevice::ExecuteCustomCommand(DeviceCommand* pCmd, std::string com, int speed, unsigned char cmd, unsigned char p1, unsigned short p2)
	return m_pDevice->ExecuteCustomCommand(this, m_commport, m_speed, m_cmdId, m_param1, m_param2);
}

void CmdExternal::SetReply(DataPacket* packet, int status/*, DataPacket* param2*/)
{

	m_finishedTime = time(NULL); //FIXME:

	switch(status) {
	case ERROR_READ_NO_ERROR:
	{
		m_finishedTime = packet->GetTimestamp();

		switch(m_cmdId) {
		// can get 0xB or ACK_ALL_SETTINGS
		case REQ_SETTING_SET:
			switch(packet->GetCmd()) {
			case ACK_ACK:
				m_rawResult = std::string("7;Ошибка. Код ") + itoa(packet->GetDataPtr()[0]) + std::string(": ") + getFaultText(packet->GetDataPtr()[0]) + std::string("\n");
				break;
			case ACK_ALL_SETTINGS:
				{
					//param1 - keeps setting number
					//param2 - new value
					//packet - old value
					m_rawResult = std::string("8;Уставка изменена\n");
					time_t system_time = time(NULL);
					//1. get old value
					unsigned short oldValue;
					if(m_pDevice->SetCurrentSettingValue(m_param1 /* id */, m_param2 /* new value */, oldValue)) {
						DBEventCommon* event = new DBEventCommon();
						event->setChannelId(m_pDevice->GetDeviceStateChannelId());
						event->setTypeId(11);
						event->setRegisterTimeDate(system_time);
						event->setArgument1(itoa(m_param2));
						event->setArgument2(itoa(oldValue));
						event->setArgument3(m_pDevice->getSettingName(m_param1));
						event->setArgument4(m_pParentCommand->m_sourceRaw+"$" + m_pParentCommand->m_cmdIdRaw);
						Log( "CheckSettigsChanged reporting");
						m_pDevice->ReportEvent(event);
					}
				}
				break;
			}
			break;
		// can get 0xB only
		case REQ_VD_OFF:
		case REQ_VD_ON:
		case REQ_VD_ROTATION:
			if (packet->GetCmd() == ACK_ACK) {
				unsigned char errorCode = packet->GetDataPtr()[0];
				m_rawResult = std::string("7;Ошибка. Код ") + itoa(errorCode) + std::string(": ") + getFaultText(errorCode) + std::string("\n");
			} else if(packet->GetCmd() == ACK_INFO){
				m_rawResult = std::string("8;Прием без ошибок\n");
				if(m_cmdId == REQ_VD_ON) {
					m_pDevice->ReportStationState(1,0,time(NULL),m_pParentCommand->m_sourceRaw+"$" + m_pParentCommand->m_cmdIdRaw);
				} else if(m_cmdId == REQ_VD_OFF) {
					m_pDevice->ReportStationState(0,1,time(NULL),m_pParentCommand->m_sourceRaw+"$" + m_pParentCommand->m_cmdIdRaw);
				}
			} else {
				m_rawResult = std::string("7;Получен неизвестный ответ\n");
			}
			break;
		}
		NotifyResultReady();
	}
	break;
	default:
		//common error handling
		DeviceCommand::SetReply(packet, status);
		break;
	}
}

