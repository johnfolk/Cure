//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2005 John Folkesson
//    

#ifndef CURE_COMMANDADDRESS_HH
#define CURE_COMMANDADDRESS_HH

#include "Command.hh"
#include "Address.hh"
namespace Cure{
// Forward declarations
class CommandExecuter;
/**
 * A CommandAddress is an address to receive Commands
 * 
 *  
 * 
 * 
 *
 * @author John Folkesson
 */
class CommandAddress: public Address
{

public:
  CommandExecuter *Executer;
  /**
   * Tells if there is any subscribed pull client.
   * If this is true the address will read PullClient on pullData(...).
   */
  bool IsPull;
  /** 
   * Number of push clients that are subscriber to the the Port's data output. 
   */
  short NumPushClients;
   /**
   * If this set true, pullData will return an error if there is no
   * PullClient.  
   * Default is true;
   */
protected:
  /**
   * The index of the pull Address.
   */
  unsigned long PullClient;
  /** 
   * Array of indices to client that want data 
   */
  unsigned long *PushClients;
  /** Space allocated for push client indices, might not all be used */
  short NumPushClientsAlloc;
  /**This is the next ID we will use for Reply or Output when we write them.*/
  unsigned long CmdID;
  /** This holds our list of recieved replies*/
  Command  **Replies;
  /**These are for managing the list of Replies*/
  short NumberOfReplies;
  short SizeOfReplies;
  short FirstReply;
  short NextReply;


public:
  CommandAddress(const unsigned short thread=0);
  ~CommandAddress(){CommandAddress::disconnectAddress();}
  void init(CommandExecuter *c);
  /**
   * Handel the commands
   *
   * @param p the command reply
   * @return value 
   */

  virtual unsigned short write(TimestampedData & p);
   /**
   * 
   *
   * @param result Data object to be copied into
   * @param t requested time will be interpreted based on interpolate 
   * @param interpolate flag for requesting interpolation to t.
   * @return  value from it Filter read(...).
   */
  virtual unsigned short read(TimestampedData & result, const Timestamp t=0, 
			      const int interpolate=0);

  /**
   * Give an Address 'pa' to our  filter's CommandAddress, which then 
   * will push data to pa.
   *
   * Calls to pustData then send pa the OutputCommand  
   * 
   * 
   *
   * @param pa the Address to receive the push data.
   * @return 0 if ok, else ADDRESS_INVALID. 
   */
  unsigned short push(Address * pa);
  /**
   * Give an Address 'pa' to this, which then 
   * will pull data from pa.
   *
   * It is possible that the filter will never call pullData
   * in which case this client will never be read.  Most Commands
   * will be written(pushed) not read(pulled).
   *
   * @param pa the Address to find the pull data.
   * @return 0 if ok, else ADDRESS_INVALID. 
   */
  unsigned short pull(Address * pa);

  /*
   * Removes Pose Address 'pa' from the object
   * @param pa the Address 
   * @return 0 if ok, else ADDRESS_INVALID. 
   */
  unsigned short disconnect(Address *pa){
    disconnect(pa->Index);
    return 0;
  }
  void disconnect(const unsigned long index);

  /**
   * This will send Executer->Reply to reply address given by c
   * It sets up the CmdID, ReplyID and Service fields 
   * 
   * @param setReplyAdd if false the Reply Address is set to 0
   *        else to this 
   * @param c the command to reply to
   */
  void  sendReply(Command &c,bool setReplyAdd=false);
  /**
   * This will get a reply and remove it from the list.
   * The reply will have ReplyID match the CmdID of p when called
   * p will return with a copy of the reply command removed from the list.
   * 
   * @param p this will hold the reply if found, Has CmdID=ReplyID looked for
   * @return 1 if fail 0 if found.
   */
  int getReply(Command &p);
  /**Remove the reply with index ind and deletes it. Used by get.*/
  void remove(short ind);
  /**Adds reply p to list of replies. */
  void add(Command & p);
  /**
   * This is called by the filter to push the OutputCommand.
   * @param setReplyAdd if false the Reply Address is set to 0
   *        else to this 
   * @return or of the returns from write except Address invalid. 
   */
  unsigned short pushData(bool setReplyAdd=false);
  /**
   * This is called by the filter to pull the input.  Will read from
   * pull client returns error if there is no client (IsPull false)
   * Data returned in InputCommand
   * @return 0 if ok else error code from pull client subject to:
   *           ADDRESS_INVALID changed to RESOURCE_ERROR;
   *           RESOURCE_ERROR also returned if DataNeeded and no pull client.
   */
  unsigned short pullData();
protected:
  virtual void disconnectAddress();
};

} // namespace Cure

#endif 
