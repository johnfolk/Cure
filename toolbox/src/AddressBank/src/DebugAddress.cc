//    $Id: DebugAddress.cc,v 1.10 2007/09/16 09:04:42 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2005 John Folkesson
//    

#include "DebugAddress.hh"
#include "ConfigFileReader.hh"
#include <sstream>  // istringstream
using namespace Cure;

DebugAddress::DebugAddress(const unsigned short thread):
  Address(thread)
{
  //  AddressBank::theAddressBank().add(this);
  PushClients = 0;
  NumPushClientsAlloc = 0;
  NumPushClients = 0;
  PullClient=0;
  IsPull=false;
  AddressName =" DebugAddress ";
  m_Level=1;
  m_StartTimeIndex=0;
  m_StopTimeIndex=0;
  m_CurrentLevel=3;
  m_NumberOfStrings=0;
  for (short i=0;i<10;i++){
    m_MessageString[i]="";
  }
}
int DebugAddress::configure(ConfigFileReader &cfg,
			  std::string  &debugname)
{
  AddressName=debugname;
  std::string desarglist="";
  std::string desparams;
  std::list<std::string> strings;
  if (cfg.getParamStrings(debugname, true, strings, desarglist))return 1;
  for (std::list<std::string>::iterator pi = strings.begin();
       pi != strings.end(); pi++) {       
        std::istringstream strdes(*pi);
    std::string cmd;
    if((strdes>>cmd)){
    if (cmd=="m_Level")
      strdes>>m_Level;
    else if (cmd=="m_StartTime"){
      if (m_StartTimeIndex<10){
	long ts,tms;
	strdes>>ts>>tms;
	m_StartTimes[m_StartTimeIndex].Seconds=ts;
	m_StartTimes[m_StartTimeIndex].Microsec=tms;
	m_StartTimeIndex++;
      }
    }else if (cmd=="m_StopTime"){
      if (m_StopTimeIndex<10){
      	long ts,tms;
	strdes>>ts>>tms;
	m_StopTimes[m_StopTimeIndex].Seconds=ts;
	m_StopTimes[m_StopTimeIndex].Microsec=tms;
	m_StopTimeIndex++;
      }
    } else if (cmd=="m_MessageString")
      if (m_NumberOfStrings<10){
	if((strdes>>m_MessageString[m_NumberOfStrings])){
	  short i=0;
	  m_MessageIndex[m_NumberOfStrings].grow(1,i+1);
	  while((strdes>>m_MessageIndex[m_NumberOfStrings](0,i))){
	    i++;
	    m_MessageIndex[m_NumberOfStrings].grow(1,i+1);
	   
	  }
	  m_MessageIndex[m_NumberOfStrings].print();
	  m_NumberOfStrings++;
	}
      }
    }
  }
  m_StartTimeIndex=0;
  m_StopTimeIndex=0;
  for (short i=0;i<9;i++)
    for(short j=i+1;j<10;j++){
      if(m_StopTimes[i]>m_StopTimes[j]){
	Timestamp t=m_StopTimes[i];
	m_StopTimes[i]=m_StopTimes[j];
	m_StopTimes[j]=t;      
      }
      if(m_StartTimes[i]>m_StartTimes[j]){
	Timestamp t=m_StartTimes[i];
	m_StartTimes[i]=m_StartTimes[j];
	m_StartTimes[j]=t;      
      }
      if (m_StartTimes[i]==0)  m_StartTimeIndex=i;
      if (m_StopTimes[i]==0)  m_StopTimeIndex=i;
    }
  m_StartTimeIndex++;
  m_StopTimeIndex++;
  
  if (m_StartTimes[m_StartTimeIndex]<m_StopTimes[m_StopTimeIndex])
    m_CurrentLevel=0;
  else m_CurrentLevel=m_Level;
  if (m_StartTimes[m_StartTimeIndex]==0)m_CurrentLevel=m_Level;
  return 0;  
}

