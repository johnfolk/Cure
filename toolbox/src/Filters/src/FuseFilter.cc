// = RCSID
//    $Id: FuseFilter.cc ,v 1.1 2004/03/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2004 John Folkesson
//    
#include "FuseFilter.hh"

namespace Cure {

FuseFilter::FuseFilter()
  :PoseFilter( 4, 3)
{
  Poses[0].setSubType(0x8000);
  Poses[2].setSubType(0x8000);
  Poses[5].setSubType(0x8000);
  FilterName = "FuseFilter";
}
 unsigned short FuseFilter::calc()
{
  if (!(Poses[6].Time<Poses[0].Time))
    return (TIMESTAMP_ERROR|TIMESTAMP_TOO_OLD);
  if (ErrorCode&NO_DATA_AVAILABLE)return (TIMESTAMP_ERROR|NO_DATA_AVAILABLE);
  Timestamp t=Poses[0].Time;
  t-=Poses[2].Time;
  double dt=t.getDouble();
  if ((dt>.01)||(dt<-.01))return TIMESTAMP_ERROR;
 
  t=Poses[0].Time;
  t-=Poses[6].Time;
  dt=t.getDouble();  
  Poses[5]=Poses[0];
  Poses[4]=Poses[1];
  return 0;
}
unsigned short FuseFilter::typeCheck(int port,TimestampedData *inputData)
{
  unsigned short r=0;
  if (inputData){
    r=PoseFilter::typeCheck(port,inputData);
    if(r)return r;
  }
  if (Checked[port]==0)return r;
  Checked[port]--;
  if ((port==0)|(port==2))
    {
      if ((Poses[port].getSubType())&(0x8000))return 0;
      std::cerr << "\nFilter \"" << FilterName <<"\" warning  "
		<< "Must have incremental Pose Data on port "<<port;
      Poses[port].print();
      Poses[port].setSubType((Poses[port].getSubType()|0x8000));
      Poses[port].zero();
    } 
  else if ((port==1)|(port==3))
    {
      if (((Poses[port].getSubType())&(0x8000))){
	std::cerr << "\nFilter \"" << FilterName <<"\" warning  "
		  << "Must pull cumulated Pose Data to port "<<port;
	Poses[port].print();
	Poses[port].setSubType((Poses[port].getSubType()&0x7FFF));
	Poses[port].zero();
      } 
      else return 0;
    }
  return TYPE_ERROR;
}

} // namespace Cure
