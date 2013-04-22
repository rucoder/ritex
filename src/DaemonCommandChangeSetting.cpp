/*
 * DaemonCommandChangeSetting.cpp
 *
 *  Created on: Apr 22, 2013
 *      Author: ruinmmal
 */

#include "DaemonCommandChangeSetting.h"
#include <stdio.h>

DaemonCommandChangeSetting::DaemonCommandChangeSetting() {
	// TODO Auto-generated constructor stub

}

DaemonCommandChangeSetting::DaemonCommandChangeSetting(Adapter* adapter, int setting, float value)
	: DaemonCommand(adapter), m_setting(setting), m_value(value)
{
}


DaemonCommandChangeSetting::~DaemonCommandChangeSetting() {
	// TODO Auto-generated destructor stub
}

bool DaemonCommandChangeSetting::Execute()
{
	unsigned char reply[256];
	unsigned char rawData[4] = {0x90, 0, 0, 0};
/*
 * 1 байт    36h 		- адрес ЦУУ;
2 байт    00h	  	- старший байт количества байтов;
3 байт    04h		- младший байт количества байтов;
4 байт    90h		- код команды;
5 байт		- код уставки;
6 байт    		- старший байт значения уставки (смотри Приложение В);
7 байт     		- младший байт значения уставки (смотри Приложение В);
8 байт    		- старший байт контрольной суммы;
9 байт    		- младший байт контрольной суммы.
 */
	DaemonCommChannel* pChannel = m_pAdapter->GetCommChannel();
	if(pChannel == NULL)
		return false;

	rawData[1] = m_setting & 0xFF;

	if(pChannel->send(rawData, sizeof(rawData)) == -1)
		return false;

	pChannel->recv(reply, 256);
	printf("REPLY: %s\n", reply);
}


