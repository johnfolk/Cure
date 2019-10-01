// = AUTHOR(S)
//    John Folkesson
//
// = COPYRIGHT
//    Copyright (c) 2005 John Folkesson
//
/*----------------------------------------------------------------------*/

#ifndef CURE_COMMAND_hh
#define CURE_COMMAND_hh

#include "TimestampedData.hh"
#include "Matrix.hh" 
#include "ShortMatrix.hh" 

#ifndef DEPEND
#include <string.h>  // memcpy
#include <unistd.h>  // NULL
#include <iostream>  // ostream
#endif


// Forward declaration(s)
class PackedCommand;
namespace Cure {


/**
 * This class is used to send commands to Cure modules
 *
 * The SubType for this is 
 * the size of CmdFlags, (Rows*Columns) 
 *
 * @author John Folkesson 
 */
class Command :  public TimestampedData 
{  
public:
  
  /**
   * Defines different types of commands
   */
  enum CmdTypes {
    STATUS = 1,
    INIT, 
    CONFIGURE,
    STOP,
    START,
    SPEED,
    GOTO,
    SAVE,
    LOAD,
    RESET,
    GET,
    SET,
    REPLY,
    RELEASE,
  };

  /**Values from 1 to 128 will be reserved for CURE */
  /**Values from 128 to 256 will be left free to the users to define*/
  enum ServiceTypes {
    SERVICE_SLAM = 1,
    SERVICE_LOCALIZATION,
    SERVICE_GOTO,
    SERVICE_DRIVE,
    SERVICE_AVOIDANCE,
    SERVICE_FOLLOW,
    SERVICE_POSE,
    SERVICE_PAN_TILT,
    SERVICE_ARM,
    SERVICE_SICK,
    SERVICE_SONAR,
    SERVICE_GPS,
    SERVICE_INERTIAL,
    SERVICE_CAMERA ,
    SERVICE_ROBOT,
    SERVICE_UNKNOWN,
    SERVICE_PATH_PLANNING,
    SERVICE_EXPLORE,
  };

public:
  
  friend class PackedCommand;
  /**
   * Create empty Command
   * @param id the id of this 
   */
  Command(unsigned short id = 0);
  /**
   * Create empty Command
   * @param t the time
   * @param id the id of this 
   */
  Command(Timestamp &t,unsigned short id = 0);

  /**
   * Create a copy of a Command
   * @param s The object to copy
   */
  Command(const Command &s);
  /**called by consturctors*/
  void init();
  /**
   * Destructor
   */
  virtual ~Command();
  /*
   * This is how we (efficiently) get a Command from a TimestampedData.
   */
  Command * narrowCommand(){return this;}
  /**
   * Copy operator
   * @param src The object to copy
   */
  Command& operator=(const Command &src);  
  /**
   * Copy operator
   * @param p The object to copy
   */
  void operator = (TimestampedData& p)
  {
    Command *ss=p.narrowCommand();
    if (ss)operator=(*ss);
    else TimestampedData::operator=(p);
  }
  /**
   * Returns the elements of the CmdData Matrix
   */
  double& operator()(const unsigned short r, const unsigned short c) {  
    return  CmdData(r,c);
  }
  /**
   * Returns the elements of the CmdData Matrix.
   */
  double operator()(const unsigned short r, const unsigned short c) const {
    return( (*(Command *)this).operator()(r,c) );
  }

  
  /**
   * Get the Priority
   * @return the Priority
   */
  unsigned short  getPriority() const { return Priority; }
  
  /**
   * Gets the service that is being sent the command. 
   * @return the Service
   */
  unsigned short getService() const { return Service; }
  
  /**
   * Gets the CmdType.
   * @return CmdType.
   */
  unsigned short getCmdType() const { return CmdType; }
  

  /**
   * Set the priority level for the command.
   * @param r the new priority.
   */
  void setPriority(unsigned short r) { Priority=r; }
   /**
   * Sets the Service.
   * @param b the code for the sevice
   */
  void setService(unsigned short b)  {Service=b; }
  /**
   * Sets the CmdType which specifies what the command is requesting
   * @param r the new CmdType
   */
  void setCmdType(unsigned short r) {CmdType=r; }
  /** 
   * Sets the subtype, if t is not equal to the size of CmdFlags then
   * CmdFlags is set to a row vector of that length.
   * @param t the new subtype/length of CmdFlags
   */ 
 void setSubType(unsigned short t);
  /** Display info on the Object*/
  void print(); 
public:
  /**
   * This is the data needed to specify the action.  It could be velocity
   * goal point, angle, timeout, distance ...
   */
  Cure::Matrix CmdData;
  /**
   * This specifies the command for instance goto, go, stop, start, load, save,
   * plan path, pan... 
   */
  unsigned short CmdType;
  /**
   * This is a broad category of service such as Pose provider, motion
   * controll, arm motion, planner...
   */
  unsigned short Service;
  /**
   * Priority is >0 for command to not be ignored.
   * Higher numbers mean higher priority
   * Execution is blocked if CurrentCommand has higher Priority,
   * 
   */
  unsigned short Priority;
  /**
   * This might be good for debuging if nothing else to see where
   * commands originated.
   */
  unsigned short Sender;
  /**
   * The Index of an Cure::Address that the reply should be sent to.
   * The AddressBank is assumed known. 0 will be considered anomynous.
   */
  unsigned long ReplyAddress;
  /**
   * A unique ID >0 is given to every command the Sender sends.
   */
  unsigned long CmdID;
  /**
   * If this is a reply of a Command then the corresponding CmdID 
   * is put here. Otherwise this is 0.
   */
  unsigned long ReplyID;
  /**
   * This can hold whatever number of flags or other intergers is needed.
   */
  Cure::ShortMatrix CmdFlags;
  
protected:
  
private:
};
}

std::ostream& operator << (std::ostream& os, const Cure::Command &cmd);

#endif // Command_hh
