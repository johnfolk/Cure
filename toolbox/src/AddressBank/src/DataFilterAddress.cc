//    $Id: DataFilterAddress.cc,v 1.20 2008/05/13 20:29:00 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#include "DataFilterAddress.hh"
#include "MutexHelper.hh"
#include "CureDebug.hh"
using namespace Cure;

DataFilterAddress::DataFilterAddress(const unsigned short thread)
  :Address(thread)
{
  //So if Index is still 0 here this is not on the bank yet.
  PushClients = 0;
  NumPushClientsAlloc = 0;
  NumPushClients = 0;
  PullClient=0;
  Interpolate=true;
  IsPull=false;
  PullExact=true;
  PullSequenced=false;
  PullLatest=false;
  DataNeeded=true;
  NextData=0;
  LastErrorCode=0;
  SetDescriptor=false;
}
void DataFilterAddress::initAddress(const unsigned short thread)
{
  if ((thread>0)||(AddressBank::theAddressBank().getNumberOfThreads()==1))
    setThread(thread);  
}
void
DataFilterAddress::init(DataFilter *f, 
			short portnumber)//, const unsigned short thread)
{
  Filter=f;
  Port=portnumber;
}

DataFilterAddress::~DataFilterAddress()
{
  DataFilterAddress::disconnectAddress();
    
}
void DataFilterAddress::disconnectAddress(){
  Address::disconnectAddress();
  if ((NumPushClientsAlloc)&&(PushClients)) delete [] PushClients;
  PushClients=0;
  NumPushClientsAlloc=0;
  NumPushClients = 0;
  PullClient=0;
  IsPull=false;
  DataNeeded=false;
}
unsigned short DataFilterAddress::pushData(TimestampedData &tp)
{  
  if (SetDescriptor)setDescriptor(tp);
    for (short i = 0; i < NumPushClients; i++) {
      if (writeTo(PushClients[i], tp) == ADDRESS_INVALID) {
	if (Filter)
	  CureCERR(30) << "\nPort "<<Port<<" DataFilter \"" 
		    << Filter->FilterName <<"\" warning  "
                    << ", failed to pushData, disconnecting!!!\n";
	disconnect(PushClients[i]);
	i--;
      }
    }
    return 0;
}

unsigned short DataFilterAddress::pullData(TimestampedData& result,
					   const Timestamp t)
{
  if (IsPull) {
    if (PullExact)
      LastErrorCode=readFrom(PullClient,result,t, 0);
    else if (PullSequenced)
      LastErrorCode=readFrom(PullClient,result,LastTime, NextData);
    else 
      LastErrorCode=readFrom(PullClient,result,LastTime, 0);
    if (LastErrorCode){
      if (LastErrorCode&ADDRESS_INVALID)
	{
	  if (Filter)
	    std::cerr << "\nPort "<<Port<<" DataFilter \"" 
		      << Filter->FilterName <<"\" warning  "
		      << "Failed to pullData, disconnecting!!!\n";
	  disconnect(PullClient);	  
	  return (RESOURCE_ERROR);
	}
      else if (LastErrorCode&TYPE_ERROR)
	{
	  if (Filter)
	    std::cerr << "\nPort "<<Port<<" DataFilter \"" 
		      << Filter->FilterName <<"\" warning  "
		      << "Got TYPE_ERROR on pull\n";
	  return (RESOURCE_ERROR);
	}
      else if (LastErrorCode&RESOURCE_ERROR){
	if (Filter)
	  std::cerr << "\nPort "<<Port<<" DataFilter \"" 
		    << Filter->FilterName <<"\" warning  "
		    << "Got RESOURCE_ERROR on pull\n";
         return RESOURCE_ERROR;
      }
      if (Interpolate)	
	{
	  if (LastErrorCode&NO_INTERPOLATE)
	    {
	      std::cerr << "\nPort "<<Port<<" DataFilter \"" 
			<< Filter->FilterName <<"\" warning  "
			<<"tried to pull data from an address that "
			<<"can't interpolate.\n";
	    }
	}
      else LastErrorCode=(LastErrorCode&~NO_INTERPOLATE);
      if (PullExact)
	{
	  return LastErrorCode;
	}
      else if (PullSequenced)
	{
	  if (LastErrorCode&TIMESTAMP_TOO_OLD){
	    NextData=1;
	    LastTime=result.Time;
	  }
	  return LastErrorCode;
	}
      else return LastErrorCode;
    }
    else if (PullSequenced){
      if (LastTime==result.getTime()) 
	{
	  if (NextData<0)
	    NextData--;
	  else NextData++;
	}
      else 
	{
	  NextData=1;
	  LastTime=result.Time;
	}
    }
    return LastErrorCode;
  }
  if (DataNeeded)return RESOURCE_ERROR;
  return 0;
}

void DataFilterAddress::disconnect(const unsigned long index)
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
  if (index==PullClient)
    IsPull=false;
}



unsigned short DataFilterAddress::push(Address * pa)
{
  if (Filter->canPush(Port))
    {
      if (NumPushClients == NumPushClientsAlloc) {
	for (short i = 0; i < NumPushClients; i++) 
	  if (pa->Index == PushClients[i])return 0;
	// Need to allocate more space for more push connections
	unsigned long *temp = new unsigned long[NumPushClientsAlloc + 1];
	for (short i = 0; i < NumPushClientsAlloc; i++) temp[i] = PushClients[i];
	temp[NumPushClientsAlloc] = pa->Index;
	if (NumPushClientsAlloc) delete [] PushClients;
	PushClients = temp;
	NumPushClientsAlloc++;
	NumPushClients++;
	return 0;
      }
      // If we got here there is already allocated space for more connections
      PushClients[NumPushClients] = pa->Index;
      NumPushClients++;
      return 0;
    }
  std::cerr << "\nDataFilter \"" << Filter->FilterName <<"\" "
	    <<" warning "<<" I Do not accept Push on Port "<<Port<<"\n";
  return ADDRESS_INVALID;
}
 unsigned short DataFilterAddress::pull(Address * pa)
{

  if (Filter->canPull(Port))
    {
      if (IsPull){
	if (Filter)
	  if (PullClient != pa->Index)
	    std::cerr << "\nDataFilter \"" << Filter->FilterName <<"\" Port "
		      <<Port<<" warning "<<" I am replacing the Pull Address "
		      <<"\n";
	
      } 
      PullClient = pa->Index;
      IsPull=true;
      return 0;
    }

  std::cerr << "\nDataFilter \"" << Filter->FilterName <<"\" "
	    <<" warning "<<" I Do not accept Pull on Port "<<Port<<"\n";
  return (RESOURCE_ERROR|ADDRESS_INVALID);
}
