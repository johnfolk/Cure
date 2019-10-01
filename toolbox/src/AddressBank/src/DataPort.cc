//    $Id: DataPort.cc,v 1.12 2008/05/05 16:55:27 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2005 John Folkesson
//    

#include "DataPort.hh"
#include "CureDebug.hh"
using namespace Cure;

DataPort::DataPort(const unsigned short thread):
  DataSlotAddress("DataPort", thread)
{
  PullClient=0;
  IsPull=false;
  Hidden=false;
  Verbose=false;
}

DataPort::DataPort(short depth,DataDescriptor &des, 
		   unsigned short subType,
		   bool compressed, 
		   bool dotypecheck,
		   const unsigned short thread)
  :DataSlotAddress(depth, 
		   des.ClassType,
		   subType,
		   compressed ,
		   dotypecheck,
		   thread)
  
{
  Slot.Descriptor=des;
  m_Descriptor=des;
  PullClient=0;
  IsPull=false;
  Verbose=false;
  Hidden=false;
}


DataPort::~DataPort()
{
}
void DataPort::disconnectAddress()
{
  Address::disconnectAddress();
  PullClient=0;
  IsPull=false;
}
unsigned short DataPort::disconnect(Address *pa){
  if (Verbose){
    std::cerr<<" DataPort ";
      Slot.Descriptor.print();
      std::cerr<<" Disconnecting from an Address ";
      std::cerr<<" \n";
  }
  unsigned long index=pa->Index;
  if (index==PullClient)
    IsPull=false;
  DataSlotAddress::disconnect(index);
  return 0;
}

unsigned short DataPort::pull(Address * pa)
{
  if (Verbose){
    std::cerr<<" DataPort ";
    Slot.Descriptor.print();
    std::cerr<<" Is being set to pull ";
    std::cerr<<" \n";
  }
  PullClient = pa->Index;
  IsPull=true;
  return 0;
}


unsigned short DataPort::write(TimestampedData & p)
{
  Cure::DataDescriptor d;
  p.getDescriptor(d);
  if (Verbose){
    std::cerr<<" DataPort ";
    Slot.Descriptor.print();
    std::cerr<<" Is being written to with";
    d.print();
    std::cerr<<" Timestamped ";
    p.Time.print();
    std::cerr<<" \n";
  }
  if (d.ClassType!=Slot.Descriptor.ClassType)
    return (TYPE_ERROR|RESOURCE_ERROR);
  if (d.SensorType!=Slot.Descriptor.SensorType)
    return (TYPE_ERROR|RESOURCE_ERROR);
  p.setDescriptor(Slot.Descriptor);
  return DataSlotAddress::write(p);
}
unsigned short DataPort::read(TimestampedData& result, 
			      int interpolate[1], const Timestamp t)
{
  if (Verbose){
    std::cerr<<" DataPort ";
    Slot.Descriptor.print();
    std::cerr<<" Is being read to with t=";
    t.print();
    std::cerr<<" TimestampedData.ClassType: "<<result.getClassType()<<"\n";
  }
  if (IsPull) {
    unsigned short r=0;
    r=readFrom(PullClient,result,t, interpolate[0]);
    if (r){
      if (r&ADDRESS_INVALID)
	{
	  IsPull=false;
	  return (RESOURCE_ERROR);
	}
      else if (r&TYPE_ERROR)
	{
	  return (RESOURCE_ERROR);
	}
      else if (r&RESOURCE_ERROR){
         return RESOURCE_ERROR;
      }
      if (r&NO_INTERPOLATE)
	{
	  CureCERR(30) << "\nDataPort "<<slotName()<<" DataPort \"" 
		       <<"\" warning  "
		       <<"tried to pull data from an address that "
		       <<"can't interpolate.\n";
	}
      return r;
    }
    Cure::DataDescriptor d;
    result.getDescriptor(d);
    if (d.ClassType!=Slot.Descriptor.ClassType)
      return (TYPE_ERROR|RESOURCE_ERROR);
    if (d.SensorType!=Slot.Descriptor.SensorType)
      return (TYPE_ERROR|RESOURCE_ERROR);
    result.setDescriptor(Slot.Descriptor);
    return 0;
  }
  return DataSlotAddress::read(result, 
			   interpolate,t);

}
