// = RCSID
//    $Id: GyroModel.cc ,v 1.1 2004/03/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2004 John Folkesson
//    
#include "GyroModel.hh"

#ifndef DEPEND
#include <sstream>
#endif

namespace Cure {

  GyroModel::GyroModel():PoseErrorModel()
{
  ThetaBias=0;
  ThetaScaleBias=1.00;
  GyroErrorRate=1E-6;
  TimeError=25E-6;
  VerticalError=1E-6;
  PrevTheta=0;
  Poses[0].setCoordinateType(0x4);
  Poses[1].setCoordinateType(0xC);
  Poses[2].setCoordinateType(0x4);
  Poses[1].setCovType(8);
  Poses[2].setCovType(0x30);
  Poses[2].setVelType(0x30);
  FilterName="GyroModel";
}
int GyroModel::config(const std::string &params)
{
  std::istringstream str(params);
  int version = -1;

  if ( !(str >> version)) {
    std::cerr << "GyroModel::config() "
              << "Failed to read version number for config params list\n";
    return 1;
  }

  int ret = 0;
  switch (version) {
  case 1:
    ret = configVer1(params);
    break;
  default:
    std::cerr << "Gyro::config() "
              << "Cannot handle config version " << version << std::endl;
    return 1;
  }

  if (ret) return ret;
  std::cerr << "Successfully configured GyroModel with \""
            << params << "\" which gave:" 
            << " GyroErrorRate=" << GyroErrorRate
            << " TimeError=" << TimeError
            << " VerticalError=" << VerticalError
            << " ThetaBias=" << ThetaBias
            << " ThetaScaleBias=" << ThetaScaleBias
             << std::endl;

  return 0;
}

int
GyroModel::configVer1(const std::string &params)
{
  std::istringstream str(params);
  int version = -1;

  if (str >> version >> GyroErrorRate >> TimeError
      >> VerticalError >> ThetaBias
       >> ThetaScaleBias) {
    return 0;
  } 

  std::cerr << "Failed to config SimpleOdoModel version 1 with \""
            << params << "\"\n";
  return 1;
}

unsigned short GyroModel::calc()
{
  if (!(Poses[2].Time<Poses[0].Time))
    return (TIMESTAMP_ERROR| TIMESTAMP_TOO_OLD);
  if (ErrorCode&NO_DATA_AVAILABLE)return NO_DATA_AVAILABLE;
  //Poses[0] has our new cumulated data and timestamp.
  //Poses[2] has the last cumulated data and timestamp.
  Poses[1].Time=Poses[0].Time;
  Poses[1].Time-=Poses[2].Time;
  //We base uncertainty on dt the time interval for the increment
  double dt=Poses[1].Time.getDouble();  
  Poses[1].Time=Poses[0].Time;
  Poses[2].Time=Poses[0].Time;
  double d=Poses[0].getTheta();
  Poses[0].setTheta(Poses[2].getTheta()+
		    (d-PrevTheta-ThetaBias*dt)*ThetaScaleBias);
  PrevTheta=d;
  Poses[1].Transformation3D::operator=((Poses[2].inverse())+Poses[0]);  
  if (dt>1E-6);
  {
    Poses[0].getAngles(&Poses[2].Velocity[3]);
    double v[3];
    Poses[2].getAngles(v);
    Poses[2].Velocity[3]-=v[0];
    Poses[2].Velocity[4]-=v[1];
    Poses[2].Velocity[5]-=v[2];
    Poses[2].Velocity[3]/=dt;
    Poses[2].Velocity[4]/=dt;
    Poses[2].Velocity[5]/=dt;
  }
  Poses[2].Transformation3D::operator=(Poses[0]);
  Poses[1].Covariance=dt*GyroErrorRate;
  for (int i=0;i<2; i++)
    for (int j=0;j<2;j++)
      Poses[2](i,j)=Poses[2].Velocity[4+i]*Poses[2].Velocity[4+j]*TimeError;
  Poses[2](0,0)+=VerticalError;
  Poses[2](1,1)+=VerticalError;
  OutSet(0)=Poses[1];
  OutSet(1)=Poses[2];
  return 0;
}
unsigned short GyroModel::typeCheck(int port,TimestampedData *inputData)
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
	Poses[port].print();
	Poses[port].setSubType((Poses[port].getSubType()&0x7FFF));
	Poses[port].zero();
      } 
      else return 0;
    }
  return TYPE_ERROR;
}

} // namespace Cure
