//    $Id: DataSlotAddress.hh,v 1.20 2008/05/05 16:55:27 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson, Patric Jensfelt
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_DATASLOTADDRESS_HH
#define CURE_DATASLOTADDRESS_HH

#include "SmartDataList.hh"
#include "AddressBank.hh"
#include "Address.hh"
#include "Pose3D.hh"
#include "DataSlot.hh"
#include <string>

namespace Cure{

/**
 * A DataSlotAddress is a Dataslot input/output port.
 * This is a reference/pointer to a DataSlot.
 * After any write the push subscribers are notified. 
 *
 * You can configure the Slot to use a mutex lock to protect the data
 * from if you use multiple threads for example.
 *
 * You can also use a separate thread for pushing out the data. Using
 * a separate thread demands that you use a mutex lock to protect the
 * data. The main idea with using a thread is to be able to write data
 * at a high constant data rate while the push might take a variable
 * and sometimes quite long time. The push subscriber's write will be
 * run in a parallel thread with shared data being in ExclusiveSlot's
 * or some other protected memory. The push thread will remember the
 * last pushed data and will take the data that was written next and
 * then continue until there is no new data.  Thus even if the thread
 * starts running behind it will try to catch up.  All data will be
 * pushed if there is no overflow. Obviously no data will be pushed
 * twice.
 *
 * @author John Folkesson and Patric Jensfelt
 */
class DataSlotAddress: public Address
{
public:

  /** The slo that stores the data */
  DataSlot Slot;

protected:
  /** Id of the thread that pushes data */
  pthread_t m_Tid;
  
  SmartDataList m_Queue;
  
  /** true when the pusher thread is running */
  bool m_PushThreadRunning;

  /** True if we should push out data to clients in separate thread */
  bool UseThread;

  /** Trye if we should protect data with locks */
  bool UseLock;

  /** Mutex that protectes the above variables */
  MutexWrapper SlotMutex;

  /** Mutex that protectes the above variables */
  MutexWrapper QueueMutex;

  /** Mutex used when signaling that new data has arrived */
  pthread_mutex_t m_SignalMutex;

  /** Variable used when signaling that new data has arrived */
  pthread_cond_t m_SignalCond;

  /** Array of indices to client that want data */
  unsigned long *PushClients;

  /** Space allocated for push client indices, might not all be used */
  short NumPushClientsAlloc;

  /** Number of push client */
  short NumPushClients;

  /** Mutex that protects the access and changing of the push client list */
  MutexWrapper ClientListMutex;

public:

  /**
   * Constructor
   */
  DataSlotAddress(const std::string name="DataSlotAddress", 
		  const unsigned short thread=0);


  /**
   * @param depth number of elements stored in the slots
   * @param classType type of data that slot is intended for. Storage
   * for this kind of data will be alloctaed for but unless you turn
   * on typechecking you can write anything you want to it.
   * @param subType fine grained description of the type of data specific to
   * each data type, for example what type of pose
   * @param compressed if you want data to be packed in the slot
   * @param dotypecheck true if you want to make sure that you can
   * only write data with the type you specified,
   */
  DataSlotAddress( short depth, 
		   unsigned char classType= POSE3D_TYPE,
                  unsigned short subType=0,
                  bool compressed=false, 
                  bool dotypecheck=false,const unsigned short thread=0);
  

  /**
   * Destructor
   */
  ~DataSlotAddress();

  /**
   * Set the name of this slot address. Which really means setting the
   * name of the underlying Slot. This is very useful when debugging
   * so that you can see what slot is complaining for example.
   * 
   * @param name name
   */
  void setName(const std::string &name){
    m_Descriptor.Name=name;
    Slot.Descriptor.Name=name;
  }

  /**
   * 
   * @return name of the slot
   */
  std::string& slotName(){
    return Slot.Descriptor.Name;
  }
  bool operator == (const std::string name){return (Slot.Descriptor.Name==name);}
  
  /** 
   * Function to call if you want to turn on/off using a mutex lock to
   * protect the data of the slot. The default is NOT to use locks.
   *
   * @param useit true if you want to use the mutex lock protection
   */
  void setUseLock(bool useit = true);

  /** 
   * Function to call if you want to turn on/off using a separate
   * thread to push out data to clients. The default is NOT to use a
   * separate thread. Be careful when using this as you will have to
   * deal with all issues that comes with having separate threads.
   *
   * @param useit true if you want to use a thread to perform the
   * pushing to clients so that the thread that calls write does not
   * block during this process.
   */
  unsigned short setUseThread(bool useit = true);

