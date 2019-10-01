//    $Id: ThreadSwitcher.hh,v 1.3 2008/05/05 16:55:28 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson, Patric Jensfelt
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_THREADSWITCHER_HH
#define CURE_THREADSWITCHER_HH

#include "AddressBank.hh"
#include "DataFilterAddress.hh"

namespace Cure{

/**
 * A ThreadSwitcher can take an input on one thread space and 
 * write/read or  the data on another  thread space blocking if needed. 
 *  
 * Inputs: 0 Anything can come in and can be set to pull.  write will
 * push the data out the out.
 *                      
 *
 * Outputs: 0 matched to ins what comes in will go out. read will pull
 * the data
 *              
 *
 * SUMMARY OF USE 
 *
 *  Write to the inputs from one threads The output port will be
 *  pushData.  Read from the output port and the input will pullData.
 *  It is blocking on the caller.  If m_PushLocked is set the
 *  thread space lock will be obtained (default).  
 *   
 *  This will behave in a hopefully smart way when push or pull 
 *  are called on its ports.
 *
 *  Calling pull will do the normal thing of registering the PullClient.
 *  It will also set a flag which on the next read on the out port will
 *  cause the ThreadSwitcher to check if the PullClient isThreadSafe.
 *  This check will be done with locking of course.  If the PullClient
 *  is set up to be threadSafe this will not bother with locking when
 *  pulling data from it.  This check is done one time only.
 *  
 *  The same is done on the push/write calls but then all the PushClients 
 *  need to be thread safe. (all or nothing). 
 *
 *
 * @author John Folkesson 
 */
class ThreadSwitcher: public DataFilter
{
public:

protected:

  bool m_PushLocked;
  bool m_PullLocked;
  bool m_PullCalled;
  bool m_PushCalled;
public:
  /**
   * Constructer is called with the threads to use.
    * @param threads this is an array length 2  that
   *        gives the thread for the in port and  out port in order
   */
  ThreadSwitcher(unsigned short threads[2]);
  ThreadSwitcher(unsigned short threadin, unsigned short threadout);
 
  ~ThreadSwitcher();

  /**
   * Calling Write causes the data to be pushed out 
   * of the out port.
   * 
   * 
   *
   * @param p Data to be copied
   * @param port must be an input port number
   * @return 0 ok.
   */
  virtual unsigned short write( Cure::TimestampedData& p,const int port=-1){
    if (port!=0)return ADDRESS_INVALID;
      if (m_PushCalled)setPushLocked();
      if (m_PushLocked){
	return Ports[1].pushDataLocked(p);  
      }  
    else return Ports[1].pushData(p);    
    return   0;
  }
 
  /**
   * Calling read will pull data from the in port.
   *
   * @param result Data to be copied into
   * @param port must be an output port number
   * @param t requested time will be interpreted based on interpolate 
   * @param interpolate flag for requesting interpolation to t.
   * @return 0 if input data read ok and calc ok,
   *    else ADDRESS_INVALID if not an output port.
   *    if calc not done due to 
   *    (TYPE_ERROR|ADDRESS_INVALID)|RESOURCE_ERROR) return RESOURCE_ERROR 
   *    or else return an indication Of the reason.
   * 
   */    
  virtual unsigned short read(Cure::TimestampedData& result,const int port, 
			      const Cure::Timestamp t=0,
			      const int interpolate=0){
    if (port!=1)return ADDRESS_INVALID;
    if (Ports[0].IsPull){
      if (m_PullCalled)setPullLocked();
      if (m_PullLocked)
	return Ports[0].pullDataLocked(result,t, interpolate);
      else  return Ports[0].pullData(result,t, interpolate);
    }
    return RESOURCE_ERROR;
  }


  /**
   * This will check the PushClients to see if locks are needed.
   *
   */
  bool setPushLocked();
  bool setPullLocked();    
  /**
   *  give permission to the DataFilterAddress 'port' to (pull) data to pa.
   *  This is called from the Ports on pull. 
   * @param port the port number    
   * @return true if ok, else false. 
   */
  bool canPull(const int port);

  /**
   *  give permission to DataFilterAddress 'port' to (push) data to pa.
   *  This is called from the Ports on push. 
   *
   * @param port the port number    
   * @return true if ok, else false. 
   */
  bool canPush(const int port);
protected:
private:
  /**
   * This returns a reference to the TimestampedData object associated with 
   * this port.  There is none So make sure to not let parent classes call.
   */
  virtual Cure::TimestampedData * data(int){ return 0;};

};
  
} // namespace Cure

#endif 
