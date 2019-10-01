//    $Id: ThreadMerger.hh,v 1.13 2009/03/29 21:18:07 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson, Patric Jensfelt
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_THREADMERGER_HH
#define CURE_THREADMERGER_HH

#include "AddressBank.hh"
#include "DataFilterAddress.hh"
#include "SmartDataList.hh"
#include <string>

namespace Cure{

/**
 * A ThreadMerger can take several threads and sync them.
 * It uses a mutex lock to protect the data
 * from if you use multiple threads for example.
 *
 *  
 * Inputs:
 *      0 to Ins-1 
 *               Anything can come in. These are not standard 'ins' as
 *               push can be called on them.  The Current m_QueueLength is 
 *               pushed out from the in port when it is written to.  The format
 *               is A TimestampedData (the actual base class object) with
 *               its time set to the time of the written data, its ID
 *               set tie the ID of the written data, and its subType set 
 *               to the m_QueueLength limited (max of 64K).
 *                      
 *
 * Outputs: 0 to Ins -1 
 *               matched to ins what goes in will come out the
 *               corresponding port
 *
 * SUMMARY OF USE 
 *
 *  Write to the inputs from as many threads as you like
 *  The cooresponding set if output ports will be pushed in 
 *  a single separate thread.
 *
 *
 * This uses a separate thread for pushing out the data. Using
 * a separate thread demands that it use a mutex lock to protect the
 * data. The main idea with using a thread is to be able to write data
 * a a high constant data rate while the push might take a variable
 * and sometimes quite long time. The push subscriber's write will be
 * run in a parallel thread with shared data being in ExclusiveSlot's
 * or some other protected memory. The push thread will remember the
 * last pushed data and will take the data that was written next and
 * then continue until there is no new data.  Thus even if the thread
 * starts running behind it will try to catch up.  All data will be
 * pushed.
 * 
 *
 * This then allows a number of threads to access a set of Filters
 * asynchronously.  The limitation is that only write of
 * TimstampedData to Addresses is protected.  Other types of access
 * might be needed for example to set up a push or pull or disconnect.
 * One might want to directly access a filter's mebers for some
 * reason.  For this pause and resume methods are provided.  One can
 * pause the pusher thread, access the objects and then resume.
 * During the pause state the data is being collected in a queue so
 * the pause should be very quick or the queue will get very long.
 *
 *
 *
 *
 * @author John Folkesson and Patric Jensfelt
 */
class ThreadMerger: public DataFilter
{
public:

protected:

  SmartDataList DataList;
  /** Id of the thread that pushes data */
  pthread_t m_Tid;
  bool m_AllowPause;  
  //**Length of queue*/
  unsigned long m_QueueLength;
 
  /** true when the pusher thread is running */
  bool m_PushThreadRunning;


  /** Mutex that protectes the above variables */
  MutexWrapper MergeMutex;

  /** Mutex that protectes the pusher thread pause */
  MutexWrapper PauseMutex;

  /** Variables used when signalling that new data has arrived */
  pthread_mutex_t m_SignalMutex;
  pthread_cond_t m_SignalCond;

  TimestampedData m_QueueData;
  /** 
   * Set to false if you don't want the pusher thread to get
   * the lock on the thread space.  This can only be safe
   * if the addresses being pushed to are thread safe themselves or
   * this is the only thread writing to them.  Still the 
   * ThreadMerger should then be made first and deleted last.
   */ 
  bool m_PushLocked;

  virtual void disconnectAddresses();
public:
  /**
   * Constructer is called with the number of input ports to use.
   * @param numins the number of input ports for this filter.x
   * @param thread the thread to create the ports initialy with
   *        (ie the creator's thread.)
   * @param threads this is an array length 2*numins that
   *        gives the thread for each in port and each out port 
   */
  ThreadMerger(short numins,const unsigned short thread=0,
	       unsigned short *threads=0);
 
  ~ThreadMerger();
  virtual int config(const std::string &params); 


  /**
   * Calling Write causes the data to be put in a queue to be pushed out 
   * of the coorresponding out port by the pussher thread.
   * 
   * 
   *
   * @param p Data to be copied
   * @param port must be an input port number
   * @return 0 if input data read ok.
   */
  virtual unsigned short write( Cure::TimestampedData& p,const int port=-1);
  /**
   * Momentarily pause the pusher thread to allow outside acess to the
   * objects being pushed to by anouther thread.  No data will be
   * lost.  This blocks untill the current data is finished being pushed.
   *
   * This works that m_AllowPause is set to true in config at startup.
   * then one can call pause/resume.  Do not call pause while doing config.
   * in another thread please and don't do config while paused.  One can
   * call config while thread is pushing. 
   * 
   * @return true if locked else false (and resume should not be called)
   */
  virtual bool pause(){
    if (m_AllowPause){
      PauseMutex.lock();
      return true;
    }
    return false;
  }
  unsigned long getQueueLength(){
    lockMerger();
    unsigned long r=m_QueueLength;
    unlockMerger();
    return r;
  }

  /*
   * Starts the pusher thread again after a pause.
   */
  virtual void resume(){
    if (m_AllowPause)
      PauseMutex.unlock();
  }
  /**
   *  give permission to DataFilterAddress 'port' to (push) data to pa.
   *  Here we allow Ins to push.
   * @param port the port number    
   * @return true if ok, else false. 
   */
  virtual bool canPush(const int port){
    if ((port<(Outs+Ins))&&(port>(-1)))
      return true;
    return false;
  }



protected:
  /** Function that creates the thread that pushes out data */  
  static void* createPusherThread(void *ptr);

  /** The function that pushes out data to clients */
  virtual void pusher();

  /** Do the actual pushing of the data to the clients */
  unsigned short pushDataLocked(TimestampedData *tp, int port){
    Ports[port].pushDataLocked(*tp);
    return 0;
  }  
  unsigned short pushData(TimestampedData *tp, int port){
    Ports[port].pushData(*tp);
    return 0;
  }



  /** Does the common initialization stuff for an ThreadMerger */
  void MergerInit();

  /** Locks if we have configured to use a lock */
  void lockMerger();

  /** Unlocks  if we have configured to use locks */
  void unlockMerger();

private:
  /**
   * This returns a reference to the TimestampedData object associated with 
   * this port.  There is none So make sure to not let parent classes call.
   */
  virtual Cure::TimestampedData * data(int port){ return 0;};
  
};
inline void
ThreadMerger::lockMerger()
{
  MergeMutex.lock();
}
  
inline void
ThreadMerger::unlockMerger()
{
  MergeMutex.unlock();
}
  
} // namespace Cure

#endif 
