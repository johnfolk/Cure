//    $Id: DataFilterAddress.hh,v 1.18 2008/05/05 16:55:27 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_DATAFILTERADDRESS_HH
#define CURE_DATAFILTERADDRESS_HH

#include "DataFilter.hh"

namespace Cure{

/**
 * A DataFilterAddress is a Filter/Port.
 * This is a reference/pointer to a port on a DataFilter.
 * Normally the filter will create these objects and set them up, so one can
 * then refer to them by calling a_DataFilter.Ports(#). 
 *
 * The address can be set to pull in three different ways either exact
 * or sequenced or latest. These are multually exclusive options.
 *
 * @author John Folkesson
 */
class DataFilterAddress: public Address
{
 public:
  DataFilter *Filter;
  /**
   * The Port number on the DataFilter.
   */
  short Port; 
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
  bool DataNeeded;
  /** Tell the port to set the descriptor on push*/
  bool SetDescriptor;
  unsigned short LastErrorCode;

  /**
   * The index of the pull Address.
   */
  unsigned long PullClient;
  /** 
   * Array of indices to client that want data 
   */
  unsigned long *PushClients;
protected:
  /** Space allocated for push client indices, might not all be used */
  short NumPushClientsAlloc;


  /**
   * If true the address will check pass NO_INTERPOLATE errors,
   * otherwise it blocks and ignors them.  This should be true if
   * the filter expects to be able to pull interpolated data form the 
   * port's pull client.  If the filter doesn't care about the time
   * for this port matching some other time it can set this to false.
   * Used only with PullExact true.
   * Default is true;
   */
  bool Interpolate;
  /**
   *  This will cause the port to try and get exactly matching timestamps.
   *  Default is true;
   */
  bool PullExact;
  /**
   * This is set with setSequenced(Timestamp & t,int nextdata=1) and
   * will set the port up to pull data one at a time in order from a slot.
   *
   * Default is false;
   */
  bool PullSequenced;
  /**
   * This will cause the address to read with t=0, (ie. the most recent data)
   * on all pullData calls.
   * Default is false;
   */
  bool PullLatest;
  /** used for pull sequenced. */
  int NextData;
  /** used for pull sequenced. */
  Timestamp LastTime;
public:
  DataFilterAddress(const unsigned short thread=0);
  virtual void initAddress(const unsigned short thread);
  /**   
   * @param f the filter that owns this 
   * @param portnumber the number of the port. 
   */
  void init(DataFilter *f, short portnumber);//, const unsigned short thread=0);
  ~DataFilterAddress();
protected:
  virtual void disconnectAddress();
public:
   /**
   * If this set true, pullData will return an error if there is no
   * PullClient.  
   */
  void setDataNeeded(bool t=true){DataNeeded=t;}

   /**
   * If this set true, pullData will return an error if 
   * PullExact and the read
   * from PullClient returns with NO_INTERPOLATE.  
   */
  void setInterpolate(bool t=true){Interpolate=t;}
  /**
   * If this is called pullData will read the data interpolated to
   * the exact requested timestamp. 
   */
  void setExact(){
    PullExact=true;
    PullSequenced=false;
    PullLatest=false;
  }
  /**
   * Set pullData to read the data in order
   * from the PullClient with no skiped data. 
   * This is meant to tbe pulling froam a slot.
   * @param t the timestamp to start sequence from. 
   * @param nextdata The next interpolate flag to use (normally just 1) 
  */
  void setSequenced(Timestamp & t,int nextdata=1){
    PullExact=false;
    PullSequenced=true;
    PullLatest=false;
    LastTime=t;
    NextData=nextdata;
  }
  /**
   * If this is called pullData will read the most recent data 
   * from the PullClient. 
   */
  void setLatest(){
    PullExact=false;
    PullSequenced=false;
    PullLatest=true;
    LastTime=0.0;
  }
  /**
   * Call write on the filter input.  This will trigger a calc.
   *
   * @param p data to be copied
   * @return value from it Filter write(...).
   */
  unsigned short write(TimestampedData & p){
    return Filter->write(p,Port);
  }  
  /**
   * Call read on the filter output port pointed to.
   *
   * @param result Data object to be copied into
   * @param t requested time will be interpreted based on interpolate 
   * @param interpolate flag for requesting interpolation to t.
   * @return  value from it Filter read(...).
   */
  unsigned short read(TimestampedData & result, const Timestamp t=0, 
		      const int interpolate=0){
    return Filter->read(result,Port,t,interpolate);
  }
  /**
   * This is called by the filter to push the output.
   * @param tp the data to write to push clients.
   * @return 0 allways 
   */
  unsigned short pushData(TimestampedData &tp);
  unsigned short pushDataLocked(TimestampedData &tp){
    AddressBank::theAddressBank().getThread(Thread);
    unsigned short r=pushData(tp);
    AddressBank::theAddressBank().releaseThread(Thread);
    return r;
  }


