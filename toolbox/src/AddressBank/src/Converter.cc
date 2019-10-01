#include <iostream>
#include <stdlib.h> 

#include "Matrix.hh" 
#include "Pose3D.hh"
#include "Pose2D.hh"
#include "FileAddress.hh"
#include <fstream>

using namespace std;
using namespace Cure;

int main(int argc, char * argv[])
{


  
  FileAddress pa1,pa2;
  if (argc>1)
    pa1.openWriteFile(argv[1]);
  ifstream rs(argv[2]);
  SICKScan s1(0);  
  unsigned short size;
  rs>>size;
  cerr<<size;
  rs>>s1.Time.Seconds>>s1.Time.Microsec;
  double angleStep=M_PI/(size-1);
  long scans[size];
  double range[size];
  for (int i=0; i<size;i++)
    {
      rs>>scans[i];
      range[i]=(double)scans[i]/1000;
    }
  s1.import(s1.Time,size,range,angleStep);
  s1=range;
  cerr<<s1;
  cerr<<s1.Type<<" ";
  cerr<<(int)s1.getClassType()<<" ";
  pa1.write(s1);      
  
  while(rs>>size)
    {
      rs>>s1.Time.Seconds>>s1.Time.Microsec;
      for (int i=0; i<size;i++)
	{
	  rs>>scans[i];
	  range[i]=(double)scans[i]/1000;
	}
      s1=range;
      pa1.write(s1);      
    }
  /*
  Pose3D p1, p2, p3;  
  p1.setType(16384);//dmu
  while(rs>>p1.Time.Seconds>>p1.Time.Microsec)
    {
      rs>>p1.EulerAngles[0]>>p1.EulerAngles[1]>>p1.EulerAngles[2];
      pa1.write(p1);      
    }
*/
  /*
  p1.setType(4107);//odometer
  double theta=0;
  while(rs>>p1.Time.Seconds>>p1.Time.Microsec)
    {
      rs>>p1.Position(0)>>p1.Position(1)>>theta;
      p1.setTheta(theta);
      rs>>p1.Velocity[0]>>p1.Velocity[1]>>p1.Velocity[3];
      pa1.write(p1);      
    }
  */  
  return 0;
} 
