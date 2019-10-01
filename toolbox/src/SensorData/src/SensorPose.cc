//
// = LIBRARY
//
// = FILENAME
//    SensorPose.cc
//
// = AUTHOR(S)
//    Patric Jensfelt
//
// = COPYRIGHT
//    Copyright (c) 2005 Patric Jensfelt
//
/*----------------------------------------------------------------------*/

#include "SensorPose.hh"
#include "SensorData.hh"

#ifndef DEPEND
#endif

namespace Cure {

SensorPose::SensorPose()
  :m_SensorType(SensorData::SENSORTYPE_UNKNOWN),
   m_SensorID(0)
{
  initSP();
}

SensorPose::SensorPose(unsigned short type, unsigned short id)
  :m_SensorType(type),
   m_SensorID(id)
{
  initSP();
}

SensorPose::SensorPose(unsigned short type, const Pose3D &p, unsigned short id)
  :Pose3D(p),
   m_SensorType(type),
   m_SensorID(id)
{
  initSP();
}

/**
 * Create a copy of a SensorPose
 */
SensorPose::SensorPose(const SensorPose &src)
{
  initSP();
  (*this) = src;
}

SensorPose::~SensorPose()
{}

void
SensorPose::initSP()
{
  m_ClassType = SENSORPOSE_TYPE;
}

void
SensorPose::print()
{
  std::cerr << "SensorPose: type=" << m_SensorType << " id="
            << m_SensorID << "  ";
  Pose3D::print();
}

} // namespace Cure

std::ostream& 
operator << (std::ostream& os, const Cure::SensorPose &sp)
{
  os << "SensorPose: type=" << sp.m_SensorType << " id="
     << sp.m_SensorID << "  ";
  os << (Cure::Pose3D&)sp;

  return os;
}
