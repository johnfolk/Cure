//    $Id: Address.hh,v 1.14 2008/05/05 16:55:26 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_ADDRESS_HH
#define CURE_ADDRESS_HH



#include "SmartData.hh"

namespace Cure{
 
class AddressBank;
/**
 * A Address is an abstract base class with a simple interface.
 * read/write/push/pull/disconnect.
 *
 * The Address knows where and how to call read/write on some object
 * That actual does something with Data.  
 * 
 * The Address acts as  regulated a pointer.  It can be 'given away' and 
 * the object it 'points' to can be deleted without having to track down all 
 * Addresses that point to it.  When the holder of the Address later
 * calls write it will get ADDRESS_INVALID returned but not 
 * a segmentation fault.
 * 
 * This magic is achieved by having some static object that mananages the 
 * Addresses.  This is the job of the AddressBank.  So all objects that 
 * 'point' to Address's actually have a AddressBank reference and an
 * index in the bank.  The bank then calls the method on the address if 
 * it is still valid.   
 *
 * @author John Folkesson
 */
class Address
{
public:
  /**
   * This is what is pased to the bank to get at this Address.
   */
  unsigned long Index;
   DataDescriptor m_Descriptor;
  unsigned short Thread;
protected:
  /** Flag to see if isThreadSafe can return true.*/
  bool CanPermit;
  /** All the threads that successfully called isThreadSafe */
  unsigned short *PermittedThreads;
  
  Address(const unsigned short thread=0);
  virtual void initAddress(const unsigned short thread);
  /**
   * Removes this from the AddressBank and disconnects  from all push/pull
   * clients.  Called prior to deletion and by disable.
   * 
   */
  virtual void disconnectAddress();
public:
  /**
   * The Subclasses all must have this code in their distructor:
   *
   *  AddressBank::theAddressBank().remove(this);
   */
  virtual ~Address(){disable();}
  /**
   * Interface to disconnectAddress().
   */
  void disable(){disconnectAddress();}
  void setThread(unsigned short thread);

  /**
   * Returns a pointer to this object. The only purpose of this
   * function is to make it possible to write for example file.port()
   * and make it look similar to the in(), out() and trigger() of the
   * DataFilters
   */
  Address* port() { return this; }

  /**
   * This will write a TimestampedData 'p' to the 'Address'.
   *
   * There are  various returns that indicate some error.
   *
   * @see CureDefines.hh
   *
   * TIMESTAMP_ERROR indicates that some object 'downstream' or
   * upstream from the object written to (one of the data sources) 
   * was unable to get data at the time needed for the calculation.
   * thus the write was did not pass as far as it would have.
   *  This is more information than an error and in many cases
   * a perfectly normal result.  The calculation may be done at a later
   * time on the next write for instance.
   * 
   * NO_INTERPOLATE indicates that some object did not get the
   * data it needed because the connections are in error.  Some address
   * that needs interpolated data is hooked up to an address that can 
   * never interpolate.  The soultionis to place a slot in between.
   * Sometimes a syncronizer is needed. 
   *
   * RESOURCE_ERROR indicates that some data was unavailble for some
   * reason but does not indicate a connection error.
   *
   * ADDRESS_INVALID is normally caught by the calling object and
   * changed to RESOURCE_ERROR befor passing it on.  This error
   * causes the Address to dissconnect from the subscriber that
   * caused the error.  
   *
   * TYPE_ERROR indicates that the wrong kind of data is being fed to
   * some address.  This must be corrected by the programer as it means 
   * some object can never work properly.
   *
   * @param p TimestampedData to be copied, it should not be changed.
   * @return 0 if ok, else see above.
   */
  virtual unsigned short write(TimestampedData&) {
      return ADDRESS_INVALID;  
  }

