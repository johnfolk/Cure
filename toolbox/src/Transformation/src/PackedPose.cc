// = RCSID
//    $Id: PackedPose.cc ,v 1.1 2004/03/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2004 John Folkesson
//    

#include "PackedPose.hh"

#ifndef DEPEND
#include <iostream>
#endif

using namespace std;
namespace Cure {

int PackedPose::VERSION = 2;

PackedPose::PackedPose()
{
  init();
}
  
PackedPose::PackedPose(PackedPose& p)
{ 
  init();
  *this=(p);
}

PackedPose::PackedPose(Pose3D& p)
{ 
  init();
  *this=(p);
}

PackedPose::PackedPose(Pose2D& p){ 
  init();
  *this=(p);
}

void 
PackedPose::init()
{
  Data = 0;
  DataSize = 0;

  ShortData = 0;
  ShortDataSize = 0;
  m_Packed =true;
  m_ClassType=(POSE3D_TYPE);
  setID(0);
  Version = VERSION;

  setSubType(0x7000); 
}

PackedPose::~PackedPose()
{
  if (Data)delete []Data;
  Data=0;
  ShortData=0;
}

void 
PackedPose::operator=(TimestampedData& p)
{
  Pose3D *p3d=p.narrowPose3D();
  if (p3d) {
    operator=(*p3d);
  } else {
    PackedData *pp=p.narrowPackedPose();
    if (pp) PackedData::operator=(*(pp->narrowPackedData()));
  }
}

void 
PackedPose::setSubType(unsigned short t) {
  // do not have to do anything unless the SubType has changed
  if (getSubType() != t) {
    setDataSize(getDim(t));
    m_SubType = t;
  }
}

unsigned short  
PackedPose::getDim(unsigned short type)const
{
  unsigned short dim=6;
  unsigned long t=(type>>12);
  t=(t&7);
  if (t&4) dim-=3;
  else if (t&1)dim--;
  if (t&2) dim-=3;
  else if (t&1)dim-=2;
  t=type;
  t=(t&63);
  unsigned short h=1;
  for (int i=0; i<6; i++,h=h<<1)
    if (t&h)dim++;
  unsigned short rows=0;
  t=(type>>6);
  t=(t&63);
  h=1;
  for (int i=0; i<6; i++,h=h<<1)
    if (t&h)rows++;
  dim+=(rows*rows);
  return dim;
}


void PackedPose::operator = (const Pose3D& p)
{
  Time=p.Time;

  setSubType(p.getSubType());
  setID(p.getID());
  int k=0;

  unsigned short t=(m_SubType>>12);
  t=(t&7);
  double EulerAngles[3];
  p.getAngles(EulerAngles);
  if (!(t&4))
  { 
    Data[k]=p.Position(0);
    Data[k+1]=p.Position(1);
    k+=2;
    if (!(t&1))
    {
      Data[k]=p.Position(2);
      k++;
    }
  } 
  if (!(t&2)) 
  {
    Data[k]=EulerAngles[0];
    k++;
    if (!(t&1))
    {
      Data[k]=EulerAngles[1];
      Data[k+1]=EulerAngles[2];
      k+=2;
    }
  }
  t=m_SubType;
  t=(t&63);  // 0x3F=63
  unsigned int h=1;
  for (int i=0; i<6;i++,h=h<<1)
  {
    if (t&h)
    {
      Data[k]=p.Velocity[i];
      k++;
    }
  }
  for (int i=0; i<p.Covariance.Rows; i++)
  {
    for (int j=0; j<p.Covariance.Columns; j++)
    {
      Data[k]=p.Covariance(i,j);
      k++;
    }
  }
}
void PackedPose::operator = (const Pose2D& p)
{
  unsigned short ptype=4096;
  unsigned short pt=(p.getSubType()>>3);
  pt=(pt&7);
  pt=(pt<<6);
  ptype+=pt;
  pt=p.getSubType();
  pt=(pt&7);
  ptype+=pt;
  setSubType(ptype);
  Time=p.Time;
  Data[0]=p.Position(0);
  Data[1]=p.Position(1);
  Data[2]=p.getTheta();
  int k=3;
  unsigned short t=(m_SubType&0x00FF);
  unsigned short h=1;
  for (int i=0; i<3;i++,h=h<<1)
  {
    if (t&h)
    {
      Data[k]=p.Velocity[i];
      k++;
    }
  }
  for (int i=0; i<p.Covariance.Rows; i++)
  {
    for (int j=0; j<p.Covariance.Columns; j++)
    {
      Data[k]=p.Covariance(i,j);
      k++;
    }
  }
}
void PackedPose::unpack(Pose3D& p)
{
  p.setSubType(getSubType());
  p.setID(getID());
  p.Time=Time;
  double x[6];
  int k=0;
  unsigned short t=(m_SubType>>12);
  t=(t&7);
  if (!(t&4))
  { 
    x[0]=Data[k];
    x[1]=Data[k+1];
    k+=2;
    if (!(t&1))
    {
      x[2]=Data[k];
      k++;
    }
    else x[2]=0;
  } 
  else 
  {
    x[0]=0;
    x[1]=0;
    x[2]=0;
  }
  if (!(t&2)) 
  {
    x[3]=Data[k];
    k++;
    if (!(t&1))
      {
	x[4]=Data[k];
	x[5]=Data[k+1];
	k+=2;
      }
    else
      {
	x[4]=0;
	x[5]=0;
      }
  }
  else 
  {
    x[3]=0;
    x[4]=0;
    x[5]=0;
  }
  p=x;
  t=m_SubType;
  t=(t&63);  // 0x3F=63
  unsigned short h=1;
  for (int i=0; i<6;i++,h=h<<1)
  {
    if (t&h)
    {
      p.Velocity[i]=Data[k];
      k++;
    }
    else p.Velocity[i]=0;
  }
  for (int i=0; i<p.Covariance.Rows; i++)
  {
    for (int j=0; j<p.Covariance.Columns; j++)
    {
      p.Covariance(i,j)=Data[k];
      k++;
    }
  }
}

void PackedPose::print()
{
  TimestampedData::print();
  std::cerr << " Pose vtype: " << m_SubType%64 
       << " Ptype: " << (0x3F&(m_SubType/64))
       << " xtype: " << (0x0F&m_SubType/4096)<<std::endl;
   
}

} // namespace Cure
