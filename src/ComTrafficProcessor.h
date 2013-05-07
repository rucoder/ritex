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


typedef struct {

	DataPacket* m_pDataPacket;
	DeviceCommand* m_pParentCommand;
} custom_command_t;

class RitexDevice;

class ComTrafficProcessor: public Thread {
protected:
	struct __tag_cmdParams {
		unsigned short m_cmd;
		unsigned short m_length;
		int m_ack;
	};

protected:
	enum eState {
		STATE_INIT,
		STATE_GET_INIT_WRITE_MODE,
		STATE_GET_INIT_PASSWORDS,
		STATE_SET_CURRENT_PASSWORD,
		STATE_SET_MODE,
		STATE_WAIT_MODE_SET,
		STATE_WAIT_ACK,
		STATE_WAIT_CMD,

		STATE_CUSTOM_CMD,
		STATE_WAIT_CUSTOM_ACK
	} m_state;
	int m_fd;
    static __tag_cmdParams m_cmdParams[];
    static __tag_cmdParams m_ackParams[];

    RitexDevice* m_pDevice;

    std::queue<unsigned char> m_echoCancelFifo;

    int m_writeMode;

    bool m_isDataCapture;

    custom_command_t* m_pendingCmd;

    pthread_mutex_t m_cmdMutex;


    bool m_doRun;
    pthread_mutex_t m_startMutex;
    pthread_cond_t m_startCond;
protected:
	virtual void* Run();
    int OpenCommPort(std::string port, int speed);


    bool isLengthMatches(unsigned char cmd, unsigned short length, int& type);
    int GetAckForCmd(int cmd, bool primary = true);

    DataPacket* ReadPacket(struct timeval* timeout, int &error);
    int Read(void* buffer, int length, struct timeval* timeout = NULL);

    bool WritePacket(DataPacket* pPacket);


    DataPacket* WaitForKsuActivity(int timeout, int& error);
    bool CheckAndReportFault(DataPacket* packet);

    //TODO: place under debug
    std::string GetErrorStr(int error);
    std::string GetStateStr(eState state);

    void ChangeMode(unsigned short mode);
    void SetSetting(unsigned char setting, unsigned short value);
    void GetAllSettings();
    void GetPasswords();
    void SetPassword(unsigned short password);
    void SetDataCaptureMode(bool isCapture = true) {
    	m_isDataCapture = isCapture;
    }


public:
	ComTrafficProcessor(RitexDevice* pDevice);
	virtual ~ComTrafficProcessor();
    virtual bool Create(std::string port, int speed, bool oneShot = false);
    bool SendCustomCmd(custom_command_t* cmd);
    bool isRunning() { return m_fd != -1; };
    void Start() {
    	pthread_mutex_lock(&m_startMutex);
    	m_doRun = true;
    	pthread_mutex_unlock(&m_startMutex);
    	pthread_cond_signal(&m_startCond);
    }
};

#endif /* COMTRAFFICPROCESSOR_H_ */