  /**
   * Copy a TimestampedData into result.
   *
   * if t=0 return the most recent Data available.
   * 
   * else if interpolate = 0 interpolate between the two Datas before
   * and after.  t (or just take the data at exactly t).
   *
   * 
   * else if interpolate=k >0 Go to first data  that was >= t and then
   * k more steps through the data, so the kth one written after the 1st
   * one >= t.
   * 
   * else if interpolate=k <0 Go to last data that was <= t and then
   * k more steps back through the data, so the kth one written before the
   * last one one <= t.  This means that there needs to be some data with
   * >t inorder to know where to start.
   * 
   * 
   * Simple but not so precise:
   * if interpolate == +k return the kth Data 'after' t.
   * if interpolate == -k return the kth Data 'before' t.
   * 
   * The idea of this is you can get the data exactly in sequence form
   * the object that doesn't remember you between calls.  Normally to
   * get the sequential data you call read with the t=last time you
   * got and interpolate=1.  Now if this returns with no errors and
   * result.Time==t, (which indicates that two data had exactly the
   * same time) you need to remember to add one to interpolate the
   * next time you call to get the next data.  On the other hand if
   * result.Time!=t; you should reset interpolate to 1 and
   * t=result.Time before the next read.  
   *
   * Going the other way back in time one starts with -1 and
   * subtracting 1 if the time is the same.
   *
   * There are  various returns that indicate some error.
   *
   * TIMESTAMP_ERROR indicates that some object 'downstream' 
   * from the object read  (one of the data sources) 
   * was unable to get data at the time needed for the calculation.
   * thus the data is not as requested.
   * This is more for information than an error and in many cases
   * a perfectly normal result. (note: there may be something 
   * useful returned from the read but it will have the wrong time)
   * 
   * NO_INTERPOLATE indicates that some object did not get the
   * data it needed because the connections are in error.  Some address
   * that needs interpolated data is hooked up to an address that is 
   * incapable of interpolation.  The soultion is to place a slot in between.
   * Sometimes a syncronizer is needed. 
   *
   * RESOURCE_ERROR indicates that some data was unavailble for some
   * reason but does not indicate a connection error.  It does
   * however  mean there is nothing usful in result.
   *
   * ADDRESS_INVALID is normally caught by the calling object and
   * changed to RESOURCE_ERROR before passing it on.  This error
   * causes the caller to disconnect from the Address that
   * caused the error. Thus an Address never returns this code
   * only the AddressBank does.  The user of the AddressBank/Index
   * would find out that this address does not exist anymore. 
   *
   * TYPE_ERROR indicates that the wrong kind of data is being fed to
   * some address downstream.  This must be corrected by the 
   * programer as it means some object can never work properly.
   *
   * Note some Address's ignore interpolate and/or t
   *
   * @param result TimestampedData to be copied into
   * @param t requested Timestamp will be interpreted based on interpolate 
   * @param interpolate flag for requesting interpolation to t.
   * @return 0 if ok, else NO_INTERPOLATE, ADDRESS_INVALID,RESOURCE_ERROR, 
   *                       TIMESTAMP_ERROR or TYPE_ERROR.
   */
  virtual unsigned short read(TimestampedData& , const Timestamp, 
			      const int) {
    return ADDRESS_INVALID;
  } 
  virtual unsigned short read(TimestampedData& result, 
			      const Timestamp t=(long)0) {
    return read(result,t,0);
  } 
  /**
   * Give a Address 'pa' to this address which will then
   * push data to pa when the address is triggered.
   *
   * What happens then is dependent on the object but normally it will
   * cause a write to be called on pa when a change in the port value 
   * occurs (for an output port). 
   *   
   * If push is not defined for this port (ie. an input port)
   * it returns ADDRESS_INVALID. 
   *
   * @param pa the Address to receive the push data.
   * @return 0 if ok, else ADDRESS_INVALID. 
   */
  virtual unsigned short push(Address *){
    return ADDRESS_INVALID;
  }

  /**
   * Give permision to an Address to read/write to this without locking
   * the thread space.
   *
   * A thread safe Address will try to lock pa->Thread when being deleted.
   * So it will save pa->Thread forever.
   * @return true if permission is granted.
   * @param pa the address whose thread will be permited.
   */
  bool isThreadSafe(Address * pa);
  
