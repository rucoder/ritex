/*
 * DeviceStatusListener.h
 *
 *  Created on: May 4, 2013
 *      Author: ruinmmal
 */

#ifndef DEVICESTATUSLISTENER_H_
#define DEVICESTATUSLISTENER_H_

typedef enum {
	DEVICE_STATUS_EXIT,
	DEVICE_STATUS_OK,
	DEVICE_STATUS_FAILURE
} eDeviceStaus;

class IDeviceStatusListener {
public:
	virtual void OnDeviceStatusChanged(eDeviceStaus status) = 0;
protected:
	virtual ~IDeviceStatusListener() {}
};

#endif /* DEVICESTATUSLISTENER_H_ */
