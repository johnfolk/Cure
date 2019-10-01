//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2005 John Folkesson
//    
#include "CommandExecuter.hh"
#include "CureDebug.hh"

using namespace Cure;

CommandExecuter::CommandExecuter()
{
  Service=0;
  OutputService=0;
  CmdProtocol=0;
  OutCmdProtocol=0;
  CmdAdd.init(this);
}

CommandExecuter::~CommandExecuter()
{
}

unsigned short CommandExecuter::writeCommand(Command &cmd)
{
  return CmdAdd.write(cmd);
}

unsigned short CommandExecuter::comReply(){
  if (processReply())CmdAdd.add(InputCommand);
  return 0;
}

unsigned short CommandExecuter::comStatus()
{
  setStatus();
  Reply=Status;
  CmdAdd.sendReply(InputCommand);
  CmdAdd.writeTo(InputCommand.ReplyAddress, Reply);
  return 0;
}

void CommandExecuter::setStatus()
{
  CurrentTime.setToCurrentTime();
  Status.Time=CurrentTime;
  Status.Sender=Service;
  Status.Priority=0;
  if ((Status.CmdFlags.Rows<1)||(Status.CmdFlags.Columns<3))
    Status.CmdFlags.reallocate(1,4);
  Status.CmdFlags(0,0)=CmdProtocol;
  Status.CmdFlags(0,1)=CurrentCommand.Priority;
  Status.CmdFlags(0,2)=CurrentCommand.CmdType;
  Status.CmdFlags(0,3)=CurrentCommand.CmdID;
}

unsigned short CommandExecuter::handleService(unsigned short s)
{
  // The default behavior is to not accept a command if the service
  // does not match. However, some components might handle multiple
  // services.
  if (s != Service) 
    return TYPE_ERROR;
  else
    return 0;
}

unsigned short CommandExecuter::processCommand()
{
  /**
   * We always answer these queries so that other objects can find out
   * our Service CmdProtocol...
   */ 
  if (InputCommand.CmdType==Command::STATUS){
    comStatus();
    return 0;
  }

  int ret = handleService(InputCommand.Service);
  if (ret) return ret;

  if (InputCommand.CmdType==Command::REPLY)
    return comReply();

  if (InputCommand.Priority<CurrentCommand.Priority)
    return (RESOURCE_ERROR|OBJECT_BUSY);    

  if (InputCommand.Priority>CurrentCommand.Priority)
    notifyUser();

  if (InputCommand.Priority==CurrentCommand.Priority){
    if (InputCommand.Sender!=CurrentCommand.Sender)
      return (RESOURCE_ERROR|OBJECT_BUSY);  
  }

  if (InputCommand.CmdType==Command::STOP)
    return comStop();
  if (InputCommand.CmdType==Command::SPEED)
    return comSpeed();
  if (InputCommand.CmdType==Command::GOTO)
    return comGoto();
  if (InputCommand.CmdType==Command::LOAD)
    return comLoad();
  if (InputCommand.CmdType==Command::SAVE)
    return comSave();
  if (InputCommand.CmdType==Command::START)
    return comStart();
  if (InputCommand.CmdType==Command::RESET)
    return comReset();
  if (InputCommand.CmdType==Command::GET)
    return comGet();
  if (InputCommand.CmdType==Command::SET)
    return comSet();
  if (InputCommand.CmdType==Command::CONFIGURE)
    return comConfigure();
  if (InputCommand.CmdType==Command::INIT)
    return comInit();
  if (InputCommand.CmdType==Command::RELEASE)
    return comRelease();
  return comOther();
}
