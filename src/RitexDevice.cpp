/*
 * RitexDevice.cpp
 *
 *  Created on: Apr 25, 2013
 *      Author: ruinmmal
 */

#include "RitexDevice.h"
#include "CmdStartMeasurement.h"

#include <assert.h>

RitexDevice::RitexDevice(IAdapter* pAdapter)
	: Device(pAdapter)
{
	AdapterParameter* p;
	// add device channels
	AddChannel(new DeviceChannel(0, false, p = new AdapterParameter(1050100010, "Канал состояния", true, "X:X:X:X")));

	// we have one and only sensor
	Sensor* pSensor = new Sensor(1);

	pSensor->AddChannel(new DeviceChannel(1, false, p = new AdapterParameter(1050109000, "Число оборотов ВД", true, "X:X:X:X")));
	//TODO: add to apropreate map


	pSensor->AddChannel(new DeviceChannel(2, false, new AdapterParameter(1050110000, "Средний ток ВД", true, "X:X:X:X")));
	pSensor->AddChannel(new DeviceChannel(3, false, new AdapterParameter(1050111010, "Напряжение сети", true, "X:X:X:X")));

	pSensor->AddChannel(new DeviceChannel(4, false, new AdapterParameter(1050100050, "Загрузка ВД", true, "X:X:X:X")));
	pSensor->AddChannel(new DeviceChannel(5, false, new AdapterParameter(1050100060, "Дисбаланс входных напряжений", true, "X:X:X:X")));
	pSensor->AddChannel(new DeviceChannel(6, false, new AdapterParameter(1050100070, "Дисбаланс выходных напряжений", true, "X:X:X:X")));
	pSensor->AddChannel(new DeviceChannel(7, false, new AdapterParameter(1050100080, "Дисбаланс выходных токов", true, "X:X:X:X")));
	pSensor->AddChannel(new DeviceChannel(8, false, new AdapterParameter(1050104000, "ТМС, температура двигателя", true, "X:X:X:X")));
	pSensor->AddChannel(new DeviceChannel(9, false, new AdapterParameter(1000601000, "Наработка оборудования НИ", true, "X:X:X:X")));

	AddSensor(pSensor);

	m_pProcessor = new ComTrafficProcessor(this);
}

RitexDevice::~RitexDevice() {
	//delete topology
	//TODO:
}

DeviceCommand* RitexDevice::CreateCommand(CmdLineCommand* cmd)
{
	//handle device specific commands
	switch(cmd->m_cmdLineCommandType)
	{
	case CMD_GET_MESUREMENTS:
	{
		//we need COM name and COM speed
		std::string comm;
		int speed;
		cmd->GetParameter("comdevice", comm);
		cmd->GetParameter("baudrate", speed);

		syslog(LOG_ERR, "comdevice: %s", comm.c_str());
		syslog(LOG_ERR, "baudrate: %d", speed);

		return new CmdStartMeasurement(this, comm, speed);
	}

	default:
		return Device::CreateCommand(cmd);
	}
}


bool RitexDevice::StartMesurements(std::string com, int speed)
{
	return m_pProcessor->Create(com, speed);
}


void RitexDevice::ReportDataPacket(DataPacket* packet)
{
	//Now get per-parameter data from packet
}