  /**
   * This is called by the filter to pull the input.  Will read from
   * pull client returns error if there is no client (IsPull false) and
   * the data is needed by the filter for calc (DataNeeded true).
   *
   * For (PullExact true) The data exactly interpolated to t will be returned
   * If t=0 the latest data is returned.
   *
   * For (PullSequenced true) the data just after the last data pulled
   * will be returned.  If LastTime=0 (no last time) the latest data
   * is returned.  Here t is ignored totally.  If  NextData<0 the data is 
   * the data just before the last data.  LastTime and NextData are set 
   * automatically each time you pull.  They can be reset when you call
   * setSequenced.
   * 
   * For (PullLatest=true) the most recent data is returned. 
   *
   *
   * @param result the data is returned here from pull client.
   * @param t the timestamp to read with, interpolate is set using internal 
   *          flags,  Interpolate, PullExact, PullSequenced and
   *          PullLatest.
   * @return 0 if ok else error code from pull client subject to:
   *           NO_INTERPOLATE Masked out if Interpolate not set.
   *           ADDRESS_INVALID changed to RESOURCE_ERROR;
   *           RESOURCE_ERROR also returned if DataNeeded and no pull client.
   */
  unsigned short pullData(TimestampedData& result,
			  const Timestamp t);
  unsigned short pullData(TimestampedData& result,
			  const Timestamp t, const int interpolate){
    if (IsPull)
      return readFrom(PullClient,result,t,interpolate);
    return RESOURCE_ERROR;
  }
  unsigned short pullDataLocked(TimestampedData& result,
				const Timestamp t, const int interpolate){
    if (IsPull)
      return readFromLocked(PullClient,result,t,interpolate);
    return RESOURCE_ERROR;
  
  }  
  unsigned short pullDataLocked(TimestampedData &result,const Timestamp t){
    AddressBank::theAddressBank().getThread(Thread);
    unsigned short r=pullData(result,t);
    AddressBank::theAddressBank().releaseThread(Thread);
    return r;
  }

  /**
   * Give an Address 'pa' to our  filter's port, which then 
   * will push data to pa.
   *
   * What happens then is dependent on the filter but normally it will
   * cause a write to be called on pa when a change in the port value 
   * occurs (for an output port). 
   *   
   * If push is not defined for this port (ie. an input port)
   * it returns ADDRESS_INVALID. 
   *
   * @param pa the Address to receive the push data.
   * @return 0 if ok, else ADDRESS_INVALID. 
   */
  unsigned short push(Address * pa);

  /**
   * Give an Address 'pa' to this port, which then 
   * will pull data from pa.
   *
   * What happens then is dependent on the filter but normally it will
   * cause a read to be called on pa when new input data  for the port 
   * is needed (for an input port). 
   *   
   * If pull is not defined for this port (ie. an output port)
   * it returns ADDRESS_INVALID. 
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
  void setDescriptor(TimestampedData &p){
    p.setDescriptor(m_Descriptor);
  }
  
  /** 
   * This is needed to write in the correct Thread space if 
   * Thread!=0.
   * This will write p to address with index on this Thread. 
   */
  unsigned short writeTo(const unsigned long index, TimestampedData &p)
  {
    return AddressBank::theAddressBank()
      .write(Thread,index,p);
  }
  /**This does writeTo from a new thread*/
  unsigned short writeToLocked(const unsigned long index, TimestampedData &p)
  {
    return AddressBank::theAddressBank()
      .writeLocked(Thread,index,p);
  }
  
   /** 
   * This is needed toread in the correct Thread space if 
   * Thread!=0.
   * This will read result from address with index on this Thread. 
   */
 unsigned short readFrom(const unsigned long index,
			  TimestampedData& result, 
			  const Timestamp t, 
			  const int interpolate)
  { 
    return AddressBank::theAddressBank()
      .read(Thread,index,result,t,interpolate);
  }
  /**This does readFrom from a new thread*/
  unsigned short readFromLocked(const unsigned long index,
				TimestampedData& result, 
				const Timestamp t, 
				const int interpolate)
  { 
    return AddressBank::theAddressBank()
      .readLocked(Thread,index,result,t,interpolate);
  }
  
  

};

} // namespace Cure

#endif // CURE_POSEFILTERADDRESS_HH
