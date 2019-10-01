// = RCSID
//    $Id: SimpleOdoModel.cc ,v 1.1 2004/06/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2004 John Folkesson
//    

#include "SimpleOdoModel.hh"
#include "MatrixStuff.hh"
#ifndef DEPEND
#include <sstream>
#endif

namespace Cure {

SimpleOdoModel::SimpleOdoModel(): PoseErrorModel()
{
  Poses[1].setCovType(0x000B);
  Poses[2].setCovType(0x0000);
  Poses[1].setCoordinateType(0x9);
  Poses[2].setCoordinateType(1);

  CovDistPerMeter     =.0001;
  CovAngPerRad        =.0001;
  CovAngPerMeter      = 2E-7;
  CovNormalPerMeter   = 1E-5;
  CovXYPerRad         = 1E-8;
  BiasAngPerMeter     = 0;
  BiasAngPerRad       = 0;
  BiasDistPerMeter    = 0;

  // Initialize the time to something <0 so we can tell if it is the
  // first iteration
  m_LastPose.setTime(-1);

  FilterName = "SimpleOdoModel";
}

int SimpleOdoModel::config(const std::string &params)
{
  std::istringstream str(params);
  int version = -1;

  if ( !(str >> version)) {
    std::cerr << "SimpleOdoModel::config() "
              << "Failed to read version number for config params list\n";
    return 1;
  }

  int ret = 0;
  switch (version) {
  case 1:
    ret = configVer1(params);
    break;
  default:
    std::cerr << "SimpleOdoModel::config() "
              << "Cannot handle config version " << version << std::endl;
    return 1;
  }

  if (ret) return ret;

  std::cerr << "Successfully configured SimpleOdoModel with \""
            << params << "\" which gave:" 
            << " CovDistPerMeter=" << CovDistPerMeter
            << " CovAngPerRad=" << CovAngPerRad
            << " CovAngPerMeter=" << CovAngPerMeter
            << " CovNormalPerMeter=" << CovNormalPerMeter
            << " CovXYPerRad=" << CovXYPerRad
            << " BiasDistPerMeter=" << BiasDistPerMeter
            << " BiasAngPerMeter=" << BiasAngPerMeter
            << " BiasAngPerRad=" << BiasAngPerRad
            << std::endl;

  return 0;
}

int
SimpleOdoModel::configVer1(const std::string &params)
{
  std::istringstream str(params);
  int version = -1;

  if (str >> version >> CovDistPerMeter >> CovAngPerRad >> CovAngPerMeter
      >> CovNormalPerMeter >> CovXYPerRad
       >> BiasDistPerMeter >> BiasAngPerMeter >> BiasAngPerRad) {
    return 0;
  } 

  std::cerr << "Failed to config SimpleOdoModel version 1 with \""
            << params << "\"\n";
  return 1;
}

unsigned short SimpleOdoModel::calc()
{
  if (!(Poses[2].Time<Poses[0].Time))
    return (TIMESTAMP_ERROR|TIMESTAMP_TOO_OLD);
  if (ErrorCode&NO_DATA_AVAILABLE)return (TIMESTAMP_ERROR|NO_DATA_AVAILABLE);
  Poses[1].Time=(Poses[0].Time);
  Poses[1].Time-=(Poses[2].Time);
  double dt=Poses[1].Time.getDouble();
  if (dt<.0001){
    Poses[1].Time=(Poses[2].Time);
    return 0;
  }
  //Here we calculate the incremental change in xytheta

  // ========== BEGIN  Bias compensation ==========
  if (BiasDistPerMeter != 0 || BiasAngPerRad != 0 || BiasAngPerMeter != 0) {
    // Initialize the last pose that we use to calculate increments 
    // on the input  
    if (m_LastPose.getDoubleTime() < 0) {
      m_LastPose = Poses[0];
    }

    // Calculate the incremental motion

    // Distance moved
    double dist = hypot(Poses[0].Position.getY() - m_LastPose.Position.getY(),
                        Poses[0].Position.getX() - m_LastPose.Position.getX());


    // Change in direction
    double da = Poses[0].getTheta() - m_LastPose.getTheta();
    while (da < -M_PI) da += 2.0 * M_PI;
    while (da > M_PI) da -= 2.0 * M_PI;

    // If we do not sample fast enough we get better estimation of the
    // motion by calculating the direction that the robot has
    // moved. This way, even if the robot has moved aong an arc for
    // example we move the robot to the end of the arc and not this
    // distance but along the tangent of the beginning of the arc.
    // Get dir of motion on odom frame
    double dir =  atan2(Poses[0].Position.getY() - m_LastPose.Position.getY(),
                        Poses[0].Position.getX() - m_LastPose.Position.getX());
    // Get it relative to the robot previous robot pose
    dir -= m_LastPose.getTheta();

    // Store the uncompensated input
    Pose3D tmp = Poses[0];
    
    // Compensate for distance scale bias
    dist *= (1.0 + BiasDistPerMeter);

    // Compensate for angular drift
    da += da * BiasAngPerRad + dist * BiasAngPerMeter;

    double a = Poses[2].getTheta() + dir;
    Poses[0].Position.setX(Poses[2].Position.getX() + dist * cos(a));
    Poses[0].Position.setY(Poses[2].Position.getY() + dist * sin(a));
    Poses[0].setTheta(Poses[2].getTheta() + da);

    m_LastPose = tmp;
  }
  // =========== END Bias compensation ============
  

  Poses[1].Transformation3D::operator=((Poses[2].inverse())+Poses[0]);
  //Poses[1].minusPlus_(Poses[2],Poses[0]);
  // Poses[1].setSubType(0x12C0);//(0x92C8);//704
  Poses[1].Time=Poses[0].Time;
  Poses[2]=Poses[0];
  double dx[3];
  Poses[1].getXYTheta(dx);
  Poses[1].Covariance=0;
  //---This rather arbitrary formula works well for Pluto's Skid Steering
  double dtheta=dx[2];
 if(dtheta<0)dtheta=(-dtheta);
  double ds=(dx[0]*dx[0]+dx[1]*dx[1]);
  ds=sqrt(ds);  
  double cnorm=(CovNormalPerMeter*ds);
  double cxy=CovXYPerRad*dtheta;
  if ((ds>1E-5)||(dtheta>1E-5))
    {
      dtheta*=CovAngPerRad;
      double d=(ds*CovAngPerMeter + 5E-12);
      ds*=CovDistPerMeter;
      if (ds<1E-10)ds=1E-10;
      dtheta+=d;
      double jx[6];
      jx[4]=0;
      jx[5]=1;
      if ((dx[2]<.025)&&(dx[2]>-.025))
	{
	  jx[0]=1;
	  jx[1]=-dx[1];
	  jx[2]=0;
	  jx[3]=dx[0];
	}
      else 
	{
	  double s=Poses[1].Trig[1];
	  jx[0]=s/dx[2];
	  jx[1]=((dx[2]-s)/(dx[2]*s));
	  jx[1]*=dx[0];
	  jx[1]-=dx[1];
	  jx[2]=(1-Poses[1].Trig[0])/dx[2];
	  jx[3]=(dx[0]-(dx[1]/dx[2]));
	}
      double min=5E-7;
      if (dtheta>ds)
	{
	  min*=dtheta;
	  if (ds<min)ds=min;
	}       
      else 
	{
	  min*=ds;
	  if (dtheta<min)dtheta=min;
	}
      min+=cnorm;
      double ev[3];
      ev[0]=jx[2]*jx[5]-jx[4]*jx[3];
      ev[1]=jx[4]*jx[1]-jx[0]*jx[5];
      ev[2]=jx[0]*jx[3]-jx[2]*jx[1];
      short irow=0;
      for (short i=0;i<3;i++,irow+=2)
	{
	  short jrow=0;
	  for (short j=0;j<3;j++,jrow+=2)
	    Poses[1](i,j)=jx[irow]*jx[jrow]*ds+
	      jx[irow+1]*jx[jrow+1]*dtheta+min*ev[i]*ev[j];
	}
      Poses[1](0,0)+=cxy;
      Poses[1](1,1)+=cxy;
      
   }
  OutSet(0)=Poses[1];
  OutSet(1)=Poses[2];
  return 0;
}
unsigned short SimpleOdoModel::typeCheck(int port, TimestampedData * inputData)
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
      if (((Poses[port].getSubType())&(0xE000))){
	std::cerr << "\nFilter \"" << FilterName <<"\" warning  "
		  << "Must input cumulated 2D Pose to port "<<port;
	Poses[port].setSubType((Poses[port].getSubType()&0x7FFF));
	Poses[port].zero();
      } 
      else return 0;
    }
  if ((port==0))
    {
      if ((((Poses[port].getSubType())&(0x02C0)))&(0xFFFF))return 0;
      std::cerr << "\nSimpleOdoModel \"" << FilterName <<"\" warning  "
		<< "Must pull cumulated Pose Data to port "<<port<<"\n";
      Poses[port].setSubType(0x72C0);
      Poses[port].zero();
    } 
  return TYPE_ERROR;
}
} // namespace Cure
