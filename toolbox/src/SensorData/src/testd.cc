//
// = LIBRARY
//
// = FILENAME
//    testp.cc
//
// = AUTHOR(S)
//    Patric Jensfelt
//
// = COPYRIGHT
//    Copyright (c) 2004 Patric Jensfelt
//
/*----------------------------------------------------------------------*/

#include "SmartData.hh"
#include "TimestampedString.hh"
#include "SICKScan.hh"
#include "GridData.hh"
#include "BinaryMatrix.hh"
#include "CureDebug.hh"

int main() {
  std::cerr<<"HI\n";
  Cure::SmartData sd;
  Cure::TimestampedString strg, str2,str3;
  strg="Hello";
  strg.setStringType(4);
  strg.setID(5);
  strg.Time.setToCurrentTime();
  strg.print();

  sd=strg;
  sd.print();
  sd.pack();
  sd.print();
  sd.setEqual(str2);

  str2.print();
  getchar();
  
  char *chr;
  int leng=Cure::SmartData::writeChar(str2, &chr,1);
  chr[0]='t';
  std::cerr<<chr<<"\n";
  sd.readChar(chr+1,leng-1);
  sd.print();
  sd.setEqual(str3);
  sd.unpack();
  sd.print();
  str3.print();
  getchar();
  
  Cure::GenericData tst;
  tst.GenericType=4;
  tst.setID(8);
  tst.setShortDataSize(2,3);
  tst.ShortData(1,1)=7;
  tst.print();


  
  char *str;
  Cure::PackedGeneric pd;
  pd=tst;
  int len=Cure::SmartData::writeChar(pd, &str,1);
  str[0]='t';
  sd.readChar(str+1,len-1);
  sd.setEqual(pd);
  pd.setEqual(tst);
  tst.print();
  return 0;
     
  Cure::TimestampedData t;
  t.print(); 
  std::vector<std::string> vec;
  

  Cure::writeString(t,vec, 16);  
  t.print();
  return 0;

  Cure::SICKScan scan;
  scan.createRandomMeasurement(1, 3);

  //  std::cout << scan << std::endl;
  Cure::GridData grid;
  Cure::Transformation3D trans;
  Cure::Matrix  mean1(2,1);
  Cure::Matrix  mean2(2,1);
  mean1(0,0)=10;
  mean1(1,0)=5;
  mean2(0,0)=0;
  mean2(1,0)=1;
  Cure::getTransform(trans,mean1,mean2);
  mean1.print();
  mean2.print();
  trans.transform2D(mean1.Element,mean1.Element);
  mean1.print();
  return 0;

}

