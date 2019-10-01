//
// = FILENAME
//    DeviceAddress.hh
//
// = FUNCTION
//
// = AUTHOR(S)
//    JohnFolkesson
//
// = COPYRIGHT
//    Copyright (c) 2006 John Folkesson
//
/*----------------------------------------------------------------------*/

#ifndef Cure_ServiceAddress_hh
#define Cure_ServiceAddress_hh

#include "ConfigFileReader.hh"
#include "Address.hh"
#include "TimestampedData.hh"
#include "AddressBank.hh"
#include "StateData.hh"

namespace Cure{

/**
* A ServiceAddress provides an Address interface to a service.
* These are a sort of backdoor/lazy man way to get data around in 
* your app.  The service is registered by name on the AddressBank 
* so objects can find it by name and one need not call push/pull.
* It allows a lot of flexable solutions.
*
*
* @author John folkesson
*/
class ServiceAddress: public Cure::Address
             
{
public:
    /** 
   * Number of push clients that are subscriber to the the Port's data output. 
   */
  short NumPushClients;
  /** Tells the object tpo set the DateDescriptor when  pushing data*/
  bool SetDescriptor;
   /**
    * Tells if there is any subscribed pull client.
   * If this is true the address will read PullClient on pullData(...).
   */
  bool IsPull;
  /**
   * The index of the pull Address.
   */
  unsigned long PullClient;

protected:
  /** 
   * Array of indices to client that want data 
   */
  unsigned long *PushClients;
  /** Space allocated for push client indices, might not all be used */
  short NumPushClientsAlloc;
  virtual void disconnectAddress();
public:
  
  /**
   * Constructor
   *
   * @param name name of the device that is used to find it.
   */
  ServiceAddress(const std::string &name, 
		 const unsigned short thread=0):Address(thread)
    {
      m_Descriptor.Name=name;
      AddressBank::theAddressBank().addService(this); 
      PushClients = 0;
      NumPushClientsAlloc = 0;
      NumPushClients = 0;
      SetDescriptor=false;
      IsPull=false;
      PullClient=0;
      std::cerr << "\nService \"" << m_Descriptor.Name 
		<<" on thread "<<Thread<<" made \n";

    }

  /**
   * Destructor
   */
  virtual ~ServiceAddress(){
    disconnectAddress(); 
  }

  /**   
   * Use this function to configure this device before starting to use
   * it.
   *
   * @param  configfile the file. 
   * @return 0 if OK, else error code
   */
  virtual int configureService(ConfigFileReader &cfg,std::string &name);
  /** Override in subcall to set up config.*/
  virtual int configService(const std::string &){return 0;} 
  virtual int configServiceGlobal(const std::string &,
			   StateData &){return 0;} 
  virtual int configServiceSensor(const std::string &,
			   const std::string &){return 0;} 

    
    /**
   * This is called  to push the output.
   * @param tp the data to write to push clients.
   * @return 0 allways 
   */
  unsigned short pushData(TimestampedData &tp)
  {  
    if (SetDescriptor)tp.setDescriptor(m_Descriptor);
    for (short i = 0; i < NumPushClients; i++) {
      if (writeTo(PushClients[i], tp) == ADDRESS_INVALID) {
	disconnect(PushClients[i]);
	i--;
      }
    }
    return 0;
  }


  unsigned short pushDataLocked(TimestampedData &tp)
  {
    if (SetDescriptor)tp.setDescriptor(m_Descriptor);
    for (short i = 0; i < NumPushClients; i++) {
      if (writeToLocked(PushClients[i], tp) == ADDRESS_INVALID) {
	disconnect(PushClients[i]);
	i--;
      }
    }
    return 0;
  }
  unsigned short pullData(TimestampedData& result,
			  const Timestamp t, const int interpolate){
    if (IsPull)
      return readFrom(PullClient,result,t,interpolate);
    return RESOURCE_ERROR;
  }
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
  virtual unsigned short pull(Address * pa){
      if (IsPull){
	if (PullClient != pa->Index)
	  std::cerr << "\nService \"" << m_Descriptor.Name 
		    <<" I am replacing the Pull Address \n";
      } 
      PullClient = pa->Index;
      IsPull=true;
      return 0;
  }

  /**
   * Give an Address 'pa' to our service, which then 
   * will push data to pa.  
   * This can be used to push data writen to the service to a 
   * mirror service in another process.
   * 
   *   
   *
   * @param pa the Address to receive the push data.
   * @return 0 if ok, else ADDRESS_INVALID. 
   */
  virtual unsigned short push(Address * pa)
  {
    if (NumPushClients == NumPushClientsAlloc) {
      unsigned long *temp = new unsigned long[NumPushClientsAlloc + 1];
      for (short i = 0; i < NumPushClientsAlloc; i++) temp[i] = PushClients[i];
      temp[NumPushClientsAlloc] = pa->Index;
      if (NumPushClientsAlloc) delete [] PushClients;
      PushClients = temp;
      NumPushClientsAlloc++;
      NumPushClients++;
      return 0;
    }
    PushClients[NumPushClients] = pa->Index;
    NumPushClients++;
    return 0;
  }
  /**
   * The default service is just a proxy for some other address
   * allowing one to pull from an address found by name.
   *
   * @param p the data pulled is returned here
   * @param t the data is pulled with this arg
   * @param interpolate the data is pulled with this arg
   * @return the return from reading from the pull address or REASOURCE_ERROR
   */
  virtual unsigned short read(TimestampedData& p, const Timestamp t, 
			      const int interpolate) {
    return pullData(p,t,interpolate);
  } 
  /**
   * The default service write simple acts as a proxy that passes all
   * data on toits push clients.
   * @param result the data to pass on.
   * @return 0 if ok (can't fail).
   */
  virtual unsigned short write(Cure::TimestampedData& result){
    return pushData(result);
  }
  /*
   * Removes Pose Address 'pa' from the object
   * @param pa the Address 
   * @return 0 if ok, else ADDRESS_INVALID. 
   */
  virtual unsigned short disconnect(Address *pa){
    disconnect(pa->Index);
    return 0;
  }
  virtual void disconnect(const unsigned long index)
  {
    short i = 0; 
    while(i < NumPushClients) {
      if (index == PushClients[i]) {
	for(short j = i + 1; j< NumPushClients; j++) {
	PushClients[j-1] = PushClients[j];
	}
	NumPushClients--;
      }
      else
	i++;
    }
    if (IsPull)
      if (index==PullClient)IsPull=false;
  }

  bool operator==(const std::string &name){return (m_Descriptor.Name==name);}

  virtual void printServiceConfig();
protected:  

};




} // namespace Cure


#endif // Cure_DeviceAddress_hh
