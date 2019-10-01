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
  const char *optstring = "hi:o:t:";
  char *args = "[-h help] [-i inputfile] [-o outputfile] "
    "[-t type 0-sick, 1-odo, 2-dmu]";
  char *inputfile = "notset";
  char *outputfile = "notset";
  int type = -1;
  char o = getopt(argc, argv, optstring);
  while (o != -1) {
    switch (o) {
    case 't':
      type = atoi(optarg);
      break;
    case 'i':
      inputfile = optarg;
      break;
    case 'o':
      outputfile = optarg;
      break;
    case 'h':
    case '?':
      std::cerr << "Usage: " << argv[0] << " " << args << std::endl;
      return -1;
      break;
    }
    o = getopt(argc, argv, optstring);
  }

  FileAddress pa1,pa2;

  if (strcmp(inputfile, "notset") == 0 ||
      strcmp(outputfile, "notset") == 0) {
    std::cerr << "Usage: " << argv[0] << " " << args << std::endl;
    return -1;
  }

  if (type < 0) {
    std::cerr << "Need to set type\n";
    std::cerr << "Usage: " << argv[0] << " " << args << std::endl;
    return -1;
  }

  pa1.openWriteFile(outputfile);
  ifstream rs(inputfile);

  int lineno = 1;
   long th, tl; 
   if (type == 0) {
     SICKScan s1(0);  
    unsigned short size;
    while (rs >> size) {
      rs >> th ;
      rs >> tl ;
      s1.Time.Seconds = th;
      s1.Time.Microsec = tl;
      double angleStep = M_PI / (size - 1);
      double range[size];
      const double startAngle = 0;
      const double maxRange = 82.0;
      const double rangeResolution = 0.05;
      s1.import(s1.Time, size, range, angleStep,
                startAngle, maxRange, rangeResolution);
      s1.setScannerType(SICKScan::SCANNER_TYPE_LMS200);
      for (int i=0; i<size;i++) {
        rs >> range[i];
	range[i]/=1000;
      }
      s1 = range;
      //cerr<<s1;
      //cerr<<s1.Type<<" ";
      //cerr<<(int)s1.getClassType()<<" ";
      pa1.write(s1);      
      lineno++;
    }
  } else if (type == 1) {
    Pose3D podo;
    podo.setSubType(0x100B);
    double v[3], x[3];
    while (rs >> th >> tl >> x[0] >> x[1] >> x[2] >> v[0] >> v[1]>> v[2]) {
      podo.Time.Seconds = th;
      podo.Time.Microsec = tl;
      podo.setXYTheta(x);
      podo.Velocity[0]=v[0];
      podo.Velocity[1]=v[1];
      podo.Velocity[3]=v[2];
      pa1.write(podo);
      lineno++;
    }
    podo.print();
  } else if (type == 2) {
    Pose3D podo;
    podo.setSubType(0x4000);
    double  x[3];
    while (rs >> th >> tl >> x[0] >> x[1] >> x[2]) {
      podo.Time.Seconds = th;
      podo.Time.Microsec = tl;
      podo.setAngles(x);
      pa1.write(podo);
      lineno++;
    }
    podo.print();
    fprintf(stderr, "Last timestamp \n");
  } else {
    std::cerr << "Conversion of type " << type << " not supported\n";
    return -1;
  }

  if (rs.eof()) return 0;
  else {
    std::cerr << "\nWARNING: Did not reach end of file!\n";
    std::cerr << "WARNING: line number " << lineno << "\n\n";
    return -1;
  }
  
  /*
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
      pa1.write(s1);      
    }
  */
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
