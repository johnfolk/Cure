//    $Id: Syncronizer.hh,v 1.6 2007/09/14 09:13:51 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_SYNCRONIZER_HH
#define CURE_SYNCRONIZER_HH

#include "SmartDataList.hh"
#include "DataFilter.hh"
namespace Cure{
  
/**
 * This class has the function of syncronizing Addresses.
 * 
 * It has N inputs, (ports 0...N-1), N outputs, (ports N...2N-1)
 * and a Trigger input (port -1).
 *
 * The filter acts as a triggered/sync. It will use the 
 * written timestamps to synchronous to.  So if one writes at times 
 *  1, 2, 5, 23, ... The outputs will be interpolated to those times
 * It will remember the trigger data if data is not currently available
 *  and try again on the next trigger/write signal.
 * It remebers the Port that the trigger signal came from and the 
 * data written.  It will eventually do a Push for each trigger written to 
 * any input in the order they were written.
 * If the trigger signal was written to Port -1 all inputs
 * are read and interpolated.  If that produces a TIMESTAMP_ERROR the 
 * triggering  write returns and the trigger signal is saved.
 * When the next trigger signal comes the earliest saved trigger is tried 
 * then if that works it is pushed out and the next trigger is tried...
 *
 * If the trigger signal was written to an input port, that port will not be  
 * read when tring to sync the other inputs.  Instead the trigger signal
 * data will be used.
 *
 * So if any read returns TIMESTAMP_ERROR then nothing more is done until
 * the next trigger/write comes. 
 *
 * Reading the outputs will only return the data stored there
 *(the last successfully interpolated set of inputs) and not cause
 * any calculation.
 * The idea is to wait until there is new data on all ports before 
 * writing to the output.
 *
 * The inputs must be set to pull from an address that can do meaningfull
 * interpolation such as a PoseSlotAddress.  Some addressed objects ignore the
 * interpolate flag and some data classes can't be interpolated. 
 * 
 * If all triggers are written to one of the inputs then that input will 
 * never need to pull (interpolate) data. 
 *
 * The filter does not care what classes are being read and written to it
 * and no initialization of ClassType ect. need be done.
 *
 * When triggering a synch from the trigger port the inputs are
 * typically pulled from slots. If it happens that the trigger signal
 * falls behind the data in the slots the Synchronizer will get stuck
 * and never be able to produce any output as it cannot get data on
 * all inputs. As a counter measure for this you can specify a maximum
 * number of signals that are saved to try later
 * (setMaxNumStoredSignals()). You can also configure the synchronizer
 * not to store any signals that resulted in a failure to read
 * (setStoreSignals()). The default is to store an infinite number of
 * signals.
 *
 * SUMMARY OF USE 
 *
 * The Syncronizer is to regulate the flow of data.  We need to wait
 * until there is new data on all input ports.  If the data has not
 * come yet on all inputs (we try to interpolate to the time) then the
 * Trigger is saved on a pending list.  On the next write the earliest
 * pending data will be tried first then the next until we don't have
 * all inputs interpolate succesfully (TIMESTAMP_ERROR) or the list is
 * empty.
 *   
 * If the inputs are slots the slots will interpolate the data to the
 * timestamps.  When they return TIMESTAMP_ERROR, write returns.
 * 
 * We tell the constructor that we need to sync n input ports.
 *
 * Syncronizer sync(n);
 *
 * The i:th input gets sent to the (i + n):th output. 
 *
 * Hooking up the In ports:
 * The trigger data should be pushed to some input (or -1), 
 * and the other ports need to know where to pull data from. 
 * Sometimes one wants to trigger with a signal that you don't
 * need an output for.  That is what port -1 is for.
 *
 * EXAMPLE: 2 inputs to synchronize, Port 0 is the trigger
 * 
 * The trigger will be pushed from someAddress.  
 * This data will be written to the Port 0  when the owner of someAddress
 * decides to push. 
 *
 * someAddress.push(&sync.Ports[0]); 
 *
 * The sync then needs to know where to pull the other data from,
 * i.e. the data that we want to sync time agsinst what we write on
 * Port 0.
 *
 * sync.pull(someOtherAddress,1);
 *
 * Here is another way to connect to a filter port. 
 *
 * sync.Ports[1].pull(&someThirdAddress);   
 * 
 *
 * Next you need to tell where to push the synched output. The same
 * type of data written to input i will be output on Port (Ins+i)
 * where Ins is the number of input Ports.
 *
 * Let destintionAddress0 and destintionAddress1 be the destination
 * addresses for the data
 *
 * sync.push(destintionAddress0,sync.Ins+0);
 * sync.push(destintionAddress1,sync.Ins+1);
 *
 * @author John Folkesson
 */
class Syncronizer: public DataFilter
{
public:
  Timestamp TimeInterval;
protected:
  SmartDataList  PendingDataList;
  Timestamp LastTime;
  SmartData * Data;
  SmartData Trigger;
  int N;

