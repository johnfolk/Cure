// = RCSID
//    $Id: Rotation2D.cc ,v 1.1 2004/03/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2004 John Folkesson
//    
#include "Rotation2D.hh"

#ifndef DEPEND
#include <iostream>
#endif

namespace Cure {

Rotation2D::Rotation2D()
{
  State=0;
  for(int i=0; i<3; i++)
    EulerAngles[i]=0;
  for(int i=0; i<3; i++)
  {  
    Trig[2*i]=1;
    Trig[2*i+1]=0;
  }
}
Rotation2D::Rotation2D(const Rotation2D& t)
{

  ((*this).Rotation2D::operator=)(t);
} 
Rotation2D::~Rotation2D()
{
}

void Rotation2D::operator = (const Rotation2D& t)
{
  State=(t.State&1);
  for(int i=0; i<3; i++)
    EulerAngles[i]=t.EulerAngles[i];
  for(int i=0; i<6; i++)
    Trig[i]=t.Trig[i];
  if (State&1)
  {
    for(int i=0; i<4; i++)
    {   
      Rot2D[i]=t.Rot2D[i];
      Rot2Dinv[i]=t.Rot2Dinv[i];
    }
  }
}

void Rotation2D::setTheta(const double ax)
{
  State=0;
  EulerAngles[0]=ax;
  Trig[0]=cos(ax);
  Trig[1]=sin(ax);
}
void Rotation2D::setPhi(const double ax)
{
  State=0;
  EulerAngles[1]=ax;
  Trig[2]=cos(ax);
  Trig[3]=sin(ax);
}
void Rotation2D::setPsi(const double ax)
{
  State=0;
  EulerAngles[2]=ax;
  Trig[4]=cos(ax);
  Trig[5]=sin(ax);
}

void Rotation2D::getEulerAngles(double angles[3]) const
{
  memcpy(angles, EulerAngles, 3*sizeof(double));
}

void Rotation2D::getRot2D(double rotation[4])
{
  calcRot2D();
  rotation[0]=Rot2D[0];
  rotation[1]=Rot2D[1];
  rotation[2]=Rot2D[2];
  rotation[3]=Rot2D[3];
}

void  Rotation2D::getRot2Dinv(double rotationInverse[4])
{
  calcRot2D();
  rotationInverse[0]=Rot2Dinv[0];
  rotationInverse[1]=Rot2Dinv[1];
  rotationInverse[2]=Rot2Dinv[2];
  rotationInverse[3]=Rot2Dinv[3];
}


int Rotation2D::getJR2DxR2Dinv(double dR[4])
{
  dR[0]=0;
  dR[1]=1;
  dR[2]=-1;
  dR[3]=0;
  return 0;
}



void Rotation2D::calcRot2D()
{
  if (State&1)return;
  State++;
  Rot2D[0]=Trig[0];
  Rot2D[3]=Trig[0];
  Rot2Dinv[0]=Trig[0];
  Rot2Dinv[3]=Trig[0];
  Rot2D[1]=Trig[1];
  Rot2D[2]=-Trig[1];
  Rot2Dinv[1]=-Trig[1];
  Rot2Dinv[2]=Trig[1];
}
void Rotation2D::rotate2D(const double fromx[2],double tox[2])
{  
  double temp_x[2];
  calcRot2D();
  temp_x[0]=fromx[0];
  temp_x[1]=fromx[1];
  tox[0]=Rot2D[0]*temp_x[0]+Rot2D[1]*temp_x[1];
  tox[1]=Rot2D[2]*temp_x[0]+Rot2D[3]*temp_x[1];
}
void Rotation2D::invRotate2D(const double fromx[2],double tox[2])
{  
  double temp[2];
  calcRot2D();
  temp[0]=Rot2Dinv[0]*fromx[0]+Rot2Dinv[1]*fromx[1];
  temp[1]=Rot2Dinv[2]*fromx[0]+Rot2Dinv[3]*fromx[1];
  tox[0]=temp[0];
  tox[1]=temp[1];
}
void Rotation2D::rotate2D(const Vector3D& fromx,Vector3D& tox)
{
  calcRot2D();
  double a0=Rot2D[0]*fromx(0)+Rot2D[1]*fromx(1);
  double a1=Rot2D[2]*fromx(0)+Rot2D[3]*fromx(1);
  tox(0)=a0;
  tox(1)=a1;

}
void Rotation2D::invRotate2D(const Vector3D& fromx,Vector3D& tox)
{
  calcRot2D();
  double a0=Rot2Dinv[0]*fromx(0)+Rot2Dinv[1]*fromx(1);
  double a1=Rot2Dinv[2]*fromx(0)+Rot2Dinv[3]*fromx(1);
  tox(0)=a0;
  tox(1)=a1;  
}
Rotation2D Rotation2D::operator + (const Rotation2D& r)const
{
  Rotation2D res;
  res.setTheta(EulerAngles[0]+r.EulerAngles[0]);
  return(res);
}
Rotation2D Rotation2D::operator - (const Rotation2D& r)const
{
  Rotation2D res;
  res.setTheta(EulerAngles[0]-r.EulerAngles[0]);
  return(res);
}

void Rotation2D::doAplusB(Rotation2D& a, Rotation2D& b, Matrix& jacobian)
{
  if ((jacobian.Rows!=1)||(jacobian.Columns!=2))jacobian.reallocate(1,2); 
  jacobian(0,0)=1;
  jacobian(0,1)=1;
  setTheta(a.EulerAngles[0]+b.EulerAngles[0]);
  calcRot2D();
}
void Rotation2D::doAminusB(Rotation2D& a, Rotation2D& b, Matrix& jacobian)
{
  if ((jacobian.Rows!=1)||(jacobian.Columns!=2))jacobian.reallocate(1,2); 
  jacobian(0,0)=1;
  jacobian(0,1)=-1;
  setTheta(a.EulerAngles[0]-b.EulerAngles[0]);
  calcRot2D();
}
void Rotation2D::dominusAplusB(Rotation2D& a, Rotation2D& b, Matrix& jacobian)
{
  if ((jacobian.Rows!=1)||(jacobian.Columns!=2))jacobian.reallocate(1,2); 
  jacobian(0,0)=-1;
  jacobian(0,1)=1;
  setTheta(-a.EulerAngles[0]+b.EulerAngles[0]);
  calcRot2D();
}
void Rotation2D::dominusAminusB(Rotation2D& a, Rotation2D& b, Matrix& jacobian)
{
  if ((jacobian.Rows!=1)||(jacobian.Columns!=2))jacobian.reallocate(1,2); 
  jacobian(0,0)=-1;
  jacobian(0,1)=-1;
  setTheta(-a.EulerAngles[0]-b.EulerAngles[0]);
  calcRot2D();
}

void Rotation2D::print(int verbose)
{
  using namespace std;

 cerr<<"Euler Angles: "<<EulerAngles[0]<<" "<<EulerAngles[1]<<" "
      <<EulerAngles[2]<<endl;

  if (verbose)
  {
    cerr<<"State: "<<State<<endl;
    if (State&1)
    {
      cerr<<"Rot2D:   ";
      cerr<<Rot2D[0]<<" "<<Rot2D[1]<<endl;
      cerr<<"         "<<Rot2D[2]<<" "<<Rot2D[3]<<endl;
      cerr<<"Rot2Dinv:";
      cerr<<Rot2Dinv[0]<<" "<<Rot2Dinv[1]<<endl;
      cerr<<"         "<<Rot2Dinv[2]<<" "<<Rot2Dinv[3]<<endl;
    }
  }
}

} // namespace Cure

std::ostream& operator<<(std::ostream& os, const Cure::Rotation2D& r)
{
  double ea[3];
  r.getEulerAngles(ea);
  os << "[" 
     << ea[0] << " "
     << ea[1] << " "
     << ea[2] << "]";
  return os;
}
