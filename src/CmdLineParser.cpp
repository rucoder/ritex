/*
 * CmdLineParser.cpp
 *
 *  Created on: Apr 21, 2013
 *      Author: ruinmmal
 */

#include "CmdLineParser.h"
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <vector>
#include <string.h>

#include <syslog.h>

#define OPT_INDEX_CMD  0
#define OPT_INDEX_SP   1
#define OPT_INDEX_EXIT 2

const char* CmdLineParser::m_ShortOptioonsStr = "-a::pdr::t";

struct option CmdLineParser::m_LongOptions[] = { { "cmd", optional_argument, 0,
		0 }, { "sp", optional_argument, 0, 0 }, { "exit", no_argument, 0, 0 }, {
		0, 0, 0, 0 } };

CmdLineParser::CmdLineParser()
	: m_Argc(0), m_Argv(NULL), m_pCommand(NULL)
{
}

CmdLineParser::CmdLineParser(int argc, char* argv[])
	: m_Argc(argc), m_Argv(argv), m_pCommand(NULL)
{
	m_Argv = new char* [argc];
	for(int i = 0; i < argc; i++) {
		m_Argv[i] = new char[strlen(argv[i]) + 1];
		memcpy(m_Argv[i], argv[i], strlen(argv[i]) + 1);
	}
}

void CmdLineParser::SetCmdLine(char* cmdLine)
{
	std::istringstream iss(cmdLine);
	std::string s;
	std::vector<std::string> args;
	int i = 0;
	syslog(LOG_ERR, "SetCmdLine: %s", cmdLine);
	while ( std::getline( iss, s, ' ' ) ) {
		i++;
		syslog(LOG_ERR,"[%d] %s\n", i,s.c_str());
		syslog(LOG_ERR,"[%d] %s\n",i, s.c_str());
		args.push_back(s);
	}

	m_Argc = args.size();
	m_Argv = new char*[m_Argc];

	for(int i = 0; i < m_Argc; i++) {
		m_Argv[i] = new char[args[i].length() + 1];
		memcpy(m_Argv[i], args[i].c_str(), args[i].length() + 1);
	}
}

std::string CmdLineParser::GetCmdLine()
{
	std::string s("");
	for(int i = 0; i < m_Argc; i++) {
		s+=m_Argv[i];
		if(i != m_Argc -1)
			s+=" ";
	}
	return s;
}


CmdLineParser::~CmdLineParser() {
	if(m_Argv) {
		for (int i = 0; i < m_Argc; i++) {
			delete [] m_Argv[i];
		}
		delete [] m_Argv;
	}
}

