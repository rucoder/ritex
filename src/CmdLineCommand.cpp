/*
 * CmdLineCommand.cpp
 *
 *  Created on: Apr 22, 2013
 *      Author: ruinmmal
 */

#include "CmdLineCommand.h"
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <string>

CmdLineCommand::CmdLineCommand() {
	// TODO Auto-generated constructor stub

}

CmdLineCommand::~CmdLineCommand() {
	// TODO Auto-generated destructor stub
}

void CmdLineCommand::AddParameter(std::string name, std::string value)
{
	m_additionalParametersRaw[name] = value;
}

bool CmdLineCommand::GetCompiledParameter(std::string name, struct compiled_parameter& param)
{
	std::map<std::string, struct compiled_parameter>::iterator it = m_additionalParameters.find(name);

	if (it == m_additionalParameters.end())
		return false;

	param = it->second;
	return true;
}


bool CmdLineCommand::GetParameter(std::string name, std::string& value)
{
	struct compiled_parameter param;
	if(GetCompiledParameter(name, param)) {
		value = param.v.s;
	}
	return false;
}

bool CmdLineCommand::GetParameter(std::string name, int& value)
{
	struct compiled_parameter param;
	if(GetCompiledParameter(name, param)) {
		value = param.v.i;
	}
	return false;
}

bool CmdLineCommand::GetParameter(std::string name, float& value)
{
	struct compiled_parameter param;
	if(GetCompiledParameter(name, param)) {
		value = param.v.f;
	}
	return false;
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

    for(std::map<std::string, std::string>::iterator  itr = m_additionalParametersRaw.begin(); itr != m_additionalParametersRaw.end(); itr++)
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

bool CmdLineCommand::CompileFloat(std::string src, float& dst)
{
	dst = strtod(src.c_str(), 0);
	if (dst == 0 && src[0] != '0') //FIXME: not valid for float
		return false;
	return true;
}

bool CmdLineCommand::CompileMessage(std::string src, int& dst_name, float& dst_val)
{
	//TODO: implement
	return true;
}


bool CmdLineCommand::IsParameterExists(std::string name) {
	return (m_additionalParameters.find(name) != m_additionalParameters.end());
}

bool CmdLineCommand::HasRequiredParameters(eCommandType type) {
	//TODO: implement it
	switch(type) {
	case CMD_GET_MESUREMENTS:
	case CMD_TEST_DEVICE:
		return IsParameterExists("comdevice") && IsParameterExists("baudrate");
	case CMD_COMMAND:
		//TODO: define it
		return true;
	default:
		return true;
	}
	return false;
}

bool CmdLineCommand::Compile(const additional_parameter_map_t& map)
{
	//compile -a parameters first. any those we got in the command line
    for(std::map<std::string, std::string>::iterator itr = m_additionalParametersRaw.begin(); itr != m_additionalParametersRaw.end(); itr++) {
    	struct compiled_parameter param;
    	//check if the parameter is known
    	additional_parameter_map_t::const_iterator itr1;
    	itr1 = map.find(itr->first);
    	if(itr1 == map.end()) {
    		printf("Unknown -a parameter: %s\n", itr->first.c_str());
    		return false;
    	}

    	//parameter type description
    	struct additional_parameter_t* aparam = itr1->second;

    	switch(aparam->type) {
			case PARAM_TYPE_INT:
				param.type = PARAM_TYPE_INT;
				//try to convert
				if(!CompileInt(itr->second, param.v.i)) {
					printf("Unknown -a parameter: %s argument must be INT found [%s]\n", itr->first.c_str(), itr->second.c_str());
					return false;
				}
				break;
			case PARAM_TYPE_FLOAT:
				param.type = PARAM_TYPE_FLOAT;
				//try to convert
				if(!CompileFloat(itr->second, param.v.f)) {
					printf("Unknown -a parameter: %s argument must be FLOAT found [%s]\n", itr->first.c_str(), itr->second.c_str());
					return false;
				}
				break;
			case PARAM_TYPE_STRING:
				//just copy
				param.type = PARAM_TYPE_STRING;
				param.v.s = new char[itr->second.length() + 1];
				strcpy(param.v.s, itr->second.c_str());
				break;
			case PARAM_TYPE_LIST:
				// need to find a value in the list first and then it will be compiled
				int i;
				for(i = 0; i < aparam->value.list.size; i++) {
					if(strcmp(aparam->value.list.data[i], itr->second.c_str() ) == 0) {
						//found
						if(aparam->value.list.dataType == LIST_VALUE_INT) {
							if(!CompileInt(itr->second, param.v.i)) {
								printf("Unknown -a parameter: %s argument must be INT found [%s]\n", itr->first.c_str(), itr->second.c_str());
								return false;
							}
							param.type = PARAM_TYPE_INT;
							break;
						} else { //this is string
							//just copy
							param.type = PARAM_TYPE_STRING;
							param.v.s = new char[itr->second.length() + 1];
							strcpy(param.v.s, itr->second.c_str());
							break;
						}
					}
				}
				//did not find
				if(i == aparam->value.list.size) {
					printf("Unknown -a parameter: %s argument must be from valid list. found [%s]\n", itr->first.c_str(), itr->second.c_str());
					return false;
				}
				break;
    	}

    	//add to the map
    	m_additionalParameters[itr->first] = param;

    }


	switch (m_cmdLineCommandType) {
		case CMD_GET_ADDITIONAL_PARAMETER_LIST:
		case CMD_LIST_COMMANDS:
		case CMD_GET_CONNECTED_DEVICE_INFO: //FIXME: do we need -a for -d?
		case CMD_SHOW_INFO:
			return true; //no params here

		case CMD_GET_MESUREMENTS:
			return CompileInt(m_deviceIdRaw, m_deviceId) && HasRequiredParameters(m_cmdLineCommandType);
		case CMD_COMMAND:
			return CompileInt(m_deviceIdRaw, m_deviceId) && CompileInt(m_cmdIdRaw, m_cmdId) &&
					CompileMessage(m_messageRaw, m_msgName, m_msgVal) && CompileInt(m_cmdTypeRaw, m_cmdType)
					&& HasRequiredParameters(m_cmdLineCommandType);
		case CMD_TEST_DEVICE:
		{
			//need to create fake device Id
			char s[32];
			m_deviceId = rand();
			snprintf(s,32,"%d", m_deviceId);
			m_deviceIdRaw = s;
			syslog(LOG_ERR, "Generate fake Device ID: m_deviceId = %d", m_deviceId);
			return HasRequiredParameters(m_cmdLineCommandType);
		}
		default:
			break;
	}
	return false;
}

