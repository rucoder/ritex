#include "SendDataCommand.h"

SendDataCommand::SendDataCommand(bool isHw, CmdLineCommand* parent)
: DeviceCommand(isHw, parent)
{

}

SendDataCommand::~SendDataCommand()
{

}

bool SendDataCommand::Execute()
{
	return false;
}

void SendDataCommand::SetReply(DataPacket* packet, int status)
{

}

void SendDataCommand::NotifyResultReady()
{

}
