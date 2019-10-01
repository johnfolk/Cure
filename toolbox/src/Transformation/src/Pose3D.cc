// = RCSID
//    $Id: Pose3D.cc ,v 1.1 2004/03/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2004 John Folkesson
//    

#include "Pose3D.hh"
#include <string>
#include <ctype.h>  // isdigit
#ifndef DEPEND
#include <iostream>
#include <sstream>  // istringstream
#endif

namespace Cure {

int Pose3D::setupPose(std::string &params)
{
  std::istringstream strdes(params);
  std::string cmd, nextcmd;
  if (!(strdes>>cmd))return 1;
  while (1){
    double x[6];
    for (int j=0;j<6;j++)x[j]=0;
    int i=0;
    bool loop=true;
    while (loop){
      std::string tmp; 
      if (strdes >> tmp){
	if (isdigit(tmp[0]) || 
	    tmp[0] == '-' || tmp[0] == '+' ||
	    tmp[0] == '.') {
	  x[i] = atof(tmp.c_str());
	  i++;
	  if (i==6)i=5;
	}else {
	  nextcmd=tmp;
	  loop=false;
	}
      } else {
	loop=false;
	nextcmd=" ";
      }
    }
    if (cmd=="Value"){
      (*this)=x;
    }else if (cmd=="x"){
      (*this).setX(x[0]);
    }else if ("cmd==y"){
      (*this).setY(x[0]);
    }else if (cmd=="z"){
      (*this).setZ(x[0]);
    }else if (cmd=="theta"){
      (*this).setTheta(x[0]);
    }else if (cmd=="phi"){
      (*this).setPhi(x[0]);
    }else if (cmd=="psi"){
      (*this).setPsi(x[0]);
    }else if (cmd=="CovType"){
      (*this).setCovType((unsigned short)x[0]);;
    }else if (cmd=="Cov"){
      for (int j=1; ((j< (*this).Covariance.Rows)&(j<i));
	   j++)
	(*this).Covariance(j,j)=x[j];
      for(int j=i; j<(*this).Covariance.Rows;j++)
	(*this).Covariance(j,j)=x[i-1];
      }
    cmd=nextcmd;
    if (cmd==" ")
      return 0;
  }
  return 0;
}
Pose3D::Pose3D(const Pose3D& p)
  :Transformation3D()
{
  init();
  *this=(p);
}

unsigned short Pose3D::calcType(int x, int y,int z,int theta,int phi,int psi )
{
  unsigned short type=0;
  if (x)type=type|1;
  if (y)type=type|2;
  if (z)type=type|4;
  if (theta)type=type|8;
  if (phi)type=type|16;
  if (psi)type=type|32;
  return type;
}

void Pose3D::init()
{
  m_Packed =false;
  m_ClassType=(POSE3D_TYPE);
  setID(0);
  setSubType(0x0000); 
  Velocity[0]=0;
  Velocity[1]=0;
  Velocity[2]=0;
  Velocity[3]=0;
  Velocity[4]=0;
  Velocity[5]=0;
}

unsigned short Pose3D::operator == (const Pose3D& p)const
{
  if (!(p.Position==Position))return 0;
  if (!(p.Time==Time))return 0;
  if (getSubType()!=p.getSubType())return 0;
  if (EulerAngles[0]!=p.EulerAngles[0])return 0;
  if (EulerAngles[1]!=p.EulerAngles[1])return 0;
  if (EulerAngles[2]!=p.EulerAngles[2])return 0;
  return 1;
}
void Pose3D::zero ()
{
  Position.X[0]=0;
  Position.X[1]=0;
  Position.X[2]=0;
  setAngles(Position.X);
  Covariance=0;
  for (int i=0; i<6; i++)Velocity[i]=0;
  Time=0.0;
}
void Pose3D::operator = (const Pose3D& p)
{
  Transformation3D::operator =(p); 
  setSubType(p.getSubType());
  setID(p.ID);
  Time=p.Time;
  Covariance=p.Covariance;
  Velocity[0]=p.Velocity[0];
  Velocity[1]=p.Velocity[1];
  Velocity[2]=p.Velocity[2];
  Velocity[3]=p.Velocity[3];
  Velocity[4]=p.Velocity[4];
  Velocity[5]=p.Velocity[5];
}

void Pose3D::setSubType(unsigned short t)
{
  t=t&0xFFFF;
  setVelType(t);
  t=t>>6;
  setCovType(t);
  t=t>>6;
  setCoordinateType(t);
}

void Pose3D::setVelType(unsigned short t)
{
  m_SubType = m_SubType & 0xFFC0;//65472;
  unsigned long temp = t & 0x3F;
  m_SubType = (m_SubType | temp);
}

void Pose3D::setCoordinateType(unsigned short t)
{
  m_SubType = (m_SubType & 0x0FFF);

  t = t & 0x000F;
  unsigned long temp=t;
  temp = (temp<<12);
  m_SubType = (m_SubType|temp);
}

void Pose3D::setCovType(unsigned short t)
{
  t=t&0x003F;
  unsigned long temp=t;
  temp=(temp<<6);
  int dim=calcRows(t);
  Covariance.reallocate(dim);  
  m_SubType = m_SubType & 0xF03F;
  m_SubType = (m_SubType | temp);
}

void Pose3D::setVelocity(const double v[6])
{
  for (int i=0; i<6;i++)
    Velocity[i]=v[i];
}
void Pose3D::getVelocity(double v[6])const
{
  for (int i=0; i<6;i++)
    v[i]=Velocity[i];
}

void Pose3D::add_(Pose3D& a,Pose3D& b, Matrix *jacobian)
{
  if(!(m_SubType&4095))
  {
    Transformation3D::operator =(a+b);
    return; 
  }  
  unsigned short ptype;
  unsigned short bptype;
  unsigned short aptype;
  Matrix jac;
  unsigned short rows=getTypeUnion(m_SubType,ptype);
  unsigned short cols=getTypeUnion(b.m_SubType,bptype);
  cols=(getTypeUnion(a.m_SubType,aptype)+(cols<<6));

  Transformation3D::doAplusB(a,b, jac,rows, cols);
  for (int i=0; i<6; i++)Velocity[i]=0;
  unsigned long h=1;
  for (int j=0;j<3;j++,h=h<<1)
  {
    if (m_SubType&h)
    {
      if (a.m_SubType&h)Velocity[j]+=a.Velocity[j];
      unsigned long k=8;
      for(int i=3; i<6;i++,k=k<<1)
	if (a.m_SubType&k)Velocity[j]+=jac(j,i)*a.Velocity[i];
      k=1;
      for(int i=6; i<9;i++,k=k<<1)
	if (b.m_SubType&k) Velocity[j]+=jac(j,i)*b.Velocity[i-6];
    }
  }

  for (int j=3; j<6; j++,h=h<<1)
  {
    if (m_SubType&h)
    {
      unsigned long k=8;
      for (int i=3;i<6; i++,k=k<<1)
      {
	if (a.m_SubType&k)Velocity[j]+=jac(j,i)*a.Velocity[i];
	if (b.m_SubType&k) Velocity[j]+=jac(j,6+i)*b.Velocity[i];
      }
    }
  }
  h=32;
  for (int i=5; i>-1; i--, h=h>>1)
    if (!(ptype&h))jac.deleteRow(i);
  h=32;
  for (int i=5; i>-1; i--,h=h>>1)
    if (!(bptype&h))jac.deleteColumn(i+6);
  h=32;
  for (int i=5; i>-1; i--,h=h>>1)
    if (!(aptype&h))jac.deleteColumn(i);
  Matrix j_a(jac.Element,Covariance.Rows,a.Covariance.Rows,jac.RowInc);
  Matrix j_b(jac.Element+a.Covariance.Rows,Covariance.Rows,
	     b.Covariance.Rows,jac.RowInc);
  Matrix tmp(6);
  tmp.multTranspose_(b.Covariance,j_b,2);
  Covariance.multiply_(j_b,tmp);
  tmp.multTranspose_(a.Covariance,j_a,2);
  Covariance.addProduct_(j_a,tmp); 
  if (!jacobian)return;
  *jacobian=jac;
}
/*
  this=a-b
  jacobian = {dEulerAngles/da.EulerAngles,dEulerAngles/db.EulerAngles}
*/

void Pose3D::subtract_(Pose3D& a,Pose3D& b, Matrix *jacobian)
{
  if(!(m_SubType&4095))
  {
    Transformation3D::operator =(a-b);
    return; 
  }
  unsigned short ptype;
  unsigned short aptype;
  unsigned short bptype;
  Matrix jac;
  unsigned short rows=getTypeUnion(m_SubType,ptype);
  unsigned short cols=getTypeUnion(b.m_SubType,bptype);
  cols=(getTypeUnion(a.m_SubType,aptype)+(cols<<6));

  Transformation3D::doAminusB(a,b, jac,rows, cols);

  for (int i=0; i<6; i++)Velocity[i]=0;
  unsigned long h=1;
  for (int j=0;j<3;j++,h=h<<1)
  {
    if (m_SubType&h)
    {
      if (a.m_SubType&h)Velocity[j]+=a.Velocity[j];
      unsigned long k=8;
      for(int i=3; i<6;i++,k=k<<1)
	if (a.m_SubType&k)Velocity[j]+=jac(j,i)*a.Velocity[i];
      k=1;
      for(int i=6; i<9;i++,k=k<<1)
	if (b.m_SubType&k) Velocity[j]+=jac(j,i)*b.Velocity[i-6];
    }
  }

  for (int j=3; j<6; j++,h=h<<1)
  {
    if (m_SubType&h)
    {
      unsigned long k=8;
      for (int i=3;i<6; i++,k=k<<1)
      {
	if (a.m_SubType&k)Velocity[j]+=jac(j,i)*a.Velocity[i];
	if (b.m_SubType&k) Velocity[j]+=jac(j,6+i)*b.Velocity[i];
      }
    }
  }
  h=32;
  for (int i=5; i>-1; i--, h=h>>1)
    if (!(ptype&h))jac.deleteRow(i);
  h=32;
  for (int i=5; i>-1; i--,h=h>>1)
    if (!(bptype&h))jac.deleteColumn(i+6);
  h=32;
  for (int i=5; i>-1; i--,h=h>>1)
    if (!(aptype&h))jac.deleteColumn(i);
  
  Matrix j_a(jac.Element,Covariance.Rows,a.Covariance.Rows,jac.RowInc);
  Matrix j_b(jac.Element+a.Covariance.Rows,Covariance.Rows,
	     b.Covariance.Rows,jac.RowInc);
  Matrix tmp(6);
  tmp.multTranspose_(b.Covariance,j_b,2);
  Covariance.multiply_(j_b,tmp);
  tmp.multTranspose_(a.Covariance,j_a,2);
  Covariance.addProduct_(j_a,tmp); 
  if (!jacobian)return;
  *jacobian=jac;
}
void Pose3D::separateVelocity(Matrix &jacC,Matrix &jacV,Matrix &jac,
	Pose3D &a, Pose3D &b)
{
  unsigned short pctype=getCovType();
  unsigned short actype=a.getCovType();
  unsigned short bctype=b.getCovType();
  unsigned short pvtype=getVelType();
  unsigned short avtype=a.getVelType();
  unsigned short bvtype=b.getVelType();
   unsigned short h=32;
  jacC=jac;
  jacV=jacC;
  int j=jac.Rows-1;
  for (int i=5; i>-1; i--, h=h>>1)
    {
     if ((pvtype&h)||(pctype&h)){
       if (!(pctype&h)){
	 jacC.deleteRow(j);
       }
       if (!(pvtype&h)){
	 jacV.deleteRow(j);
       }
       j--;
     } 
    }
  j=jac.Columns-1;
  for (int i=5; i>-1; i--, h=h>>1)
   {
     if ((bvtype&h)||(bctype&h)){
       if (!(bctype&h)){
	 jacC.deleteColumn(j);
       }
       if (!(bvtype&h)){
	 jacV.deleteColumn(j);
       }
       j--; 
     }
   }
 for (int i=5; i>-1; i--, h=h>>1)
   {
     if ((avtype&h)||(actype&h)){
       if (!(actype&h)){
	 jacC.deleteColumn(j);
       }
       if (!(avtype&h)){
	 jacV.deleteColumn(j);
       }
       j--; 
     }
   }
}
void Pose3D::minus_(Pose3D& b, Matrix *jacobian)
{
  if(!(m_SubType&4095))
  {
    Transformation3D::operator=(b.inverse());
    return; 
  }
  Pose3D c;
  subtract_(c,b,jacobian);
}
  //this = b = -a + c
void Pose3D::minusPlus_(Pose3D& a,Pose3D& b, Matrix *jacobian)
{

  if(!(m_SubType&4095))
  {
    Transformation3D::operator=((a.inverse())+b);
    return; 
  }
  unsigned short ptype;
  unsigned short aptype;
  unsigned short bptype;
  Matrix jac;
  unsigned short rows=getTypeUnion(m_SubType,ptype);
  unsigned short cols=getTypeUnion(b.m_SubType,bptype);
  cols=(getTypeUnion(a.m_SubType,aptype)+(cols<<6));

  Transformation3D::dominusAplusB(a,b, jac,rows, cols);

  for (int i=0; i<6; i++)
    Velocity[i]=0;
  unsigned long h=1;
  for (int j=0;j<3;j++,h=h<<1)
  {
    if (m_SubType&h)
    {
      unsigned long k=1;
      for(int i=0; i<6;i++,k=k<<1)
	if (a.m_SubType&k)Velocity[j]+=jac(j,i)*a.Velocity[i];
      k=1;
      for(int i=6; i<9;i++,k=k<<1)
	if (b.m_SubType&k) Velocity[j]+=jac(j,i)*b.Velocity[i-6];
    }
  }
  for (int j=3; j<6; j++,h=h<<1)
  {
    if (m_SubType&h)
    {
      unsigned long k=8;
      for (int i=3;i<6; i++,k=k<<1)
      {
	if (a.m_SubType&k)Velocity[j]+=jac(j,i)*a.Velocity[i];
	if (b.m_SubType&k) Velocity[j]+=jac(j,6+i)*b.Velocity[i];
      }
    }
  }
  h=32;
  for (int i=5; i>-1; i--, h=h>>1)
    if (!(ptype&h))jac.deleteRow(i);
  h=32;
  for (int i=5; i>-1; i--,h=h>>1)
    if (!(bptype&h))jac.deleteColumn(i+6);
  h=32;
  for (int i=5; i>-1; i--,h=h>>1)
    if (!(aptype&h))jac.deleteColumn(i);
  
  Matrix j_a(jac.Element,Covariance.Rows,a.Covariance.Rows,jac.RowInc);
  Matrix j_b(jac.Element+a.Covariance.Rows,Covariance.Rows,
	     b.Covariance.Rows,jac.RowInc);
  
  Matrix tmp(6);
  tmp.multTranspose_(b.Covariance,j_b,2);
  Covariance.multiply_(j_b,tmp);
  tmp.multTranspose_(a.Covariance,j_a,2);
  Covariance.addProduct_(j_a,tmp);
  if (!jacobian)return;
  *jacobian=jac;
}

