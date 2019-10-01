//
// = FILENAME
//    DeviceAddress.hh
//
// = FUNCTION
//
// = AUTHOR(S)
//    Patric Jensfelt
//
// = COPYRIGHT
//    Copyright (c) 2005 Patric Jensfelt
//
/*----------------------------------------------------------------------*/

#ifndef Cure_DeviceAddress_hh
#define Cure_DeviceAddress_hh

#include "Address.hh"
#include "TimestampedData.hh"
#include "CommandExecuter.hh"
#include "FilterBankAddress.hh"

namespace Cure{
  namespace Hal {

    /**
     * A DeviceAddress provides an Address interface to a device
     *
     * @author Patric Jensfelt
     */
    class DeviceAddress: public Cure::Address,
			 public Cure::CommandExecuter 
    {
    public:
  
      /**
       * Name of the device
       */
      std::string m_DeviceName;
      std::string m_CannedDataFileName;
      /** 
       * Number of push clients that are subscriber to the the Port's data output. 
       */
      short m_NumPushClients;
  
      /** 
       * This address needs lots of descriptors as it can produce a lot.
       * These allow names on the output data.
       * By matching up input to output descriptors one can runtime change
       * filter hookups ect.  
       */
      DataDescriptor *m_OutputDescriptors;
      short m_NumberDescriptors;  

    protected:

      /** 
       * Array of indices to client that want data 
       */
      unsigned long *m_PushClients;
    
      /** Space allocated for push client indices, might not all be used */
      short m_NumPushClientsAlloc;
  
      virtual void disconnectAddress();
    public:
    
      /**
       * Constructor
       *
       * @param name name of the device (just for debug messages)
       */
      DeviceAddress(const std::string &name,
		    unsigned short numofdescriptors=0,
		    const unsigned short thread=0);

      /**
       * Destructor
       */
      virtual ~DeviceAddress();

      /**   
       * Use this function to configure this device before starting to use
       * it.
       *
       * @param str configuration string that could have come from a configfile. 
       * @return 0 if OK, else error code
       */
      virtual unsigned short configDevice(const std::string &str) = 0;

      /*
       * This will find the m_OutputDescriptors for this devicename in cfgFile.
       * It will then set the rest of the config by calling config.
       * @return 1 if fail else 0.
       */
      virtual int configure(const std::string &cfgFile, 
			    const std::string  &devicename);
  
      /**
       * This is overwritten in subclass to confgure global data with a
       * string name using state data
       */
      virtual int configGlobal(const std::string &,
			       StateData &){return 0;} 
  
      /**
       * This will add DataPorts to fb for all of the Device's Descriptors.
       * If the data then is written to the bank it will get parsed and buffered.
       * That works even with a ThreadMerger in between.
       */
      void addToBank(Cure::FilterBankAddress &fb,long buffersize=10);

      bool operator==(const std::string &name){return (m_DeviceName==name);}
      DeviceAddress * operator()(const std::string dataname);

      /** get a pointer into the descriptor array and the length*/
      virtual void getDescriptors(short &numberofdescriptors, DataDescriptor *des){
	des=m_OutputDescriptors;
	numberofdescriptors=m_NumberDescriptors;
      }
      /**
       * Connect and start the device. 
       */
      virtual unsigned short startDevice() = 0;
    
      virtual unsigned short runEventLoop() = 0;
    
      /**
       * Stop the loop that is running to service the device and stop the
       * device.
       */
      virtual unsigned short stopDevice() = 0;
    
    
      /**
       * Called when there is data from the device to read. Normally this
       * would be called from inside the startDevice function but in cases
       * where you have several devices in the same thread and some other
       * mechanism to determine who got data you can call this function to
       * handle the data
       */
      virtual unsigned short handleData() { return 0; }

      /**
       * Write to the Address
       *
       * @param p data to be copied
       */
      virtual unsigned short write(TimestampedData & p);

 
  
      /**
       * Use this function to register a client address to be pushed to
       */
      unsigned short push(Address *pa);

      /**
       * This is called by the filter to push the output.
       * @param tp the data to write to push clients.
       * @return 0 allways 
       */
      unsigned short pushData(TimestampedData &tp);

      /**
       * This is called by the filter to push the output.
       * This shoule be used by devices that are pushing in a space
       * that other threads may be using.  If it is alone then
       * pushData is more efficient.
       * @param tp the data to write to push clients.
       * @return 0 allways 
       */
      unsigned short pushDataLocked(TimestampedData &tp);


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
       * This opens a file to replace the live data with.  
       * 
       * @param filename wonder what that is.
       *
       * @return 1 if fail else 0
       */
      virtual int openCannedDataFile(std::string filename);

      /**
       *
       * This closes the file that replaced the live data.
       */
      virtual void closeCannedDataFile();

      /** 
       * This will read from file and push the data out.
       * 
       * @param stoptime the device will read and push data until the stop
       * time.  The time of the last data read will then be returned in
       * stoptime.  If the stoptime =0 when calling only one line of data
       * will be read.
       * 
       * @return 1 if file no good. 
       */
      virtual int readCannedDataFile(Cure::Timestamp &stoptime);
  
      /**
       * This is called from the default readfile to parse a file format
       * and then set up normal handing of data as if from the device.
       *
       * param  a single line read from the m_CanDataFile
       * 
       * @return 0 if ok;
       */
      virtual int  parse(std::string &){return 0;} 
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
      /**
       * 
       * The timestamp of the last data input from device or file shouldbe
       * set here
       */ 
      Timestamp m_LastDataTime;
  
      /** This is the fake device being read from a file*/
      std::ifstream m_CannedDataFile;

    };

  } // namespace Cure
} // namespace Hal

#endif // Cure_DeviceAddress_hh
