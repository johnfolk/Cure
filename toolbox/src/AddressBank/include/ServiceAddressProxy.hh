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
//    Copyright (c) 2007 John Folkesson
//
/*----------------------------------------------------------------------*/

#ifndef Cure_ServiceAddressProxy_hh
#define Cure_ServiceAddressProxy_hh

#include "ConfigFileReader.hh"
#include "ServiceAddress.hh"
#include "TimestampedData.hh"
#include "AddressBank.hh"
#include "StateData.hh"

namespace Cure{

/**
* A ServiceAddressProxy provides an Address interface to a service.
* It allows one to have an address in one thread space (or this one) with the
* service name from another thread space.  One then can call
* read/write/push/pull on the proxy as well as finding it wit
* getService readService and so on.  The proxy will call lock on the
* true service space and then read/write to the true service.  This
* can lead to DEADLOCK if one has a proxy in each of two spaces reading from
* true services in each others thread space.  
*
* If this service has the same name as the true service everything
* will work except that calling push/pull or looking for a named
* address in the AddressMaker will always return the first address
* with this name.  that is the one that was made first.  The getService will
* be able to distinguish based on the thread value.
*
* You can call push and pull on this but the push and pull clients are never
* used.
*
* @author John folkesson
*/
class ServiceAddressProxy: public Cure::ServiceAddress
             
{
public:
  /** 
   * The Thread of the true service
   */
  unsigned short m_ServiceThread;
  /**
   * The name of the true service
   */
  std::string  m_ServiceName;
  /** The index of the the service */
  unsigned long m_Service;
protected:

public:
  
  /**
   * Constructor
   *
   * @param name name of the device that is used to find it.
   */
  ServiceAddressProxy(const std::string &name, 
		      const unsigned short thread=0,
		      const unsigned short servicethread=0)
    :ServiceAddress(name,thread)
    {
      m_ServiceName=m_Descriptor.Name;
      m_Service=0;
      m_ServiceThread=servicethread;
    }


  /**   
   * Use this function to configure this device before starting to use
   * it.
   *
   * @param str  the config strin
   * @return 0 if OK, else error code
   */
  virtual int configService(const std::string & str);
 
    
  /**
   * The default service is just a proxy for some other address
   * allowing one to pull from a named service address in another 
   * thread space.
   *
   * @param p the data pulled is returned here
   * @param t the data is pulled with this arg
   * @param interpolate the data is pulled with this arg
   * @return the return from reading from the pull address or REASOURCE_ERROR
   */
  virtual unsigned short read(TimestampedData& p, const Timestamp t, 
			      const int interpolate) {
    unsigned short r=getService();
    if (r)return r;
    if (Thread==m_ServiceThread)
      r=readFrom(m_Service,p,t,interpolate);
    else r=AddressBank::theAddressBank()
	   .readLocked(m_ServiceThread,m_Service,p,t,interpolate);
    if (r&ADDRESS_INVALID){
      m_Service=0;
      r=getService();
      if (r)return r;
      if (Thread==m_ServiceThread)
	r=readFrom(m_Service,p,t,interpolate);
      else  
	r=AddressBank::theAddressBank()
	.readLocked(m_ServiceThread,m_Service,p,t,interpolate);
    }
    return r;
  }
   
  /**
   * The default service write simple acts as a proxy that passes all
   * data on to the true service
   * @param p the data to pass on.
   *
   * @return 0 if ok, RESOURCE_ERROR if no service found else return
   * value from the service write .
   */
  virtual unsigned short write(Cure::TimestampedData& p){
    unsigned short r=getService();
    if (r)return r;
    if (Thread==m_ServiceThread)
      r=writeTo(m_Service,p);
    else  
      r=AddressBank::theAddressBank()
	.writeLocked(m_ServiceThread,m_Service,p);
    if (r&ADDRESS_INVALID){
      m_Service=0;
      r=getService();
      if (r)return r;
        if (Thread==m_ServiceThread)
	  r=writeTo(m_Service,p);
	else  
	  r=AddressBank::theAddressBank()
	    .writeLocked(m_ServiceThread,m_Service,p);
    }
    return r;
  }
  unsigned short getService(){
    if (!m_Service)
      if (Thread==m_ServiceThread){
	if  (!AddressBank::theAddressBank()
	    .getService(m_ServiceThread,m_ServiceName,m_Service))
	  {
	    m_Service=0;
	    return RESOURCE_ERROR;;
	  }    
      }else{
	if (!AddressBank::theAddressBank()
	    .getServiceLocked(m_ServiceThread,m_ServiceName,m_Service))
	  {
	    m_Service=0;
	    return RESOURCE_ERROR;;
	  }    
      }
    return 0;
  }
  virtual void printServiceConfig();
protected:  

};




} // namespace Cure


#endif // Cure_DeviceAddress_hh
