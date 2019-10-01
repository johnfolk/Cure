//    $Id: DataFilter.hh,v 1.25 2008/05/05 16:55:27 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson, Patric Jensfelt
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_DATAFILTER_HH
#define CURE_DATAFILTER_HH

#include "AddressBank.hh"
#include "Address.hh"
#include "TimestampedData.hh"

#include <iostream>
#include <string>

namespace Cure{

// Forward declarations
class DataFilterAddress;
class CommandAddress;
class FilterTriggerAddress;
class ConfigFileReader;

/**
 * DataFilter is a processor of TimestampedData. 
 * It has input and output ports that can be 
 * connected to 'Address's that can be connected to other ports... 
 * Either in other DataFilters or DataSlots.  
 *
 * Normally each input needs an Address for pulling input data 
 * from (read).  These are set up by calling in(#).pull(pa).
 * Alternatively or additionally, there can be push Address's pointing 
 * to the input port as well (write), these are not known by the Filter.
 * 
 * Normally each output can have an Address for pushing Pose output data 
 * to (write).  There can be pull DataFilterAddress's pointing to the output 
 * port as well (read), these are not known by the Filter.
 * 
 * Writing to input ports and reading from output ports normally causes 
 * the other input ports to be read and the calc() done.
 *  
 * On writing to input ports the results are normally written (pushed) 
 * to the output ports.
 * 
 * Writing any kind of TimestamedData to trigger input will trigger 
 * all inputs to be pulled (read), the calc done and then the outputs 
 * pushed (write).
 *
 * On reading from output ports the results are normally only passed 
 * to the DataFilterAddress that called the read, (no push).
 *
 * If reading or writing to a subscriber (push or pull) 
 * returns ADDRESS_INVALID then the subscriber is disconnected.
 *
 *
 * Each output port can have any number of push Address's connected to it.  
 * Each input port can have at most one pull Address.  If the input data
 * is needed there must either be a pull address or the data must be written 
 * to the port directly.  In ports can be configured to need data or not
 * based on whether they are essential to the filter calc or not.  The
 * Ins can also be set to pull data in one of three ways.  Interpolated to
 * an exact timestamp, sequentially reading the data in order without
 * skipping any data, or just simply reading the latest data from the 
 * pull source.  The in port can also be set to care or not about 
 * the NO_INTERPOLATE flag.
 *
 *  Error codes are associated with the data passed.  So that a read on the 
 *  output will get an error code that is informative about the data read.
 *  A write will get a more general information on how the calculation went.
 *  that might be 0 all ok or some problem that caused the calc to not be 
 *  done.  
 * 
 * 
 *
 * @author John Folkesson, Patric Jensfelt
 */
class DataFilter
{
friend class DataFilterAddress;

protected:

  /** 
   * The Ports/Addresses that handles the inputs and outputs of the filter
   * Users should use the in, out and trigger fuctions to get at these.
   */
  Cure::DataFilterAddress *Ports;


  /**
   * Checked is a counter, one per Input port. The counter is set to 5
   * initially, which when it reaches zero suspends most type checking on
   * that port.  This saves time as the type on a pull address is
   * probably not changing. This counter is reset when a new pull is
   * called.
   */
  short *Checked;


public:
  /** 
   * Variable that is used for warning and error printouts. A
   * subclass can set this to distinguish itself from the base class
   */
  std::string FilterName;
  unsigned short Thread;

  
  /** Number of inputs */
  short Ins;

  /** Number of outputs */
  short Outs;

  /** True if this filter has a special trigger port */
  bool HasTriggerPort;
  

  /** 
   * @return numer of inputs
   */
  short getNumInputs() const { return Ins; }

  /** 
   * @return numer of outputs
   */
  short getNumOutputs() const { return Outs; }

  /** 
   * @return true if this filter has a trigger port
   */
  bool hasTriggerPort() const { return HasTriggerPort; }

  /**
   * Get a pointer to a certain input port
   *
   * @param num input port number (0 to getNumInputs()-1)
   * @return 0 if invalid port.
   */
  DataFilterAddress* in(short port = 0);

