// = RCSID
//    $Id: AddressBank.cc ,v 1.1 2004/03/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2004 John Folkesson
//    
#include "AddressBank.hh"
//#include "Utils/MutexHelper.hh"
#include "CureDebug.hh"
namespace Cure {
  using namespace std;


AddressBankThread::AddressBankThread(short depth)
{
  if (depth>10){
    CureCERR(10)<<"Did you realize that the depth of the Address bank is"
	     <<"the log to the base 2 of the size"
	     <<"This depth seems quite large "<<depth<<"\n";
  }
  NextIndex=1;
  Size=1;
  Size=(Size<<depth);
  Mask=Size-1;
  Addresses=new AddressList[Size];
  Permanent=new bool[Size];
  for (long i=0;i<Size;i++)
    Permanent[i]=0;
  CureCERR(30)<<"Made Thread Space\n";;
}
AddressBankThread::~AddressBankThread()
{
  
  bool printmsg=true;
  for (short i=0; i<Size;i++) {
    while (Addresses[i].Next!=0)
      {
        Address *removedAddr = Addresses[i].Element;
	CureCERR(10)<<"bad "<<removedAddr->Index<<" "<<removedAddr->Thread<<"\n";;
	Addresses[i].remove(removedAddr);
        removedAddr->Index=0;
	if (printmsg){
	  printmsg=false;	  
	  CureCERR(10)<<"AddressBankThread being deleted before its Addresses"<<endl;
	  CureCERR(10)<<"The bank is being cleared"<<endl;     ;
	} 
      }
  }

  if (Addresses) delete [] Addresses;
  if (Permanent)delete []Permanent;
  Addresses = 0;
  Size = 0;
}
void AddressBankThread::operator=(AddressBankThread &a)
{
  for (unsigned long index=0; index<a.NextIndex; index++)    
    {
      unsigned long inx=(index&a.Mask);
      Address *pa=a.Addresses[inx].get(index);
      Address *ps=a.Services.get(index);
      if (pa)
	{
	  inx=(pa->Index&Mask);
	  Addresses[inx].add(pa);
	}
      if (ps)Services.add(ps);
    }
}

void AddressBankThread::resizeHash(short depth)
{
  if (depth>10){
    CureCERR(10)<<"Did you realize that the depth of the Address bank is"
	     <<"the log to the base 2 of the size"
	     <<"This depth seems quite large "<<depth<<"\n";
  }
  AddressList *oldAddresses=Addresses;
  bool *oldperm=Permanent;
  long oldMask=Mask;
  Size=1;
  Size=Size<<depth;
  Mask=Size-1;
  CureCERR(50)<<"Resizing Thread Space to "<<Size<<"\n";
  Addresses=new AddressList[Size];
  Permanent=new bool[Size];
  for (long i=0;i<Size;i++)
    Permanent[i]=0;
  delete [] oldperm;
  for (unsigned long index=0; index<NextIndex; index++)    
    {
      unsigned long inx=(index&oldMask);
      Address *pa=oldAddresses[inx].get(index);
      if (pa)
	{
	  inx=(pa->Index&Mask);
	  Addresses[inx].add(pa);
	}
    }
  delete [] oldAddresses;
}
void AddressBankThread::optimizeHash()
{
  long cnt=0;
  for (unsigned long index=0; index<NextIndex; index++)    
    if(!query(index))cnt++;
  short s=1;
  short depth=0;
  while (s<cnt){
    s=(s<<1);
    depth++;
  }
  resizeHash(depth);
}
void AddressBankThread::add(Address *pa)
{
  if (pa->Index!=0)return;
  unsigned long index=(NextIndex&Mask);
  pa->Index=NextIndex;
  Addresses[index].add(pa);
  NextIndex++;
}

bool AddressBankThread::makePermanent(Address *pa)
{
  if (pa->Index==0)add(pa);
  unsigned long inx=((pa->Index)&Mask);
  if (Addresses[inx].Element==pa){
    Permanent[inx]=true;
    return true;
  }
  remove(pa);
  for (long k=0; k<Size;k++){
    unsigned long index=(NextIndex+k);
    index=(k&Mask);
    if (!Permanent[index]){
      pa->Index=NextIndex+k;
      Address *a=Addresses[index].Element;
      Addresses[index].Element=pa;
      Addresses[index].add(a);
      Permanent[index]=true;
      return true;
    }
  }
  return false;
}

void AddressBankThread::remove(Address *pa)
{
  if (pa->Index==0)return;    
  unsigned long inx=((pa->Index)&Mask);
  Addresses[inx].remove(pa);
  pa->Index=0;
}

void AddressBankThread::addService(Address *pa)
{
   Services.add(pa);
}
bool AddressBankThread::getService(const std::string &service, unsigned long &index)
{
  index=0;
  bool r=Services.get(service,index);
  return r;
}
void AddressBankThread::removeService(Address *pa)
{
  Services.remove(pa);
}

unsigned short AddressBankThread::write(const unsigned long index, 
				  TimestampedData& p)
{
  if (index==0)return (RESOURCE_ERROR|ADDRESS_INVALID);
  unsigned long inx=(index&Mask);
  Address *pa=Addresses[inx].get(index);
  if (pa==0)
    {
      return (RESOURCE_ERROR|ADDRESS_INVALID);
    }  
  return (pa->write(p)&(~ADDRESS_INVALID));  
}

unsigned short AddressBankThread::read(const unsigned long index, 
				 TimestampedData &p, 
				 const Timestamp t,
				 const int interpolate)
{
  if (index==0)return (RESOURCE_ERROR|ADDRESS_INVALID);
  unsigned long inx=(index&Mask);
  Address *pa=Addresses[inx].get(index);
  if (pa==0)    {
      return (RESOURCE_ERROR|ADDRESS_INVALID);
    }  
  return ((pa->read(p,t,interpolate))&(~ADDRESS_INVALID));
}

unsigned short AddressBankThread::push(Address * p, const unsigned long index)
{ 
  if (index==0)return (RESOURCE_ERROR|ADDRESS_INVALID);
  unsigned long inx=(index&Mask);
  Address *pa=Addresses[inx].get(index); 
  if (pa==0) {
      return (RESOURCE_ERROR|ADDRESS_INVALID);
    }  
  return pa->push(p);
}

bool AddressBankThread::isThreadSafe(const unsigned long index,Address * pa)
{
  if (index==0)return false;
  unsigned long inx=(index&Mask);
  Address *a=Addresses[inx].get(index); 
  if (a==0) {
    return false;
    }  
  return a->isThreadSafe(pa);
}
unsigned short AddressBankThread::pull(Address * p, const unsigned long index)
{ 
  if (index==0)return (RESOURCE_ERROR|ADDRESS_INVALID);
  unsigned long inx=(index&Mask);
  Address *pa=Addresses[inx].get(index); 
  if (pa==0) {
      return (RESOURCE_ERROR|ADDRESS_INVALID);
    }  
  return pa->pull(p);
}


unsigned short AddressBankThread::disconnect(Address * p, const unsigned long index)    
{ 
  unsigned long inx=(index&Mask);
  Address *pa=Addresses[inx].get(index);
  if (pa==0) {
    return (RESOURCE_ERROR|ADDRESS_INVALID);
  }  
  return pa->disconnect(p);
}

unsigned short AddressBankThread::query(const unsigned long index)    
{ 
  unsigned long inx=(index&Mask);
  Address *pa=Addresses[inx].get(index);
  if (pa==0)return (RESOURCE_ERROR|ADDRESS_INVALID);
  return 0;
}
bool AddressBank::isThreadSafe(const unsigned short thread, 
			       const unsigned long index,Address * pa)
{
  if (pa->Thread==thread)return true;
  getThread(thread);
  bool r=Thread[thread].isThreadSafe(index,pa);
  releaseThread(thread);
  return r;
}


} // namespace Cure
