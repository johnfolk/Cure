#include <iostream>
#include <stdlib.h> 

#include "Matrix.hh" 
#include "Pose3D.hh"
#include "Pose2D.hh"
#include "FileAddress.hh"
#include "SmartData.hh"
#include <fstream>
#include "SocketAddress.hh"
#include "DebugAddress.hh"
#include "GenericData.hh"
#include "TimestampedString.hh"

using namespace std;
using namespace Cure;

int main(int argc, char * argv[])
{

  Cure::MeasurementSet ms,ms1;
  ms.setNumberOfElements(1);
  ms.Time=54.2;
  ms(0).W.reallocate(1,4);
  ms(0).V.reallocate(1);
  ms(0).V=3;
  ms(0).W(0,0)=2.1;
  DataSlotAddress dsa(256,MEASUREMENTSET_TYPE,0,false,false,0);
  dsa.write(ms);
  ms.Time=64.2;
  ms(0).W(0,0)=1.1;
  ms.print();
  dsa.write(ms);
  ms1.Time= 50.3;

  ms.print();
  ms(0).print();
  ms(0).W.print();
  ms1.print();
  dsa.read(ms1,ms1.Time);
  ms1.print();
  ms1(0).W.print();
  getchar();
  Cure::Hal::SocketAddress *pingSock;
  pingSock=new Cure::Hal::SocketAddress("PingSock",0);
  pingSock->m_Hostname="127.0.0.1";
  pingSock->m_Server=0; 
  pingSock->m_Port=3316;
  pingSock->m_Verbose=25;
  pingSock->m_RetryPeriod=1.0;
  pingSock->m_Persistent=1;
  pingSock->startDevice();
  DebugAddress bugsy;
  bugsy.m_Level=1;


  Cure::Hal::SocketAddress *pingSockS;
  pingSockS=new Cure::Hal::SocketAddress("PingSockServer",0);
  pingSockS->m_Hostname="127.0.0.1";
  pingSockS->m_Server=1; 
  pingSockS->m_Port=3316;
  pingSockS->m_Verbose=25;
  pingSockS->m_RetryPeriod=0.0;
  pingSock->m_Persistent=1;
  pingSockS->startDevice();
  // pingSockS->push(&bugsy);
  Cure::TimestampedString str, str2;
  
  str.Time.setToCurrentTime();
  str2.Time.setToCurrentTime();
  str.print();
  char tmp[200];
  snprintf(tmp,200,"../live/logs/Sonar080417_0959_23.son");
  str=tmp;
  str.setID(5);
  str.setStringType(57);
  str.print();
  getchar();
  tmp[3]=0;
  tmp[4]=0;
  for (int i=0;i<5;i++){
    str.print();
    pingSock->write(str);
    //    pingSock->write(ms);
    str.Time=str.Time.getDouble()+.1;
    getchar();
  }
  delete pingSockS;
  for (int i=0;i<1;i++){
    str.print();
    pingSock->write(str);
    // pingSock->write(ms);
    str.Time=str.Time.getDouble()+.1;
    getchar();
  }
  pingSockS=new Cure::Hal::SocketAddress("PingSockServer",0);
  pingSockS->m_Hostname="127.0.0.1";
  pingSockS->m_Server=1; 
  pingSockS->m_Port=3316;
  pingSockS->m_Verbose=45;
  pingSockS->m_RetryPeriod=0.0;
  pingSock->m_Persistent=1;
  pingSockS->startDevice();
  // pingSockS->push(&bugsy);
  for (int i=0;i<5;i++){
    str.print();
    pingSockS->write(str);
    //pingSock->write(ms);
    str.Time=str.Time.getDouble()+.1;
    getchar();
  }
  delete pingSock;
  for (int i=0;i<1;i++){
    str.print();
    pingSockS->write(str);
    // pingSockS->write(ms);
    str.Time=str.Time.getDouble()+.1;
    getchar();
  }
  pingSock=new Cure::Hal::SocketAddress("PingSock",0);
  pingSock->m_Hostname="127.0.0.1";
  pingSock->m_Server=0; 
  pingSock->m_Port=3316;
  pingSock->m_Verbose=5;
  pingSock->m_RetryPeriod=5.0;
  pingSock->startDevice();
  getchar();
  for (int i=0;i<5;i++){
    str.print();
    pingSock->write(str);
    // pingSock->write(ms);
    str.Time=str.Time.getDouble()+.1;
    getchar();
  }

  delete pingSock;
  delete pingSockS;
  return 0;

  const char *optstring = "hi:o:";
  char *args = "[-h help] [-i inputfile] [-o outputfile]";
  char *inputfile = "notset";
  char *outputfile = "notset";
  char o = getopt(argc, argv, optstring);
  while (o != -1) {
    switch (o) {
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

  if (strcmp(inputfile,"notset")==0) {
    std::cerr << "Usage: " << argv[0] << " " << args << std::endl;
    return -1;
  }
  
  FileAddress pa1,pa2;
  if(strcmp(outputfile,"notset")!=0) pa1.openWriteFile(outputfile);
  ifstream rs(inputfile);
  SICKScan s1(0);  
  unsigned short size;
  double t;
  rs>>t>>size;
  cerr << size << std::endl;
  s1.Time = t;
  double angleStep=M_PI/(size-1);
  double range[size];
  for (int i=0; i<size;i++)
    {
      rs>>range[i];
    }
  s1.import(s1.Time,size,range,angleStep);
  cerr << (int)s1.getClassType() << " "  // need to cast to in since its a char
       << s1.getSubType() << std::endl;


  pa1.write(s1);      
  cerr << "Written s1\n";

  while(rs>>t>>size)
    {
      s1.Time = t;
      for (int i=0; i<size;i++)
	{
	  rs>>range[i];
	}
      s1=range;
      pa1.write(s1);      
    }

  return 0;
} 

