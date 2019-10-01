//    $Id: DebugAddress.hh,v 1.12 2008/05/05 16:55:27 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2005 John Folkesson
//    

#ifndef CURE_DEBUGADDRESS_HH
#define CURE_DEBUGADDRESS_HH


#include "AddressBank.hh"
#include "Address.hh"

#include <iostream>
#include <string>

namespace Cure{
  class ConfigFileReader;
/**
 * A DebugAddress is a Address that helps to debug a filter app..
 * By placing tis address between 2 linked addresses one can see the
 * data being passed between them.
 *
 * The Config ccf file can say what to do:
 *
 * DEBUGADDRESS
 * BugsBunny
 *
 * BugsBunny
 *
 * m_Level 15     
 *
 * m_MessageString Data 0 0 7 6
 *
 * m_MessageString ShortData 0 0 1 1
 *
 * m_StartTime 1154611100 178368 
 *
 * m_StopTime 1154611200 178368 
 *
 * Spaces between lines don't work in ccf files but I put them here to have 
 * doyxgen format this better.
 *
 * The m_Level is a set of binary flags that each set b1 causes 
 * some additional debug info.
 *
 *  if m_Level&64 then the program will pause at each message
 * to give you a chance to read. Entering a 's' from the 
 * keyboard will cause this pauseing to cease.
 * 
 * @author John Folkesson
 */
  class DebugAddress: public Address
{
public:
  /** 
   * Variable that is used for warning and error printouts. 
   * One can set this to whatever string is a good descripttion.
   */
  std::string AddressName;
  /** 
   * Number of push clients that are subscriber to the address. 
   */
  short NumPushClients;
  /**
   * Tells if there is any subscribed pull client.
   * If this is true the address will read PullClient on pullData(...).
   */
  bool IsPull;
  /**Higher numbers print more messages.*/
  long m_Level;
  
  short m_NumberOfStrings;
  ShortMatrix m_MessageIndex[10];
  std::string m_MessageString[10];
  Timestamp m_StartTimes[10];
  Timestamp m_StopTimes[10];

protected:
  long m_CurrentLevel;
  long m_StartTimeIndex;
  long m_StopTimeIndex;
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

protected:
  virtual void disconnectAddress(){
    Address::disconnectAddress();
    if ((NumPushClientsAlloc)&&(PushClients)) delete [] PushClients;
    PushClients=0;
    NumPushClientsAlloc=0;
    NumPushClients = 0;
    PullClient=0;
  }
public:
  /**   
   *Constructor
   */
  DebugAddress(const unsigned short thread=0);

  ~DebugAddress(){
    disconnectAddress();
  }
  virtual int configure(ConfigFileReader &cfg, std::string  &filtername);
  virtual unsigned short message(TimestampedData& tp);
  bool operator == (const std::string name){return (AddressName==name);}

  /**
   * This will write a TimestampedData 'p' to any push addresses.
   * It also prints the data along with a header.
   *
   * @param p  to be copied
   * @return 0 if ok, else ADDRESS_INVALID.
   */
  unsigned short write(TimestampedData& p);

  /**
   * Calls read on the pull address.
   * It also prints the data along with a header.
   *
   * @param result object  to be copied into
   * @param t requested time must be 0 (the default)
   * @param interpolate flag is ignored
   * @return 0 if ok, else ADDRESS_INVALID, NO_INTERRPOLATE, TIMESTAMP_ERROR.
   */
  unsigned short read(TimestampedData& result, const Timestamp t=0,
		      const int interpolate=0);


  /**
   * Give an Address 'pa' to our address, which then 
   * will push data to pa.
   *
   * @param pa the Address to receive the push data.
   * @return 0 if ok, else ADDRESS_INVALID. 
   */
  unsigned short push(Address * pa);
  
  /**
   * Give an Address 'pa' to this Address, which then 
   * will pull data from pa
   *
   * @param pa the Address to find the pull data.
   * @return 0 if ok, else ADDRESS_INVALID. 
   */
  unsigned short pull(Address * pa);



  /*
   * Removes Address 'pa' from the object
   * @param pa the PoseAddress 
   * @return 0 if ok, else ADDRESS_INVALID. 
   */
  unsigned short disconnect(Address *pa){
    disconnect(pa->Index);
    return 0;
  }
  void disconnect(const unsigned long index);

private:
};

} // namespace Cure

#endif 
