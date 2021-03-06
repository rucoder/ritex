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


typedef struct _tag_custom_command_t{

	DataPacket* m_pDataPacket;
	DeviceCommand& m_pParentCommand;
	_tag_custom_command_t(DeviceCommand& parent)
		: m_pParentCommand(parent)
	{
		m_pDataPacket = NULL;
	}
	~_tag_custom_command_t() {
		if(m_pDataPacket) {
			delete m_pDataPacket;
			m_pDataPacket = NULL;
		}
	}
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
		STATE_PROCESS_KSU_TRAFFIC,
		STATE_CAPTURE_DEVICE,
	} m_state, m_nextState;

	int m_fd;
    static __tag_cmdParams m_cmdParams[];
    static __tag_cmdParams m_ackParams[];

    RitexDevice* m_pDevice;

    std::queue<unsigned char> m_echoCancelFifo;

    int m_writeMode;

    bool m_isDataCapture;

    std::queue<custom_command_t*> m_pendingCmdQueue;

    pthread_mutex_t m_cmdMutex;


    bool m_doRun;
    pthread_mutex_t m_startMutex;
    pthread_cond_t m_startCond;

    //fault and event reporting
    bool m_isInFault;
    int m_faultCode;

    int m_number_of_ksu_failures;

    int m_vd_state;

    int m_dataCapturePeriod;

    time_t m_nextModeLoopCycle;
    bool m_isAutoMode;

    unsigned char m_motorStatus;

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
    bool HandleError(int error);


    //TODO: place under debug
    std::string GetErrorStr(int error);
    std::string GetStateStr(eState state);

    bool ChangeMode(unsigned short mode, int& error);
    void SetSetting(unsigned char setting, unsigned short value);
    void GetAllSettings();
    bool GetPasswords(int& error, unsigned char& passHi, unsigned char& passLow);
    bool SetPassword(unsigned short password, int& error, DataPacket** settings);
    void SetDataCaptureMode(bool isCapture = true) {
    	m_isDataCapture = isCapture;
    }

    bool NeedCapture();
    bool CaptureDevice(int& error);
    void RequestCurrentData();
    void ProcessMotorCmd(int cmd, int param, int& error);

    bool ProcessCommand(custom_command_t* pCmd, int& error);

public:
	ComTrafficProcessor(RitexDevice* pDevice);
	virtual ~ComTrafficProcessor();
    virtual bool Create(std::string port, int speed, bool oneShot = false);
    bool SendCustomCmd(custom_command_t* cmd);
    bool isRunning() { return m_fd != -1; };
    void Start() {
    	Log("DataReaderThread: Start OK1\n");
    	pthread_mutex_lock(&m_startMutex);
    	Log("DataReaderThread: Start OK2\n");
    	m_doRun = true;
    	Log("DataReaderThread: Start OK3\n");
    	pthread_mutex_unlock(&m_startMutex);
    	pthread_cond_signal(&m_startCond);
    	Log("DataReaderThread: Start OK4\n");
    }
};

#endif /* COMTRAFFICPROCESSOR_H_ */
