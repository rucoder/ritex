/*
 * CmdLineParser.h
 *
 *  Created on: Apr 21, 2013
 *      Author: ruinmmal
 */

#ifndef CMDLINEPARSER_H_
#define CMDLINEPARSER_H_

#include <getopt.h>
#include "CmdLineCommand.h"

class CmdLineParser {
private:
	//Infinite state machine states. not very elegant solution
	typedef enum state {
		OPTION_FIRST,
		OPTION_A_FIRST,
		OPTION_A_LIST,
		OPTION_A_NAME,
		OPTION_A_VALUE,
		OPTION_P,
		OPTION_D,
		OPTION_R,
		OPTION_R_DEVID,
		OPTION_T,
		OPTION_CMD,
		OPTION_CMD_DEVID,
		OPTION_CMD_TYPE,
		OPTION_CMD_MSG,
		OPTION_CMD_SRC,
		OPTION_CMD_ID,
		OPTION_SP,
		OPTION_SP_PATH,
		OPTION_EXIT
	} parser_state_t;
protected:
	// command line parsing
	static struct option m_LongOptions[];
	static const char* m_ShortOptioonsStr;
	int m_Argc;
	char** m_Argv;
	CmdLineCommand* m_pCommand;
	bool isParameterValid(std::string name, char* value);

public:
	CmdLineParser();
	CmdLineParser(int argc, char* argv[]);
	void SetCmdLine(char* cmdLine);
	virtual ~CmdLineParser();
	bool Parse();
	CmdLineCommand* GetCommand() { return m_pCommand; }
	std::string GetCmdLine();
};

#endif /* CMDLINEPARSER_H_ */
