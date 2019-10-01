// = RCSID
//    $Id: PoseFilter.cc ,v 1.1 2004/03/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2004 John Folkesson
//    
#include "PoseFilter.hh"

using namespace Cure;

PoseFilter::PoseFilter(short nIn, short nOut,unsigned short thread)
  :DataFilter(nIn, nOut, true, thread) // We use a trigger port
{ 
  Poses = new Pose3D[Ins + Outs];
  FilterName = "PoseFilter"; 
}

PoseFilter::~PoseFilter() 
{
  if (Poses) {
    delete [] Poses;
    Poses = 0;
  }
}
unsigned short Cure::PoseFilter::typeCheck(int port, TimestampedData *inputData)
{
  if (inputData)
    {
      if (inputData->getClassType()==POSE3D_TYPE)
	{
	  if (inputData->isPacked())
	    {
	      inputData->narrowPackedData()->unpack(*data(port));
	    }
	else *data(port)=*inputData;   
	  return 0;
	}
      else if (inputData->narrowSmartData())
	{
	  SmartData *sd=inputData->narrowSmartData();
	  if (sd->getTPointer()->getClassType()==POSE3D_TYPE)
	    {
	      sd->setEqual(*data(port));
	      return 0;
	    }
	}      
      std::cerr << "\nPoseFilter \"" << FilterName <<"\" warning  "
		<< "Wrong type of input to  "<<port<<":\n";
      return TYPE_ERROR;
    }
  return 0;
}


