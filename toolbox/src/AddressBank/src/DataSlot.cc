// = RCSID
//    $Id: DataSlot.cc ,v 1.1 2004/03/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2004 John Folkesson
//    
#include "DataSlot.hh"
#include "CureDebug.hh"

namespace Cure {

DataSlot::DataSlot()
{
  SubType=0;
  Packed=0;
  Size=0;
  History=0;
  Last=0;
  DoTypeCheck=false;
  BugMe=false;
  NumItems = 0;
}
DataSlot::DataSlot(const long depth, 
                   unsigned char classType,
                   unsigned short subType,
		   bool compressed,bool dotypecheck)
{
  Size=0; 
  History=0;
  NumItems = 0;
  BugMe=false;
  setup(depth,classType,subType,compressed,dotypecheck);
}
void DataSlot::setup(const long depth, 
                    unsigned char classType,
                    unsigned short subType,
		    bool compressed,bool dotypecheck)
{
  if (History)
    delete[]History;
  Size=depth;  
  Last=0;
  SubType=subType;
  Descriptor.ClassType=classType;
  History=0;
  DoTypeCheck=dotypecheck;
  Packed=compressed;
  History=new SmartData[Size];
  for (long i=0; i<Size; i++)
    History[i].setup(classType, SubType, Packed);
}
DataSlot::~DataSlot()
{
  if (History)
    delete []History;
  History=0;
}

unsigned short DataSlot::write(TimestampedData &p)
{
  if (DoTypeCheck)
    {
      DataDescriptor d;
      p.getDescriptor(d);
       if ((!Descriptor.sameClass(d))){
	return (RESOURCE_ERROR|TYPE_ERROR);
      }
    } 
  if (Size==0)return 0; 
  unsigned short r=0;
  if (p.Time<History[Last].Time)
    {
      if (BugMe)
	CureCERR(20)<<Descriptor.Name<<" timing sequence ERROR.\n"
		    <<"Writing to DataSlot with Time out of sequence!\n"
		    <<"This doesn't work correctly.  You wrote: "
		    <<p.Time<<std::endl
		    <<"And  last data:"
		    <<History[Last].Time<<std::endl;
       r= (TIMESTAMP_ERROR|TIMESTAMP_TOO_OLD);
       if (Size<3)return r;
       long ind=Last+1;
       if (ind>(Size-1))ind=0;
       long i=ind+1;
       if (i>(Size-1))i=0;
       if (p.Time<=History[ind].Time)return r;
       if (p.Time<=History[i].Time)return r;
       i=Last;
       Last=ind;
       if (NumItems < Size) NumItems++;
       while (p.Time<History[i].Time)
	 {
	   History[ind]=History[i];
	   ind=i;
	   i--;
	   if (i<0) i=(Size-1);
	 }
       if (Packed)
	 History[ind].setPacked(p);
       else
	 History[ind].setUnpacked(p);
       return r;
    }
  Last+=1;
  if (Last>(Size-1)) Last=0;
  if (Packed)
    History[Last].setPacked(p);
  else
    History[Last].setUnpacked(p);

  if (NumItems < Size) NumItems++;
  return r;
}


unsigned short DataSlot::read(TimestampedData &p,
			      int interpolate[1], const Timestamp t)
{  
  if (DoTypeCheck) {
    DataDescriptor d;
    p.getDescriptor(d);
    if ((!Descriptor.sameClass(d))){
      {
	  CureCERR(10)  << Descriptor.Name << "Type mismatch " << 
	    (int)d.ClassType <<
	    " not " << (int)Descriptor.ClassType << std::endl;
	  return (RESOURCE_ERROR|TYPE_ERROR);
	}
    }
  }
  if (Size==0)return 0; 
  if (NumItems == 0) {
    CureDO(40) { 
      if (BugMe)
	CureCERR(20) << Descriptor.Name
		     <<"ERROR: DataSlot::read(...) cannot read before anything "
		     << "is written (classType=" << (int)Descriptor.ClassType << ")\n";
    }  
    return (TIMESTAMP_ERROR|NO_DATA_AVAILABLE);
  }

  if (t==0) {
    // When t==0 we want the last data written to the slot
    History[Last].setEqual(p);
    return 0;
  } else if (t>History[Last].Time) {
    // We are asking for data that is not yet there
    History[Last].setEqual(p);
    return  (TIMESTAMP_ERROR|NO_DATA_AVAILABLE|TIMESTAMP_IN_FUTURE);
    //you want data not here yet
  }

  // Get time for oldest data written and still left in the slot
  long oIndex;
  if (NumItems < Size) {
    oIndex = 0;
  } else {
    oIndex = (Last + 1) % Size;
  }
  Timestamp timeO = History[oIndex].Time;
  
  if (t < timeO) {
    CureCERR(30) << Descriptor.Name<<
      "WARNING: DataSlot::read trying to read too old data t="
		 << t << " < oldest " << timeO << std::endl;
    History[oIndex].setEqual(p);
    return ( TIMESTAMP_ERROR | TIMESTAMP_TOO_OLD);
  }

  // Get time for last data written to slot
  long index = Last;
  Timestamp timeS=History[index].Time;

  // Create a variable that points to the oldest data in the slot
  long ind=index+1;
  if (ind>Size-1)ind=0;

  if (History[ind].Time==timeS) {
    // First and last item in slot has same timestamp, we cannot do
    // anything but return this data with a TIMESTAMP_ERROR
    History[index].setEqual(p);
    return TIMESTAMP_ERROR;
  }

  if (timeS < t) {
    //You want data in the future;
    History[index].setEqual(p);
    return (TIMESTAMP_ERROR|NO_DATA_AVAILABLE|TIMESTAMP_IN_FUTURE);
  }
  if (timeS == t) {

    // Asked for data with the same timestamp as the last data in the
    // slot but have not specifically asked for the very last one
    // since t!=0. This is for the case of multiple data with the same timestamp
    if (interpolate[0] < 0) {
         //I can't go back before I know where to start 
      History[index].setEqual(p);
      return (TIMESTAMP_ERROR|NO_DATA_AVAILABLE) ;
    }

  } else {
    // We get here when the time we ask for t is less than the last
    // one stored in the slot (time currently given by timeS).

    // Go back through time 
    while (timeS > t) {
      // Step back in time 
      index--;
      if (index<0) index=Size-1;
      timeS=History[index].Time;

      // Check if we reached the last written item again, i.e. we have
      // gone through the whole buffer
      if (index==Last) {
        // If we get here it means that the data we are looking for is
        // older than the oldest we have stored. Since we took care of
        // this case above there is some kind of logic error.
        std::cerr <<Descriptor.Name<<
	  "WARNING: DataSlot logic error! This is a bug in DataSlot "<<
	  "Alert johnfolk@mit.edu\n";

        // Step back ot the oldest one again.
        index++;
        if (index>(Size-1)) index=0;

        // We return the oldest data we have
        History[index].setEqual(p);
        return (TIMESTAMP_ERROR| TIMESTAMP_TOO_OLD) ;
      }
    }
  }  
  //This is the first one <=t going back from Last

  long high=index;
  if (interpolate[0]<0)
    {
      for (long i=0; i>interpolate[0];i--)
	{
	  index--;
	  if (index<0) index=Size-1;
	  if (index==Last) return (TIMESTAMP_ERROR| TIMESTAMP_TOO_OLD);
	}
      History[index].setEqual(p);
     if (p.Time==t) interpolate[0]--;
     else interpolate[0]=-1;
     return 0;
    }
  if ((timeS==t)&&(interpolate[0]==0))
    {
      History[index].setEqual(p);
      return 0;
    }
  while (timeS==t) 
    {
      if (index==0)index=Size;
      index--;
      timeS=History[index].Time;
      if ((index==Last)) {
	//too young but take the oldest we have.
	index++;
	if (index>(Size-1)) index=0;
	History[index].setEqual(p);
	return (TIMESTAMP_ERROR| TIMESTAMP_TOO_OLD);
      }
    }
  //This is the first one <t
  long low=index;
  if (interpolate[0]>0) {
    index++;
    if (index>(Size-1)) index=0;
    if (index==Last){
      return (TIMESTAMP_ERROR|NO_DATA_AVAILABLE);
    }   
    for (long i=0;i<interpolate[0]; i++)
      {
	if (index==Last)return (TIMESTAMP_ERROR|NO_DATA_AVAILABLE);
	index++;
	if (index>(Size-1)) index=0;
      }
    History[index].setEqual(p);
    if (p.Time==t) interpolate[0]++;
    else interpolate[0]=1;
    return 0;
  }
  if (high==Last)return (TIMESTAMP_ERROR|NO_DATA_AVAILABLE);
  high++;
  if (high>(Size-1)) high=0;
  Interpolator.interpolate_(History[low],History[high],t);
  Interpolator.Time=t;
  Interpolator.setEqual(p);
  p.Time=t;
  return 0;
}

} // namespace Cure
