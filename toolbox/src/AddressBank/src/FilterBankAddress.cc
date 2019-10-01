//    $Id: FilterBankAddress.cc,v 1.13 2008/05/05 16:55:27 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#include "FilterBankAddress.hh"
#include "MutexHelper.hh"
#include "CureDebug.hh"
using namespace Cure;

FilterBankAddress::FilterBankAddress(const unsigned short thread)
  :Address(thread)
{
  //  AddressBank::theAddressBank().add(this);
  m_Descriptor.Name="FilterBankAddress";
  m_ComDescriptor.Name="FilterBankAddressCom";
  m_ComDescriptor.ClassType=COMMAND_TYPE;
  SizeFilters=0;
  PushClients = 0;
  NumPushClientsAlloc = 0;
  NumPushClients = 0;
  NumberFilters=0;
  Filters=0;
  for (short i=0; i<10; i++)
    {
      Ports[i]=0;  
      NumberPorts[i]=0;
      SizePorts[i]=0;
    }
}
FilterBankAddress::FilterBankAddress(DataDescriptor &comdes)
{
  m_ComDescriptor=comdes;
  PushClients = 0;
  NumPushClientsAlloc = 0;
  NumPushClients = 0;
  SizeFilters=0;
  NumberFilters=0;
  Filters=0;
  for (short i=0; i<10; i++)
    {
      Ports[i]=0;  
      NumberPorts[i]=0;
      SizePorts[i]=0;
    }
}
FilterBankAddress::~FilterBankAddress()
{
  disconnectAddress();
  for (short i=0; i<10; i++)
    {
      if (Ports[i])
	{
	  for (short j=0;j<NumberPorts[i];j++)
	    delete Ports[i][j];
	  delete[] Ports[i];
	  Ports[i]=0;
	  NumberPorts[i]=0;
	  SizePorts[i]=0;
	}
    }
   delete[] Filters;
   SizeFilters=0;
   NumberFilters=0;
   Filters=0;
}
void FilterBankAddress::disconnectAddress()
{
  Address::disconnectAddress();
  if ((NumPushClientsAlloc)&&(PushClients)) delete [] PushClients;
  PushClients=0;
  NumPushClientsAlloc=0;
  NumPushClients = 0;
  for (short i=0; i<10; i++)
    {
      if (Ports[i])
	{
	  for (short j=0;j<NumberPorts[i];j++)
	     Ports[i][j]->disable();
	}
    }
}
DataPort * FilterBankAddress::operator()(const std::string dataname)
{
  DataDescriptor d;
  d.Name=dataname;
  for (short i=0; i<10; i++)
    for (short j=0;j<NumberPorts[i];j++)
      if (Ports[i][j]->sameName(d))return Ports[i][j];
  return 0;

}
unsigned short FilterBankAddress::portPush(const std::string name, Address *pa)
{
  DataPort *d=(*this)(name);  
  if (d)return d->push(pa);
  return ADDRESS_INVALID;
}

unsigned short FilterBankAddress::portPull(const std::string name, Address *pa)
{
  DataPort *d=(*this)(name);  
  if (d)return d->pull(pa);
  return ADDRESS_INVALID;
}


DataPort *  FilterBankAddress::addPort( long depth,DataDescriptor &des, 
	    unsigned short subType,
	    bool compressed, 
	    bool dotypecheck)
{
  if(des==m_ComDescriptor)
    {
      CureCERR(20) <<m_ComDescriptor.Name<<" Tried to add Port with my com descriptor\n";
      return 0;
    }
  unsigned short i=hash(des);
  for (short j=0;j<NumberPorts[i];j++)
    if (Ports[i][j]->match(des))
      {
	CureCERR(20) <<m_Descriptor.Name
		 <<" Tried to add Port that matched an existing descriptor "
		 <<des.Name<<"\n";
	return Ports[i][j];
      }
  if (SizePorts[i]==NumberPorts[i])
    {
      DataPort **p=Ports[i];;
      SizePorts[i]+=10;
      Ports[i]=new DataPort*[SizePorts[i]];
      for (int j=0; j<NumberPorts[i]; j++)
	Ports[i][j]=p[j];
      if (p)delete[] p;
    }
  Ports[i][NumberPorts[i]]
    =new DataPort(depth,des,subType,compressed,dotypecheck);
  NumberPorts[i]++;
  CureCERR(100)<<"FilterBank "<<m_Descriptor.Name<<" added Port: ";
  return Ports[i][NumberPorts[i]-1];
}

unsigned short  FilterBankAddress::addFilter( DataFilter *f)
{
  if (SizeFilters==NumberFilters)
    {
      DataFilter **p=Filters;
      SizeFilters+=10;
      Filters=new DataFilter*[SizeFilters];
      for (short j=0; j<NumberFilters; j++)
	Filters[j]=p[j];
      if (p)delete []p;
    }
  Filters[NumberFilters]=f;
  NumberFilters++;
  return (NumberFilters-1);
}
unsigned short FilterBankAddress::write(TimestampedData & p)
{
  pushData(p);
  DataDescriptor des;
  p.getDescriptor(des);
  unsigned short i=hash(des);
  for (short j=0;j<NumberPorts[i];j++)
    if (Ports[i][j]->match(des)){
      return Ports[i][j]->write(p);
    }  
return (ADDRESS_INVALID|RESOURCE_ERROR);
}
unsigned short FilterBankAddress::read(TimestampedData & result, 
				       const Timestamp t, 
				       const int interpolate)
{
  DataDescriptor des;
  result.getDescriptor(des);
  if(des==m_ComDescriptor)
    return processCommand(result,t,interpolate);
  unsigned short i=hash(des);
  for (short j=0;j<NumberPorts[i];j++)
    if (Ports[i][j]->match(des))return Ports[i][j]->read(result,t,interpolate);
  return (ADDRESS_INVALID|RESOURCE_ERROR);
}
    /**
   * This is called  to push the output.
   * @param tp the data to write to push clients.
   * @return 0 allways 
   */
unsigned short FilterBankAddress::pushData(TimestampedData &tp)
  {  
    for (short i = 0; i < NumPushClients; i++) {
      if (writeTo(PushClients[i], tp) == ADDRESS_INVALID) {
	disconnect(PushClients[i]);
	i--;
      }
    }
    return 0;
  }


  
  /**
   * Give an Address 'pa' to our bank, which then 
   * will push data to pa on writes.  
   * This can be used to push data writen to the service to a 
   * mirror service in another process.
   * 
   *   
   *
   * @param pa the Address to receive the push data.
   * @return 0 if ok, else ADDRESS_INVALID. 
   */
unsigned short FilterBankAddress::push(Address * pa)
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

  /*
   * Removes Pose Address 'pa' from the object
   * @param pa the Address 
   * @return 0 if ok, else ADDRESS_INVALID. 
   */
unsigned short FilterBankAddress::disconnect(Address *pa){
  disconnect(pa->Index);
  return 0;
}
void FilterBankAddress::disconnect(const unsigned long index)
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
}
