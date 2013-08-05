/*
 * DataLoggerThread.cpp
 *
 *  Created on: Apr 25, 2013
 *      Author: ruinmmal
 */

#include "DataLoggerThread.h"
#include "Utils.h"
#include <syslog.h>
#include <assert.h>

DataLoggerThread::DataLoggerThread(std::string db_name)
	: LoggerThread<DBDataPacket*>(db_name, "DataLoggerThread")
{
	// int,int,text (date-time),float
	m_sSqlQuery = "INSERT INTO tblparamdata (ChannelId, ParamId, RegisterDate, Value)  VALUES(?,?,?,?)";
}

bool DataLoggerThread::BindParams(DBDataPacket* event, int sock)
{
	//::Log("Inserting data: chId=%d, paramId=%d, td=%lu %s val=%g",event->getChannelId(), event->getParamId(),
	// event->getRegisterDate(), TimeToString(event->getRegisterDate()).c_str(), event->getValue());
	/*return  Bind(1, event->getChannelId()) &&
			Bind( 2, event->getParamId()) &&
			Bind( 3, TimeToString(event->getRegisterDate())) &&
			Bind( 4, event->getValue());*/
 		

    sample_t s;

    s.id = event->getParamId();
    s.channel.bulk = event->getChannelId();
    for(int i=0; i<s.date.size(); i++){
		s.date.bulk[i] = TimeToString(event->getRegisterDate())[i];
	}
    s.value = event->getValue();

    wrp_command_t * cmd = new wrp_command_t();
    cmd->samples.push_back( s );

  //  DLOG( "Command: " << * cmd );
    osstream_t ostr;
    cmd->bin_write( ostr );
 //   DLOG( "Bytes  : " << c_literal( ostr.str() ) );
    ssize_t written = ::write( sock, ostr.str().data(), ostr.str().size() );
    Log("[dbgateway-ritex] sent param %u I:%u C:%u V:%f ", written, s.id,s.channel.bulk,s.value);
//    SYSERR( "write", written == -1 );

    char buffer[ 1000 ];
    ssize_t read = ::read( sock, buffer, sizeof( buffer ) );
  //  SYSERR( "read", read == -1 );
   // DLOG( "Answer : " << c_literal( string_t( buffer, read ) ) );
    Log("[dbgateway-ritex] read buffer size %d buffer:%s.", read, buffer);

    //for(int i=0; i<read; i++)
    //	Log("[dbgateway] read buffer %x", buffer[i]);


 
    return true;
}

 



