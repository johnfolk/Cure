//
// = FILENAME
//    AddressMaker.hh
//
// = FUNCTION
//
// = AUTHOR(S)
//    John Folkesson
//
// = COPYRIGHT
//    Copyright (c) 2006 John folkesson
//
/*----------------------------------------------------------------------*/

#ifndef Cure_AddressMaker_hh
#define Cure_AddressMaker_hh

#include "FileAddress.hh"
#include "DebugAddress.hh"
#include "FilterBankAddress.hh"
#include "DataSlotAddress.hh"
#include "DeviceAddress.hh"
#include "ServiceAddress.hh"
#include "DataFilter.hh"
#include "ThreadSwitcher.hh"


namespace Cure{

  class ConfigFileReader;
/**
 * An AddressMaker provides a way to make the FileAddresses, DataFilters, 
 * FilterBanks DebugAddress and DataSlots from a ccf file.
 * In order for it to make DataFilters that it doesn't know about 
 * (all except ThreadMerger) one has to create a subclass and override
 * the method makeFilterSubClass(...).
 *
 * The subclasses of this class are easy to write. Just write how to do the
 * constructors for the objects of your app.  
 * 
 * The AddressMaker Base class will then be able to read the scripts
 * of ccf file and make your app.  It creates the various Devices,
 * Files, Filters, Services, FilterbankAddresses, Debug...
 *
 * It configures them from the file.
 *
 * It hooks them up even doing the thread protection for you.
 *
 * It starts the devices.
 *
 * It stops the devices
 *
 * It deletes the whole thing.
 *
 * You can ask it for any address by name.
 *
 *
 * @author John Folkesson
 */
  class AddressMaker{

 public:
    
    ServiceAddress **m_Services;
    unsigned short m_NumberServices;
    ConfigFileReader *m_Cfg;
    std::string m_Configfile;
    FileAddress **m_Files;
    unsigned short m_NumberFiles;
    DebugAddress **m_Debugs;
    unsigned short m_NumberDebugs;
    FilterBankAddress **m_Banks;
    unsigned short m_NumberBanks;
    DataSlotAddress **m_Slots;
    unsigned short m_NumberSlots;
    DataFilter **m_Filters;
    unsigned short m_NumberFilters;
    Hal::DeviceAddress **m_Devices;
    short m_NumberDevices;
  protected:
    
  public:
    
  /**
   * Constructor
   *
   * 
   */
    AddressMaker();
  
    /**
   * Destructor
   */
    virtual ~AddressMaker();
  /**
   * Call at start of process to make multithreading safe if you care.
   * If you only make address at the start and delete them at the end
   * you don't care.  Having one thread add/remove addresses while
   * another thread is working with some addresses is the danger. 
   * Thread safe causes a brief mutex lock during each read and write. 
   * That assures that the Address bank works correctly.  You
   * still might do something dumb and cause problems. Like
   * writing to a Filter whie another thread is working with it.
   * That is what the ThreadMerger is for.
   *
   * param p true for multithreading false to turn off protection.
   */
    //void threadSafe(bool p=true){
    //  AddressBank::theAddressBank().threadSafe(p);
    //}      
    int open(const std::string &cfgFile);
    int checkDescriptors(const std::string &cfgFile);
    virtual int configure(const std::string &cfgFile);
     virtual void disableAll();
    virtual void deleteAll();
    /**
     * This Finds a FileAddress, Slot Bank or Debug Address with a 
     * name. 
     */
    virtual Address * operator()(const std::string dataname);

    /**
     * This Finds a [FilterBank, name] or [FilterName, dataname].
     * In the bank case it returns a DataPort type address and in the 
     * case of a DataFilter it returns a DataFilterAddress.
     * The data name is the same as the DataDescriptor.name 
     * asssociated with this address, as specified in the ccf.
     * 
     */
    virtual Address * operator()(const std::string objectname,
			 const std::string dataname);


    /*
     * This will find the m_Descriptor for the SERVICEADDRESS in cfgFile.
     * Call this to make the Service addresses then you can access them 
     * from 
     *
     * @return number of files made else 0.
     */
    virtual short makeServices(const std::string &cfgFile);

    /*
     * This will find the m_Descriptor for the FILEADDRESS in cfgFile.
     * Call this to make the file addresses then you can access them 
     * from 
     *
     * @return number of files made else 0.
     */
    virtual short makeFiles(const std::string &cfgFile);

    /*
     * This will find the m_Descriptor for the DEBUGADDRESS in cfgFile.
     * Call this to make the debug addresses then you can access them 
     * from 
     *
     * @return number of files made else 0.
     */
    virtual short makeDebugs(const std::string &cfgFile);
   
