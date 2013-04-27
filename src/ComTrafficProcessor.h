/*
 * ComTrafficProcessor.h
 *
 *  Created on: Apr 26, 2013
 *      Author: ruinmmal
 */

#ifndef COMTRAFFICPROCESSOR_H_
#define COMTRAFFICPROCESSOR_H_

#include "Thread.h"
#include "DataPacket.h"

#include <string>

//for timeval
#include <sys/time.h>

class RitexDevice;

class ComTrafficProcessor: public Thread {
protected:
	struct __tag_cmdParams {
		unsigned char m_cmd;
		unsigned short m_length;
		unsigned char m_ack;
	};

protected:
	enum eState {
		STATE_INIT,
		STATE_WAIT_ACK,
		STATE_WAIT_CMD,
		STATE_CUSTOM_CMD,
		STATE_WAIT_CUSTOM_ACK
	} m_state;
	int m_fd;
    static __tag_cmdParams m_cmdParams[];
    static __tag_cmdParams m_ackParams[];

    RitexDevice* m_pDevice;


protected:
	virtual void* Run();
    int OpenCommPort(std::string port, int speed);
    bool isRunning() { return m_fd != -1; };


    bool isLengthMatches(unsigned char cmd, unsigned short length, int& type);
    int GetAckForCmd(int cmd, bool primary = true);

    DataPacket* ReadPacket(struct timeval* timeout, int &error);
    int Read(void* buffer, int length, struct timeval* timeout = NULL);

    DataPacket* WaitForKsuActivity(int timeout, int& error);

    //TODO: place under debug
    std::string GetErrorStr(int error);

    //CRC16
    unsigned short Crc16(unsigned short crcInit, unsigned char byte);
    unsigned short Crc16(unsigned short crcInit, unsigned char buffer[], int size);


public:
	ComTrafficProcessor(RitexDevice* pDevice);
	virtual ~ComTrafficProcessor();
    virtual bool Create(std::string port, int speed);
};

#endif /* COMTRAFFICPROCESSOR_H_ */
