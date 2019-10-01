// = RCSID
//    $Id: Pose2D.cc ,v 1.1 2004/03/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2004 John Folkesson
//    


#include "Pose2D.hh"

#ifndef DEPEND
#include <iostream>
#endif

namespace Cure {

//Private Helper Method 
//jb=grad aR^T *aR *x
void getJ(double jb[2],double x[3],Rotation2D aR)
{
  double ra[4];
  aR.getRot2D(ra);
  jb[0]=-x[1];
  jb[3]=x[0];
}

Pose2D::Pose2D():Transformation2D()
{
  m_SubType=0;
  Velocity[0]=0;
  Velocity[1]=0;
  Velocity[2]=0;
}
void Pose2D::operator = (Pose2D& p)
{
  Transformation2D::operator =(p); 
  m_SubType=p.m_SubType;
  Time=p.Time;
  Covariance=p.Covariance;
  Velocity[0]=p.Velocity[0];
  Velocity[1]=p.Velocity[1];
  Velocity[2]=p.Velocity[2];
}


void Pose2D::setVelocity(const double v[3])
{
  for (int i=0; i<3;i++)
    Velocity[i]=v[i];
}
void Pose2D::getVelocity(double v[3])const
{
  for (int i=0; i<3;i++)
    v[i]=Velocity[i];
}

void Pose2D::add_(Pose2D& a,Pose2D& b)
{
  if(!m_SubType)
  {
    Transformation2D::operator =(a+b);
    return; 
  }
  for (int i=0; i<3; i++)
    Velocity[i]=0;
  short vrows=m_SubType&7;
  short prows=m_SubType>>3;
  prows=(prows&7);
  short bvcols=b.m_SubType&7;
  short bpcols=b.m_SubType>>3;
  bpcols=(bpcols&7);
  short avcols=a.m_SubType&7;
  short apcols=a.m_SubType>>3;
  apcols=(apcols&7);
  double  ja[2];
  Matrix jac;
  a.invRotate2D(b.Position,Position);
  b.calcRot2D();
  Transformation2D::doAplusB(a,b, jac);
  getJ(ja,Position.X,a);
  Position+=a.Position;
  int h=1;
  for (int j=0;j<2;j++,h=h<<1)
  {
    if (vrows&h)
    {
      if (avcols&h)Velocity[j]+=a.Velocity[j];
      if (avcols&4)Velocity[j]+=a.Velocity[2]*ja[j];
      if (bvcols&1) Velocity[j]+=a.Rot2D[j]*b.Velocity[0];
      if (bvcols&2) Velocity[j]+=a.Rot2D[2+j]*b.Velocity[1];
    }
  }
  if (vrows&h)
  {
    if (avcols&4)Velocity[2]+=jac(0,0)*a.Velocity[2];
    if (bvcols&4) Velocity[2]+=jac(0,1)*b.Velocity[2];
  }
  Matrix j_b(Covariance.Rows,b.Covariance.Rows);
  Matrix j_a(Covariance.Rows,a.Covariance.Rows);
  Matrix tmp(b.Covariance.Rows,Covariance.Rows);
  j_b=0;
  j_a=0;
  short bindex[3],aindex[3];
  int k=1;
  int j=1;
  h=1;
  for (int i=0; i<3; i++,k=k<<1)
  {
    if (bpcols&k)
    {
      bindex[i]=j;
      j++;
    }
    else 
      bindex[i]=0;
    if (apcols&k)
    {
      aindex[i]=h;
      h++;
    }
    else 
      aindex[i]=0;
  }

  k=1;
  h=0;
  for (int i=0; i<2; i++,k=k<<1)
  {
    if (prows&k)
    {
      if (aindex[i])j_a(h,aindex[i]-1)=1;
      if (aindex[2])j_a(h,aindex[2]-1)=ja[i];      
      if (bindex[0])j_b(h,bindex[0]-1)=b.Rot2D[i];            
      if (bindex[1])j_b(h,bindex[1]-1)=b.Rot2D[2+i];            
    
      h++;
    }
  }
  if (prows&k)
  {
    if (aindex[2])j_a(h,aindex[2]-1)=jac(0,0);
    if (bindex[2])j_b(h,bindex[2]-1)=jac(0,1);
  }
    
  tmp.multTranspose_(b.Covariance,j_b,2);
  Covariance.multiply_(j_b,tmp);
  tmp.multTranspose_(a.Covariance,j_a,2);
  Covariance.addProduct_(j_a,tmp);
 
}
/*
  this=c-b
    jacobian = {dEulerAngles/dc.EulerAngles,dEulerAngles/db.EulerAngles}
*/

void Pose2D::subtract_(Pose2D& c,Pose2D& b)
{
  if(!m_SubType)
  {
    Transformation2D::operator =(c-b);
    return; 
  }
  for (int i=0; i<3; i++)
    Velocity[i]=0;
  
  short vrows=m_SubType&7;
  short prows=m_SubType>>3;
  prows=(prows&7);
  short bvcols=b.m_SubType&7;
  short bpcols=b.m_SubType>>3;
  bpcols=(bpcols&7);
  short cvcols=c.m_SubType&7;
  short cpcols=c.m_SubType>>3;
  double  jc[2], jb[2];
  Matrix jac;
  
  Transformation2D::doAminusB(c,b, jac);
  getJ(jc,b.Position.X,b);
  jb[0]=Rot2D[0]*jc[0]+Rot2D[2]*jc[1];
  jb[1]=Rot2D[1]*jc[0]+Rot2D[3]*jc[1];
  invRotate2D(b.Position,Position);
  b.calcRot2D();
  c.calcRot2D();
  Position*=-1;
  getJ(jc,Position.X,c);
  Position+=c.Position;
  int h=1;
  for (int j=0;j<2;j++,h=h<<1)
  {
    if (vrows&h)
    {
      if (cvcols&h)Velocity[j]+=c.Velocity[j];
      if (cvcols&4)Velocity[j]+=c.Velocity[2]*jc[j];
      if (bvcols&1) Velocity[j]-=Rot2D[j]*b.Velocity[0];
      if (bvcols&2) Velocity[j]-=Rot2D[2+j]*b.Velocity[1];
      if (bvcols&4)Velocity[j]+=b.Velocity[2]*jb[j];
    }
    if (vrows&h)
    {
      if (cvcols&4)Velocity[2]+=jac(0,0)*c.Velocity[2];
      if (bvcols&4) Velocity[2]+=jac(0,1)*b.Velocity[2];
    }
  }
  Matrix j_b(Covariance.Rows,b.Covariance.Rows);
  Matrix j_c(Covariance.Rows,c.Covariance.Rows);
  Matrix tmp(b.Covariance.Rows,Covariance.Rows);
  j_b=0;
  j_c=0;
  short bindex[3],cindex[3];
  int k=1;
  int j=1;
  h=1;
  for (int i=0; i<3; i++,k=k<<1)
  {
    if (bpcols&k)
    {
      bindex[i]=j;
      j++;
    }
    else 
      bindex[i]=0;
    if (cpcols&k)
    {
      cindex[i]=h;
      h++;
    }
    else 
      cindex[i]=0;
  }
  k=1;
  h=0;
  for (int i=0; i<2; i++,k=k<<1)
  {
    if (prows&k)
    {
      if (cindex[i])j_c(h,cindex[i]-1)=1;
      if (cindex[2])j_c(h,cindex[2]-1)=jc[i];
      if (bindex[0])j_b(h,bindex[0]-1)=-Rot2D[i];            
      if (bindex[1])j_b(h,bindex[1]-1)=-Rot2D[2+i];            
      if (bindex[2])j_b(h,bindex[2]-1)=jb[i];
      h++;
    }
  }
  if (prows&4)
  {
    if (cindex[2])j_c(h,cindex[2]-1)=jac(0,0);      
    if (bindex[2])j_b(h,bindex[2]-1)=jac(0,1);  
    h++;
  }
  tmp.multTranspose_(b.Covariance,j_b,2);
  Covariance.multiply_(j_b,tmp);
  tmp.multTranspose_(c.Covariance,j_c,2);
  Covariance.addProduct_(j_c,tmp);
}
void Pose2D::minus_(Pose2D& b)
{
  Pose2D c;
  subtract_(c,b);
}
//These are dumb implementations:
//this = b = -a + c
void Pose2D::minusPlus_(Pose2D& a,Pose2D& c)
{
  Pose2D p;
  p.m_SubType=63;
  p.Covariance.reallocate(3);
  p.minus_(a);
  add_(p,c);
}

//this = c = -a - b
void Pose2D::minusMinus_(Pose2D& a,Pose2D& b)
{
  Pose2D p;
  p.m_SubType=63;
  p.Covariance.reallocate(3);
  p.add_(b,a);
  minus_(p);
}

void Pose2D::interpolate_(Pose2D& a, Pose2D& b, Timestamp t)
{
  a.limitAngles();
  b.limitAngles();
  *this=a;
  Time=t;
  double d=0;  
  if(a.Time>b.Time)
  {
    t-=b.Time;
    d=t.getDouble();
    t=a.Time;
    t-=b.Time;
    d/=t.getDouble();
  }
  else
  {
    t-=a.Time;
    d=t.getDouble();
    t=b.Time;
    t-=a.Time;
    d/=t.getDouble();
    d=1-d;
  }
  double w=1-d;

  Covariance*=d;
  Covariance.addProduct_(b.Covariance,w);
  State=0;
  for(int i=0; i<3;i++)
  {
    Velocity[i]*=d;
    Velocity[i]+=(w*b.Velocity[i]);
  }
  Position*=d;
  Vector3D trl(b.Position);
  trl*=w;
  Position+=trl;
  double dx=b.EulerAngles[0]-EulerAngles[0];
  if (dx>M_PI)dx=dx-TWO_PI;
  if (dx<-M_PI)dx=TWO_PI+dx;
  EulerAngles[0]+=(w*dx);
}
void Pose2D::print()
{
  using namespace std;

  cerr<<endl<<" timestamp: "<<Time.Seconds<<"."<<Time.Microsec;
  cerr<<" vtype: "<<int(m_SubType&7)<<" Ptype: "<<m_SubType/8;  
  Transformation2D::print();
  cerr<<" v:"; 
  for (int i=0; i<3;i++)
    cerr<<" "<<Velocity[i];
  cerr<<endl;
  cerr<<" P: "; 
  Covariance.print();
}

} // namespace Cure