  //this = c = -a - b
void Pose3D::minusMinus_(Pose3D& a,Pose3D& b, Matrix *jacobian)
{
  if(!(m_SubType&4095))
  {
    Transformation3D::operator=((b+a).inverse());
    return; 
  }
  //This is to check
  /*
    Pose3D p;
    p.m_SubType=4095;
    p.Covariance.reallocate(6);
    p.add_(b,a);
    minus_(p);
    return;
  */

  unsigned short ptype;
  unsigned short aptype;
  unsigned short bptype;
  Matrix jac;
  unsigned short rows=getTypeUnion(m_SubType,ptype);
  unsigned short cols=getTypeUnion(b.m_SubType,bptype);
  cols=(getTypeUnion(a.m_SubType,aptype)+(cols<<6));

  Transformation3D::dominusAminusB(a,b, jac,rows, cols);

  for (int i=0; i<6; i++)
    Velocity[i]=0;
  unsigned long h=1;
  for (int j=0;j<3;j++,h=h<<1)
  {
    if (m_SubType&h)
    {
      unsigned long k=1;
      for(int i=0; i<6;i++,k=k<<1)
      {
	if (a.m_SubType&k)Velocity[j]+=jac(j,i)*a.Velocity[i];
	if (b.m_SubType&k) Velocity[j]+=jac(j,i+6)*b.Velocity[i];
      }
    }
  }

  for (int j=3; j<6; j++,h=h<<1)
  {
    if (m_SubType&h)
    {
      unsigned long k=8;
      for (int i=3;i<6; i++,k=k<<1)
      {
	if (a.m_SubType&k)Velocity[j]+=jac(j,i)*a.Velocity[i];
	if (b.m_SubType&k) Velocity[j]+=jac(j,6+i)*b.Velocity[i];
      }
    }
  }
  h=32;
  for (int i=5; i>-1; i--, h=h>>1)
    if (!(ptype&h))jac.deleteRow(i);
  h=32;
  for (int i=5; i>-1; i--,h=h>>1)
    if (!(bptype&h))jac.deleteColumn(i+6);
  h=32;
  for (int i=5; i>-1; i--,h=h>>1)
    if (!(aptype&h))jac.deleteColumn(i);
  
  Matrix j_a(jac.Element,Covariance.Rows,a.Covariance.Rows,jac.RowInc);
  Matrix j_b(jac.Element+a.Covariance.Rows,Covariance.Rows,
	     b.Covariance.Rows,jac.RowInc);
  Matrix tmp(6);
  tmp.multTranspose_(b.Covariance,j_b,2);
  Covariance.multiply_(j_b,tmp);
  tmp.multTranspose_(a.Covariance,j_a,2);
  Covariance.addProduct_(j_a,tmp); 
  if (!jacobian)return;
  *jacobian=jac;
}


void Pose3D::interpolate_(Pose3D& a, Pose3D& b, Timestamp t)
{
  a.limitAngles();
  b.limitAngles();
  if (a.Time==t){
    *this=a;
    return;
  }
  if (b.Time==t){
    *this=b;
    return;
  }
  if (b.Time==a.Time){
    *this=b;
    return;
  }

  if (!((a.getSubType())&(0x8000))){
    *this=a;
    Time=t;
    double d=0;  
    if(a.Time>b.Time)
      {
	t-=b.Time;
	d=t.getDouble();
	t=a.Time;
	t-=b.Time;
	double dd=t.getDouble();
	if (dd>1E-9)
	  d/=dd;
	else d*=1E9;
      }
    else
      {
	t-=a.Time;
	d=t.getDouble();
	t=b.Time;
	t-=a.Time;
	double dd=t.getDouble();
	if (dd>1E-9)
	  d/=dd;
	else d*=1E9;
	d=1-d;
      }
    double w=1-d;
    Covariance*=d;
    Covariance.addProduct_(b.Covariance,w);
    State=0;
    for(int i=0; i<6;i++)
      {
	Velocity[i]*=d;
	Velocity[i]+=(w*b.Velocity[i]);
      }
    Position*=d;
    Vector3D trl(b.Position);
    trl*=w;
    Position+=trl;
    if ((b.Trig[2]>.7)&&(a.Trig[2]>.7))
      {
	double dx=b.EulerAngles[0]-EulerAngles[0];
	if (dx>M_PI)dx=dx-TWO_PI;
	if (dx<-M_PI)dx=TWO_PI+dx;
	EulerAngles[0]+=(w*dx);
	
	dx=b.EulerAngles[2]-EulerAngles[2];
	if (dx>M_PI)dx=dx-TWO_PI;
	if (dx<-M_PI)dx=TWO_PI+dx;
	EulerAngles[2]+=(w*dx);
	dx=b.EulerAngles[1]-EulerAngles[1];
	EulerAngles[1]+=(w*dx);
	setAngles(EulerAngles);
	limitAngles();
      }
    else 
      {
	Rotation3D *r=this;
	*r-=b;
	EulerAngles[0]*=d;
	EulerAngles[1]*=d;
	EulerAngles[2]*=d;
	setAngles(EulerAngles);
	limitAngles();
	*r+=b;
      }
    return;
  }
  //Incremental Poses:
  double d=0;  
  if(a.Time>b.Time)
    {
      if ((t<b.Time)||(t>a.Time)){
	setSubType(a.m_SubType);
	zero();
	return;
      }
      *this=a;//****b
      Time=t;
      t-=b.Time;
      d=t.getDouble();
      t=a.Time;
      t-=b.Time;
      d/=t.getDouble();
    }
  else
    {
      if ((t>b.Time)||(t<a.Time)){
	setSubType(a.m_SubType);
	zero();
	return;
      }
      *this=b;// a********
      Time=t;
      t-=a.Time;
      d=t.getDouble();
      t=b.Time;
      t-=a.Time;
      d/=t.getDouble();
    }
 
  Covariance*=d;
  State=0;
  //  for(int i=0; i<6;i++)
  //Velocity[i]*=d;
  Position*=d;
  EulerAngles[0]*=d;
  EulerAngles[1]*=d;
  EulerAngles[2]*=d;
  setAngles(EulerAngles); 
  limitAngles();
}

void Pose3D::print()
{
  using namespace std;

  TimestampedData::print();
  cerr<<" vtype: "<<m_SubType%64<<" Ptype: "<<((m_SubType>>6)&(0x3F));  
  Transformation3D::print();
  cerr<<" v:"; 
  for (int i=0; i<6;i++)
    cerr<<" "<<Velocity[i];
  cerr<<endl;
  cerr<<" P: "; 
  Covariance.print();
}
bool Pose3D::getMatrix(const std::string & str, ShortMatrix & index,
		       Cure::Matrix & mat)
{
  if ((index.Rows<1)||(index.Columns<4))
    return TimestampedData::getMatrix(str,index,mat);
  if ((index(0,0)<0)||(index(0,1)<0))
    return TimestampedData::getMatrix(str,index,mat);
  if ((index(0,2)<0)||(index(0,3)<0))
    return TimestampedData::getMatrix(str,index,mat);
  int n=0;
  Cure::Matrix x;
  if (str=="x"){
    n=6;
    x.reallocate(6,1);;
    getCoordinates(x.Element);
  } else if ((str=="Covariance")||(str=="Cov")){
    n=Covariance.Rows*Covariance.Columns;
    x=Covariance;
  } else if ((str=="Velocity")||(str=="Vel")){
    x.reallocate(6,1);
    n=6;
    for (int i=0;i<6;i++)x(i,0)=Velocity[i];
  }
  if (n>0){
    mat.reallocate(index(0,2),index(0,3));
    int r=index(0,0)+index(0,2);
    int c=index(0,1)+index(0,3);
    if (!(r*c>n)){
      x.offset(index(0,0),index(0,1),index(0,2),index(0,3));
    }
    mat=x;
    return true;
  }
  return TimestampedData::getMatrix(str,index,mat);
}
bool Pose3D::setMatrix(const std::string & str, ShortMatrix & index,
		       Cure::Matrix & mat)
{
  if ((index.Rows<1)||(index.Columns<4))
    return TimestampedData::setMatrix(str,index,mat);
  if ((index(0,0)<0)||(index(0,1)<0))
    return TimestampedData::setMatrix(str,index,mat);
  if ((index(0,2)<0)||(index(0,3)<0))
    return TimestampedData::setMatrix(str,index,mat);
  if (mat.Rows!=index(0,2))return false;
  if (mat.Columns!=index(0,3))return false;
  Cure::Matrix x;
  if (str=="x"){
    x.reallocate(6,1);;
    getCoordinates(x.Element);
    int r=index(0,0)+index(0,2);
    int c=index(0,1)+index(0,3);
    if ((r*c>6))return false;
    x.offset(index(0,0),index(0,1),index(0,2),index(0,3));
    x=mat;
    x.offset(-index(0,0),-index(0,1),6,1);
    (*this)=(x.Element);
  } else if ((str=="Covariance")||(str=="Cov")){
    int n=Covariance.Rows*Covariance.Columns;
    int r=index(0,0)+index(0,2);
    int c=index(0,1)+index(0,3);
    int rr=Covariance.Rows; 
    if ((r*c>n))return false;
    Covariance.offset(index(0,0),index(0,1),index(0,2),index(0,3));
    Covariance=mat;
    Covariance.offset(-index(0,0),-index(0,1),rr,rr);
  } else if ((str=="Velocity")||(str=="Vel")){
    x.reallocate(6,1);
    for (int i=0;i<6;i++)x(i,0)=Velocity[i];
    int r=index(0,0)+index(0,2);
    int c=index(0,1)+index(0,3);
    if ((r*c>6))return false;
    x.offset(index(0,0),index(0,1),index(0,2),index(0,3));
    x=mat;
    x.offset(-index(0,0),-index(0,1),6,1);
    for (int i=0;i<6;i++)Velocity[i]=x(i,0);
  }
  else  return TimestampedData::setMatrix(str,index,mat);
  return true;
}

} // namespace Cure

std::ostream& operator<< (std::ostream& os, const Cure::Pose3D &p)
{
  os << (Cure::TimestampedData)p << " "
     << " vtype:"<<p.getSubType()%64 
     << " Ptype:" << ((p.getSubType()>>6)&(0x3F))  
     << std::endl;
  os << (Cure::Transformation3D)p << " "
     << " v:[";
  for (int i=0; i<6;i++)
    os << p.Velocity[i] << " ";
  os << "]\n";
  os << " P=" << p.Covariance << std::endl;
  return os;
}