  /**
   * Get a pointer to a certain output port
   *
   * @param num output port number (0 to getNumOutputs()-1)
  * @return 0 if invalid port.
   */
  DataFilterAddress* out(short port = 0);

  /** Match the FilterName. */
  bool operator==(const std::string &name){return (FilterName==name);}

  /** get a named port. */
  DataFilterAddress * operator()(const std::string dataname);




  /** 
   * Get a pointer to the trigger Port if there is any for this filter
  * @return 0 if ther is none.
   */
  DataFilterAddress* trigger();
  /**If a CommandAddress is added to a subclass this can return it*/
  virtual CommandAddress *command(){return 0;}
  
  /** The existance of read errors is indicated here. */ 
  unsigned short ErrorCode;
  

protected:
  virtual void disconnectAddresses();
  /**
   * Function is called just before calc() and is intended to be overloaded 
   */
  virtual void precalc(int port) {
    if (port)return;
  }
  
public:
  /** 
   * Constructor that wants to know how many input and output ports
   * the filter has.
   *   
   * @param nIn number of input ports
   * @param nOut number of output ports
   * @param addTriggerPort true if you want to add a special Port[-1] 
   * for trigger purposes
   */
  DataFilter( short nIn, short nOut, bool addTriggerPort,
	      const unsigned short thread=0);

  /** 
   * Destructor that frees the resources that was alloctaed in the
   * constructor.  It will getThread and change all the Ports
   * to the Filter's Thread before deleting as needed (only if 
   * Ports are not on same thread as filter.)
   *
   * NOTE: If you overlaod this class you need to be careful not to
   * try to delete any resources that have already been deleted.
   */
  virtual ~DataFilter();
  void disable(){disconnectAddresses();}
  /** Depreciated call initPorts intead*/
  //  void setThread(const unsigned short thread);
  /**
   * this sets up the ports to operate in other thread spaces than that
   * of the Filter.
   * This will get the locks except the Filter.Thread lock and set the
   * thread index for all the Ports.  The use of this method is in the
   * constructor of the filter subclass.  Best is to set up all your
   * ports there if you want them on different threads.  So first call
   * DataFilter::DataFilter(nIn,nOut,addTrigger,thread) from the
   * subclass constructor.  Then all the ports are on thread and any
   * changes like PullExact .... can be made.  Then call this to
   * change the threads of the Ports at the end of the constructor.
   * Alternatively one can wait to call this untill config is done.
   * Then the last line of the config file section should case this to
   * be called.  That allows config of the Ports in the Filter's
   * Thread.
   * 
   * @param threads an array of length Ins+Outs that holds the thread
   *        indecies for all the Ports
   * @param triggerthread (defalut =Thread) the thread indes for the trigger
   *
   */
  void initPorts(unsigned short *threads, short triggerthread=-1);
  /*
   * This will find the m_Descriptors for this filstername in cfgFile.
   * It will then set the rest of the config.
   * @return 1 if fail else 0.
   */
  virtual int configure(ConfigFileReader &cfg, std::string  &filtername);
  
  /** Override in subcall to set up config.*/
  virtual int config(const std::string &){return 0;} 
  /**
   * This is overwritten in subclass to confgure global data with a
   * string name using state data
   */
  virtual int configGlobal(const std::string &,
			   StateData &){return 0;} 
  /**
   * This is overwritten in subclass to confgure sensor data with a
   * string name using values stored in a string
   */
  virtual int configSensor(const std::string &,
			   const std::string &){return 0;} 
  /** copy the descriptor from port index to des*/
  virtual int getDescriptor(short index, DataDescriptor &des);
  
  /**  @return -1 if no match else the port number of the matching in port*/
  virtual int matchInputDescriptor(DataDescriptor &des);
  

