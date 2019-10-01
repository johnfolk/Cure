// = RCSID
//    $Id: Rotation3D.cc ,v 1.1 2004/03/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2004 John Folkesson
//    

#include "MatrixStuff.hh"
#include "Rotation3D.hh"

namespace Cure {
  
Rotation3D::Rotation3D()
{
  State=0;
  for(int i=0; i<3; i++)
    EulerAngles[i]=0;
  for(int i=0; i<3; i++)
  {  
    Trig[2*i]=1;
    Trig[2*i+1]=0;
  }
  for(int i=0; i<6; i++)Beta[i]=0;
}
Rotation3D::~Rotation3D()
{
}

void Rotation3D::operator = (const Rotation3D& r)
{
  Rotation2D::operator =(r);
  State=r.State;
  if (State&2)for(int i=0; i<6; i++)Beta[i]=r.Beta[i];
  if (State&4)
    for(int i=0; i<9; i++)Rot[i]=r.Rot[i];
}

void Rotation3D::setAngles(const double ang[3])
{
  State=0;
  for(int i=0; i<3;i++)
  {
    EulerAngles[i]=ang[i];
    Trig[2*i]=cos(EulerAngles[i]);
    Trig[2*i+1]=sin(EulerAngles[i]);
  }  
}

void Rotation3D::setR(const double nR[9])
{
  for (int i=0; i<9; i++)Rot[i]=nR[i];
  extractAngles(Rot,EulerAngles);
  setAngles(EulerAngles); 
}

void Rotation3D::rotate(const Vector3D& fromx,Vector3D& tox)
{
  calcRot();
  Matrix r(Rot,3);
  tox=fromx.leftMultBy(r);
}
void Rotation3D::invRotate(const Vector3D& fromx,Vector3D& tox)
{
  calcRot();
  Matrix r(Rot,3);
  tox=fromx*r;
}

void Rotation3D::invRotate(const double fromx[3],double tox[3])
{ 
  double temp[3];
  calcRot();
  // NOTE that the inverse of a rotation matrix is equal to its transpose
  temp[0]=Rot[0]*fromx[0]+Rot[3]*fromx[1]+Rot[6]*fromx[2];
  temp[1]=Rot[1]*fromx[0]+Rot[4]*fromx[1]+Rot[7]*fromx[2];
  temp[2]=Rot[2]*fromx[0]+Rot[5]*fromx[1]+Rot[8]*fromx[2];
  tox[0]=temp[0];
  tox[1]=temp[1];
  tox[2]=temp[2];
}
void Rotation3D::rotate(const double fromx[3],double tox[3])
{  
  double temp[3];
  calcRot();
  temp[0]=Rot[0]*fromx[0]+Rot[1]*fromx[1]+Rot[2]*fromx[2];
  temp[1]=Rot[3]*fromx[0]+Rot[4]*fromx[1]+Rot[5]*fromx[2];
  temp[2]=Rot[6]*fromx[0]+Rot[7]*fromx[1]+Rot[8]*fromx[2];
  tox[0]=temp[0];
  tox[1]=temp[1];
  tox[2]=temp[2];
}
//Change into a new Rotation3D which is 
//Tnew=T+Told
//Tnew(X)=Told(T(X)) Rnew(X-Xnew)=Rold(Rt(X-Xt)-Xold)
/*
void Rotation3D::addTo(Rotation3D& t)
{
  double tr[9], nr[9]; 
  t.getR(tr);
  calcRot();
  MatrixStuff::mat3x3Mult(nr,Rot,tr);
  setR(nr);
}
*/
//Change into a new Rotation3D which is 
//Tnew=Told+T
void Rotation3D::operator += (Rotation3D& t)
{
   double tr[9], nr[9]; 
  t.getR(tr);
  calcRot();
  MatrixStuff::mat3x3Mult(nr,tr,Rot);
  setR(nr);
}
//Change into a new Rotation3D which is  
//Tnew=Told-T
void Rotation3D::operator -= ( Rotation3D& t)
{
  double tr[9],nr[9]; 
  t.getR(tr);
  calcRot();
  MatrixStuff::mat3x3TrnMult(nr,tr,Rot);
  setR(nr);
}
  /** b=This Become c =a+b*/
 void Rotation3D::leftAdd(Rotation3D& a)
{
   double tr[9], nr[9]; 
  a.getR(tr);
  calcRot();
  MatrixStuff::mat3x3Mult(nr,Rot,tr);
  setR(nr);
}
 /** c=This Become b = -a+c*/
void Rotation3D::leftSubtract(Rotation3D& a)
{
  double tr[9],nr[9]; 
  a.getRinv(tr);
  calcRot();
  MatrixStuff::mat3x3Mult(nr,Rot,tr);
  setR(nr);
}


//Change into a new Rotation3D which is  
//Tnew=T-Told


/*
void Rotation3D::minusFrom(Rotation3D& t)
{
  double tr[9],nr[9]; 
  t.getR(tr);
  calcRot();
  MatrixStuff::mat3x3TrnMult(nr,Rot,tr);
  setR(nr);
}
*/
//Tres=This + T
Rotation3D Rotation3D::operator + (Rotation3D &t)
{
  Rotation3D res(*this);
  (res.Rotation3D::operator+=)(t);
  return( res );
}
//Tres=This - T
Rotation3D Rotation3D::operator - (Rotation3D& t)
{
  Rotation3D res(*this);
  (res.Rotation3D::operator-=)(t);
  return( res );
}
//Tres= - This
/*
Rotation3D Rotation3D::operator - ()
{
  Rotation3D res;
  calcRot();
  MatrixStuff::mat3x3Trn(Rot,res.Rot);
  res.setR(res.Rot);
  return( res );
}
*/
void Rotation3D::calcDR2D()
{
  if (State&2)
    return;

  State+=2;  
  if (((Trig[2]>.001)||(Trig[2]<-.001))&&
      ((Trig[4]>.001)||(Trig[4]<-.001)))
  {
    Beta[4]=Trig[3]/Trig[2];
    Beta[5]=Trig[5]/Trig[4];
    Beta[0]=Trig[3]*Beta[5];
    Beta[1]=Trig[2]/Trig[4];
    Beta[2]=1/Beta[1];  
    Beta[3]=-Trig[3]/(Trig[4]*Trig[4]);
    Beta[2]+=Beta[0]*Beta[4]*Trig[5];  
  }
  else
  {
    Beta[1]=0;
  }
  return;
}
void Rotation3D::getAngles(double inertialAngles[3])const
{
  inertialAngles[0]=EulerAngles[0];
  inertialAngles[1]=EulerAngles[1];
  inertialAngles[2]=EulerAngles[2];
}  
void Rotation3D::getJacobianR(Matrix jacobian[3])
{
  calcRot();
  jacobian[0].reallocate(3);
  jacobian[1].reallocate(3);
  jacobian[2].reallocate(3);
 
  jacobian[0](0,0)=-Rot[1];
  jacobian[0](1,0)=-Rot[4];
  jacobian[0](2,0)=-Rot[7];
  jacobian[0](0,1)=Rot[0];
  jacobian[0](1,1)=Rot[3];
  jacobian[0](2,1)=Rot[6];
  jacobian[0](0,2)=0;
  jacobian[0](1,2)=0;
  jacobian[0](2,2)=0;

  jacobian[1](0,0)=Rot[2]*Trig[0];
  jacobian[1](1,0)=Rot[5]*Trig[0];
  jacobian[1](2,0)=Rot[8]*Trig[0];
  jacobian[1](0,1)=Rot[2]*Trig[1];
  jacobian[1](1,1)=Rot[5]*Trig[1];
  jacobian[1](2,1)=Rot[8]*Trig[1];
  jacobian[1](0,2)=-Trig[2];
  jacobian[1](1,2)=-Trig[3]*Trig[5];
  jacobian[1](2,2)=-Trig[3]*Trig[4];

  jacobian[2](2,0)=-Rot[3];
  jacobian[2](2,1)=-Rot[4];
  jacobian[2](2,2)=-Rot[5];
  jacobian[2](1,0)=Rot[6];
  jacobian[2](1,1)=Rot[7];
  jacobian[2](1,2)=Rot[8];
  jacobian[2](0,0)=0;
  jacobian[2](0,1)=0;
  jacobian[2](0,2)=0;

}
int Rotation3D::transformedAngluarVel(Matrix &incomega,
				      Matrix &absomega, Matrix & a, 
				       Matrix jac[3])
{
  calcRot();
  double d=Trig[2];//Rot[8]*Trig[4]+Rot[5]*Trig[5]);
  if ((d<1E-4)&&(d>-1E-4))return 1;
  a.reallocate(3);
  a(0,0)=Trig[4]/d;
  a(0,1)=Trig[5]/d;
  a(1,0)=-Trig[5];
  a(1,0)=Trig[4];
  a(0,2)=0;
  a(1,2)=0;
  a(2,0)=Trig[3]*a(0,0);
  a(2,1)=Trig[3]*a(0,1);
  a(2,2)=1;
  absomega.multiply_(a,incomega);
  if (jac){
    jac[0].reallocate(3,3);
    jac[1].reallocate(3,3);
    jac[2].reallocate(3,3);
    jac[0]=0;
    jac[1](0,0)=Trig[3]*a(0,0)/d;
    jac[1](0,1)=Trig[3]*a(0,1)/d;
    jac[1](0,2)=0;
    jac[1](1,0)=0;
    jac[1](1,1)=0;
    jac[1](1,2)=0;
    jac[1](2,0)=a(0,0)/d;
    jac[1](2,1)=a(0,1)/d;
    jac[1](2,2)=0;
    jac[2](0,0)=-a(0,1);
    jac[2](0,1)=a(0,0);
    jac[2](0,2)=0;
    jac[2](1,0)=-a(1,1);
    jac[2](1,1)=-a(1,0);
    jac[2](1,2)=0;
    jac[2](2,0)=-a(2,1);
    jac[2](2,1)=a(2,0);
    jac[2](2,2)=0;
  }
  return 0;
}
void Rotation3D::getHessianR(Matrix hessian[9])
{
  calcRot();
  for (int i=0;i<9;i++){
    hessian[i].reallocate(3);
  }
  hessian[0](0,0)=-Rot[0];
  hessian[0](1,0)=-Rot[3];
  hessian[0](2,0)=-Rot[6];
  hessian[0](0,1)=-Rot[1];
  hessian[0](1,1)=-Rot[4];
  hessian[0](2,1)=-Rot[7];
  hessian[0](0,2)=0;
  hessian[0](1,2)=0;
  hessian[0](2,2)=0;

  hessian[1](0,0)=Trig[1]*Trig[3];
  hessian[1](1,0)=-Trig[1]*Trig[2]*Trig[5];
  hessian[1](2,0)=-Trig[1]*Trig[2]*Trig[4];
  hessian[1](0,1)=-Trig[0]*Trig[3];
  hessian[1](1,1)=Trig[0]*Trig[2]*Trig[5];
  hessian[1](2,1)=Trig[0]*Trig[2]*Trig[4];
  hessian[1](0,2)=0;
  hessian[1](1,2)=0;
  hessian[1](2,2)=0;

  hessian[2](0,0)=0;
  hessian[2](1,0)=-Rot[7];
    hessian[2](2,0)=Rot[4];
  hessian[2](0,1)=0;
  hessian[2](1,1)=Rot[6];
  hessian[2](2,1)=-Rot[3];
  hessian[2](0,2)=0;
  hessian[2](1,2)=0;
  hessian[2](2,2)=0;

  hessian[3]=hessian[1];

  hessian[4](0,0)=-Rot[0];
  hessian[4](1,0)=-Trig[0]*Trig[3]*Trig[5];
  hessian[4](2,0)=-Trig[0]*Trig[3]*Trig[4];
  hessian[4](0,1)=-Rot[1];
  hessian[4](1,1)=-Trig[1]*Trig[3]*Trig[5];
  hessian[4](2,1)=-Trig[1]*Trig[3]*Trig[4];
  hessian[4](0,2)=-Rot[2];
  hessian[4](1,2)=-Rot[5];
  hessian[4](2,2)=-Rot[8];

  hessian[5](0,0)=0;
  hessian[5](1,0)=hessian[1](2,1);//Trig[0]*Trig[2]*Trig[4];
  hessian[5](2,0)=-hessian[1](1,1);//-Trig[0]*Trig[2]*Trig[5];
  hessian[5](0,1)=0;
  hessian[5](1,1)=-hessian[1](2,0);//Trig[1]*Trig[2]*Trig[4];
  hessian[5](2,1)=hessian[1](1,0);//-Trig[1]*Trig[2]*Trig[5];
  hessian[5](0,2)=0;
  hessian[5](1,2)=-Trig[3]*Trig[4];
  hessian[5](2,2)=Trig[3]*Trig[5];

  hessian[6]=hessian[2];
  hessian[7]=hessian[5];

  hessian[8](0,0)=0;
  hessian[8](1,0)=-Rot[3];
  hessian[8](2,0)=-Rot[6];
  hessian[8](0,1)=0;
  hessian[8](1,1)=-Rot[4];
  hessian[8](1,2)=-Rot[7];
  hessian[8](0,2)=-Rot[2];
  hessian[8](1,2)=-Rot[5];
  hessian[8](2,2)=-Rot[8];
}
void Rotation3D::getJac(Matrix dR[5])
{
  getJRxRinv(dR);
  dR[3].reallocate(3);
  dR[4].reallocate(3);
  Matrix temp(Rot,3);
  dR[3]=temp;
  dR[4]=temp;
  dR[4]*=-1;
}
/*

  Xr=T(p)=R(Xp-Xt)
  
  J={Jrt Jrp} = {-R dR[0]*Xr,dR[1]*Xr,dR[2]*Xr, R}
  
  dR=grad R * transpose R
*/
void Rotation3D::getJRxRinv(Matrix dR[3])
{
  calcRot();
  dR[0].reallocate(3);
  dR[0](0,0)=0;
  dR[0](1,1)=0;
  dR[0](2,2)=0;
  dR[0](0,1)=Rot[8];
  dR[0](1,0)=-Rot[8];
  dR[0](0,2)=-Rot[5];
  dR[0](2,0)=Rot[5];
  dR[0](1,2)=Rot[2];
  dR[0](2,1)=-Rot[2];
  dR[1].reallocate(3);
  dR[1](0,0)=0;
  dR[1](1,1)=0;
  dR[1](2,2)=0;
  dR[1](0,1)=-Trig[5];
  dR[1](1,0)=Trig[5];
  dR[1](0,2)=-Trig[4];
  dR[1](2,0)=Trig[4];
  dR[1](1,2)=0;
  dR[1](2,1)=0;
  dR[2].reallocate(3);
  dR[2](0,0)=0;
  dR[2](1,1)=0;
  dR[2](2,2)=0;
  dR[2](0,1)=0;
  dR[2](1,0)=0;
  dR[2](0,2)=0;
  dR[2](2,0)=0;
  dR[2](1,2)=1;
  dR[2](2,1)=-1;

}
int Rotation3D::getJR2DxR2Dinv(double dR[4])
{
  calcDR2D();
  if(Beta[1]==0)return 1; 
  dR[0]=Beta[0];
  dR[1]=Beta[2];
  dR[2]=-Beta[1];
  dR[3]=-Beta[0];
  return 0;
}
void Rotation3D::get2DJac(Matrix dR[5])
{
  calcRot2D();
  getJR2DxR2Dinv(dR);
  dR[3].reallocate(2);
  dR[4].reallocate(2);
  dR[3](0,0)=Rot2D[0];
  dR[4](0,0)=-Rot2D[0];
  dR[3](0,1)=Rot2D[1];
  dR[4](0,1)=-Rot2D[1];
  dR[3](1,0)=Rot2D[2];
  dR[4](1,0)=-Rot2D[2];
  dR[3](1,1)=Rot2D[3];
  dR[4](1,1)=-Rot2D[3];
}
int Rotation3D::getJR2DxR2Dinv(Matrix dR[3])
{  
  calcDR2D();
  dR[0].reallocate(2);
  dR[1].reallocate(2);
  dR[2].reallocate(2);
  if(Beta[1]==0)return 1; 
  dR[0](0,0)=Beta[0];
  dR[0](0,1)=Beta[2];
  dR[0](1,0)=-Beta[1];
  dR[0](1,1)=-Beta[0];
  dR[1](0,0)=Beta[4];
  dR[1](0,1)=-Trig[5];
  dR[1](1,0)=0;
  dR[1](1,1)=0;      
  dR[2](0,0)=0;
  dR[2](0,1)=Beta[3];
  dR[2](1,0)=0;
  dR[2](1,1)=Beta[5];
  return 0;
}

void Rotation3D::calcRot2D()
{
  if (State&1)return;
  State++;
  double d=Trig[3]*Trig[5];
  Rot2Dinv[0]=Trig[0]*Trig[2];
  Rot2Dinv[2]=Trig[1]*Trig[2];      
  Rot2Dinv[1]=-(Trig[1]*Trig[4]-Trig[0]*d);
  Rot2Dinv[3]=(Trig[0]*Trig[4]+Trig[1]*d);
  if (((Trig[2]>.001)||(Trig[2]<-.001))&&
      ((Trig[4]>.001)||(Trig[4]<-.001)))
  {
    d/=Trig[4];
    Rot2D[0]=(Trig[0]+Trig[1]*d);
    Rot2D[1]=(Trig[1]-Trig[0]*d);
    Rot2D[2]=-Trig[1]/Trig[4];
    Rot2D[3]=Trig[0]/Trig[4];
    Rot2D[0]/=Trig[2];
    Rot2D[1]/=Trig[2];
  }
  else
  {
    Rot2D[0]=1;
    Rot2D[1]=0;
    Rot2D[3]=1;
    Rot2D[2]=0;
  }
}
void Rotation3D::calcRot()
{
  if (State&4)return;
  State+=4;
  Rot[0]=Trig[0]*Trig[2];
  Rot[1]=Trig[1]*Trig[2];
  Rot[2]=-Trig[3];
  Rot[3]=Trig[3]*Trig[5];
  Rot[4]=Trig[0]*Trig[4]+Trig[1]*Rot[3];
  Rot[3]=-Trig[1]*Trig[4]+Trig[0]*Rot[3];    
  Rot[5]=Trig[2]*Trig[5];
  Rot[6]=Trig[3]*Trig[4];
  Rot[7]=-Trig[0]*Trig[5]+Trig[1]*Rot[6];
  Rot[6]=Trig[1]*Trig[5]+Trig[0]*Rot[6];
  Rot[8]=Trig[2]*Trig[4];  
}
void Rotation3D::print(int verbose)
{
  using namespace std;

  Rotation2D::print(verbose);
  if (verbose)
  {
    if (State&4)
    {
      cerr<<"Rot:     "<<Rot[0]<<" "<<Rot[1]<<" "<<Rot[2]<<endl;
      cerr<<"         "<<Rot[3]<<" "<<Rot[4]<<" "<<Rot[5]<<endl;
      cerr<<"         "<<Rot[6]<<" "<<Rot[7]<<" "<<Rot[8]<<endl;
    }
  }
}
void Rotation3D::limitAngles()
{
  for (int i=0;i<3;i++){
    if (EulerAngles[i]>300){
      double d=EulerAngles[i]/TWO_PI;
      unsigned long a=(unsigned long)d;
      d-=(double)a;
      EulerAngles[i]=(d*TWO_PI);
    }else  if (EulerAngles[i]<-300){
      double d=(-EulerAngles[i])/TWO_PI;
      unsigned long a=(unsigned long)d;
      d-=(double)a;
      EulerAngles[i]=-(d*TWO_PI);
    } 
  }
  while (EulerAngles[1]>M_PI)EulerAngles[1]-=TWO_PI;
  while (EulerAngles[1]<-M_PI)EulerAngles[1]+=TWO_PI;
  if ((EulerAngles[1]<-M_PI_2)||(EulerAngles[1]>M_PI_2))
  {
    State=0;
    EulerAngles[1]=M_PI-EulerAngles[1];
    EulerAngles[2]+=M_PI;
    EulerAngles[0]+=M_PI;
    Trig[4]=-Trig[4];
    Trig[5]=-Trig[5];
    Trig[2]=-Trig[2];
    Trig[1]=-Trig[1];
    Trig[0]=-Trig[0];
  }
  while (EulerAngles[1]>M_PI)EulerAngles[1]-=TWO_PI;
  while (EulerAngles[1]<-M_PI)EulerAngles[1]+=TWO_PI;
  while (EulerAngles[0]>M_PI)EulerAngles[0]-=TWO_PI;
  while (EulerAngles[0]<-M_PI)EulerAngles[0]+=TWO_PI;
  while (EulerAngles[2]>M_PI)EulerAngles[2]-=TWO_PI;
  while (EulerAngles[2]<-M_PI)EulerAngles[2]+=TWO_PI;
}

//Bottom Checked
void Rotation3D::doAplusB(Rotation3D& a, Rotation3D& b, Matrix& jacobian,
			       int whichRows, int whichCols, int deleteExtra)
{
  if ((jacobian.Rows!=3)||(jacobian.Columns!=6))jacobian.reallocate(3,6);
  double rb[9],ra[9],temp[5];
  //copy important stuff to make value safe.
  double ass=a.Trig[3]*a.Trig[5];
  double asc=a.Trig[3]*a.Trig[4];  
  double ac0=a.Trig[0];
  double as0=a.Trig[1];
  double ac1=a.Trig[2];
  double bss=b.Trig[3]*b.Trig[5];
  double bsc=b.Trig[3]*b.Trig[4];
  double bc0=b.Trig[0];
  double bs0=b.Trig[1];
  double bc1=b.Trig[2];  
  b.getR(rb);
  a.getR(ra);

  MatrixStuff::mat3x3Mult(Rot,rb,ra);
  setR(Rot);
  double sec2=1/(Rot[0]*Rot[0]+Rot[1]*Rot[1]);
  temp[0]=-Rot[1]*sec2;
  temp[1]=Rot[0]*sec2;
  temp[2]=-sqrt(sec2);  //dphi/dr2
  temp[3]=Rot[8]*sec2;
  temp[4]=-Rot[5]*sec2;
  if (whichCols&1)
  {
    if (whichRows&1)
    {
      jacobian(0,0)=temp[0]*(-rb[0]*ra[1]-rb[1]*ra[4]-rb[2]*ra[7]);
      jacobian(0,0)+=temp[1]*(rb[0]*ra[0]+rb[1]*ra[3]+rb[2]*ra[6]);
    }
    jacobian(1,0)=0;
    jacobian(2,0)=0;
  }
  if (whichCols&2)
  {
    if (whichRows&1)
      jacobian(0,1)=(temp[0]*ac0+temp[1]*as0)*
	(rb[0]*ra[2]+rb[1]*ra[5]+rb[2]*ra[8]);
    if (whichRows&2)
      jacobian(1,1)=temp[2]*(-rb[0]*ac1-rb[1]*ass-
			     rb[2]*asc);
    if (whichRows&4)      
    {
      jacobian(2,1)=temp[3]*(-rb[3]*ac1-rb[4]*ass-
			     rb[5]*asc);
      jacobian(2,1)+=temp[4]*(-rb[6]*ac1-rb[7]*ass-
			      rb[8]*asc);
    }      
  }
  if (whichCols&4)
  {
    if (whichRows&1)
    {
      jacobian(0,2)=temp[0]*(rb[1]*ra[6]-rb[2]*ra[3]);
      jacobian(0,2)+=temp[1]*(rb[1]*ra[7]-rb[2]*ra[4]);  
    }    
    if (whichRows&2)jacobian(1,2)=temp[2]*(rb[1]*ra[8]-rb[2]*ra[5]);  
    if (whichRows&4)
    {     
      jacobian(2,2)=temp[3]*(rb[4]*ra[8]-rb[5]*ra[5]);
      jacobian(2,2)+=temp[4]*(rb[7]*ra[8]-rb[8]*ra[5]);
    }
  }
  if (whichCols&8)
  {
    if (whichRows&1)
    {
      jacobian(0,3)=temp[0]*(-rb[1]*ra[0]+rb[0]*ra[3]);
      jacobian(0,3)+=temp[1]*(-rb[1]*ra[1]+rb[0]*ra[4]);
    }    
    if (whichRows&2)
      jacobian(1,3)=temp[2]*(-rb[1]*ra[2]+rb[0]*ra[5]);
    if (whichRows&4)
    {
      jacobian(2,3)=temp[3]*(-rb[4]*ra[2]+rb[3]*ra[5]);
      jacobian(2,3)+=temp[4]*(-rb[7]*ra[2]+rb[6]*ra[5]);
    }    
  }
  if (whichCols&16)
  {
    if (whichRows&1)
    {
      jacobian(0,4)=temp[0]*(rb[2]*(bc0*ra[0]+bs0*ra[3])-
			     bc1*ra[6]);
      jacobian(0,4)+=temp[1]*(rb[2]*(bc0*ra[1]+bs0*ra[4])-
			      bc1*ra[7]);
    }      
    if (whichRows&2)jacobian(1,4)=temp[2]*(bc0*rb[2]*ra[2]+
					   bs0*rb[2]*ra[5]-bc1*ra[8]);
    if (whichRows&4)
    {
      jacobian(2,4)=(temp[3]*rb[5]+temp[4]*rb[8])*(bc0*ra[2]+bs0*ra[5]);
      jacobian(2,4)-=(temp[3]*bss+temp[4]*bsc)*ra[8];
    }
  }
  if (whichCols&32)
  {
    jacobian(0,5)=0;
    jacobian(1,5)=0;
    if (whichRows&4)
    {
      jacobian(2,5)=temp[3]*(rb[6]*ra[2]+rb[7]*ra[5]+rb[8]*ra[8]);
      jacobian(2,5)+=temp[4]*(-rb[3]*ra[2]-rb[4]*ra[5]-rb[5]*ra[8]);
    }
  }
  if (deleteExtra)
  {
    if (!(whichRows&4))jacobian.deleteRow(2);
    if (!(whichRows&2))jacobian.deleteRow(1);
    if (!(whichRows&1))jacobian.deleteRow(0);
    if (!(whichCols&32))jacobian.deleteColumn(5);
    if (!(whichCols&16))jacobian.deleteColumn(4);
    if (!(whichCols&8))jacobian.deleteColumn(3);
    if (!(whichCols&4))jacobian.deleteColumn(2);
    if (!(whichCols&2))jacobian.deleteColumn(1);
    if (!(whichCols&1))jacobian.deleteColumn(0);
  }
}
//Bottom Checked
void Rotation3D::doAminusB(Rotation3D& a, Rotation3D& b, Matrix& jacobian,
			   int whichRows, int whichCols, int deleteExtra)
{
  if ((jacobian.Rows!=3)||(jacobian.Columns!=6))jacobian.reallocate(3,6);
  double rb[9],ra[9],temp[5];
  double ass=a.Trig[3]*a.Trig[5];
  double asc=a.Trig[3]*a.Trig[4];  
  double ac0=a.Trig[0];
  double as0=a.Trig[1];
  double ac1=a.Trig[2];
  double bss=b.Trig[3]*b.Trig[5];
  double bsc=b.Trig[3]*b.Trig[4];
  double bc0=b.Trig[0];
  double bs0=b.Trig[1];
  double bc1=b.Trig[2];  
  b.getR(rb);
  a.getR(ra);
  MatrixStuff::mat3x3TrnMult(Rot,rb,ra);
  setR(Rot);
  double sec2=1/(Rot[0]*Rot[0]+Rot[1]*Rot[1]);
  temp[0]=-Rot[1]*sec2;
  temp[1]=Rot[0]*sec2;
  temp[2]=-sqrt(sec2);  //dphi/dr2
  temp[3]=Rot[8]*sec2;
  temp[4]=-Rot[5]*sec2;
  if (whichCols&1)
  {
    if (whichRows&1)
    {
      jacobian(0,0)=temp[0]*(-rb[0]*ra[1]-rb[3]*ra[4]-rb[6]*ra[7]);
      jacobian(0,0)+=temp[1]*(rb[0]*ra[0]+rb[3]*ra[3]+rb[6]*ra[6]);
    }
    jacobian(1,0)=0;
    jacobian(2,0)=0;
  }
  if (whichCols&2)
  {
    if (whichRows&1)
      jacobian(0,1)=(temp[0]*ac0+temp[1]*as0)*
	(rb[0]*ra[2]+rb[3]*ra[5]+rb[6]*ra[8]);
    if (whichRows&2)
      jacobian(1,1)=temp[2]*(-rb[0]*ac1-rb[3]*ass-
			     rb[6]*asc);
    if (whichRows&4)      
    {
      jacobian(2,1)=temp[3]*(-rb[1]*ac1-rb[4]*ass-
			     rb[7]*asc);
      jacobian(2,1)+=temp[4]*(-rb[2]*ac1-rb[5]*ass-
			      rb[8]*asc);
    }
    
  }
  if (whichCols&4)
  {
    if (whichRows&1)
    {
      jacobian(0,2)=temp[0]*(rb[3]*ra[6]-rb[6]*ra[3]);
      jacobian(0,2)+=temp[1]*(rb[3]*ra[7]-rb[6]*ra[4]);  
    }    
    if (whichRows&2)jacobian(1,2)=temp[2]*(rb[3]*ra[8]-rb[6]*ra[5]);  
    if (whichRows&4)
    {     
      jacobian(2,2)=temp[3]*(rb[4]*ra[8]-rb[7]*ra[5]);
      jacobian(2,2)+=temp[4]*(rb[5]*ra[8]-rb[8]*ra[5]);
    }
  }
  if (whichCols&8)
  {
    if (whichRows&1)
    {
      jacobian(0,3)=temp[0]*(-rb[1]*ra[0]-rb[4]*ra[3]-rb[7]*ra[6]);
      jacobian(0,3)+=temp[1]*(-rb[1]*ra[1]-rb[4]*ra[4]-rb[7]*ra[7]);
    }    
    if (whichRows&2)
      jacobian(1,3)=temp[2]*(-rb[1]*ra[2]-rb[4]*ra[5]-rb[7]*ra[8]);
    if (whichRows&4)
    {
      jacobian(2,3)=temp[3]*(rb[0]*ra[2]+rb[3]*ra[5]+rb[6]*ra[8]);
    }    
  }
  
  if (whichCols&16)
  {
    if (whichRows&1)
    {
      jacobian(0,4)=temp[0]*bc0*(rb[2]*ra[0]+rb[5]*ra[3]+
				 rb[8]*ra[6]);
      jacobian(0,4)+=temp[1]*bc0*(rb[2]*ra[1]+rb[5]*ra[4]+
				  rb[8]*ra[7]);
    }      
    if (whichRows&2)
    {
      jacobian(1,4)=temp[2]*bc0*(rb[2]*ra[2]+rb[5]*ra[5]+rb[8]*ra[8]);
    }
    if (whichRows&4)
    {
      jacobian(2,4)=temp[3]*bs0*(rb[2]*ra[2]+rb[5]*ra[5]+rb[8]*ra[8]);
      jacobian(2,4)-=(temp[4]*(bc1*ra[2]+(bss*ra[5]+bsc*ra[8])));
    }
  }
  if (whichCols&32)
  {
    if (whichRows&1)
    {
      jacobian(0,5)=temp[0]*(rb[6]*ra[3]-rb[3]*ra[6]);
      jacobian(0,5)+=temp[1]*(rb[6]*ra[4]-rb[3]*ra[7]);
    }      
    if (whichRows&1)
    {
      jacobian(1,5)=temp[2]*(rb[6]*ra[5]-rb[3]*ra[8]);
    }
    if (whichRows&4){
      jacobian(2,5)=temp[3]*(rb[7]*ra[5]-rb[4]*ra[8]);
      jacobian(2,5)+=temp[4]*(rb[8]*ra[5]-rb[5]*ra[8]);
    }
  }
  if (deleteExtra)
  {
    if (!(whichRows&4))jacobian.deleteRow(2);
    if (!(whichRows&2))jacobian.deleteRow(1);
    if (!(whichRows&1))jacobian.deleteRow(0);
    if (!(whichCols&32))jacobian.deleteColumn(5);
    if (!(whichCols&16))jacobian.deleteColumn(4);
    if (!(whichCols&8))jacobian.deleteColumn(3);
    if (!(whichCols&4))jacobian.deleteColumn(2);
    if (!(whichCols&2))jacobian.deleteColumn(1);
    if (!(whichCols&1))jacobian.deleteColumn(0);
  }
}
//checked
void Rotation3D::dominusAplusB(Rotation3D& a, Rotation3D& b, Matrix& jacobian,
                               int whichRows, int whichCols, int deleteExtra)
{
  if ((jacobian.Rows!=3)||(jacobian.Columns!=6))jacobian.reallocate(3,6);
  double rb[9],ra[9],temp[5];
  double ass=a.Trig[3]*a.Trig[5];
  double asc=a.Trig[3]*a.Trig[4];  
  double ac0=a.Trig[0];
  double as0=a.Trig[1];
  double ac1=a.Trig[2];
  double bss=b.Trig[3]*b.Trig[5];
  double bsc=b.Trig[3]*b.Trig[4];
  double bc0=b.Trig[0];
  double bs0=b.Trig[1];
  double bc1=b.Trig[2];  
  b.getR(rb);
  a.getR(ra);
  MatrixStuff::mat3x3Trn(ra,ra);
  MatrixStuff::mat3x3Mult(Rot,rb,ra);
  MatrixStuff::mat3x3Trn(ra,ra);
  setR(Rot);
  double sec2=1/(Rot[0]*Rot[0]+Rot[1]*Rot[1]);
  temp[0]=-Rot[1]*sec2;
  temp[1]=Rot[0]*sec2;
  temp[2]=-sqrt(sec2);  //dphi/dr2
  temp[3]=Rot[8]*sec2;
  temp[4]=-Rot[5]*sec2;
  if (whichCols&1)
  {
    if (whichRows&1)
    {
      jacobian(0,0)=temp[0]*(-rb[0]*ra[1]+rb[1]*ra[0]);
      jacobian(0,0)+=temp[1]*(-rb[0]*ra[4]+rb[1]*ra[3]);
    }
    jacobian(1,0)=temp[2]*(-rb[0]*ra[7]+rb[1]*ra[6]);
    jacobian(2,0)=temp[3]*(-rb[3]*ra[7]+rb[4]*ra[6]);
    jacobian(2,0)+=temp[4]*(-rb[6]*ra[7]+rb[7]*ra[6]);
  }
  if (whichCols&2)
  {
    if (whichRows&1)
      jacobian(0,1)=(temp[0]*ra[2]+temp[1]*ra[5])*
	(ac0*rb[0]+as0*rb[1])-rb[2]*(ac1*temp[0]+ass*temp[1]);
    if (whichRows&2)
      jacobian(1,1)=temp[2]*(ra[8]*(ac0*rb[0]+as0*rb[1])-rb[2]*asc);
    if (whichRows&4)      
    {
      jacobian(2,1)=temp[3]*(ra[8]*(ac0*rb[3]+as0*rb[4])-rb[5]*asc);
      
      jacobian(2,1)+=temp[4]*(ra[8]*(ac0*rb[6]+as0*rb[7])-rb[8]*asc);
    }
  }
  if (whichCols&4)
  {
    if (whichRows&1){
      jacobian(0,2)=temp[1]*(rb[0]*ra[6]+rb[1]*ra[7]+rb[2]*ra[8]);
    }    
    if (whichRows&2){
      jacobian(1,2)=temp[2]*(-rb[0]*ra[3]-rb[1]*ra[4]-rb[2]*ra[5]);
    }
    if (whichRows&4)
    {     
      jacobian(2,2)=temp[3]*(-rb[3]*ra[3]-rb[4]*ra[4]-rb[5]*ra[5]);
      jacobian(2,2)+=temp[4]*(-rb[6]*ra[3]-rb[7]*ra[4]-rb[8]*ra[5]);
    }
  }
  if (whichCols&8)
  {
    if (whichRows&1)
    {
      jacobian(0,3)=temp[0]*(-rb[1]*ra[0]+rb[0]*ra[1]);
      jacobian(0,3)+=temp[1]*(-rb[1]*ra[3]+rb[0]*ra[4]);
    }    
    if (whichRows&2)
      jacobian(1,3)=temp[2]*(-rb[1]*ra[6]+rb[0]*ra[7]);
    if (whichRows&4)
    {
      jacobian(2,3)=temp[3]*(-rb[4]*ra[6]+rb[3]*ra[7]);
      jacobian(2,3)+=temp[4]*(-rb[7]*ra[6]+rb[6]*ra[7]);
    }    
  }
  if (whichCols&16)
  {
    if (whichRows&1)
    {
      jacobian(0,4)=temp[0]*(rb[2]*(bc0*ra[0]+bs0*ra[1])-bc1*ra[6]);
      jacobian(0,4)+=temp[1]*(rb[2]*(bc0*ra[3]+bs0*ra[4])-bc1*ra[7]);
    }      
    if (whichRows&2)jacobian(1,4)=temp[2]*(bc0*rb[2]*ra[6]+bs0*rb[2]*ra[7]
					   -bc1*ra[8]);
    if (whichRows&4){
      jacobian(2,4)=(temp[3]*rb[5]+temp[4]*rb[8])*(bc0*ra[6]+bs0*ra[7]);
      jacobian(2,4)-=(temp[3]*bss+temp[4]*bsc)*ra[8];
    }
  }
  if (whichCols&32)
  {
    jacobian(0,5)=0;
    jacobian(1,5)=0;
    if (whichRows&4){
      jacobian(2,5)=temp[3]*(rb[6]*ra[6]+rb[7]*ra[7]+rb[8]*ra[8]);
      jacobian(2,5)+=temp[4]*(-rb[3]*ra[6]-rb[4]*ra[7]-rb[5]*ra[8]);
    }
  }
  if (deleteExtra)
  {
    if (!(whichRows&4))jacobian.deleteRow(2);
    if (!(whichRows&2))jacobian.deleteRow(1);
    if (!(whichRows&1))jacobian.deleteRow(0);
    if (!(whichCols&32))jacobian.deleteColumn(5);
    if (!(whichCols&16))jacobian.deleteColumn(4);
    if (!(whichCols&8))jacobian.deleteColumn(3);
    if (!(whichCols&4))jacobian.deleteColumn(2);
    if (!(whichCols&2))jacobian.deleteColumn(1);
    if (!(whichCols&1))jacobian.deleteColumn(0);
  }
}
//Checked  
void Rotation3D::dominusAminusB(Rotation3D& a, Rotation3D& b, Matrix& jacobian,
                                int whichRows, int whichCols, int deleteExtra)
{
  if ((jacobian.Rows!=3)||(jacobian.Columns!=6))jacobian.reallocate(3,6);
  double rb[9],ra[9],temp[5];
  double ass=a.Trig[3]*a.Trig[5];
  double asc=a.Trig[3]*a.Trig[4];  
  double ac0=a.Trig[0];
  double as0=a.Trig[1];
  double ac1=a.Trig[2];
  double bss=b.Trig[3]*b.Trig[5];
  double bsc=b.Trig[3]*b.Trig[4];
  double bc0=b.Trig[0];
  double bs0=b.Trig[1];
  double bc1=b.Trig[2];  
  b.getR(rb);
  a.getR(ra);
  MatrixStuff::mat3x3Mult(Rot,ra,rb);
  MatrixStuff::mat3x3Trn(Rot,Rot);
  setR(Rot);
  double sec2=1/(Rot[0]*Rot[0]+Rot[1]*Rot[1]);
  temp[0]=-Rot[1]*sec2;
  temp[1]=Rot[0]*sec2;
  temp[2]=-sqrt(sec2);  //dphi/dr2
  temp[3]=Rot[8]*sec2;
  temp[4]=-Rot[5]*sec2;
  if (whichCols&1)
  {
    if (whichRows&1)
    {
      jacobian(0,0)=temp[0]*(-rb[0]*ra[1]+rb[3]*ra[0]);
      jacobian(0,0)+=temp[1]*(-rb[0]*ra[4]+rb[3]*ra[3]);
    }
    jacobian(1,0)=temp[2]*(-rb[0]*ra[7]+rb[3]*ra[6]);
    jacobian(2,0)=temp[3]*(-rb[1]*ra[7]+rb[4]*ra[6]);
    jacobian(2,0)+=temp[4]*(-rb[2]*ra[7]+rb[5]*ra[6]);
  }
  if (whichCols&2)
  {
    if (whichRows&1)
      jacobian(0,1)=(temp[0]*ra[2]+temp[1]*ra[5])*
	(ac0*rb[0]+as0*rb[3])-rb[6]*
	(ac1*temp[0]+ass*temp[1]);
    if (whichRows&2)
      jacobian(1,1)=temp[2]*(ra[8]*(ac0*rb[0]+as0*rb[3])-
			     rb[6]*asc);
    if (whichRows&4)      
    {
      jacobian(2,1)=temp[3]*(ra[8]*(ac0*rb[1]+as0*rb[4])-
			     rb[7]*asc);
      
      jacobian(2,1)+=temp[4]*(ra[8]*(ac0*rb[2]+as0*rb[5])-
			      rb[8]*asc);
    }
  }
  if (whichCols&4)
  {
    if (whichRows&1){
      jacobian(0,2)=temp[1]*(rb[0]*ra[6]+rb[3]*ra[7]+rb[6]*ra[8]);
    }    
    if (whichRows&2){
      jacobian(1,2)=temp[2]*(-rb[0]*ra[3]-rb[3]*ra[4]-rb[6]*ra[5]);
    }
    if (whichRows&4)
    {     
      jacobian(2,2)=temp[3]*(-rb[1]*ra[3]-rb[4]*ra[4]-rb[7]*ra[5]);
      jacobian(2,2)+=temp[4]*(-rb[2]*ra[3]-rb[5]*ra[4]-rb[8]*ra[5]);
    }
  }
  
  if (whichCols&8)
  {
    if (whichRows&1)
    {
      jacobian(0,3)=temp[0]*(-rb[1]*ra[0]-rb[4]*ra[1]-rb[7]*ra[2]);
      jacobian(0,3)+=temp[1]*(-rb[1]*ra[3]-rb[4]*ra[4]-rb[7]*ra[5]);
    }    
    if (whichRows&2)
      jacobian(1,3)=temp[2]*(-rb[1]*ra[6]-rb[4]*ra[7]-rb[7]*ra[8]);
    if (whichRows&4)
    {
      jacobian(2,3)=temp[3]*(rb[0]*ra[6]+rb[3]*ra[7]+rb[6]*ra[8]);
    }    
  }
  
  if (whichCols&16)
  {
    if (whichRows&1)
    {
      jacobian(0,4)=temp[0]*bc0*(rb[2]*ra[0]+rb[5]*ra[1]+
				 rb[8]*ra[2]);
      jacobian(0,4)+=temp[1]*bc0*(rb[2]*ra[3]+rb[5]*ra[4]+
				  rb[8]*ra[5]);
    }      
    if (whichRows&2){
      jacobian(1,4)=temp[2]*bc0*(rb[2]*ra[6]+rb[5]*ra[7]+rb[8]*ra[8]);
    }
    if (whichRows&4){
      jacobian(2,4)=temp[3]*bs0*(rb[2]*ra[6]+rb[5]*ra[7]+rb[8]*ra[8]);
      jacobian(2,4)-=(temp[4]*(bc1*ra[6]+bss*ra[7]+bsc*ra[8]));
    }
  }
  if (whichCols&32)
  {
    if (whichRows&1){
      jacobian(0,5)=temp[0]*(rb[6]*ra[1]-rb[3]*ra[1]);
      jacobian(0,5)+=temp[1]*(rb[6]*ra[4]-rb[3]*ra[5]);
    }      
    if (whichRows&1){
      jacobian(1,5)=temp[2]*(rb[6]*ra[7]-rb[3]*ra[8]);
    }
    if (whichRows&4){
      jacobian(2,5)=temp[3]*(rb[7]*ra[7]-rb[4]*ra[8]);
      jacobian(2,5)+=temp[4]*(rb[8]*ra[7]-rb[5]*ra[8]);
    }
  }
  if (deleteExtra)
  {
    if (!(whichRows&4))jacobian.deleteRow(2);
    if (!(whichRows&2))jacobian.deleteRow(1);
    if (!(whichRows&1))jacobian.deleteRow(0);
    if (!(whichCols&32))jacobian.deleteColumn(5);
    if (!(whichCols&16))jacobian.deleteColumn(4);
    if (!(whichCols&8))jacobian.deleteColumn(3);
    if (!(whichCols&4))jacobian.deleteColumn(2);
    if (!(whichCols&2))jacobian.deleteColumn(1);
    if (!(whichCols&1))jacobian.deleteColumn(0);
  }
}
void Rotation3D::extractAngles(double nr[9],double x[3])
{
  double d=0;
  int tst=0;
  if ((nr[2]-1)>=1E-65)
    {
      tst=1;
      x[1]=-M_PI_2;
    }
  else if ((nr[2]+1)<=-1E-65)
    {
      tst=1;
      x[1]=M_PI_2;  
    }
  if (tst)
    {
      x[0]=atan2(-nr[3],nr[4]);
      x[2]=0;
      return;      
    }
  else x[1]=asin(-nr[2]);
  if (!(nr[0]>0))
    if (!(nr[0]<0))
      if (!(nr[1]<0))
	if (!(nr[1]>0))
	  tst=1;
  if (tst)
  {
    if (nr[2]>0) 
    {
      d=(nr[3]+nr[7])/(-1-nr[2]);
      if ((d<1)&&(d>-1))
      {
	d=asin(d);
	if (nr[3]>0)x[0]=d;
	else x[0]=M_PI-d;
	x[2]=0;
      }
      else 
      {
	x[0]=0;
	x[2]=0;
      }
    }
    else 
    {
      d=(nr[4]+nr[6])/(1-nr[2]);
      if ((d<1)&&(d>-1))
      {
	d=acos(d);
	if (nr[4]>0)x[0]=d;
	else x[0]=-d;
	x[2]=0;
      }
      else 
      {
	x[0]=0;
	x[2]=0;
      }
    }
  }
  else
  {
    x[0]=atan2(nr[1],nr[0]);
    if(!((nr[5]==0)&&(nr[8]==0)))
      x[2]=atan2(nr[5],nr[8]);
    else  x[2]=0;
   }
}

} // namespace Cure

std::ostream& operator<<(std::ostream& os, const Cure::Rotation3D& r)
{
  os << (Cure::Rotation2D)r;
  return os;
}
