/*
 * RitexDevice.cpp
 *
 *  Created on: Apr 25, 2013
 *      Author: ruinmmal
 */

#include "RitexDevice.h"
#include "CmdStartMeasurement.h"
#include "CmdTestDevice.h"
#include "Utils.h"

#include <assert.h>

#include <stdio.h>
#include <string.h>

struct controller_data_t {
	int m_offset;
	int m_size;
	int m_cmd;
	controller_data_t(int cmd, int offset, int size) {
		syslog(LOG_ERR, "^^^^^^^^^^CONSTRUCTOR^^^^^^^^^");
		m_cmd = cmd;
		m_offset = offset;
		m_size = size;
	}
	virtual ~controller_data_t() {}
};

RitexDevice::RitexDevice(IAdapter* pAdapter)
	: Device(pAdapter), m_writeMode(WRITE_MODE_0)
{
	// add device channels
	AddChannel(new DeviceChannel(0, false, new AdapterParameter(1050100010, "Канал состояния", true, "X:X:X:X")));

	// we have one and only sensor
	Sensor* pSensor = new Sensor(1);

	pSensor->AddChannel(new DeviceChannel(1, false, new AdapterParameter(1050109000, "Число оборотов ВД", true, "X:X:X:X",
			false, new controller_data_t(ACK_INFO_MODE_0,2,2))));


	pSensor->AddChannel(new DeviceChannel(2, false, new AdapterParameter(1050110000, "Средний ток ВД", true, "X:X:X:X")));
	pSensor->AddChannel(new DeviceChannel(3, false, new AdapterParameter(1050111010, "Напряжение сети", true, "X:X:X:X")));

	pSensor->AddChannel(new DeviceChannel(4, false, new AdapterParameter(1050100050, "Загрузка ВД", true, "X:X:X:X",
			false, new controller_data_t(ACK_INFO_MODE_0,7,1))));

	pSensor->AddChannel(new DeviceChannel(5, false, new AdapterParameter(1050100060, "Дисбаланс входных напряжений", true, "X:X:X:X")));
	pSensor->AddChannel(new DeviceChannel(6, false, new AdapterParameter(1050100070, "Дисбаланс выходных напряжений", true, "X:X:X:X")));
	pSensor->AddChannel(new DeviceChannel(7, false, new AdapterParameter(1050100080, "Дисбаланс выходных токов", true, "X:X:X:X")));
	pSensor->AddChannel(new DeviceChannel(8, false, new AdapterParameter(1050104000, "ТМС, температура двигателя", true, "X:X:X:X")));
	pSensor->AddChannel(new DeviceChannel(9, false, new AdapterParameter(1000601000, "Наработка оборудования НИ", true, "X:X:X:X",
			false, new controller_data_t(ACK_INFO_MODE_0,33,2))));

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
	case CMD_TEST_DEVICE:
	{
		//we need COM name and COM speed
		std::string comm;
		int speed;
		cmd->GetParameter("comdevice", comm);
		cmd->GetParameter("baudrate", speed);

		syslog(LOG_ERR, "comdevice: %s", comm.c_str());
		syslog(LOG_ERR, "baudrate: %d", speed);
		return new CmdTestDevice(this, comm, speed);
	}
	default:
		return Device::CreateCommand(cmd);
	}
}




void RitexDevice::CreateOffsetTable()
{
	ParameterFilter filter = m_pAdapter->GetParameterFilter();

	for(int i = 0; i < filter.GetSize(); i++) {
		int param = filter[i].m_paramId;

		//find parameter in topology
		AdapterParameter* pParam = FindGarameter(param);
		if(pParam) {
			//TODO: templetize it!!!!
			controller_data_t* pData = reinterpret_cast<controller_data_t*>(pParam->getControllerData());
			syslog(LOG_ERR,"Param: %d found. ControllerData == %p", param, pParam->getControllerData());
			if(pData) {
				//store data in offset table
				offset_table_entry_t entry;
				entry.m_channelId = filter[i].m_channelId;
				entry.m_paramId = param;
				entry.m_cmd = pData->m_cmd;
				entry.m_size = pData->m_size;
				entry.m_offset = pData->m_offset;
				m_offsetTable.push_back(entry);
			}
		}
	}

	printf("CreateOffsetTable: size = [%lu]\n", m_offsetTable.size());
}

bool RitexDevice::StartMesurements(DeviceCommand* pCmd, std::string com, int speed)
{
	if(m_pProcessor->isRunning())
		return false;
	CreateOffsetTable();
	return m_pProcessor->Create(com, speed);
}

/*
 * cannot be called when data capture is in progress
 */
bool RitexDevice::TestDevice(DeviceCommand* pCmd, std::string com, int speed)
{
	if(m_pProcessor->isRunning())
		return false;
	bool result = m_pProcessor->Create(com, speed,true);

	if(result) {
		custom_command_t* pCustomCmd = new custom_command_t();
		pCustomCmd->m_pDataPacket = new DataPacket(TYPE_CMD, KSU_ADDRESS, REQ_INFO_REQUEST_MODE_0, time(NULL));
		pCustomCmd->m_pParentCommand = pCmd;
		m_pProcessor->SendCustomCmd(pCustomCmd);
		m_pProcessor->Start();
	}

	return result;
}



void RitexDevice::ReportDataPacket(DataPacket* packet)
{
	printf("Reporting packet\n");

	for(unsigned int i = 0; i < m_offsetTable.size(); i++) {
		// parameter value is stored in this packet
		if(m_offsetTable[i].m_cmd == packet->GetCmd()) {
			DBDataPacket* event = new DBDataPacket();

			unsigned char* pData = packet->GetDataPtr();

			event->setChannelId(m_offsetTable[i].m_channelId);
			event->setParamId(m_offsetTable[i].m_paramId);
			event->setRegisterDate(packet->GetTimestamp());

			if(m_offsetTable[i].m_size == 1) {
				unsigned char value;
				memcpy(&value, pData+m_offsetTable[i].m_offset, 1);
				event->setValue((float)value);
			} else if (m_offsetTable[i].m_size == 2) {
				unsigned short value;
				memcpy(&value, pData+m_offsetTable[i].m_offset, 2);
				value = swap16(value);
				event->setValue((float)value);
			}
			m_pAdapter->getDataLogger()->EnqueData(event);
		}
	}

}

