// = RCSID
//    $Id: Address.cc ,v 1.1 2006/03/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2006 John Folkesson
//    
#include "Address.hh"
#include "AddressBank.hh"
namespace Cure {
  using namespace std;
Address::Address(const unsigned short thread)
{
  CanPermit=false;
  PermittedThreads=0;
  Index=0;
  Thread=0;
  initAddress(thread);
}
void Address::initAddress(const unsigned short thread)
{
  setThread(thread);
}

void Address::disconnectAddress(){
  short n=AddressBank::theAddressBank().getNumberOfThreads(); 
  if (CanPermit){
    for (short i=0;i<n;i++){
      if (PermittedThreads[i]==Thread)break;
      AddressBank::theAddressBank().getThread(PermittedThreads[i]);
    }
  }
  if (Index!=0)
    AddressBank::theAddressBank().remove(this);
  if (CanPermit){
    for (short i=0;i<n;i++){
      if (PermittedThreads[i]==Thread)break;
      AddressBank::theAddressBank().releaseThread(PermittedThreads[i]);
    }
    CanPermit=false;   
    delete [] PermittedThreads;
    PermittedThreads=0;
  }


}

void Address::setThread(unsigned short thread)
{
  AddressBank::theAddressBank().remove(this);
  Thread=thread;
  AddressBank::theAddressBank().add(this);
} 

unsigned short Address::writeTo(const unsigned long index, TimestampedData &p)
{
  return AddressBank::theAddressBank()
    .write(Thread,index,p);
}
unsigned short Address::writeToLocked(const unsigned long index, TimestampedData &p)
{
  return AddressBank::theAddressBank()
    .writeLocked(Thread,index,p);
}

unsigned short Address::readFrom(const unsigned long index,
				  TimestampedData& result, 
				  const Timestamp t, 
				 const int interpolate)
{ 
  return AddressBank::theAddressBank()
    .read(Thread,index,result,t,interpolate);
}
unsigned short Address::readFromLocked(const unsigned long index,
				  TimestampedData& result, 
				  const Timestamp t, 
				 const int interpolate)
{ 
  return AddressBank::theAddressBank()
    .readLocked(Thread,index,result,t,interpolate);
}
bool  Address::getService(const std::string &service, 
				     unsigned long &index)
{
  return AddressBank::theAddressBank()
    .getService(Thread,service,index);

}
bool Address::setCanPermit()
{
  if (CanPermit)return true;
  CanPermit=AddressBank::theAddressBank().makePermanent(this);
  if (CanPermit){
    short n=AddressBank::theAddressBank().getNumberOfThreads();
    PermittedThreads=new unsigned short[n];
    for (short i=0;i<n;i++)
      PermittedThreads[i]=Thread;
    return true;
  }
  return false;
}
bool Address::isThreadSafe(Address * pa)
{
  if (!CanPermit)return false;
  short n=AddressBank::theAddressBank().getNumberOfThreads();
  for (short i=0;i<n;i++){
    if (PermittedThreads[i]==pa->Thread)return true;
      if (PermittedThreads[i]==Thread){
	PermittedThreads[i]=pa->Thread;
	return true;
      }
  }
  return false;
}

}
 // namespace Cure
