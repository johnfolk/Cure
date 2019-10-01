// = LIBRARY
//
// = FILENAME
//    PackedSensorPose.cc
//
// = AUTHOR(S)
//    Patric Jensfelt
//
// = COPYRIGHT
//    Copyright (c) 2005 Patric Jensfelt
//
/*----------------------------------------------------------------------*/

#include "PackedSensorPose.hh"
#include "SensorPose.hh"

#ifndef DEPEND
#endif

using namespace std;
using namespace Cure;

int PackedSensorPose::VERSION = 1;

PackedSensorPose::PackedSensorPose()
  :PackedPose()
{
  init();
}
  
PackedSensorPose::PackedSensorPose(PackedSensorPose& p)
{ 
  init();
  *this=(p);
}

PackedSensorPose::PackedSensorPose(SensorPose& p)
{ 
  init();
  *this=(p);
}

void 
PackedSensorPose::init()
{
  Data = 0;
  DataSize = 0;

  ShortDataSize = 2;
  ShortData     = new unsigned short[ShortDataSize * sizeof(short)];

  m_Packed      = true;
  m_ClassType   = SENSORPOSE_TYPE;

  setID(0);
  setSubType(0);
  Version = VERSION;
}

PackedSensorPose::~PackedSensorPose()
{
  if (Data)delete []Data;
  Data = 0;
  DataSize = 0;

  if (ShortData)delete []ShortData;
  ShortData = 0;
  ShortDataSize = 0;
}

void 
PackedSensorPose::operator=(TimestampedData& p){
  SensorPose *sp = p.narrowSensorPose();
  if (sp) {
    operator=(*sp);
  } else {
    PackedData *pp = p.narrowPackedSensorPose();
    if (pp) PackedData::operator=(*pp);
  }
}

void 
PackedSensorPose::unpack(TimestampedData& p){
  SensorPose *sp = p.narrowSensorPose();
  if (sp) unpack(*sp);
}

void 
PackedSensorPose::setSubType(unsigned short t)
{
  PackedPose::setSubType(t);
}

void PackedSensorPose::operator = (const SensorPose& p)
{
  ShortData[0] = p.m_SensorType;
  ShortData[1] = p.m_SensorID;
  PackedPose::operator=((Pose3D&)p);
}

void PackedSensorPose::unpack(SensorPose& p)
{  
  p.m_SensorType = ShortData[0];
  p.m_SensorID = ShortData[1];
  PackedPose::unpack((Pose3D&)p);
}

void PackedSensorPose::print()
{
  TimestampedData::print();
  std::cerr << "m_SensorType=" << ShortData[0]
            << " m_SensorID=" << ShortData[1] << " ";
  PackedPose::print();
}
