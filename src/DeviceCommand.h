/*
 * DeviceCommand.h
 *
 *  Created on: Apr 22, 2013
 *      Author: ruinmmal
 */

#ifndef DEVICECOMMAND_H_
#define DEVICECOMMAND_H_

class DeviceCommand {
private:
	bool m_isHWCommand;
public:
	struct __serial_data {
		int m_cmd;
	};

protected:
	DeviceCommand();
	unsigned char* m_rawResult;
	int m_rawResultLength;
	unsigned char* m_rawCommand;
	int m_rawCommandLength;
	virtual void Serialize(){};
	int m_cmdId;
public:
	DeviceCommand(bool isHw);
	virtual ~DeviceCommand();
	bool isHWCommand() { return m_isHWCommand; };
	unsigned char* getRawResult() { return m_rawResult; };
	int getRawResultLength() { return m_rawResultLength; };
	unsigned char* getRawCommand();
	int getRawCommandLength();
	virtual bool Execute() = 0;
};

#endif /* DEVICECOMMAND_H_ */
