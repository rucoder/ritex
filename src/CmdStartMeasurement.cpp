/*
 * CmdStartMeasurement.cpp
 *
 *  Created on: Apr 26, 2013
 *      Author: ruinmmal
 */

#include "CmdStartMeasurement.h"

CmdStartMeasurement::CmdStartMeasurement(RitexDevice* p_device, std::string commport, int speed)
	: DeviceCommand(true), m_commport(commport), m_speed(speed), m_pDevice(p_device)
{

}

CmdStartMeasurement::~CmdStartMeasurement() {
	// TODO Auto-generated destructor stub
}

bool CmdStartMeasurement::Execute()
{
	bool result =  m_pDevice->StartMesurements(this, m_commport, m_speed);
	NotifyResultReady();
	return result;
}



