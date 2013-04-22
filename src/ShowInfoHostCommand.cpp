/*
 * ShowInfoHostCommand.cpp
 *
 *  Created on: Apr 22, 2013
 *      Author: ruinmmal
 */

#include "ShowInfoHostCommand.h"
#include <stdio.h>

ShowInfoHostCommand::ShowInfoHostCommand() {
	// TODO Auto-generated constructor stub

}

ShowInfoHostCommand::ShowInfoHostCommand(Adapter* adapter)
	: HostCommand(adapter)
{

}


ShowInfoHostCommand::~ShowInfoHostCommand() {
	// TODO Auto-generated destructor stub
}

bool ShowInfoHostCommand::Execute()
{
	std::list<AdapterParameter*> params = m_pAdapter->GetParameterList();


	printf("\"%s\"|\"%s\"|\"%s\"\n", m_pAdapter->GetName().c_str(), m_pAdapter->GetVersion().c_str(),m_pAdapter->GetDescription().c_str());

	// print all parameters
    for(std::list<AdapterParameter*>::iterator itr = params.begin(); itr != params.end(); itr++)
    {
        std::string s = (*itr)->FormatString();
        printf("%s\n", s.c_str());
    }
    return true;
}