  /**
   * Calling Write normally causes the Filter to gather its inputs 
   * (other than the input given with the write (specified by 'port'))
   * Then calculate (calc()) and write to its outputs.
   * 
   * port=-1 means you want the Filter to gather all inputs, (ie a 
   * trigger of the calc.
   * 
   * The Timestamp for the Filter calc will be set to p->Time.
   * 
   * If this is what you want you can just write code for calc() 
   * in a subclass to fillin the actual functionality.
   *
   * @param p Data to be copied
   * @param port must be an input port number
   * @return 0 if input data read ok and calc ok,
   *    else RESOURCE_ERROR for (TYPE_ERROR|ADDRESS_INVALID)|RESOURCE_ERROR)
   *     else it returns any calc error without setting outputs.   
   * 
   */
  virtual unsigned short write( Cure::TimestampedData& p,const int port=-1);
  
  /**
   * A Read normally gathers inputs and calculates but returns the 
   * result in result rather than calling a write on its outputs.
   *
   * In the case that the requested timestamp matches the timestamp
   * already in the port's pose (and interpolate is 1) then the 
   * data is simply returned without any other side effect.
   *
   * If this is what you want you can just write code for calc() 
   * in a subclass to fillin the actual functionality.
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
			      const int interpolate=0);

  /**
   *  give permission to the DataFilterAddress 'port' to (pull) data to pa.
   *  This is called from the Ports on pull.  So it can be called
   *  from another thread if Porst are on different threads.  
   * @param port the port number    
   * @return true if ok, else false. 
   */
  virtual bool canPull(const int port){
    if ((port<Ins)&&(port>-1))
      return true;
    return false;
  }

  /**
   *  give permission to DataFilterAddress 'port' to (push) data to pa.
   *  This is called from the Ports on push.  So it can be called
   *  from another thread if Porst are on different threads.  
   *
   * @param port the port number    
   * @return true if ok, else false. 
   */
  virtual bool canPush(const int port){
    if ((port<(Outs+Ins))&&(port>(Ins-1)))
      return true;
    return false;
  }

  /**
   * Calling this gathers the inputs.
   *
   * Normally the default behaviour is what you want but one can override 
   * this. 
   * The port is the number of the port that will not be read as 
   * its data is already in inputData.  Instead p is copied to the port's. 
   * data object.  If port<0 then all inputs are read.
   *
   * inputData's timestamp is used to do all the pullData calls to
   * the input ports. 
   * 
   * Return TYPE_ERROR will abort but this should never happen normally,
   * (ie there is a bug if this happens)
   *
   * @param port the port number. 
   * @return 0 if ok, else ADDRESS_INVALID, TYPE_ERROR or TIMESTAMP_ERROR. 
   */
  virtual unsigned short getInputs(TimestampedData &inputData,int port=-1);

  /**
   * Calling this writes all the outputs.
   * Normally the default behaviour is what you want but one  can override 
   * this. 
   * @return 0 if ok, else ADDRESS_INVALID, TIMESTAMP_ERROR
   */
  virtual unsigned short setOutputs();

  /**
   * This is the function of the filter. 
   * This is called after gathering the inputs and before writing the outputs.
   * By overiding this in a subclass one can have the filter do something 
   * useful. 
   *
   */
  virtual unsigned short calc(){return 0;}


  /**
   * This returns a reference to the Timestamped associated with 
   * this port.
   */
  virtual Cure::Timestamp & time(int port)
  {
    return data(port)->Time;
  }

  /**
   * This returns a reference to the TimestampedData object associated with 
   * this port.
   */
  virtual Cure::TimestampedData * data(int port) = 0;

  /**
   * This is overridden to make sure p gets as much data as it can.
   */
  virtual  void setEqual(int port,Cure::TimestampedData &p)
  {
    data(port)->setEqual(p);
    //    p=*data(port);
  }

