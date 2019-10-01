// = RCSID
//    $Id: Line2D.cc ,v 1.1 2004/03/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2004 John Folkesson
//    

#include "Line2D.hh"
#include "MatrixStuff.hh"

namespace Cure {

Line2D::Line2D(Point2D& start,Point2D& end)
{
  StartPoint=start;
  EndPoint=end;
}
Line2D::Line2D(double start[2],double end[2])
{
  StartPoint.setXY(start);
  EndPoint.setXY(end);
}

void Line2D::operator = (const Line2D& l)
{
  StartPoint=l.StartPoint;
  EndPoint=l.EndPoint;  
}

int Line2D::getIntersection(Point2D& res, const Line2D& l)const
{
  double n[4],p[2],a[4];
  n[0]=EndPoint.X[0]-StartPoint.X[0];
  n[2]=EndPoint.X[1]-StartPoint.X[1];
  n[1]=l.StartPoint.X[0]-l.EndPoint.X[0];
  n[3]=l.StartPoint.X[1]-l.EndPoint.X[1];
  p[0]=l.StartPoint.X[0]-StartPoint.X[0];
  p[1]=l.StartPoint.X[1]-StartPoint.X[1];
  if (MatrixStuff::invert2x2(n,a))
      return 1;
  double t=a[0]*p[0]+a[1]*p[1];
  res.X[0]=StartPoint.X[0]+n[0]*t;
  res.X[1]=StartPoint.X[1]+n[1]*t;
  if (t<0)return -1;
  if (t>1)return -1;
  t=a[2]*p[0]+a[3]*p[1];
  if (t<0)return -1;
  if (t>1)return -1;
  return 0;
}
double Line2D::getDistance(const Point2D& p)const
{
  double n[2];
  n[0]=EndPoint.X[0]-StartPoint.X[0];
  n[1]=EndPoint.X[1]-StartPoint.X[1];
  double d=n[0]*n[0]+n[1]*n[1];
  double t=p.X[0]-StartPoint.X[0]+p.X[1]-StartPoint.X[1];
  t/=d;
  d=n[0]*t+StartPoint.X[0]-p.X[0];
  d*=d;
  t*=n[1];
  t+=(StartPoint.X[1]-p.X[1]);
  t*=t;
  d+=t;
  d=sqrt(d);
  return d;
}
void Line2D::transform2D(Transformation3D&  trans,const Line2D& p)
{
  StartPoint.transform2D(trans,p.StartPoint);
  EndPoint.transform2D(trans,p.EndPoint);
}
void Line2D::invTransform2D(Transformation3D&  trans,const Line2D& p)
{
  StartPoint.invTransform2D(trans,p.StartPoint);
  EndPoint.invTransform2D(trans,p.EndPoint);
}
//It is assumed type includes xytheta if z is there it will have a column of 0
void Line2D::get2DJacobian(Matrix& jacobian, Matrix dR[5],
			   int type)const
{
  type=type&63;
  int cols=7;
  if (type&4)cols++;
  if (type&16)cols++;
  if (type&32)cols++;
  jacobian.reallocate(4,cols);
  jacobian.offset(0,cols-4,2,2);
  jacobian=dR[3];
  jacobian.offset(2,2,2,2);
  jacobian=dR[3];
  jacobian.reset(2,2);
  jacobian=dR[4];
  jacobian.offset(2,0,2,2);
  jacobian=dR[4];
  jacobian.reset(4,cols);
  cols=2;
  if (type&4)
    {
      cols++;
      jacobian(0,2)=0;
      jacobian(1,2)=0;
      jacobian(2,2)=0;
      jacobian(3,2)=0;
    }
  jacobian(0,cols)=dR[0](0,0)*StartPoint.X[0]+dR[0](0,1)*StartPoint.X[1];
  jacobian(1,cols)=dR[0](1,0)*StartPoint.X[0]+dR[0](1,1)*StartPoint.X[1];
  jacobian(2,cols)=dR[0](0,0)*EndPoint.X[0]+dR[0](0,1)*EndPoint.X[1];
  jacobian(3,cols)=dR[0](1,0)*EndPoint.X[0]+dR[0](1,1)*EndPoint.X[1];
  cols++;
  if (type&16)
    {
      jacobian(0,cols)=dR[1](0,0)*StartPoint.X[0]+dR[1](0,1)*StartPoint.X[1];
      jacobian(1,cols)=dR[1](1,0)*StartPoint.X[0]+dR[1](1,1)*StartPoint.X[1];
      jacobian(2,cols)=dR[1](0,0)*EndPoint.X[0]+dR[1](0,1)*EndPoint.X[1];
      jacobian(3,cols)=dR[1](1,0)*EndPoint.X[0]+dR[1](1,1)*EndPoint.X[1];
      cols++;
    }
  if (type&32)
    {
      jacobian(0,cols)=dR[2](0,0)*StartPoint.X[0]+dR[2](0,1)*StartPoint.X[1];
      jacobian(1,cols)=dR[2](1,0)*StartPoint.X[0]+dR[2](1,1)*StartPoint.X[1];
      jacobian(2,cols)=dR[2](0,0)*EndPoint.X[0]+dR[2](0,1)*EndPoint.X[1];
      jacobian(3,cols)=dR[2](1,0)*EndPoint.X[0]+dR[2](1,1)*EndPoint.X[1];
      cols++;
    }
  double *m=jacobian.Element;
  jacobian.Element=m+cols+2;
  jacobian.Rows=2;
  jacobian.Columns=2;
  jacobian=0;
  jacobian.Element=m+cols+2*jacobian.RowInc;
  jacobian=0;
  jacobian.Element=m;
  jacobian.Rows=4;
  jacobian.Columns=cols+4;
} 

void Line2D::getPolarValues(double *z, int type)const
{
  double dy=EndPoint.X[1]-StartPoint.X[1];
  double dx=EndPoint.X[0]-StartPoint.X[0];
  if ((dx==NAN)||(dy==NAN)){
    z[0]=0;
    z[1]=0;
    int k=2;
    if (type&1)
      {
	z[k]=0;
	k++;
      }
    if (type&2)
	z[k]=0;
    return;

  }
  z[0]=atan2(-dx,dy);
  z[1]=dy*StartPoint.X[0]-dx*StartPoint.X[1];
  z[1]/=sqrt(dx*dx+dy*dy);
  int k=2;
  if (type&1)
  {
    z[k]=atan2(StartPoint(1),StartPoint(0));
    k++;
  }
  if (type&2)
  {
    z[k]=atan2(EndPoint(1),EndPoint(0));
  }
} 
void Line2D::getJacobian(Matrix& jacobian,int type)const
{
  int rows=2;
  int t=(type>>6);
  t=t&3;
  if (t&1)rows++;
  if (t&2)rows++;
  jacobian.reallocate(rows,4);
  double dy=EndPoint.X[1]-StartPoint.X[1];
  double dx=EndPoint.X[0]-StartPoint.X[0];
  double l2=(dx*dx+dy*dy);
  if (l2==0)
    {
      jacobian=0;
      return;
    }
  double temp=1/l2;
  double nx=sqrt(l2);
  double ny=-dx/nx;
  nx=dy/nx;
  jacobian(0,0)=dy*temp;
  jacobian(0,2)=-jacobian(0,0);
  jacobian(0,1)=-dx*temp;
  jacobian(0,3)=-jacobian(0,1);
  jacobian(1,0)=nx/2;
  jacobian(1,1)=ny/2;
  jacobian(1,2)=nx/2;
  jacobian(1,3)=ny/2;
  dx=(EndPoint.X[0]+StartPoint.X[0])/2;
  dy=(EndPoint.X[1]+StartPoint.X[1])/2;
  nx*=dy;
  nx-=(ny*dx);
  for (int i=0; i<4;i++)
    jacobian(1,i)+=(nx*jacobian(0,i));
  rows=2;
  if (t&1)
  {
    rows++;
    dx=StartPoint.X[0];
    dy=StartPoint.X[1];
    l2=(dx*dx+dy*dy);
    if (l2==0)
    {
      dx=0;
      dy=0;
    }
    else
      {
	dx/=l2;
	dy/=l2;
      }
    jacobian(rows,0)=-dy;
    jacobian(rows,2)=0;
    jacobian(rows,1)=dx;
    jacobian(rows,3)=0;
  }
  if (t&2)
  {
    rows++;
    dx=EndPoint.X[0];
    dy=EndPoint.X[1];
    l2=(dx*dx+dy*dy);
    if (l2==0)
    {
      dx=0;
      dy=0;
    }
    else
      {
	dx/=l2;
	dy/=l2;
      }
    jacobian(rows,2)=-dy;
    jacobian(rows,0)=0;
    jacobian(rows,3)=dx;
    jacobian(rows,1)=0;
  }
}

} // namespace Cure
