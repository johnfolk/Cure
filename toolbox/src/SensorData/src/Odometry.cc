//
// = LIBRARY
//
// = FILENAME
//    Odometry.cc
//
// = AUTHOR(S)
//    Patric Jensfelt
//
// = COPYRIGHT
//    Copyright (c) 2004 Patric Jensfelt
//
/*----------------------------------------------------------------------*/

#include "Odometry.hh"

#ifndef DEPEND
#endif

using namespace Cure;

Odometry::Odometry()
 
{
  init();
  m_Encoder[0] = 0;
  m_Encoder[1] = 0;
  m_Encoder[2] = 0;
}

Odometry::Odometry(const Timestamp &t,
                   double x, double y, double theta,
                   double v, double w)
{
  init();
  m_Pose.Position.X[0] = x;
  m_Pose.Position.X[1] = y;
  m_VelTranslation = v;
  m_VelRotation = w;

  m_Encoder[0] = 0;
  m_Encoder[1] = 0;
  m_Encoder[2] = 0;
}

Odometry::Odometry(const Timestamp &t, const Transformation2D &p)
  :m_Pose(p)
{
  init();
  m_VelTranslation = 0;
  m_VelRotation = 0;
  m_Encoder[0] = 0;
  m_Encoder[1] = 0;
  m_Encoder[2] = 0;
}
void Odometry::init()
{
  m_Packed =false;
  m_ClassType=(ODOMETRY_TYPE);
  setSubType(0);
  setID(0);
}
Odometry::~Odometry()
{}
Odometry& Odometry::operator=(const Odometry &od)
{
  Time=od.Time;
  setSubType(od.getSubType());
  m_Pose = od.m_Pose;
  m_VelTranslation = od.m_VelTranslation;
  m_VelRotation =od.m_VelRotation;

  m_Encoder[0] = od.m_Encoder[0];
  m_Encoder[1] = od.m_Encoder[1];
  m_Encoder[2] = od.m_Encoder[2];
  return *this; 
}