  /**
   * This will write the data 'p' to the DataSlot. This will also
   * cause a push out from the slot to all objects registered to be
   * pushed to.
   *
   * @param p Data to be copied into the slot
   * @return 0 if ok,
   */
  virtual unsigned short write(TimestampedData& p);
  
  /**
   * Call read on the DataSlot.
   * Copy a TimestampedData into result.
   * if t=0 return the most recent Data available.
   * 
   * if interpolate=k >0 Go to first data written that was >= t and then
   * k more steps through the data, so the kth one written after the 1st
   * one >= t.
   * if interpolate=k <0 Go to last data written that was <= t and then
   * k more steps back through the data, so the kth one written before the
   * last one one <= t.  This means that there needs to be some data with
   * >t inorder to know where to start.
   * if interpolate =  0 interpolate between the two Datas before and 
   * after t (or just take the data at exactly t).  
   *
   * Simple but not presise:
   * if interpolate == +k return the kth Data 'after' t.
   * if interpolate == -k return the kth Data 'before' t.
   * if interpolate =  0 interpolate between the two Datas before and 
   *  
   * Upon return intepolate will have been set for reading the next
   * Data (before or after depending on its sign)
   * 
   * So one would call read with the the timestamp t=prior results Time
   * and *interpolate = the prior returned intepolate.
   *
   * If the data is not available as requested:
   * return TIMSTAMP_ERROR  (note: there may be something useful in p)
   * 
   *
   * @param result Data to be copied into
   * @param t requested time will be interpreted based on interpolate 
   * @param interpolate flag for requesting interpolation to t.
   * @return 0 if ok, else TIMESTAMP_ERROR, NO_INTERPOLATE
   *                NO_DATA_AVAILABLE  TIMESTAMP_TOO_OLD or TYPE_ERROR.
   */
  virtual unsigned short read(TimestampedData& result, 
		   int interpolate[1], const Timestamp t=0){
    lockSlot();
    unsigned short r=Slot.read(result,interpolate,t);
    unlockSlot();
    if (!(t==0))
      {
	if (interpolate[0]==0)
	  if (!(t==result.Time) && !(r&TIMESTAMP_ERROR))
	    r=r|NO_INTERPOLATE|TIMESTAMP_ERROR;
      }
    return r;
  }


  /**
   * See descrptoin for previous read function
   * 
   */
  virtual unsigned short read(TimestampedData& result, const Timestamp t=0,
		   const int interpolate=0){
    int i=interpolate;
    return read(result,&i,t);
  }
  
  /**
   * Adds pa to the notify list.
   *
   * @param pa the DataAddress to be notified
   * @return 0.
   */
  virtual unsigned short push(Address * pa);

  /*
   * Removes Data Address 'pa' from the Notify list
   * @param pa the DataAddress 
   * @return 0. 
   */
  virtual unsigned short disconnect(Address *pa){
    disconnect(pa->Index);
    return 0;
  }  
 
  /*
   * Removes Data Address with a certan index from the Notify list
   * @param index index of the DataAddress to disconnect
   * @return 0 if ok.
   */
  void disconnect(const unsigned long index);
  /**
   * This accumulates the poses stored in the slot form before time to 
   * nexttime and returns the result ininc.  The slot shoudl contain
   * incremental pose data.  returns an error if the time stamps are
   * out of the buffer range.
   *
   */
  int accumulatePose(Cure::Timestamp &beforeTime,
		     Cure::Timestamp &nextTime, Cure::Pose3D &inc);

protected:
  virtual void disconnectAddress();

  /** Function that creates the thread that pushes out data */  
  static void* createPusherThread(void *ptr);

  /** The function that pushes out data to clients */
  virtual void pusher();

  /** Do the actual pushing of the data to the clients */
  unsigned short pushData(TimestampedData *tp);

  /** Do the actual pushing of the data to the clients */
  unsigned short pushDataLocked(TimestampedData *tp);

  /** Does the common initialization stuff for an DataSlotAddress */
  void slotInit();

  /** Locks the slot if we have configured to use a lock */
  void lockSlot();

  /** Unlocks the slot if we have configured to use locks */
  void unlockSlot();

  void lockQueue();
  void unlockQueue();
private:
};

inline void
DataSlotAddress::lockSlot()
{
  if (UseLock) SlotMutex.lock();
}
  
inline void
DataSlotAddress::unlockSlot()
{
  if (UseLock) SlotMutex.unlock();
}
inline void
DataSlotAddress::lockQueue()
{
  QueueMutex.lock();
}
  
inline void
DataSlotAddress::unlockQueue()
{
  QueueMutex.unlock();
}
  
} // namespace Cure

#endif // CURE_DATASLOTADDRESS_HH