    /*
     * This will find the m_Descriptor for the FILTERBANKADDRESS in cfgFile.
     * Call this to make the addresses then you can access them 
     * from 
     *
     * @return number of banks made else 0.
     */
    virtual short makeBanks(const std::string &cfgFile);
    /*
     * This will find the m_Descriptor for the FILTERBANKADDRESS in cfgFile.
     * Call this to make the addresses then you can access them 
     * from 
     *
     * @return number of slots made else 0.
     */
    virtual short makeSlots(const std::string &cfgFile);
    
    /*
     * This will find the m_Descriptor for the DATAFILTER in cfgFile.
     * Call this to make the addresses then you can access them 
     * from 
     *
     * @return number of filters made else 0.
     */
    virtual short makeFilters(const std::string &cfgFile);
    
    /**
     * This needs to be overwritten in a subclass that can 
     * make more DataFilters than this class knows about.
     * We only make a ThreadMerger here to show principle.
     *
     */
    virtual DataFilter * makeFilterSubClass(const std::string classname,
					    const std::string constargs,
					    unsigned short thread=0);

    Hal::DeviceAddress * getDevice(short i){
      if (i<m_NumberDevices)return m_Devices[i];
      return 0;
    }
    Hal::DeviceAddress * getDevice(const std::string dataname){
      for (short i=0;i<m_NumberDevices;i++)
	if ((*m_Devices[i])==dataname)return m_Devices[i];
      return 0;

    }
    DataFilter * getFilter(const std::string dataname){
      for (short i=0;i<m_NumberFilters;i++)
	if ((*m_Filters[i])==dataname)return m_Filters[i];
      return 0;

    }
    /**
     * This will call startDevice on each devices in turn and in the 
     * order they were made in (ie. the order in the DEVICEADDRESS 
     * section fo the ccf file.
     * @return the number successfully started
     */
    short startDevices();
    /**
     * This will call stopDevice on each devices in turn and in the 
     * reverse order to the way startDevices would start them.
     * @see AddressMaker::startDevice()
     * @return the number successfully stopped
     */
    short stopDevices();
    /*
     * This will find the m_Descriptor for this devicename in cfgFile.
     * It will then set the rest of the config by calling config.
     * @return 1 if fail else 0.
     */
    virtual int makeDevices(const std::string &cfgFile);
    /**Return 1 if device made else 0*/
    virtual Hal::DeviceAddress * 
    makeDeviceSubClass(const std::string classname,
		       std::string constargs,
		       unsigned short thread=0);

    virtual ServiceAddress * makeServiceSubClass(const std::string classname,
						 std::string constargs,
						 unsigned short thread=0);

    ThreadSwitcher* addSwitcher(unsigned short threads[2]);
    
    
    unsigned short read(const std::string fromobjname,
			const std::string fdataname,
			TimestampedData& result, 
			const Timestamp t=0, 
			const int interpolate=0)
    { 
      Address *a=(*this)(fromobjname,fdataname);
      if (!a)return ADDRESS_INVALID;
      return AddressBank::theAddressBank()
	.readLocked(a->Thread,a->Index,result,t,interpolate);
    }
    
    unsigned short write(const std::string tromobjname,
			 const std::string tdataname,
			 TimestampedData& result)
    { 
      Address *a=(*this)(tromobjname,tdataname);
      if (!a)return ADDRESS_INVALID;
      return AddressBank::theAddressBank()
	.writeLocked(a->Thread,a->Index,result);
    }
    unsigned short push(const std::string fromobjname,
			const std::string fdataname,
			const std::string toobjname,
			const std::string tdataname);

    unsigned short pull(const std::string fromobjname,
			const std::string fdataname,
			const std::string toobjname,
			const std::string tdataname);

    /**
     * This calls Push and Pull as specified in cfgFile.  If you
     * accidentally try to hookup from one thread to a different one
     * this will automatically insert a ThreadSwitcher for you.
     * @param ccfFile The config file with the hookup script.
     */

    int hookup(const std::string &ccfFile);



};
  /**
   * This sets up the AddressBank thread space if it hasn't already been done.
   * This calls Cure::initAddressBank(ConfigFileReader &cfg).
   *
   * @param cfgfile the file name that has the THREADSPACES depths in it
   * @return -1 if  file no good else 0
   */
  int initAddressBank(const std::string &cfgFile);
  /**
   * This sets up the AddressBank thread space if it hasn't already
   * been done.  It should be called before any other cure thing is
   * done with addresses.  The config file shoule have a section
   * THREADSPACES under which is a list of integers that
   * give the depths of each address space.  The depth is an exponent
   * of 2 so the size of the space is 2^depth.  The number of
   * addresses to be put in each space should be about equal to (more
   * or less both ok) the size of the space.
   *
   * @param cfg this should have its config file opened.  
   * @return 0 of ok else 1 if no depths found;
   */
  int initAddressBank(ConfigFileReader &cfg);
} // namespace Cure


#endif // Cure_FileAddressMaker_hh
