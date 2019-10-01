// = RCSID
//    $Id: PanTiltModel.cc ,v 1.1 2004/03/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2004 John Folkesson
//    
#include "PanTiltModel.hh"

namespace Cure {

  PanTiltModel::PanTiltModel():PoseErrorModel()
{
  TimeError=25E-4;
  Poses[0].setCoordinateType(0x4);
  Poses[1].setCoordinateType(0xC);
  Poses[2].setCoordinateType(0x4);
  Poses[1].setCovType(0);
  Poses[2].setCovType(0x28);
  Poses[2].setVelType(0x28);
}

unsigned short PanTiltModel::calc()
{
  if (!(Poses[2].Time<Poses[0].Time))
    return (TIMESTAMP_ERROR|TIMESTAMP_TOO_OLD);
  if (ErrorCode&NO_DATA_AVAILABLE)return (TIMESTAMP_ERROR|NO_DATA_AVAILABLE);
  //Poses[0] has our new cumulated data and timestamp.
  //Poses[2] has the last cumulated data and timestamp.
  Poses[1].Time=Poses[0].Time;
  Poses[1].Time-=Poses[2].Time;
  //We base uncertainty on dt the time interval for the increment
  double dt=Poses[1].Time.getDouble();  
  Poses[1].Time=Poses[0].Time;
  Poses[2].Time=Poses[0].Time;
  Poses[1].Transformation3D::operator=((Poses[2].inverse())+Poses[0]);  
  if (dt>1E-6);
  {
    Poses[0].getAngles(&Poses[2].Velocity[3]);
    double v[3];
    Poses[2].getAngles(v);
    Poses[2].Velocity[3]-=v[0];
    Poses[2].Velocity[5]-=v[2];
    Poses[2].Velocity[3]/=dt;
    Poses[2].Velocity[5]/=dt;
    Poses[2].Velocity[4]=0;
  }
  Poses[2].Transformation3D::operator=(Poses[0]);
  for (int i=0;i<2; i++)
    for (int j=0;j<2;j++)
      Poses[2](i,j)=Poses[2].Velocity[3+(2*i)]*Poses[2].Velocity[3+(2*j)]*
	TimeError;
  OutSet(0)=Poses[1];
  OutSet(1)=Poses[2];
  return 0;
}

unsigned short PanTiltModel::typeCheck(int port,TimestampedData *inputData)
{
   unsigned short r=0;
  if (inputData)
    {
      r=PoseErrorModel::typeCheck(port,inputData);
      if (r)return r;
    }  
  if (!Checked[port])return r;
  Checked[port]--;
  if ((port==0))
    {
      if (((Poses[port].getSubType())&(0xA000))){
	std::cerr << "\nFilter \"" << FilterName <<"\" warning  "
		  << "Must pull cumulated Rotation Data to port "<<port;
	Poses[port].setSubType((Poses[port].getSubType()&0x7FFF));
	Poses[port].zero();
      } 
      else return 0;
    }
  return TYPE_ERROR;
}

} // namespace Cure
