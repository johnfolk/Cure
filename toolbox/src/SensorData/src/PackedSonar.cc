//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2005 John Folkesson
//    

#include "PackedSonar.hh"

#ifndef DEPEND
#include <iostream>
#endif


using namespace std;
namespace Cure {

int PackedSonar::VERSION = 1;

PackedSonar::PackedSonar(){
  init();
}
  
PackedSonar::PackedSonar(PackedSonar& p){ 
  PackedSonar();
  *this=(p);
}

PackedSonar::PackedSonar(SonarData& p){ 
  PackedSonar();
  *this=(p);
}

void 
PackedSonar::init()
{
  Data = 0;
  DataSize = 0;
  ShortData = 0;
  ShortDataSize = 0;
  m_Packed =true;
  m_ClassType=(SONAR_TYPE);
  setID(0);
  setSubType(0);
  Version = VERSION;
}

PackedSonar::~PackedSonar()
{
  if (Data)delete []Data;
  Data = 0;
  DataSize = 0;

  if (ShortData)delete []ShortData;
  ShortData = 0;
  ShortDataSize = 0;
}

void 
PackedSonar::operator=(TimestampedData& p){
  SonarData *s=p.narrowSonarData();
  if (s) {
    operator=(*s);
  } else {
    PackedSonar *pp=p.narrowPackedSonar();
    if (pp) PackedSonar::operator=(*pp);
  }
}

void 
PackedSonar::unpack(TimestampedData& p){
  SonarData *s=p.narrowSonarData();
  if (s) unpack(*s);
}

void 
PackedSonar::setSubType(unsigned short t)
{
  if (getSubType() == 0 || getSubType() != t) {
    m_SubType = t;
    setDataSize(getDataSizeFromSubType(t));
    ShortDataSize = getShortSizeFromSubType(t);
    if (ShortData) delete []ShortData;
    ShortData = new unsigned short[ShortDataSize];
  }
}



void PackedSonar::operator = (const SonarData& p)
{
  Time=p.Time;
  setSubType(p.getSubType());
  setID(p.getID());
  ShortData[0]=p.SensorID;
  Data[0]=p.BeamWidth;
  Data[1]=p.MaxRange;
  Data[2]=p.RangeResolution;
  Data[3]=p.Range;
}
void PackedSonar::unpack(SonarData& p)
{  
  p.setSubType(getSubType());
  p.setID(getID());
  p.Time=Time;
  if (!Data)return;
  if (!ShortData)return;
  p.BeamWidth=Data[0];
  p.MaxRange=Data[1];
  p.RangeResolution=Data[2];
  p.Range=Data[3];
  p.SensorID=ShortData[0];
}



} // namespace Cure