  /** 
   * true when you want to store trigger signals that did not lead to
   * any output and try them later. The default is to do so.
   */
  bool m_StoreUnsuccTriggerSignals;

  /** 
   * In some cases you really do not want too many pending data to be
   * stored. If all inputs cannot be read at for a number of
   * iterations it is likely that it will not be read at all after
   * some time simply because the slots from where you read the data
   * will have removed the data all together.
   *
   * <0 means infinite num of data can be stored
   */
  long m_MaxNumPendingData;

public:
  /**
   * 
   * 
   */
  Syncronizer(int numberOfIns);

  virtual ~Syncronizer();

  /**
   * This is the trigger signal to sync to.
   * If the timestamp is older than the last trigger the trigger is
   * ignored. 
   * @param p The trigger data.
   * @param port This port will not be read and p sent to port+Ins.
   * @return 0 if ok, else ADRESS_INVALID, TIMESTAMP_ERROR or RESOURCE_ERROR;
   *                TIMESTAMP_ERROR means the trigger time was too old.
   *                The others indicate errors returned from pull's 
   */
  virtual unsigned short write(TimestampedData& p,const int port=-1);

  /**
   * This overrides the default filter read and instead just returns the 
   * last pose calculated.
   * @param result Pose to be copied into
   * @param port must be an output port number
   * @param t requested time will be ignored
   * @param interpolate flag for requesting interpolation to t.
   * @return 0 if ok, ADRESS_INVALID or TIMESTAMP_ERROR.
  */    
  virtual unsigned short read(TimestampedData & result,const int port, 
			      const Timestamp t=0,
			      const int interpolate=0);


  /**
   * Tell how many signals that did not result in any output that
   * should be stored and tried later. The rsik is that the
   * Syncronizer deadlocks as it wants data that is no longer
   * available.
   *
   * @param n max number of signals to store
   */
  void setMaxNumStoredSignals(long n);

  /**
   * Set behavior when triggering signal did not result in an output
   * because not all inputs could be read. You can choose to store the
   * signal and try with this again the next time something is written
   * to the Syncronizer or you can simply discard it.
   */
  void setStoreSignals(bool v);

protected:
  TimestampedData * data(int port) {
    if (port < -1) 
      std::cerr << "WARNING Synchronizer::data  port " << port
                << " out of bounds\n";
    if (port >= Ins + Outs) 
      std::cerr << "WARNING synchronizer::data port " << port
                << " out of bounds, must be less than "
                << Ins+Outs << std::endl;
    if (port<Ins) return &Data[port];
    return Data[port].getTPointer();
 }
  virtual  void setEqual(int port,Cure::TimestampedData &p){
    Data[port].setEqual(p);
  }

};

inline void
Syncronizer::setMaxNumStoredSignals(long n)
{ 
  m_MaxNumPendingData = n; 
}

inline void
Syncronizer::setStoreSignals(bool v)
{
  m_StoreUnsuccTriggerSignals = v;
}

} // namespace Cure

#endif 
