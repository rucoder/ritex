#ifndef SENDDATACOMMAND_H_
#define SENDDATACOMMAND_H_

#include "DeviceCommand.h"

class SendDataCommand : public DeviceCommand
{
public:
	SendDataCommand(bool isHw, CmdLineCommand* parent = NULL);
	virtual ~SendDataCommand();
	virtual bool Execute();
	virtual void SetReply(DataPacket* packet, int status);

protected:
	virtual void NotifyResultReady();
};

#endif /* SENDDATACOMMAND_H_ */
