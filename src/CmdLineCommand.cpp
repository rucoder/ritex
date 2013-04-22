/*
 * CmdLineCommand.cpp
 *
 *  Created on: Apr 22, 2013
 *      Author: ruinmmal
 */

#include "CmdLineCommand.h"
#include <stdio.h>

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
	printf("m_message=%s\n", m_message.c_str());
	printf("m_cmdId=%s\n", m_cmdId.c_str());
	printf("m_cmdType=%s\n", m_cmdType.c_str());
	printf("m_deviceId=%s\n", m_deviceId.c_str());
	printf("m_scriptPath=%s\n", m_scriptPath.c_str());
	printf("m_source=%s\n", m_source.c_str());

    for(std::map<std::string, std::string>::iterator  itr = m_additionalParameters.begin(); itr != m_additionalParameters.end(); itr++)
    {
    	printf("[%s, %s]\n", itr->first.c_str(), itr->second.c_str());
    }

}


