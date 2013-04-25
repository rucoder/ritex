/*
 * CmdStartMeasurement.cpp
 *
 *  Created on: Apr 26, 2013
 *      Author: ruinmmal
 */

#include "CmdStartMeasurement.h"

CmdStartMeasurement::CmdStartMeasurement()
	: DeviceCommand(true), m_pDevice(NULL)
{
	m_cmdId = 2;
}

CmdStartMeasurement::CmdStartMeasurement(RitexDevice* p_device)
	: DeviceCommand(true), m_pDevice(p_device)
{
	m_cmdId = 2;
}

CmdStartMeasurement::~CmdStartMeasurement() {
	// TODO Auto-generated destructor stub
}

void CmdStartMeasurement::Serialize()
{
	m_rawCommandLength = sizeof(__sm_serial);
	m_rawCommand = new unsigned char[m_rawCommandLength];
	__sm_serial* cmd = (__sm_serial*)m_rawCommand;
	cmd->m_cmd = m_cmdId;
}

bool CmdStartMeasurement::Execute()
{
	return m_pDevice->StartMesurements();
}



