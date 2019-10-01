#include <iostream>
#include <stdlib.h> 

#include "Matrix.hh" 
#include "Pose3D.hh"
#include "Pose2D.hh"

int main(int argc, char * argv[])
{

  std::cout << "------------ xsR ----------\n";
  Cure::Pose3D xsR;
  xsR.setX(0.1);
  xsR.setTheta(-M_PI_2);
  xsR.setCovType(0);
  xsR.print();

  std::cout << "------------ xrW ----------\n";
  Cure::Pose3D xrW;
  xrW.setX(1);
  xrW.setY(2);
  xrW.setTheta(0);
  xrW.setCovType(11);
  xrW.Covariance(0,0) = xrW.Covariance(1,1) = 1;
  xrW.Covariance(2,2) = 0.1;
  xrW.print();

  std::cout << "------------ xfW ----------\n";
  Cure::Pose3D xfW;
  xfW.setX(2);
  xfW.setY(2);
  xfW.setZ(2);
  xfW.setCovType(7);
  xfW.Covariance(0,0) = xfW.Covariance(1,1) = xfW.Covariance(2,2) = 1;
  xfW.print();

  std::cout << "------------ xfS ----------\n";
  Cure::Pose3D xfS;
  xfS.setCovType(63);
#if 0
  xfS.minusMinus(xsR, xrW);
#else
  xfS.minus(xsR);
  xfS.subtract(xfS, xrW);
#endif
  //  xfS.add(xfS, xfW);
  xfS.print();
  return 0;


  struct timeval tv;
  tv.tv_sec = 1;
  tv.tv_usec = 300000;

  Cure::Timestamp t0;
  Cure::Timestamp t1(1,200000);
  Cure::Timestamp t2(tv);
  Cure::Timestamp t3(t1); 
  Cure::Timestamp t4(1.1);
  Cure::Timestamp t5(0,100000);

  std::cerr << "t0=" << t0 << " "
            << "t1=" << t1 << " "
            << "t2=" << t2 << " "
            << "t3=" << t3 << " "
            << "t4=" << t4 << " "
            << "t5=" << t5 << std::endl;
  std::cerr << "t1<t2=" << (t1<t2) << std::endl;
  std::cerr << "t1<t3=" << (t1<t3) << std::endl;
  std::cerr << "t1<=t3=" << (t1<=t3) << std::endl;
  std::cerr << "t2>t1=" << (t2>t1) << std::endl;
  std::cerr << "t3>t1=" << (t3>t1) << std::endl;
  std::cerr << "t3>=t1=" << (t3>=t1) << std::endl;
  std::cerr << "t3>=t1=" << (t3>=t1) << std::endl;
  std::cerr << "t3==t1=" << (t3==t1) << std::endl;
  std::cerr << "t3==t2=" << (t3==t2) << std::endl;
  std::cerr << "t3!=t1=" << (t3!=t1) << std::endl;
  std::cerr << "t3!=t2=" << (t3!=t2) << std::endl;
  std::cerr << "t1+t5=" << (t1+t5) << std::endl;
  std::cerr << "t1-t5=" << (t1-t5) << std::endl;
  std::cerr << "t0-t5=" << (t0-t5) << std::endl;

  Cure::Timestamp t6=t0;
  Cure::Timestamp t7=t1;
  std::cerr << "t6=" << t6 << std::endl;
  std::cerr << "t7=" << t7 << std::endl;
  t6-=t7;
  std::cerr << "t6=" << t6 << std::endl;
  std::cerr << "t6=" << t6.getDouble() << std::endl;

  Cure::Transformation3D t3d1, t3d2, t3d3;
  double x[6];
  x[0]=1;
  x[1]=2;
  x[3]=M_PI;
  x[2]=0;
  x[4]=0;
  x[5]=0;
  t3d2=x;
  x[0]-=.5;
  t3d3=x;
  //t3d2.print();
  //t3d3.print();

  //  (t3d2.inverse()+t3d3).print();
  
  //t3d2.print();
  //(t3d2.inverse()).print();

  Cure::Pose3D p1,p2,p3;
  p3.setCovType(8);
  p1.setCovType(24);

  x[4]=.5;
  x[5]=.5;
  p1=x;
  x[3]+=.1;
  p2=x;
  
  p3.minusPlus_(p1,p2);
  p1.print();
  p2.print();
  p3.print();

  Cure::Pose3D  tempdp, tempp;  
  
  //tempp.setTheta(0);
  tempdp.setCovType(8); //cov theta,
  //tempp.setType(0);
  x[3]=-.0005;
  x[4]=-.00621918;
  x[5]=.0228204;
  p1=x;
  x[3]=-6.4E-8;
  x[4]=-.0041;
  x[5]=.022341;
  p3=x;
  p1.setSubType(8);
  p2.setSubType(0);
  p3.setSubType(0);
  tempp=p3;
  tempdp.minusPlus_(tempp,p1);
  std::cerr<<"one"<<std::endl;
  p1.print();
  p3.print();
  tempp.print();
  std::cerr<<"dp"<<std::endl;
  tempdp.print();
  tempp.Transformation3D::operator=(p3.Transformation3D::operator+(tempdp));
  std::cerr<<"five"<<std::endl;
  tempp.print();

  return 0;





  
 
  
  return 0;
} 