unsigned short DebugAddress::message(TimestampedData& tp)
{
  if (m_CurrentLevel){
    std::cerr<<AddressName<<"\" write\n ";
    if (m_CurrentLevel&1){
      DataDescriptor d;
      tp.getDescriptor(d);
      d.print();
      if (m_CurrentLevel&2)
	tp.print();
    }
    if (m_CurrentLevel&4)
      tp.Time.print();
    
    if (m_CurrentLevel&8)
      std::cerr<<"Subtype= "<<tp.getSubType()<<"\n";
    for (short i=0; i<m_NumberOfStrings;i++){
      Matrix mat;
      if (tp.getMatrix(m_MessageString[i],m_MessageIndex[i],mat))
	{
	  std::cerr<<m_MessageString[i]<<" = ";
	  mat.print();
	}else  std::cerr<<m_MessageString[i]<<" ERRROR";
    }
    if (m_CurrentLevel&64){
      unsigned char s=getchar();
      if ('s'==(s))m_CurrentLevel=(m_CurrentLevel&(~(64)));
    }
  }
  return 0;
}

unsigned short DebugAddress::read(TimestampedData& result, 
				   const Timestamp t,
				   const int interpolate)
{
  if (m_CurrentLevel&16){
    std::cerr<< AddressName <<"\" read: interpolate="<<interpolate<<" t=";
    t.print();
  }

  if (IsPull) {
    unsigned short r=readFrom(PullClient,result,t, interpolate);
    if (m_CurrentLevel&16)
      std::cerr<< AddressName <<"\" read return "<<r;  
    message(result);
    Timestamp t=result.Time;
    if (((m_CurrentLevel)&&(t>m_StopTimes[m_StopTimeIndex])&&
	 (m_StopTimes[m_StopTimeIndex]!=0))){
      m_StopTimeIndex++;
      if (m_StopTimeIndex==10){
	m_StopTimeIndex=0;
	m_StopTimes[m_StopTimeIndex]=0.0;
      }
      m_CurrentLevel=0;
    }
    if (((!m_CurrentLevel)&&(t>m_StartTimes[m_StartTimeIndex])&&
	 (m_StartTimes[m_StartTimeIndex]!=0))){
      m_StartTimeIndex++;
      if (m_StartTimeIndex==10){
	m_StartTimeIndex=0;
	m_StartTimes[m_StartTimeIndex]=0.0;
      }
      m_CurrentLevel=m_Level;
    }
  return r;
  }
  return 0;
}
unsigned short DebugAddress::write(TimestampedData &tp)
{
    Timestamp t=tp.Time;
    if (((m_CurrentLevel)&&(t>m_StopTimes[m_StopTimeIndex])&&
	    (m_StopTimes[m_StopTimeIndex]!=0))){
      m_StopTimeIndex++;
      if (m_StopTimeIndex==10){
	m_StopTimeIndex=0;
	m_StopTimes[m_StopTimeIndex]=0.0;
      }
      m_CurrentLevel=0;
    }
    if (((!m_CurrentLevel)&&(t>m_StartTimes[m_StartTimeIndex])&&
	 (m_StartTimes[m_StartTimeIndex]!=0))){
      m_StartTimeIndex++;
      if (m_StartTimeIndex==10){
	m_StartTimeIndex=0;
	m_StartTimes[m_StartTimeIndex]=0.0;
      }
      m_CurrentLevel=m_Level;
    }
    message(tp);
      
    for (short i = 0; i < NumPushClients; i++) {
      if (m_CurrentLevel>4)std::cerr<<AddressName<<"Pushing "<<i<<"\n";
      if (writeTo(PushClients[i], tp) == ADDRESS_INVALID) {
	std::cerr<< AddressName <<"\" warning  "
		 << ", failed to pushData, disconnecting!!!\n";
	disconnect(PushClients[i]);
	i--;
      }
    }
    if (m_CurrentLevel)
      std::cerr<< AddressName <<"\" write done\n ";
    return 0;
}

void DebugAddress::disconnect(const unsigned long index)
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
unsigned short DebugAddress::pull(Address * pa)
{
  if (IsPull){
    std::cerr <<" warning "<<" I am replacing the Pull Address "
	      <<"\n";
    
  } 
  PullClient = pa->Index;
  IsPull=true;
  return 0;
}


unsigned short DebugAddress::push(Address * pa)
{
    if (NumPushClients == NumPushClientsAlloc) {
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
