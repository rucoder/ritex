/*
 * CmdLineCommand.h
 *
 *  Created on: Apr 22, 2013
 *      Author: ruinmmal
 */

#ifndef CMDLINECOMMAND_H_
#define CMDLINECOMMAND_H_

#include <map>
#include <string>

enum eCommandType {
	CMD_EXIT,
	CMD_SHOW_INFO, // -p option
	CMD_LIST_COMMANDS, // -cmd, no args
	CMD_COMMAND, // -cmd with args
	CMD_GET_MESUREMENTS, // -r
	CMD_GET_CONNECTED_DEVICE_INFO, // -d
	CMD_GET_ADDITIONAL_PARAMETER_LIST, // -a as first option
	CMD_TEST_DEVICE
};

class CmdLineCommand {
public:

protected:
	std::map<std::string, std::string> m_additionalParameters; // -a list
public: //TODO: make protected and create setter/getter
	eCommandType m_cmdLineCommandType;
	std::string m_scriptPathRaw; // for -sp
	std::string m_deviceIdRaw; // for -cmd
	std::string m_cmdTypeRaw; //for -cmd
	std::string m_messageRaw; // for -cmd
	std::string m_sourceRaw; // for -cmd
	std::string m_cmdIdRaw; // for -cmd

	//compiled values
	int m_deviceId;
	int m_cmdId;
	int m_msgName;
	float m_msgVal;
private:
	bool CompileInt(std::string src, int& dst);
	bool CompileMessage(std::string src, int& dst_name, float& dst_val);
public:
	CmdLineCommand();
	virtual ~CmdLineCommand();
	void SetCmdLineCmdType(eCommandType cmd) { m_cmdLineCommandType = cmd; };
	void SetDeviceId(std::string devId) { m_deviceIdRaw = devId; }
	void AddParameter(std::string name, std::string value);
	bool GetParameter(std::string name, std::string& value);
	void Dump();
	bool Compile();

};

#endif /* CMDLINECOMMAND_H_ */
