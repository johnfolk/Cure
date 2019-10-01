// = RCSID
//    $Id: GyroAdjOdometry.cc ,v 1.1 2005/05/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2005 John Folkesson
//    
#include "GyroAdjOdometry.hh"

namespace Cure {

GyroAdjOdometry::GyroAdjOdometry()
:FuseFilter()
{
  FilterName = "GyroAdjOdometry";
  Poses[0].setCovType(11);
  Poses[1].setCovType(0);
  Poses[2].setCovType(8);
  Poses[3].setCovType(0x30);
  Poses[5].setCovType(11);
  Poses[5].setVelType(0);
  Poses[4].setCovType(48);
  Poses[6].setVelType(11);  
  Poses[5].setCoordinateType(0x8);
  Poses[6].setCoordinateType(0x0);  
  Poses[4].setCoordinateType(0x4);  
}

unsigned short GyroAdjOdometry::calc()
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
  Poses[0].limitAngles();
  Poses[1].limitAngles();
  Poses[2].limitAngles();
  Poses[3].limitAngles();
  Pose3D temp; //to be the movement in the xy plane of the initail robot pose
  temp.setCovType(11);;
  temp.setCoordinateType(0x8);
  temp=Poses[0];
  temp.setCoordinateType(0x8);
  double neweuler[3];
  Poses[3].getAngles(neweuler);
  double euler[3];
  Poses[2].getAngles(euler);
  //Here we adjust euler[0] based on Poses[0]

  double wgt[2];
  wgt[0]=Poses[0](2,2);
  wgt[1]=Poses[2](0,0)+wgt[0];
  double dtheta=Poses[0].getTheta();
  if (wgt[1]>1E-12)
    {
      wgt[0]/=wgt[1];
      double z=euler[0]-dtheta;
      z*=z;
      z/=wgt[1];
      if (z>8)
	wgt[0]=1;     
      else if((dtheta<5E-5)&&(dtheta>-5E-5))
	wgt[0]=0;
      if (Poses[2](0,0)>5)
	wgt[0]=0;
    }
  else wgt[0]=.5;
  wgt[1]=1-wgt[0];
  double dangle[3];
  euler[0]*=wgt[0];
  euler[0]+=dtheta*wgt[1];
  temp.setAngles(euler);
  temp(2,2)*=(wgt[1]*wgt[1]);
  temp(2,2)+=(Poses[2](0,0)*wgt[0]*wgt[0]);
  temp(1,2)*=(wgt[1]);
  temp(2,1)=temp(1,2);
  temp(0,2)*=(wgt[1]);
  temp(2,0)=temp(0,2);
  Poses[5].add_(Poses[4],temp);
  Poses[5].getAngles(dangle);
  dangle[1]=0;
  dangle[2]=0;
  Poses[5].setAngles(dangle);
  Transformation3D trans;
  trans-=Poses[6];
  Poses[6].Transformation3D::operator+=(Poses[5]);  
  dangle[0]=Poses[6].getTheta();
  Poses[6].setAngles(dangle);  
  Poses[6].Time=Poses[0].Time;
  Poses[5].Time=Poses[0].Time;
  Poses[4].Time=Poses[0].Time;
  Poses[4].Transformation3D::operator=(Poses[3]);
  Poses[4].setTheta(0);
  Poses[4](0,0)=Poses[3](0,0);
  Poses[4](1,0)=Poses[3](1,0);
  Poses[4](0,1)=Poses[3](0,1);
  Poses[4](1,1)=Poses[3](1,1);
  if (dt<1E-3)return 0;
  //Here we check for major bumps and ruts    
  Poses[5].getAngles(dangle);
  double bigchange=dt*5E-2;
  double tst=0;
  if (dangle[1]>bigchange)
    tst=dangle[1];
  else if  (dangle[1]<-bigchange)
    tst=-dangle[1];
  if (dangle[2]>bigchange)
    tst+=dangle[2];
  else if (dangle[2]<-bigchange)
    tst-=dangle[2];
  if (tst>0)
    {
      tst/=bigchange;
      for( int i=0; i<3; i++)
	for( int j=0; j<3; j++) Poses[5](i,j)*=tst;
    }


 //Here we check for sliding conditions    
  double d=cos(neweuler[1])*cos(neweuler[2]);
  d*=d;
  d=1-d;
  d*=2;

  double r=Poses[5].Position(0);
  r*=r;
  double temp2=euler[0];
  temp2*=temp2;
  if (temp2>1E-4)
    r/=temp2;
  else r=9;
  r/=10;
  if (r<1)r=(1/(1+r));
  else r=1;
  d*=r;
  bigchange=0; 
  if (d>.01){
    bigchange=euler[0]*euler[0];
    bigchange+=(Poses[5].Position(0)*(Poses[5].Position(0)/100));
    bigchange/=dt;
    bigchange/=dt;
    if (bigchange>.25)bigchange=.25;
  }

  if (bigchange>.0001)
    {
      bigchange*=(dt*dt);
      d*=bigchange;
      for( int i=0; i<3; i++)
	  Poses[5](i,i)+=d;
    }
  return 0;
}
unsigned short GyroAdjOdometry::typeCheck(int port,TimestampedData *inputData)
{
  unsigned short r=0;
  if (inputData){
    r=PoseFilter::typeCheck(port,inputData);
    if(r)return r;
  }
  if (Checked[port]!=0){
    Checked[port]--;
    r=TYPE_ERROR;
    if ((port==0)|(port==2))
      {
	if ((Poses[port].getSubType())&(0x8000))r=0;
	else{
	  std::cerr << "\nFilter \"" << FilterName <<"\" warning  "
		    << "Must have incremental Pose Data on port "<<port;
	  Poses[port].print();
	  Poses[port].setSubType((Poses[port].getSubType()|0x8000));
	  Poses[port].zero();
	}
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
	else r=0;
      }
  }
  if(r)return r;
  if (Checked[port]==0)return r;
  if (port==0){
    if 	(Poses[0].getCovType()==11)
      return 0;
    Poses[port].setCovType(11);
  }
  else  if (port==1) {
    if (Poses[1].getCovType()==0)
      return 0;
    Poses[port].setCovType(0);
  }
  else if (port==2){
    if (Poses[2].getCovType()==8)
      return 0;
    Poses[port].setCovType(8);
  }
  else  if (port==3) {
    if 	(Poses[3].getCovType()==0x30)
      return 0;
    Poses[port].setCovType(0x30);
  }
  std::cerr << "\nFilter \"" << FilterName <<"\" warning  "
	    << "Wrong Covtype of input to  "<<port;
  return TYPE_ERROR;
  }
} // namespace Cure
