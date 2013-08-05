/*
 * EventLoggerThread.cpp
 *
 *  Created on: Apr 24, 2013
 *      Author: ruinmmal
 */

#include "EventLoggerThread.h"
#include "Utils.h"
#include <syslog.h>

#include <assert.h>
#include "sqlite3.h"
#include "Log.h"


EventLoggerThread::EventLoggerThread(std::string db_name)
	: LoggerThread<DBEventCommon*>(db_name, "EventLoggerThread")
{
	m_sSqlQuery = "insert  into tbleventbus (TypeId, ChannelId, RegisterDate, Argument1, Argument2, Argument3, Argument4) values(?,?,?,?,?,?,?)";
}

bool EventLoggerThread::BindParams(DBEventCommon* event, int sock)
{
	::Log("Inserting event: %d", event->getTypeId());
/*	return Bind(1, event->getTypeId()) && Bind(2, event->getChannelId()) &&
	Bind(3, TimeToString(event->getRegisterTimeDate())) &&
	Bind(4, event->getArgument1())&&
	Bind(5, event->getArgument2())&&
	Bind(6, event->getArgument3())&&
	Bind(7, event->getArgument4());*/


    event_t e;

    e.id = event->getTypeId();
    e.channel.bulk = event->getChannelId();
    for(int i=0; i<e.date.size(); i++){
		e.date.bulk[i] = TimeToString(event->getRegisterTimeDate())[i];
	}
    e.arg1 = event->getArgument1();
    e.arg2 = event->getArgument2();
    e.arg3 = event->getArgument3();
    e.arg4 = event->getArgument4();


    wre_command_t * cmd = new wre_command_t();
    cmd->events.push_back( e );

   // DLOG( "Command: " << * cmd );
    osstream_t ostr;
    cmd->bin_write( ostr );
  //  DLOG( "Bytes  : " << c_literal( ostr.str() ) );
    ssize_t written = ::write( sock, ostr.str().data(), ostr.str().size() );
  //  SYSERR( "write", written == -1 );
    Log("[dbgateway-ritex] sent event %u", written);
    char buffer[ 1000 ];
    ssize_t read = ::read( sock, buffer, sizeof( buffer ) );
//    SYSERR( "read", read == -1 );
 //   DLOG( "Answer : " << c_literal( string_t( buffer, read ) ) );

	return true;
}
