//    $Id: FilterBankAddress.hh,v 1.10 2008/05/05 16:55:27 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_FILTERBANKADDRESS_HH
#define CURE_FILTERBANKADDRESS_HH

#include "DataPort.hh"
#include "DataFilter.hh"


namespace Cure{

/**
 *
 * A FilterBankAddress is a bank of DataPort objects.  It has the main
 * job of sorting a single stream of diverse data and putting each
 * into the right port where it is buffered and can be set to push to
 * any other Address.  Its a TimestampedData parser.  It parses based
 * on the DataDescriptor given to its DataPorts.
 *
 * The DataPorts have the Name of the DataDescriptor and can be found by name
 * as in FiterBankAddress("PortAliabbaba").
 *
 *  One can call push on the FilerBankAddress itself and get its data
 * pushed ot you on each write.  This is good for logging all data in.
 *
 * The Address can be used to access the stored DataPorts for read and write.
 * The DataDescriptor of the DataPort is matched to the incoming read/write 
 * TimeStampedData argument and the call passed to the matching Port. 
 *
 * If the Port is hidden it can't be found this way.
 *
 * This managed access can be protected by a mutex to prevent two
 * outside threads from entering simultaneously.
 *
 * One can also request the full descriptor of a Named Port by sending this 
 * Address a get Command 
 *
 * One can also get the index of the named Port's Address with a get
 * Command That index can then be used to call push/pull/read/write
 * via the AddressBankAddress in the normal way.  (Note of course that this is
 * not recommended for address outside the FilterBankAddress if multithreading
 * is being used.  It will bypass the thread protection afforded by
 * reading via this Address.)
 *
 * One can have the DataDescriptors read from a *.ccf file and DataPorts 
 * set up automatically.  Then filters can be added.
 *
 * This filters can then be referenced and interconnected to Ports and
 * other Filters by giving the Name and number of the in or out port
 * as applicable.  All this will be specified in the *.ccf file to be run
 * time configurable.
 *
 * It is not possible to pull or push from this address.  Might be
 * implemented later though.
 *
 * @author John Folkesson
 */
class FilterBankAddress: public Address
{
 public:
  /**The Descriptor for this object's Commands*/
  DataDescriptor m_ComDescriptor;
  std::string AddressName;

protected:
  /** 
   * An Array of Filters. 
   */
  DataFilter **Filters;
  /**
   * The Ports 
   */
  DataPort **Ports[10]; 
  /**
   * The size of the Filters Array
   */
  unsigned short SizeFilters;
  /**
   * The number of the Filters elements that are actually pointing to a filter
   */
  unsigned short NumberFilters;
  /**
   * The size of the Ports Arrays
   */
  unsigned short SizePorts[10];
  /**
   * Holds an array of 
   * the sizes of the arrays of DataPort that Ports
   * elements are pointing to.  
   */
  unsigned short NumberPorts[10];
    /** 
   * Number of push clients that are subscriber to the the Port's data output. 
   */
  short NumPushClients;

  
protected:
  /** 
   * Array of indices to client that want data 
   */
  unsigned long *PushClients;
  /** Space allocated for push client indices, might not all be used */
  short NumPushClientsAlloc;

  virtual void disconnectAddress();
public:
  FilterBankAddress(const unsigned short thread=0);
  /**
   * 
   * @param d the descriptor for this Objects Commands
   */
  FilterBankAddress(DataDescriptor &comdes);
  ~FilterBankAddress();

  void setName(const std::string name){AddressName=name;}
  void getName(std::string &name){name=AddressName;}
  bool operator==(const std::string &name){return (AddressName==name);}
  DataPort * operator()(const std::string dataname);
  /**
   * Call write on the correct Port deterimined by the matching
   * the DataDescriptor of p to the Port.
   *
   * @param p data to be copied
   * @return value from it Filter write(...).
   */
  unsigned short write(TimestampedData & p);
  /**
   * Call read  on the correct Port deterimined by the matching
   * the DataDescriptor of p to the Port.
   * Also can send get Commands to this Address here.
   *
   * @param result Data object to be copied into
   * @param t requested time will be interpreted based on interpolate 
   * @param interpolate flag for requesting interpolation to t.
   * @return  value from it Filter read(...).
   */
  unsigned short read(TimestampedData & result, const Timestamp t=0, 
		      const int interpolate=0);

    /**
   * This is called  to push the output.
   * @param tp the data to write to push clients.
   * @return 0 allways 
   */
  unsigned short pushData(TimestampedData &tp);
  

  
  /**
   * Give an Address 'pa' to our bank, which then 
   * will push data to pa on writes.  
   * This can be used to push data writen to the bank to a 
   * file for logging.
   * 
   *   
   *
   * @param pa the Address to receive the push data.
   * @return 0 if ok, else ADDRESS_INVALID. 
   */
  unsigned short push(Address * pa);

  /*
   * Removes Pose Address 'pa' from the object
   * @param pa the Address 
   * @return 0 if ok, else ADDRESS_INVALID. 
   */
  unsigned short disconnect(Address *pa);
  void disconnect(const unsigned long index);

  
  /**
   * This will add a Port of the specified descriptor ect. 
   *
   * @param depth slots length (buffersize)
   * @param des type of data that slot is intended for. Storage for
   * this kind of data will be allocted.
   * @param subType fine grained description of the type of data
   * specific to each data type, for example what type of pose
   * @param compressed if you want data to be packed in the slot 
   * @param dotypecheck true if you want to make sure that you can only
   * write data with the type you specified,
   * @return the DataPort created; 
   */
  DataPort *  addPort(long depth,DataDescriptor &des, 
		      unsigned short subType=0,
		      bool compressed=false, 
		      bool dotypecheck=false); 
  /**
   * Ths adds a data filter pointer to the bank.
   * @param f the filter to add to the Bank.
   * @return the index of the filter in the Filters array.
   */
  unsigned short  addFilter( DataFilter *f);

  /**This finds a port with this name and calls push on it*/
  unsigned short portPush(const std::string name, Address *pa);

  /**This finds a port with this name and calls pull on it*/
  unsigned short portPull(const std::string name, Address *pa);

protected:
  /**
   * This is just to speed up look ups a bit.  Not to fancy.
   */
  unsigned short hash(DataDescriptor & des)
  {

    if (des.ClassType==POSE3D_TYPE)
      return ((unsigned short) (des.ID&0x03));	
    if (des.ClassType==COMMAND_TYPE)
      return 4;
    if (des.ClassType==DATASET_TYPE)
      return 5;
    return (6+(unsigned short) (des.ID&0x03));	
  }
  /**
   * We need to writer this to implement the interface
   */
  virtual unsigned short processCommand(TimestampedData &, const Timestamp, 
					const int){
    return 0;
  }
  virtual unsigned short processCommand(TimestampedData & result, 
					const Timestamp t=0){
    return processCommand(result,t,0);
  }
};

} // namespace Cure

#endif // CURE_POSEFILTERADDRESS_HH