bool CmdLineParser::Parse() {

	parser_state_t state = OPTION_FIRST;

	std::string lastArgName;

	bool isError = false;

	m_pCommand = new CmdLineCommand();

	//syslog(LOG_ERR,"\nRaw Command line: ");
	syslog(LOG_ERR,"Raw Command line: ");
	for(int i = 0; i < m_Argc; i++) {
		//printf(LOG_ERR,"%s ", m_Argv[i]);
		syslog(LOG_ERR, "[%s]", m_Argv[i]);
	}
	syslog(LOG_ERR,"\n");

	optind = 1;

	if (m_pCommand == NULL)
		return false;

	while (!isError) {
		int option_index = 0;
		int c;

		c = ::getopt_long_only(m_Argc, m_Argv, m_ShortOptioonsStr,
				m_LongOptions, &option_index);

		syslog(LOG_ERR,"state=%d c=%d optarg=%s\n", state, c, optarg);

		switch (state) {

		//handle options we can start with
		case OPTION_FIRST:
			switch (c) {
			case 0:
				switch(option_index) {
				case 0:
					//may have parameters so change state
					state = OPTION_CMD;
					break;
				case 2:
					state = OPTION_EXIT;
					break;
				default:
					syslog(LOG_ERR,"ERROR: Only -cmd, -p ,-a or -exit can be the first option\n");
					isError = true;
					break;
				}
				break;
			case 'p':
				state = OPTION_P;
				break;
			case 'a':
				state = OPTION_A_FIRST;
				break;
			case 'd':
				state = OPTION_D;
				break;
			case 'r':
				state = OPTION_R;
				break;
			case 't':
				state = OPTION_T;
				break;
			case -1: //EOF
				isError = true;
				break;
			default:
				syslog(LOG_ERR,
						"ERROR: Only -cmd, -p or -a can be the first option and should not have parameters\n");
				isError = true;
				break;
			}
			break;
			//  only -1 is accepted here
		case OPTION_A_FIRST:
			switch (c) {
			case -1:
				m_pCommand->SetCmdLineCmdType(CMD_GET_ADDITIONAL_PARAMETER_LIST);
				return true;
			default:
				syslog(LOG_ERR,
						"ERROR: option -a has no parameter when comes first. other options are not allowed\n");
				isError = true;
				break;

			}
			break;
			//  only -1 is accepted here
		case OPTION_P:
			switch (c) {
			case -1:
				m_pCommand->SetCmdLineCmdType(CMD_SHOW_INFO);
				return true;
			default:
				syslog(LOG_ERR,
						"ERROR: option -p has no parameter. other options are not allowed\n");
				isError = true;
				break;

			}
			break;

			// may have parameters
		case OPTION_CMD:
			switch (c) {
			case -1: //EOF. so -cmd one and only argument
				m_pCommand->SetCmdLineCmdType(CMD_LIST_COMMANDS);
				return true;
			case 1: // we expect cmd device number here
				m_pCommand->SetCmdLineCmdType(CMD_COMMAND);
				m_pCommand->SetDeviceId(optarg);
				state = OPTION_CMD_DEVID;
				break;
			}
			break;

		case OPTION_CMD_DEVID:
			switch (c) {
				case 1:
					m_pCommand->m_cmdTypeRaw = optarg;
					state = OPTION_CMD_TYPE;
					break;
				default:
					syslog(LOG_ERR,"ERROR: [-cmd] device ID expected.\n");
					isError = true;
					break;

			}
			break;
		case OPTION_CMD_TYPE:
			switch (c) {
				case 1:
					m_pCommand->m_messageRaw = optarg;
					state = OPTION_CMD_MSG;
					break;
				default:
					syslog(LOG_ERR,"ERROR: [-cmd] command type expected.\n");
					isError = true;
					break;

			}
			break;

		case OPTION_CMD_MSG:
			switch (c) {
				case 1:
					m_pCommand->m_sourceRaw = optarg;
					state = OPTION_CMD_SRC;
					break;
				default:
					syslog(LOG_ERR,"ERROR: [-cmd] command message expected.\n");
					isError = true;
					break;

			}
			break;

		case OPTION_CMD_SRC:
			switch (c) {
				case 1:
					m_pCommand->m_cmdIdRaw = optarg;
					state = OPTION_CMD_ID;
					break;
				default:
					syslog(LOG_ERR,"ERROR: [-cmd] command source expected.\n");
					isError = true;
					break;

			}
			break;

		case OPTION_CMD_ID:
			switch (c) {
				case 0:
					if(option_index == OPT_INDEX_SP) {
						state = OPTION_SP;
					} else {
						syslog(LOG_ERR,"ERROR: [-cmd] -sp expected.\n");
						isError = true;
					}
					break;
				default:
					syslog(LOG_ERR,"ERROR: [-cmd] -sp expected.\n");
					isError = true;
					break;

			}
			break;


			// -t may only have -a parameter list in format '-a name value -a name value ...'
		case OPTION_T:
			syslog(LOG_ERR,"command -t: \n");
			m_pCommand->SetCmdLineCmdType(CMD_TEST_DEVICE);
			switch (c) {
			case 'a':
				state = OPTION_A_NAME;
				break;
			default:
				syslog(LOG_ERR,"ERROR: option -t requires -a parameter list\n");
				isError = true;
				break;

			}
			break;
			// -d may only have -a parameter list in format '-a name value -a name value ...'
		case OPTION_D:
			syslog(LOG_ERR,"command -d: ");
			m_pCommand->SetCmdLineCmdType(CMD_GET_CONNECTED_DEVICE_INFO);
			switch (c) {
			case 'a':
				state = OPTION_A_NAME;
				break;
			default:
				syslog(LOG_ERR,"ERROR: option -d requires -a parameter list\n");
				isError = true;
				break;

			}
			break;
		case OPTION_A_LIST:
			switch(c) {
				case 'a': //one more parameter
					state = OPTION_A_NAME;
					break;
				case -1:
					return true; //parsing done FIXME: if -a is a must then do not process -1
			}
			break;
			// only non-options and -a are allowed
		case OPTION_A_NAME:
			switch (c) {
			case 1:
				lastArgName = optarg;
				state = OPTION_A_VALUE;
				break;
			default:
				syslog(LOG_ERR,
						"ERROR: wrong -a parameter list: unexpected token: %d(%c) \n",
						c, c);
				isError = true;
				break;

			}
			break;
		case OPTION_A_VALUE:
			switch (c) {
			case 1:
				state = OPTION_A_LIST;
				m_pCommand->AddParameter(lastArgName, optarg);
				break;
			default:
				syslog(LOG_ERR,"ERROR: wrong -a parameter list\n");
				isError = true;
				break;

			}
			break;
		// -r id -sp path -a-list
		case OPTION_R:
			switch (c)
			{
				case 1:
					syslog(LOG_ERR,"[-r] device ID: %s\n", optarg);
					m_pCommand->SetCmdLineCmdType(CMD_GET_MESUREMENTS);
					m_pCommand->SetDeviceId(optarg);
					state = OPTION_R_DEVID;
					break;
				default:
					syslog(LOG_ERR,"ERROR: [-r] device ID expected.\n");
					isError = true;
					break;

			}
			break;
		case OPTION_R_DEVID:
			switch (c) {
				case 0:
					if(option_index == OPT_INDEX_SP) {
						state = OPTION_SP;
					} else {
						syslog(LOG_ERR,"ERROR: [-r] -sp expected\n");
						isError = true;
					}
					break;
				default:
					syslog(LOG_ERR,"ERROR: [-r] -sp expected\n");
					isError = true;
					break;

			}
			break;
		// -sp option
		case OPTION_SP:
			switch (c) {
				case 1:
					m_pCommand->m_scriptPathRaw = optarg;
					state = OPTION_SP_PATH;
					break;
				default:
					syslog(LOG_ERR,"ERROR: option -sp should have parameter\n");
					isError = true;
					break;

			}
			break;
		case OPTION_SP_PATH:
			switch (c) {
				case 'a':
					state = OPTION_A_NAME;
					break;
				//TODO: handle -1 as 'true' if -a list  is not mandatory
				default:
					syslog(LOG_ERR,"ERROR: -a list expected\n");
					isError = true;
					break;

			}
			break;
		case OPTION_EXIT:
			switch (c) {
				case -1:
					m_pCommand->SetCmdLineCmdType(CMD_EXIT);
					return true;
				default:
					syslog(LOG_ERR,"ERROR: option -exit has no parameter\n");
					isError = true;
					break;

			}
			break;
		}
	}
	return !isError;
}


