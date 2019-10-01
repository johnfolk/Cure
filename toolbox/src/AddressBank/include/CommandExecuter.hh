//
// = AUTHOR(S)
//    John Folkesson, Patric Jensfelt
//    
//    Copyright (c) 2005 John Folkesson
//    

#ifndef CURE_COMMANDEXECUTER_HH
#define CURE_COMMANDEXECUTER_HH

#include "AddressBank.hh"
#include "CommandAddress.hh"
#include "TimestampedData.hh"

namespace Cure{
  class CommandAddress;


/**
 * CommandExecuter is a processor of CommandData. 
 * One can implement just those commands that one is listening to.
 * 
 *
 * @author John Folkesson, Patric Jensfelt
 */
class CommandExecuter
{
public:
  /**This is what we are executing now*/
  Command CurrentCommand;

  /**This holds the last written/read command*/
  Command InputCommand;

  /**This is our Reply to input commands that ask for it*/
  Command Reply;

  /**This is what we answer for generic status calls now*/
  Command Status;

  /**This is what we push out*/
  Command OutputCommand;

  Cure::Timestamp CurrentTime;
  unsigned short OutputService;
  unsigned short CmdProtocol;
  unsigned short OutCmdProtocol;
  unsigned short Service;

  CommandAddress CmdAdd;
protected:

public:
  /** 
   * Constructor 
   *
   *   
   *
   *
   *
   *
   */
  CommandExecuter();
  
  /** 
   * Destructor that frees the resources that was alloctaed in the
   * constructor.
   *
   */
  virtual ~CommandExecuter();
 
  /**
   * This is called when a reply arrives at the reply address
   * By overridding this you can deal with replies ass they come in.
   * This return 0 version simply throws away any reply.
   *
   * Just to explain what you might find in a subclass that
   * returns 1.  It assumes that the Command Executer will
   * call CmdAdd.getReply(cmd) after sending a command with ReplyAddress 
   * set to this->Index. to remove the reply and process it.
   *
   * So you can chose to have the CommandAddress handel all replies 
   * by calling functions on Filter if needed and then return 0.
   *
   * This uses the InputData object at the reply to process.
   *
   * Or you can return 1 here and have all Replies pile up on a list
   * and the Filter check for specific Replies that it is waiting for
   * with CmdAdd.getReply(cmd). 
   *
   * @return 0 if you don't want the ReplyAddress to save c
   */
  virtual unsigned short processReply(){return 0;}

  /**
   * This method is called when new commands are written to this address.
   * This can be overridden to do filter specific parsing of the command
   * or just the more specific methods stop,speed, ect can be overrided as 
   * for just what the filter needs to implement.
   * 
   * The InputCommand object is what is processed 
   * Status commands will allways return status() 
   *
   * @return TYPE_ERROR if not a Command or not right Service
   *         (RESOURCE_ERROR|OBJECT_BUSY) if priority is too low compared to 
   *         CurrentCommand.Priority
   */
  virtual unsigned short processCommand();

  /**
   * Use this function to write a command to the CommandExecuter. It
   * will return whatever CmdAdd.write returns.
   *
   * @param cmd Command to write to this object
   *
   * @return 0 if OK, else error code
   */
  virtual unsigned short writeCommand(Command &cmd);

protected:
  /**
   * This is called to update the status. 
   */
  virtual void setStatus();

  /**
   * This function is caled from within processCommand to check if
   * this object handles the service specified in a command. The
   * default behavior is to not accept a command if the service does
   * not match the service specifed in the CommandExecuter
   * interface. However, some components might handle multiple
   * services, such as the interface for the robot hardware. In this
   * case you do not have to overload processCommand but only this
   * function.
   *
   * @param s service code such as 
   */
  virtual unsigned short handleService(unsigned short s);

  /**
   * This is the special meethod called when
   * c.CmdType==Command::STATUS.  It is called even if the Priority
   * Service or Sender are different than the CurrentCommand.  It can
   * be used topass information needed to set up communication,
   * CmdProtocol, Service ect.
   */
  virtual unsigned short comStatus();

  virtual unsigned short comStop()
  {return (RESOURCE_ERROR|TYPE_ERROR);}

  virtual unsigned short comSpeed()
  {return (RESOURCE_ERROR|TYPE_ERROR);}

  virtual unsigned short comGoto()
  {return (RESOURCE_ERROR|TYPE_ERROR);}

  virtual unsigned short comLoad()
  {return (RESOURCE_ERROR|TYPE_ERROR);}

  virtual unsigned short comSave()
  {return (RESOURCE_ERROR|TYPE_ERROR);}

  virtual unsigned short comStart()
  {return (RESOURCE_ERROR|TYPE_ERROR);}

  virtual unsigned short comReset()
  {return (RESOURCE_ERROR|TYPE_ERROR);}

  virtual unsigned short comSet()
  {return (RESOURCE_ERROR|TYPE_ERROR);}

  virtual unsigned short comGet()
  {return (RESOURCE_ERROR|TYPE_ERROR);}

  virtual unsigned short comReply();

  virtual unsigned short comConfigure()
  {return (RESOURCE_ERROR|TYPE_ERROR);}

  virtual unsigned short comInit()
  {return (RESOURCE_ERROR|TYPE_ERROR);}

  virtual unsigned short comOther()
  {return (RESOURCE_ERROR|TYPE_ERROR);}

  /**
   * This will be called if the Priority,Service and Sender of c is
   * equal to the CurrentCommand.Priority ect.  It
   * then sets the Current Command to c and the Priority to 0.  After
   * that any new command with Priority>0 will be executed.
   * A subclass might want to add a call to stop() here.
   */
  virtual unsigned short comRelease()  {
    CurrentCommand=InputCommand;
    CurrentCommand.Priority=0;
    return 0;
  }

  /**
   *
   * This is called before Preempting a User due to recieving
   * a higher Priority command.  So the current user can be notified
   * that its command is no longer being executed.  This default does nothing.
   */
  virtual void notifyUser(){}
};
  
} // namespace Cure

#endif // CURE_DATAFILTER_HH
