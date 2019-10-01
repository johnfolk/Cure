// = RCSID
//    $Id: Line3D.cc ,v 1.1 2004/03/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2004 John Folkesson
//    
#include "Line3D.hh"

namespace Cure {

Line3D::Line3D(Point3D& start,Point3D& end)
{
  StartPoint=start;
  EndPoint=end;
}

void Line3D::operator = (const Line3D& l)
{
  StartPoint=l.StartPoint;
  EndPoint=l.EndPoint;  
}
void Line3D::operator = (const Line2D& l)
{
  StartPoint=l.StartPoint;
  EndPoint=l.EndPoint;  
}
void Line3D::transform(Transformation3D& trans,const Line3D& p)
{
  StartPoint.transform(trans,p.StartPoint);
  EndPoint.transform(trans,p.EndPoint);
}
void Line3D::invTransform(Transformation3D&  trans,const Line3D& p)
{
  StartPoint.invTransform(trans,p.StartPoint);
  EndPoint.invTransform(trans,p.EndPoint);
}
int Line3D::getPixels(double startpixel[2],double endpixel[2],
		      double focalLength)const
{
  Point3D p;
  double f=0;
 
  if (StartPoint.getPixels(startpixel,focalLength))
    {
      if (EndPoint.getPixels(endpixel,focalLength))
	return 1;
      Vector3D v=(EndPoint-StartPoint);
      f=(.01*EndPoint(1)-StartPoint(1)+.99*(focalLength+1E-15))/v(1);
      p(0)=StartPoint(0)+f*v(0);
      p(1)=StartPoint(1)+f*v(1);
      p(2)=StartPoint(2)+f*v(2);
      if (p.getPixels(startpixel,focalLength))      
	return 1;
    }
  else if (EndPoint.getPixels(endpixel,focalLength))
    {
      Vector3D v=(StartPoint-EndPoint);
      f=(.01*StartPoint(1)-EndPoint(1)+.99*(focalLength+1E-15))/v(1);
      p(0)=EndPoint(0)+f*v(0);
      p(1)=EndPoint(1)+f*v(1);
      p(2)=EndPoint(2)+f*v(2);
      if (p.getPixels(endpixel,focalLength))      
	return 1;
    }
  return 0;
}
int Line3D::getPixels(double startpixel[2],double endpixel[2],
		      double focalLength,Matrix & jacobian)const
{
  Point3D p;
  double f=0;
  jacobian.reallocate(4,6);
  jacobian.Rows=2;
  jacobian.Columns=3;
  if (StartPoint.getPixels(startpixel,focalLength))
    {
      if (EndPoint.getPixels(endpixel,focalLength))
	return 1;
      Vector3D v=(EndPoint-StartPoint);
      f=(.01*EndPoint(1)-StartPoint(1)+.99*(focalLength+1E-15))/v(1);
      p(0)=StartPoint(0)+f*v(0);
      p(1)=StartPoint(1)+f*v(1);
      p(2)=StartPoint(2)+f*v(2);
      if (p.getPixels(startpixel,focalLength))      
	return 1;
      p.getPixelJacobian(jacobian,focalLength);
      double *m=jacobian.Element;
      jacobian.Element=m+(2*jacobian.RowInc+3);
      EndPoint.getPixelJacobian(jacobian,focalLength);
      jacobian.Element=m+(2*jacobian.RowInc);
      jacobian=0;
      jacobian.Element=m;	
      double jv[2];
      jv[0]=jacobian(0,0)*v(0)+jacobian(0,1)*v(1)+jacobian(0,3)*v(2);
      jv[1]=jacobian(1,0)*v(0)+jacobian(1,1)*v(1)+jacobian(1,3)*v(2);
      jacobian.Columns=6;
      double  temp=-(f-.01)/v(1);
      jacobian(0,3)=f*jacobian(0,0);
      jacobian(0,4)=f*jacobian(0,1)+temp*jv[0];
      jacobian(0,5)=f*jacobian(0,2);
      jacobian(1,3)=f*jacobian(1,0);
      jacobian(1,4)=f*jacobian(1,1)+temp*jv[1];
      jacobian(1,5)=f*jacobian(1,2);
      f=1-f;
      temp=-(f)/v(1);
      jacobian(0,0)=f*jacobian(0,0);
      jacobian(0,1)=f*jacobian(0,1)+temp*jv[0];
      jacobian(0,2)=f*jacobian(0,2);
      jacobian(1,0)=f*jacobian(1,0);
      jacobian(1,1)=f*jacobian(1,1)+temp*jv[1];
      jacobian(1,2)=f*jacobian(1,2);
      jacobian.Rows=4;
    }
  else if (EndPoint.getPixels(endpixel,focalLength))
    {
      Vector3D v=(StartPoint-EndPoint);
      f=(.01*StartPoint(1)-EndPoint(1)+.99*(focalLength+1E-15))/v(1);
      p(0)=EndPoint(0)+f*v(0);
      p(1)=EndPoint(1)+f*v(1);
      p(2)=EndPoint(2)+f*v(2);
      if (p.getPixels(endpixel,focalLength))      
	return 1;
      StartPoint.getPixelJacobian(jacobian,focalLength);
      double *m=jacobian.Element;
      jacobian.Element=m+3;
      jacobian=0;
      jacobian.Element=m+(2*jacobian.RowInc+3);
      p.getPixelJacobian(jacobian,focalLength);
      double jv[2];
      jv[0]=jacobian(0,0)*v(0)+jacobian(0,1)*v(1)+jacobian(0,3)*v(2);
      jv[1]=jacobian(1,0)*v(0)+jacobian(1,1)*v(1)+jacobian(1,3)*v(2);
      jacobian.Element=m+(2*jacobian.RowInc);
      jacobian.Columns=6;
      double  temp=-(f-.01)/v(1);
      jacobian(0,0)=f*jacobian(0,3);
      jacobian(0,1)=f*jacobian(0,4)+temp*jv[0];
      jacobian(0,2)=f*jacobian(0,5);
      jacobian(1,0)=f*jacobian(1,3);
      jacobian(1,1)=f*jacobian(1,4)+temp*jv[1];
      jacobian(1,2)=f*jacobian(1,5);
      f=1-f;
      temp=-(f)/v(1);
      jacobian(0,3)=f*jacobian(0,3);
      jacobian(0,4)=f*jacobian(0,4)+temp*jv[0];
      jacobian(0,5)=f*jacobian(0,5);
      jacobian(1,3)=f*jacobian(1,3);
      jacobian(1,4)=f*jacobian(1,4)+temp*jv[1];
      jacobian(1,5)=f*jacobian(1,5);
      jacobian.Rows=4;
    }
  else
    {
      StartPoint.getPixelJacobian(jacobian,focalLength);
      double *m=jacobian.Element;
      jacobian.Element=m+(2*jacobian.RowInc+3);
      EndPoint.getPixelJacobian(jacobian,focalLength);
      jacobian.Element=m+(2*jacobian.RowInc);
      jacobian=0;
      jacobian.Element=m+3;
      jacobian=0;
      jacobian.Element=m;
      jacobian.Columns=6;
      jacobian.Rows=4;
    }
  return 0;
}
void Line3D::getJacobian(Matrix& jacobian, Matrix dR[5],int type)
{
  type=type&63;
  int cols=8;
  if (type&4)cols++;
  if (type&8)cols++;
  if (type&16)cols++;
  if (type&32)cols++;
  jacobian.reallocate(6,cols);
  jacobian.offset(cols-6,3,3);
  jacobian=dR[3];
  jacobian.offset(3,3);
  jacobian=dR[3];
  jacobian.reset(3,3);
  jacobian=dR[4];
  jacobian.offset(3,0,3,3);
  jacobian=dR[4];
  jacobian.reset(6,cols);
  double *x=jacobian.Element;
  jacobian.Element=x+cols-3;
  jacobian.Rows=3;
  jacobian.Columns=3;
  jacobian=0;
  jacobian.Element=x+cols-6+3*jacobian.RowInc;
  jacobian=0;
  jacobian.Rows=6;
  jacobian.Columns=cols;
  jacobian.Element=x;
  x=StartPoint.X;
  int row=0;
  for (int c=0; c<2; c++)
    {
      cols=3;  
      if (!(type&4)) cols--;
      int k=8;
      for (int i=0; i<3; i++,k*=2)
	{
	  if (type&k)
	    {
	      for(int j=0;j<3;j++)
		jacobian(j+row,cols)=dR[i](j,0)*x[0]+
		  dR[i](j,1)*x[1]+dR[i](j,2)*x[2];
	      cols++;
	    }
	}
      x=EndPoint.X;
      row=3;
    } 
  
}  
  
} // namespace Cure
