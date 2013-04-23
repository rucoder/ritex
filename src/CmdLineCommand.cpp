/*
 * CmdLineCommand.cpp
 *
 *  Created on: Apr 22, 2013
 *      Author: ruinmmal
 */

#include "CmdLineCommand.h"
#include <stdio.h>
#include <stdlib.h>

CmdLineCommand::CmdLineCommand() {
	// TODO Auto-generated constructor stub

}

CmdLineCommand::~CmdLineCommand() {
	// TODO Auto-generated destructor stub
}

void CmdLineCommand::AddParameter(std::string name, std::string value)
{
	m_additionalParameters[name] = value;
}

bool CmdLineCommand::GetParameter(std::string name, std::string& value)
{
	std::map<std::string, std::string>::iterator it = m_additionalParameters.find(name);

	if (it == m_additionalParameters.end())
		return false;

	value = it->second;

	return true;
}

void CmdLineCommand::Dump()
{
	printf("m_cmdLineCommandType=%d\n", m_cmdLineCommandType);
	printf("m_message=%s\n", m_messageRaw.c_str());
	printf("m_cmdId=%s\n", m_cmdIdRaw.c_str());
	printf("m_cmdType=%s\n", m_cmdTypeRaw.c_str());
	printf("m_deviceId=%s\n", m_deviceIdRaw.c_str());
	printf("m_scriptPath=%s\n", m_scriptPathRaw.c_str());
	printf("m_sourceRaw=%s\n", m_sourceRaw.c_str());

    for(std::map<std::string, std::string>::iterator  itr = m_additionalParameters.begin(); itr != m_additionalParameters.end(); itr++)
    {
    	printf("[%s, %s]\n", itr->first.c_str(), itr->second.c_str());
    }

}

bool CmdLineCommand::CompileInt(std::string src, int& dst)
{
	char* pEnd;
	dst = strtol(src.c_str(), &pEnd, 10);
	if (dst == 0 && src[0] != '0')
		return false;
	return true;
}

bool CmdLineCommand::CompileMessage(std::string src, int& dst_name, float& dst_val)
{
	//TODO: implement
	return true;
}


bool CmdLineCommand::Compile()
{
	switch (m_cmdLineCommandType) {
		case CMD_GET_ADDITIONAL_PARAMETER_LIST:
		case CMD_LIST_COMMANDS:
		case CMD_GET_CONNECTED_DEVICE_INFO: //FIXME: do we need -a for -d?
		case CMD_SHOW_INFO:
			return true; //no params here

		case CMD_GET_MESUREMENTS:
			return CompileInt(m_deviceIdRaw, m_deviceId);
		case CMD_COMMAND:
			return CompileInt(m_deviceIdRaw, m_deviceId) && CompileInt(m_cmdIdRaw, m_cmdId) &&
					CompileMessage(m_messageRaw, m_msgName, m_msgVal);
		default:
			break;
	}
	return false;
}