  /**
   * If this succeeds, threads can read/write to this address without
   * gtting the Address space lock.  Obviously the Address will need
   * internal protection of data.  Other calls (Push/Pull/ ect still need the 
   * lock.  The threads must first get permission by calling isThreadSafe.
   * It can fail if the AddressBankThread is full of permanent Addresses.
   * Pretty rare unless one sets the depth in the bank to 1 or 2.   
   * @return true if set else false.
   */
  bool setCanPermit();
  
  /**
   * Give a Address 'pa' to this address, which then 
   * will pull data from pa as needed.
   *
   * What happens then is dependent on the object but normally it will
   * cause a read to be called on pa when new input data  for the port 
   * is needed (for an input port). 
   *   
   * If pull is not defined for this port (ie. an output port)
   * it returns ADDRESS_INVALID. 
   *
   * @return 0 if ok, else ADDRESS_INVALID. 
   */

  virtual unsigned short pull(Address *){
    return ADDRESS_INVALID;
  }
  /**
   * Removes Address  from the object
   * @return 0 if ok, else ADDRESS_INVALID. 
   */
  virtual unsigned short disconnect(Address *){
    return ADDRESS_INVALID;
  }  

  /** 
   * This is needed to write in the correct Thread space if 
   * Thread!=0.
   * This will write p to address with index on this Thread. 
   */
  virtual unsigned short writeTo(const unsigned long index, TimestampedData &p);
  /**This does writeTo from a new thread*/
  virtual unsigned short writeToLocked(const unsigned long index, 
				 TimestampedData &p);
  /** 
   * This is needed toread in the correct Thread space if 
   * Thread!=0.
   * This will read result from address with index on this Thread. 
   */
  virtual unsigned short readFrom(const unsigned long index,
				  TimestampedData& result, 
				  const Timestamp t=0, 
				  const int interpolate=0);
  /**This does readFrom from a new thread*/
  virtual unsigned short readFromLocked(const unsigned long index,
					TimestampedData& result, 
					const Timestamp t=0, 
					const int interpolate=0);
  bool getService(const std::string &service, unsigned long &index);
  virtual void setDescriptor(const DataDescriptor &d){
    m_Descriptor=d;
  }
  /**
   * Conditional equality test if ID is 0 returns true 
   * else does ==.
   * @param d the object to compare. 
   * @return 1 if the objects are the same else 0.
   */
  bool matchWild(const DataDescriptor &d)const{
    return (m_Descriptor.matchWild(d));
  }
  /**
   * @return true if ClassType ID SensorType and SensorID all match  
   */
  bool match(const DataDescriptor& d)const {
    return (d==m_Descriptor);
  }
  bool sameID(const DataDescriptor& d)const {
    return (m_Descriptor.ID==d.ID);
  }
  bool sameSensorID(const DataDescriptor& d)const {
    return (m_Descriptor.SensorID==d.SensorID);
  }

  /**
   * Test equality (Names not considered).
   * @param d the object to compare. 
   * @return 1 if the objects are the same else 0.
   */
  bool operator ==(const DataDescriptor &d)const{
    return (m_Descriptor==d);
  }
  void operator =(const DataDescriptor &d){
    return (m_Descriptor=d);
  }
  /**
   * Test equality (Names only considered).
   * @param t the object to compare. 
   * @return 1 if the objects are the same else 0.
   */
  bool sameName(const DataDescriptor& t)const {
    return (m_Descriptor.Name==t.Name);
  }
  /**
   * Test equality (ClassType only considered).
   * @param t the object to compare. 
   * @return 1 if the objects are the same else 0.
   */
  bool sameClass(const DataDescriptor& t)const {
    return m_Descriptor.sameClass(t);
  }

};

} // namespace Cure

#endif // CURE_ADDRESS_HH