  /**
   * A conditional set of the value of an input ports data object.
   * The data object is the working data for doing the calc step.
   * This will set *data(port)=inputData if inputData is the right type.
   * The default right type is anything at all.
   * This will be called after data is written or read to the input port to
   * check the type data written to the port.
   *
   * This is overridden to make sure inputData is copied to correctly to 
   * the ports data object and gets as much of the data as possible.
   *
   * Some filters will not check anything.
   * 
   * If inputData is the wrong type for this port this function should return
   * TYPE_ERROR.
   *
   *
   * If inputData==0 then data(port) is checked.
   * 
   * @param port must be an input port number or -1.  
   * @param inputData This is the data writen to the port but not yet 
   * copied to data(port). 
   * @return  0 if ok else TYPE_ERROR  
   */
  virtual unsigned short typeCheck(int port,TimestampedData *inputData=0){
    if (inputData)
      {
      SmartData *sd=inputData->narrowSmartData();
      if (sd){ 	   
	sd->setEqual(*data(port));
      } else if (inputData->isPacked()){
	inputData->narrowPackedData()->unpack(*data(port));
      } else *data(port)=*inputData;   
      }
    return 0;
  }
  /** 
   * This is a method that is not called by the base class but 
   * can be used by a subclass as an override to typeCheck that
   * is a little smarter. 
   * This will copy the
   * input data it data(port) if the class matches the Ports
   * m_Descriptor.
   * 
   * @param port must be an input port number or -1.  
   * @param inputData This is the data writen to the port but not yet 
   * copied to data(port). 
   * @return  0 if ok else TYPE_ERROR  
   */
  unsigned short classCheck(int port,TimestampedData * inputData);


  /** 
   * This is a method that is not called by the base class but 
   * can be used by a subclass as an override to typeCheck that
   * is a little smarter.  It does a check of the full descriptor
   * if the ID of the Ports descriptor!=0  and consideres
   * more situations like writing SmartData.  This will copy the
   * input data it data(port) if the Descriptor matches the Ports
   * m_Descriptor or the Portsdescriptor has 0 ID.
   * 
   * @param port must be an input port number or -1.  
   * @param inputData This is the data writen to the port but not yet 
   * copied to data(port). 
   * @return  0 if ok else TYPE_ERROR  
   */
  unsigned short matchWild(int port,TimestampedData * inputData);

  virtual void printConfig();


  /**
   * gets a named ServiceAddress's index and .
   * @param service The name of the service
   * @param index the index is returned here
   * @return true if found else false
   */
  bool getService(const std::string &service, unsigned long &index)
  {
    return AddressBank::theAddressBank().getService(Thread,service,index);
  }
  
  unsigned short readData(unsigned long &index, 
				    Cure::TimestampedData &p,
				    const Timestamp t=0, 
				    const int interpolate=0)
  {
    return AddressBank::theAddressBank()
      .read(Thread,index,p,t,interpolate);
  }

  unsigned short writeData(unsigned long &index, 
				       Cure::TimestampedData &p)
				
  {
    return AddressBank::theAddressBank()
      .write(Thread,index,p);
  }
  
  /**
   * Finds the service by name if it exists in the thread space.
   * Then callse read on it with the params here.
   * @param index the index to the service if known
   *        if 0 the index will be found and returned here.
   * @return -1 if fail else the return from read 
   */
  int readService(const std::string &name,
		  unsigned long &index,
		  TimestampedData &td,
		  const Timestamp t=0,
		  const int interpolate=0)
  {
    if (!index)
      if (!getService(name,index) )
	{
	  index=0;
	  return -1;;
	}    
    int r = readData(index,td,t,interpolate);
    if (r&ADDRESS_INVALID)index=0;
    return r;
  }
  /**
   * Finds the service by name if it exists in the thread space.
   * Then calls write on it with the params here.
   * @param index the index to the service if known
   *        if 0 the index will be found and returned here.
   * @return -1 if fail else the return from write 
   */
  int writeService(const std::string &name,
		   unsigned long &index,
		   TimestampedData &td)
  {
    if (!index)
      if (!getService(name,index) )
	{
	  index=0;
	  return -1;;
	}    
    int r =writeData(index,td);
    if (r&ADDRESS_INVALID)index=0;
    return r;
  }
};
  
} // namespace Cure

#endif // CURE_DATAFILTER_HH
