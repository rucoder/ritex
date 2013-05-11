/*
 * AdapterCommand.cpp
 *
 *  Created on: Apr 24, 2013
 *      Author: ruinmmal
 */

#include "AdapterCommand.h"

AdapterCommand::AdapterCommand(int id, std::string name, std::string description)
	: m_cmdId(id), m_name(name), m_description(description)
{
	// TODO Auto-generated constructor stub

}

AdapterCommand::~AdapterCommand() {
	//delete args
	for(unsigned int i = 0; i < m_args.size(); i++) {
		delete m_args[i];
	}
}

