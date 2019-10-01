#ifndef DEPEND
#include <iostream>
#include <stdlib.h> 
#endif

#include "Matrix.hh" 
#include "Pose3D.hh"
#include "Point3D.hh"

using namespace std;
using namespace Cure;

int main(int argc, char * argv[])
{
  Point3D p1, p2, p3;
  double x[6];
  x[0]=1;
  x[1]=2;
  x[2]=M_PI_2;
  Transformation3D T1;
  T1.setXYTheta(x);
  p1=x;
  p1.print();
  Point2D p4(p1);
  p4.print();
  p2=p4;
  p2.print();
  p3=p1+p4;

  p3.print();
  cerr<<p3.getDistance(p2)<<endl;
  p3.getPolarValues(x);
  p1=x;
  p1.print();
  T1.print();
  p1.transform2D(T1,p3);
  p1.print();
  p3.invTransform2D(T1,p1);
  p3.print();
  p1.rotate2D(T1,p3);
  p1.print();
  p3.invRotate2D(T1,p1);
  p3.print();

  return 0;
} 
