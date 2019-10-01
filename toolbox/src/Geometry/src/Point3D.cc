// = RCSID
//    $Id: Point3D.cc ,v 1.1 2004/03/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2004 John Folkesson
//    
#include "Point3D.hh"

namespace Cure {

double Point3D::getDistance(const Point3D& p)const
{
  Vector3D temp(*this-p);
  return (sqrt(temp*temp));
} 

void Point3D::transform(Transformation3D&  trans,const Point3D& p)
{  
  trans.transform(p,*this);
}
void Point3D::invTransform(Transformation3D& trans, const Point3D& p)
{  
  trans.invTransform(p,*this);
}
void Point3D::rotate(Rotation3D&  r,const Point3D& p)
{  
  r.rotate(p,*this);
}
void Point3D::invRotate(Rotation3D&  r,const Point3D& p)
{  
  r.invRotate(p,*this);
}
void Point3D::getJacobian(Matrix& jacobian, Matrix dR[5],int type)const
{
  type=type&63;
  int cols=5;
  if (type&4)cols++;
  if (type&8)cols++;
  if (type&16)cols++;
  if (type&32)cols++;
  jacobian.reallocate(3,cols);
  jacobian.offset(0,cols-3,3,3);
  jacobian=dR[3];
  jacobian.reset(3,3);
  jacobian=dR[4];
  jacobian.reset(3,cols);
  cols=3;  
  if (!(type&4)) cols--;
  int k=8;
  for (int i=0; i<3; i++,k*=2)
    {
      if (type&k)
	{
	  jacobian(0,cols)=dR[i](0,0)*X[0]+dR[i](0,1)*X[1]+dR[i](0,2)*X[2];
	  jacobian(1,cols)=dR[i](1,0)*X[0]+dR[i](1,1)*X[1]+dR[i](1,2)*X[2];;
	  jacobian(2,cols)=dR[i](2,0)*X[0]+dR[i](2,1)*X[1]+dR[i](2,2)*X[2];;
	  cols++;
	}
    }
} 

int Point3D::getPixels(double pixels[2], double focalLength)const
{
  if (X[1]<(focalLength+1E-15))return 1;      
  pixels[0]=-focalLength*X[0]/(X[1]-focalLength);
  pixels[1]=-focalLength*X[2]/(X[1]-focalLength);
  return 0;
}
int Point3D::getPixelJacobian(Matrix& jacobian,
			       double focalLength)const
{
  if (X[1]<(focalLength+1E-15))return 1;      
  double temp=1/(X[1]-focalLength);
  jacobian.reallocate(2,3);
  double a=-focalLength*temp;
  temp*=temp;
  temp*=focalLength;
  jacobian(0,0)=a;
  jacobian(0,1)=X[0]*temp;
  jacobian(0,2)=0;
  jacobian(1,0)=0;
  jacobian(1,1)=X[2]*temp;
  jacobian(1,2)=a;
  return 0;
}

} // namespace Cure
