//
// = LIBRARY
//
// = FILENAME
//    SonarData.cc
//
// = AUTHOR(S)
//    john Folkesson
//
// = COPYRIGHT
//    Copyright (c) 2005 John Folkesson
//
/*----------------------------------------------------------------------*/

#include "SonarData.hh"

#ifndef DEPEND
#include <string.h>  // memcpy
#include <time.h>    // time
#include <stdio.h>   // fprintf
#endif

namespace Cure {

SonarData::SonarData(unsigned short id)
  :SensorData(SensorData::SENSORTYPE_SONAR, id)
{
  init();
}

SonarData::SonarData(Timestamp &t, unsigned short id)
  :SensorData(SensorData::SENSORTYPE_SONAR, id)
{
  init();
  Time=t;
}

/**
 * Create a copy of a SonarData
 */
SonarData::SonarData(const SonarData &src)
  :SensorData(SensorData::SENSORTYPE_SONAR, src.SensorID)
{
  init();
  (*this) = src;
}

SonarData::~SonarData()
{}

void SonarData::init()
{
  m_Packed =false;
  m_ClassType=(SONAR_TYPE);
  setSubType(0);
  setID(0);
  MaxRange = 8;
  BeamWidth = .36;
  RangeResolution = 0.01;
}

SonarData& SonarData::operator=(const SonarData &src)
{
  Time=src.Time;
  setSubType(src.getSubType());
  setID(src.getID());
  SensorID=src.SensorID;
  SensorType=src.SensorType;
  MaxRange = src.MaxRange;
  BeamWidth = src.BeamWidth;
  RangeResolution = src.RangeResolution;
  Range=src.Range;
  return (*this);
}


void SonarData::setSubType(unsigned short t)
{
  m_SubType = t;
}


} // namespace Cure

std::ostream& 
operator << (std::ostream& os, const Cure::SonarData &scan)
{
  os << "BeamWidth=" << scan.getBeamWidth()<< std::endl
     << "MaxRange=" << scan.getMaxRange() << std::endl
     << "RangeResolution=" << scan.getRangeResolution() << std::endl
     << "ID=" << scan.SensorID << std::endl
     << "Range=" << scan.getRange() << std::endl;
  return os;
}
