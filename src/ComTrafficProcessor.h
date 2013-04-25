/*
 * ComTrafficProcessor.h
 *
 *  Created on: Apr 26, 2013
 *      Author: ruinmmal
 */

#ifndef COMTRAFFICPROCESSOR_H_
#define COMTRAFFICPROCESSOR_H_

#include "Thread.h"

#include <string>

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
//#define TYPE_UNKNOWN -1


class ComTrafficProcessor: public Thread {
protected:
	struct __tag_cmdParams {
		unsigned char m_cmd;
		unsigned short m_length;
		unsigned char m_ack;
	};

#pragma pack(push, 1)

	struct __tag_packet {
		unsigned char address;
		unsigned short length;
		unsigned char cmd;
		int type;
		unsigned char data[1];
	};
#pragma pack(pop)

	enum eState {
		STATE_INIT,
		STATE_WAIT_ACK,
		STATE_PROCESS_CMD,
		STATE_CUSTOM_CMD,
		STATE_WAIT_CUSTOM_ACK
	} m_state;
	virtual void* Run();
    int OpenCommPort(std::string port, int speed);
    int m_fd;
    bool isRunning() { return m_fd != -1; };
    bool isLengthMatches(unsigned char cmd, unsigned short length, int& type);
    struct __tag_packet* ReadPacket(int &error);
    int Read(void* buffer, int length, long time_ms = 0);
    static __tag_cmdParams m_cmdParams[];
    static __tag_cmdParams m_ackParams[];
public:
	ComTrafficProcessor();
	virtual ~ComTrafficProcessor();
    virtual bool Create(std::string port, int speed);
};

#endif /* COMTRAFFICPROCESSOR_H_ */
