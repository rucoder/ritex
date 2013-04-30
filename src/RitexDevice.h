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

#define KSU_ADDRESS 0x36

#define WRITE_MODE_0 0x0000
#define WRITE_MODE_1 0x0100
#define WRITE_MODE_2 0x0200
#define WRITE_MODE_3 0x0300
#define WRITE_MODE_4 0x0400
#define WRITE_MODE_5 0x0500
#define WRITE_MODE_6 0x0600

#define CMD_MASK 0x00FF
#define MODE_MASK 0xFF00
#define CMD_SHIFT 0
#define MODE_SHIFT 8

//Command definitions
#define REQ_VD_ON               0x80
#define REQ_VD_OFF              0x84
#define REQ_VD_ROTATION         0x8C
#define REQ_SETTING_SET         0x90
#define REQ_DEVICE_REQ          0x88

#define REQ_INFO_REQUEST_MODE_0        (WRITE_MODE_0 | 0x0094)
#define REQ_INFO_REQUEST_MODE_1        (WRITE_MODE_1 | 0x0094)
#define REQ_INFO_REQUEST_MODE_2        (WRITE_MODE_2 | 0x0094)
#define REQ_INFO_REQUEST_MODE_3        (WRITE_MODE_3 | 0x0094)
#define REQ_INFO_REQUEST_MODE_4        (WRITE_MODE_4 | 0x0094)
#define REQ_INFO_REQUEST_MODE_5        (WRITE_MODE_5 | 0x0094)
#define REQ_INFO_REQUEST_MODE_6        (WRITE_MODE_6 | 0x0094)

#define REQ_STORED_INFO_REQUEST 0x98
#define REQ_SETTING_ALL_GET     0x9C
#define REQ_SET_SPEED           0xA0
#define REQ_PASSWORD_GET        0xA4

#define ACK_INFO_MODE_0                (WRITE_MODE_0 | 0x0064)
#define ACK_INFO_MODE_1                (WRITE_MODE_1 | 0x0064)
#define ACK_INFO_MODE_2                (WRITE_MODE_2 | 0x0064)
#define ACK_INFO_MODE_3                (WRITE_MODE_3 | 0x0064)
#define ACK_INFO_MODE_4                (WRITE_MODE_4 | 0x0064)
#define ACK_INFO_MODE_5                (WRITE_MODE_5 | 0x0064)
#define ACK_INFO_MODE_6                (WRITE_MODE_6 | 0x0064)


#define ACK_STORED_INFO         0x68
#define ACK_ALL_SETTINGS        0x6C
#define ACK_ACK                 0x0B
#define ACK_PASSWORDS           0x70

#define TYPE_CMD 0
#define TYPE_ACK 1

#define GET_CMD(x) ((x & CMD_MASK) >> CMD_SHIFT)

//for command without mode always return 0
#define GET_MODE(x) ((x & MODE_MASK) >> MODE_SHIFT)


class RitexDevice: public Device {
protected:
	RitexDevice();
	ComTrafficProcessor* m_pProcessor;
	int m_writeMode;
public:
	RitexDevice(IAdapter* pAdapter);
	virtual ~RitexDevice();
	DeviceCommand* CreateCommand(void* rawCommand, int length);
	DeviceCommand* CreateCommand(CmdLineCommand* cmd);

	//event/data reporting
	void ReportDataPacket(DataPacket* packet);

	// command implementations
	bool StartMesurements(std::string com, int speed);
};

#endif /* RITEXDEVICE_H_ */
