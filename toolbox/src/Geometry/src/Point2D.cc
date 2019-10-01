// = RCSID
//    $Id: Point2D.cc ,v 1.1 2004/03/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2004 John Folkesson
//    
#include "Point2D.hh"

namespace Cure {

double Point2D::getDistance(const Point2D& p)const
{
  Vector3D temp;
  temp(0)=X[0]-p.X[0];
  temp(1)=X[1]-p.X[1];
  return (sqrt(temp*temp));
} 

void Point2D::transform2D(Transformation3D& trans,const Point2D& p)
{  
  trans.transform2D(p.X,X);
}
void Point2D::invTransform2D(Transformation3D&  trans,
				      const Point2D& p)
{  
  trans.invTransform2D(p.X,X);
}
void Point2D::rotate2D(Rotation2D&  r,const Point2D& p)
{  
  r.rotate2D(p.X,X);
}
void Point2D::invRotate2D(Rotation2D&  r,const Point2D& p)
{  
 r.invRotate2D(p.X,X);
}
void Point2D::getPolarValues(double thetaRho[2])const
{
  thetaRho[1]=sqrt(X[1]*X[1]+X[0]*X[0]);
  if (thetaRho[1])
    thetaRho[0]=atan2(X[1],X[0]);
  else
    thetaRho[0]=0;
}
void Point2D::get2DJacobian(Matrix& jacobian, Matrix dR[5],int type)const
{
  type=type&63;
  int cols=5;
  if (type&4)cols++;
  if (type&16)cols++;
  if (type&32)cols++;
  jacobian.reallocate(2,cols);
 
  jacobian.offset(0,cols-2,2,2);
  jacobian=dR[3];
  jacobian.reset(2,2);
  jacobian=dR[4];
  jacobian.reset(2,cols);
  cols=2;
  if (type&4)
    {
      jacobian(0,2)=0;
      jacobian(1,2)=0;
      cols++;
    }
  jacobian(0,cols)=dR[0](0,0)*X[0]+dR[0](0,1)*X[1];
  jacobian(1,cols)=dR[0](1,0)*X[0]+dR[0](1,1)*X[1];
  cols++;
  if (type&16)
    {
      jacobian(0,cols)=dR[1](0,0)*X[0]+dR[1](0,1)*X[1];
      jacobian(1,cols)=dR[1](1,0)*X[0]+dR[1](1,1)*X[1];
      cols++;
    }
  if (type&32)
    {
      jacobian(0,cols)=dR[2](0,0)*X[0]+dR[2](0,1)*X[1];
      jacobian(1,cols)=dR[2](1,0)*X[0]+dR[2](1,1)*X[1];
      cols++;
    }
} 

void Point2D::getPolarJacobian(Matrix& jacobian)const
{
  jacobian.reallocate(2,2);
  double l2=(X[1]*X[1]+X[0]*X[0]);
  if (l2==0)
    {
      jacobian=1;
      return;
    }
  double temp=1/l2;
  double nx=sqrt(l2);
  double ny=X[1]/nx;
  nx=X[0]/nx;
  jacobian(0,0)=-X[1]*temp;
  jacobian(0,1)=X[0]*temp;
  jacobian(1,0)=nx;
  jacobian(1,1)=ny;
}

} // namespace Cure
