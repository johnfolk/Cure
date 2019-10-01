// = RCSID
//    $Id: PackedSICK.cc ,v 1.1 2004/03/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2004 John Folkesson
//    

#include "PackedSICK.hh"

#ifndef DEPEND
#include <iostream>
#endif

const int SICK_SHORT_OVERHEAD = 4;

using namespace std;
namespace Cure {

int PackedSICK::VERSION = 4;

PackedSICK::PackedSICK(){
  init();
}
  
PackedSICK::PackedSICK(PackedSICK& p){ 
  PackedSICK();
  *this=(p);
}

PackedSICK::PackedSICK(SICKScan& p){ 
  PackedSICK();
  *this=(p);
}

void 
PackedSICK::init()
{
  Data = 0;
  DataSize = 0;

  ShortData = 0;
  ShortDataSize = 0;
  m_Packed =true;
  m_ClassType=(SICKSCAN_TYPE);
  setID(0);
  setSubType(0);
  Version = VERSION;
}

PackedSICK::~PackedSICK()
{
  if (Data)delete []Data;
  Data = 0;
  DataSize = 0;

  if (ShortData)delete []ShortData;
  ShortData = 0;
  ShortDataSize = 0;
}

void 
PackedSICK::operator=(TimestampedData& p){
  SICKScan *scan=p.narrowSICKScan();
  if (scan) {
    operator=(*scan);
  } else {
    PackedData *pp=p.narrowPackedSICK();
    if (pp) PackedData::operator=(*pp);
  }
}

void 
PackedSICK::unpack(TimestampedData& p){
  SICKScan *scan=p.narrowSICKScan();
  if (scan) unpack(*scan);
}

void 
PackedSICK::setSubType(unsigned short t)
{
  if (getSubType() == 0 || getSubType() != t) {
    m_SubType = t;
    setDataSize(getDataSizeFromSubType(t));
    ShortDataSize = getShortSizeFromSubType(t);
    if (ShortData) delete []ShortData;
    if (ShortDataSize>0){
      ShortData = new unsigned short[ShortDataSize];
      memset(ShortData, 0, ShortDataSize*sizeof(unsigned short));
    }else ShortData=0;
  }
}


unsigned long 
PackedSICK::getShortSizeFromSubType(unsigned short type) const
{
  unsigned long t=((type>>10)&0x3);
  unsigned long m=SICK_SHORT_OVERHEAD;
  if (t&3)m+=(getDataSize()-4);
  return m;
}

unsigned long 
PackedSICK::getDataSizeFromSubType(unsigned short type)const
{
  unsigned long t=(type&0x03FF);
  if (t>721)t=721;
  return (t+4);
}

void PackedSICK::operator = (const SICKScan& p)
{
  Time=p.Time;
  setSubType(p.getSubType());
  setID(p.getID());

  int n=p.m_NPts;
  ShortData[0]=(unsigned short)((p.m_NFlags)&0xFFFF);
  ShortData[1]=(unsigned short)((p.m_NIntensityLevels)&0xFFFF);
  ShortData[2]=p.m_ScannerType;
  ShortData[3]=p.SensorID;
  Data[0]=p.m_AngleStep;
  Data[1]=p.m_StartAngle;
  Data[2]=p.m_MaxRange;
  Data[3]=p.m_RangeResolution;
  if (n==0)return;
  memcpy(Data+4,p.m_Ranges, n*sizeof(double));
  if (!(m_SubType&0xC00))return;
  if ((m_SubType&0x400)&&(m_SubType&0x800)) { 
    for (int i=0; i<n; i++) {
      ShortData[i+SICK_SHORT_OVERHEAD] = (unsigned short)p.m_Flags[i];
      ShortData[i+SICK_SHORT_OVERHEAD] = (ShortData[i+2]<<8);
      ShortData[i+SICK_SHORT_OVERHEAD] += (unsigned short)p.m_Intensities[i];
    }
  } else if (m_SubType&0x400) {
    for (int i=0; i<n; i++) {
      ShortData[i+SICK_SHORT_OVERHEAD] = (unsigned short)p.m_Flags[i];
      ShortData[i+SICK_SHORT_OVERHEAD] = (ShortData[i+2]<<8);
    }
  } else {
    for (int i=0; i<n; i++) {
      ShortData[i+SICK_SHORT_OVERHEAD] += (unsigned short)p.m_Intensities[i];
    }
  }
}
void PackedSICK::unpack(SICKScan& p)
{  
  p.setSubType(getSubType());
  p.setID(getID());
  p.Time=Time;
  if (!Data)return;
  if (!ShortData)return;
  p.m_AngleStep=Data[0];
  p.m_StartAngle=Data[1];
  p.m_MaxRange=Data[2];
  p.m_RangeResolution=Data[3];
  p.m_NFlags=(int)ShortData[0];
  p.m_NIntensityLevels=(int)ShortData[1];
  p.m_ScannerType = ShortData[2];
  p.SensorID = ShortData[3];
  int n=p.m_NPts;
  if (n) {
    memcpy(p.m_Ranges, Data+4, n*sizeof(double));
    if (!(m_SubType&0xC00))return;
    if ((m_SubType&0x400)&&(m_SubType&0x800)) { 
      for (int i=0; i<n; i++) {
        p.m_Flags[i]=(unsigned char)(ShortData[i+SICK_SHORT_OVERHEAD]>>8);
        p.m_Intensities[i]=
          (unsigned char)(ShortData[i+SICK_SHORT_OVERHEAD]&0xFF);
      }
    } else if (m_SubType&0x400) {
      for (int i=0; i<n; i++) {
        p.m_Flags[i]=(unsigned char)(ShortData[i+SICK_SHORT_OVERHEAD]>>8);
      }
   } else  {
     for (int i=0; i<n; i++) {
       p.m_Intensities[i]=
         (unsigned char)(ShortData[i+SICK_SHORT_OVERHEAD]&0xFF);
     }
   }
  }
}



} // namespace Cure
