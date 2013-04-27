/*
 * RitexDevice.h
 *
 *  Created on: Apr 25, 2013
 *      Author: ruinmmal
 */

#ifndef RITEXDEVICE_H_
#define RITEXDEVICE_H_

#include "Device.h"
#include "DataPacket.h"
#include "ComTrafficProcessor.h"

//Command definitions
#define REQ_VD_ON               0x80
#define REQ_VD_OFF              0x84
#define REQ_VD_ROTATION         0x8C
#define REQ_SETTING_SET         0x90
#define REQ_DEVICE_REQ          0x88
#define REQ_INFO_REQUEST        0x94
#define REQ_STORED_INFO_REQUEST 0x98
#define REQ_SETTING_ALL_GET     0x9C
#define REQ_SET_SPEED           0xA0
#define REQ_PASSWORD_GET        0xA4

#define ACK_INFO                0x64
#define ACK_STORED_INFO         0x68
#define ACK_ALL_SETTINGS        0x6C
#define ACK_ACK                 0x0B
#define ACK_PASSWORDS           0x70

#define TYPE_CMD 0
#define TYPE_ACK 1


class RitexDevice: public Device {
protected:
	RitexDevice();
	ComTrafficProcessor* m_pProcessor;
public:
	RitexDevice(IAdapter* pAdapter);
	virtual ~RitexDevice();
	DeviceCommand* CreateCommand(void* rawCommand, int length);
	DeviceCommand* CreateCommand(CmdLineCommand* cmd);

	//event/data reporting
	void ReportDataPacket(DataPacket* packet);

	// command implementations
	bool StartMesurements();
};

#endif /* RITEXDEVICE_H_ */
