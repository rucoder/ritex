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

CmdStartMeasurement::CmdStartMeasurement(RitexDevice* p_device, std::string commport, int speed)
	: DeviceCommand(true), m_commport(commport), m_speed(speed), m_pDevice(p_device)
{
	m_cmdId = 2;
}

CmdStartMeasurement::~CmdStartMeasurement() {
	// TODO Auto-generated destructor stub
}

bool CmdStartMeasurement::Execute()
{
	return m_pDevice->StartMesurements(m_commport, m_speed);
}



