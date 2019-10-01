// = RCSID
//    $Id: PoseErrorModel.cc ,v 1.1 2004/03/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2004 John Folkesson
//    
/**
 * @author John Folkesson.
 */
#include "PoseErrorModel.hh"


namespace Cure {
PoseErrorModel::PoseErrorModel()
  :DataFilter(1, 3, true) // :PoseFilter(1, 2)
{
  Poses[0].setSubType(0x8000);
  Poses[2].setTime(-1);
  OutSet.setSetSize(2);
  OutSet(0)=Poses[1];
  OutSet(1)=Poses[2];
  FilterName = "PoseErrorModel";
}

void PoseErrorModel::precalc(int)
{
  // If this is the first time we write to the PoseErrorModel we must
  // make sure that the output is the same as the input to force the
  // increment to be 0.
  if (Poses[2].getDoubleTime() < 0) {
    Poses[2] = Poses[0];
  }
}

//Here you should change from P=I*dt, the simple example error model    
unsigned short PoseErrorModel::calc()
{
  if (!(Poses[2].Time<Poses[0].Time))
    return (TIMESTAMP_ERROR|TIMESTAMP_TOO_OLD);
  if (ErrorCode&NO_DATA_AVAILABLE)return (TIMESTAMP_ERROR|NO_DATA_AVAILABLE);
  //Poses[0] has our new cumulated data and timestamp.
  //Poses[2] has the last cumulated data and timestamp.

  Poses[1].Time=Poses[0].Time;
  Poses[1].Time-=Poses[2].Time;
  //You might base some uncertainty on dt the time interval for the increment
  double dt=Poses[1].Time.getDouble();  
  //Here we calculate the increamental change
  Poses[1].Transformation3D::operator=(Poses[2].inverse()+Poses[0]);
  Poses[1].Time=Poses[0].Time;
  Poses[1].Covariance=0; 
  double dx[6];
  Poses[0].getCoordinates(dx);
  double v[3];
  Poses[2].getAngles(v);
  dx[0]-=Poses[2].Position(0);
  dx[1]-=Poses[2].Position(1);
  dx[2]-=Poses[2].Position(2);
  dx[3]-=v[0];
  dx[4]-=v[1];
  dx[5]-=v[2];
  if (dt>1E-6);
  {
    for (int i=0;i<6;i++)
      Poses[2].Velocity[i]=(dx[i]/dt);
  }
  Poses[2].Transformation3D::operator=(Poses[0]);
  Poses[2].Time=Poses[0].Time; 
  //Here you should change from P=I*dt, the simple example error model
  Poses[1].Covariance=dt;
  OutSet(0)=Poses[1];
  OutSet(1)=Poses[2];
  return 0;
}
unsigned short PoseErrorModel::typeCheck(int port,TimestampedData * inputData)
{

  unsigned short r=0;
  if (inputData)
    {
      if (inputData->getClassType()==POSE3D_TYPE)
	{
	  if (inputData->isPacked())
	    {
	      inputData->narrowPackedData()->unpack(*data(port));
	    }
	  else *data(port)=*inputData;   
	  r=0;
	}
      else if (inputData->narrowSmartData())
	{
	  SmartData *sd=inputData->narrowSmartData();
	  if (sd->getTPointer()->getClassType()==POSE3D_TYPE)
	    {
	      sd->setEqual(*data(port));
	      r=0;
	    }
	}      
      if (r!=0){
	std::cerr << "\nPoseFilter \"" << FilterName <<"\" warning  "
		  << "Wrong type of input to  "<<port<<":\n";
	return TYPE_ERROR;
      }
    }
  r=0;
  if (!Checked[port])return r;
  Checked[port]--;
  if ((port==0))
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
