/*
 * AdapterCommand.h
 *
 *  Created on: Apr 21, 2013
 *      Author: ruinmmal
 */

#ifndef ADAPTERCOMMAND_H_
#define ADAPTERCOMMAND_H_

class AdapterCommand {
public:
	enum eCommandType {
		CMD_EXIT,
		CMD_SHOW_INFO // -p option
	};
	AdapterCommand();
	void SetCmdType(eCommandType cmd) { m_cmdType = cmd; };
	virtual ~AdapterCommand();
protected:
	eCommandType m_cmdType;
};

#endif /* ADAPTERCOMMAND_H_ */
