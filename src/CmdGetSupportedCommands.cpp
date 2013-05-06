/*
 * CmdGetSupportedCommands.cpp
 *
 *  Created on: May 5, 2013
 *      Author: ruinmmal
 */

#include "CmdGetSupportedCommands.h"

#include <iostream>

CmdGetSupportedCommands::CmdGetSupportedCommands(Device* device)
	: DeviceCommand(false), m_pDevice(device)
{
	// TODO Auto-generated constructor stub

}

CmdGetSupportedCommands::~CmdGetSupportedCommands() {
	// TODO Auto-generated destructor stub
}

bool CmdGetSupportedCommands::Execute()
{
	std::vector<AdapterCommand*>& list = m_pDevice->GetExternaCommandList();

	for(int i = 0; i < list.size(); i++) {
		AdapterCommand* pCmd = list[i];

		//printf("%d|%s|%s\n", i, pCmd->m_name.c_str(), );
		std::cout << pCmd->m_cmdId << "|" << pCmd->m_name << "|" << pCmd->m_description << std::endl;

		std::vector<AdapterCommandArg*>& args = pCmd->GetAgrList();

		for(int j = 0; j < args.size(); j++) {
			std::cout << "~" << (j + 1) << "|" << args[j]->m_name;
			switch(args[j]->m_type) {
			case ARG_TYPE_INT:
				std::cout<<"|int"; //TODO: min-max val
				break;
			case ARG_TYPE_FLOAT:
				std::cout<<"|float"; //TODO: min-max val
				break;
			case ARG_TYPE_LIST:
				std::cout<<"|list";
				for(int k = 0; k < args[j]->m_list_val.size(); k++) {
					std::cout<<"|" << (k+1) << "~" << args[j]->m_list_val[k]->m_name;
				}
				break;
			default:
				break;
			}
			std::cout << std::endl;
		}


	}


	return false;
}
