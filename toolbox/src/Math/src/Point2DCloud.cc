// = RCSID
//    $Id: Point2DCloud.cc ,v 1.1 2004/03/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2004 John Folkesson
//    
#include "Point2DCloud.hh"


namespace Cure {
Point2DCloud::Point2DCloud()
{
  Orientation=1000;
  LineFramed=0;
  SensorVar=.0001;
  Gamma = 0;
  Normal[0] = 1;
  Normal[1] = 0;
  // Radius = 1e10;
  SigmaRho = 1e10;
  SigmaGamma = 1e10;
}
void Point2DCloud::operator = (const Point2DCloud & c)
{
  Cloud=c.Cloud;
  SensorVar=c.SensorVar;
  Orientation=c.Orientation; 
  Rho=c.Rho;
  Gamma=c.Gamma;
  Center[0]=c.Center[0];
  Center[1]=c.Center[1];
  //Radius=c.Radius;
  SigmaRho=c.SigmaRho; 
  SigmaGamma=c.SigmaGamma;
  Normal[0]=c.Normal[0];
  Normal[1]=c.Normal[1];
  LineFramed=c.LineFramed;
}
void Point2DCloud::operator =(const Matrix & c)
{
  LineFramed=0;
  Cloud=c;
  if (Orientation==1000)
    Orientation=atan2((Cloud(0,1)-Cloud(Cloud.Rows-1,1)),
   (Cloud(Cloud.Rows-1,2)-Cloud(0,2)));
}		   

int Point2DCloud::findRow(double x)
{
  if (x<Cloud(0,1))return 0;
  for (int i=1; i<Cloud.Rows;i++)
    if (x<Cloud(i,1))return i;
  return Cloud.Rows-1;
}
void Point2DCloud::lineFrame()
{
  Matrix t(2);

  t(0,0)=-Normal[1];
  t(1,0)=Normal[0];
  t(0,1)=-Normal[0];
  t(1,1)=-Normal[1];
  Cloud.Columns=2;
  Cloud.Element++;
  Matrix c=Cloud;
  Cloud.multiply_(c,t);
  Cloud.Columns=3;
  Cloud.Element--; 
  Orientation=-M_PI_2;
  LineFramed=1;
}
void Point2DCloud::lineOrder()
{
  for (int i=1; i<Cloud.Rows; i++)
    {
      int k=i-1;
      while ((k>-1)&&(Cloud(i,1)<Cloud(k,1)))k--;
      k++;
      while (i!=k)
	{
	  Cloud.swapRows(i,k);
	  k++;       
	}
    }

}
int Point2DCloud::fitLine()
{
  if (Cloud.Rows<3)return 1;
  double oldGamma=Gamma;
  double oldNormal[2];
  oldNormal[0]=Normal[0];
  oldNormal[1]=Normal[1];
  Center[0] = 0;
  Center[1] = 0;
  double ss[3];
  ss[0] = 0;
  ss[1] = 0;
  ss[2] = 0;
  for (int i=0; i<Cloud.Rows;i++)
    {
      Center[0]+=Cloud(i,1);
      ss[0]+=(Cloud(i,1)*Cloud(i,1));
      ss[2]+=(Cloud(i,1)*Cloud(i,2));
      Center[1]+=Cloud(i,2);
      ss[1]+=(Cloud(i,2)*Cloud(i,2));
    }
  Center[0]/=(double)Cloud.Rows;
  Center[1]/=(double)Cloud.Rows;
  ss[0]-=((double)Cloud.Rows)*(Center[0]*Center[0]);
  ss[2]-=(((double)Cloud.Rows)*Center[0]*Center[1]);   
  ss[2]*=2;
  ss[1]-=(((double)Cloud.Rows)*(Center[1]*Center[1]));

  Gamma=atan2(ss[2],(ss[0]-ss[1]));              
  Gamma/=2;
  Gamma-=M_PI_2;
  double d=Orientation-Gamma;
  while (d>M_PI)d-=2*M_PI;
  while (d<-M_PI)d+=2*M_PI;
  if ((d>1.57)|(d<-1.57))
    Gamma+=M_PI;
  while (Gamma>M_PI)Gamma-=2*M_PI;
  while (Gamma<-M_PI)Gamma+=2*M_PI;
  Orientation=Gamma;
  Normal[0]=cos(Gamma);
  Normal[1]=sin(Gamma);
  Rho=Center[0]*Normal[0]+Center[1]*Normal[1];
  SigmaRho=SensorVar*Cloud.Rows;
  SigmaRho+=Normal[0]*Normal[0]*ss[0];
  SigmaRho+=Normal[1]*Normal[1]*ss[1];
  SigmaRho+=Normal[1]*Normal[0]*ss[2];
  SigmaRho/=(double)(Cloud.Rows-2);
  SigmaRho/=(double)(Cloud.Rows);//************************************+++++
  //s is the variance of the data points in the rho direction off the line.   
  double   t=2*SensorVar;
  t+=Normal[1]*Normal[1]*ss[0];
  t+=Normal[0]*Normal[0]*ss[1];
  t-=Normal[1]*Normal[0]*ss[2];
  t/=(Cloud.Rows-2);
  t*=.6;
  SigmaGamma=(SigmaRho)/(t);
  if (!LineFramed) return 0;
  lineFrame();
  lineOrder();
  t=Center[0]*oldNormal[0]-Center[1]*oldNormal[1];
  Center[1]=Center[0]*oldNormal[1]+Center[1]*oldNormal[0];
  Center[0]=t;

  if (Gamma>0)
    Gamma-=M_PI_2; 
  else
    Gamma+=M_PI_2;
  Gamma+=oldGamma;
  Normal[0]=cos(Gamma);
  Normal[1]=sin(Gamma);  
  return 0;
}
void Point2DCloud::merge(Point2DCloud & c2,double minDistance)
{
  if (!LineFramed)
    {
      Matrix c1=Cloud;
      Cloud.reallocate(Cloud.Rows+c2.Cloud.Rows,3);
      int k=0;
      for (int i=0; i<c1.Rows; i++)
	{
	  if (c1(i,0)>minDistance)
	    {
	      Cloud(k,0)=c1(i,0);
	      Cloud(k,1)=c1(i,1);
	      Cloud(k,2)=c1(i,2);
	      k++;
	    }
	}
      for (int i=0; i<c2.Cloud.Rows; i++)
	{
	  
	  Cloud(k,0)=c2(i,0);
	  Cloud(k,1)=c2(i,1);
	  Cloud(k,2)=c2(i,2);
	  k++;;
 	}
      Cloud.Rows=k; 
      return;
    }
  c2.Normal[0]=Normal[0];
  c2.Normal[1]=Normal[1];
  c2.Orientation=Gamma;
  c2.lineFrame();
  c2.lineOrder();
  Matrix c1=Cloud;
  Cloud.reallocate(c1.Rows+c2.Cloud.Rows,3);
  int k=0;
  int j=0;
  for (int i=0; i<c1.Rows; i++)
    {
      if (c1(i,0)>minDistance)
	{
	  while ((k<c2.Cloud.Rows)&&(c2.Cloud(k,1)<c1(i,1)))
	    {
	      Cloud(j,0)=c2.Cloud(k,0);
	      Cloud(j,1)=c2.Cloud(k,1);
	      Cloud(j,2)=c2.Cloud(k,2);
	      k++;
	      j++;
	    }
	  Cloud(j,0)=c1(i,0);
	  Cloud(j,1)=c1(i,1);
	  Cloud(j,2)=c1(i,2);
	  j++;
	}
    }
  while ((k<c2.Cloud.Rows))
    {
      Cloud(j,0)=c2.Cloud(k,0);
      Cloud(j,1)=c2.Cloud(k,1);
      Cloud(j,2)=c2.Cloud(k,2);
      k++;
      j++;
    }
  Cloud.Rows=j;
  lineOrder();
}
void Point2DCloud::replace(Point2DCloud & c2)
{
  if (c2.Cloud.Rows==0)return;
  double dis=c2.Cloud(0,0);
  if (LineFramed)
    {
      c2.Normal[0]=Normal[0];
      c2.Normal[1]=Normal[1];
      c2.Orientation=Gamma;
      c2.lineFrame();
      c2.lineOrder();
    }
  int k=0;
  for (int i=0; i<Cloud.Rows; i++)
    {
      if (Cloud(i,0)==dis)
	{
	  Cloud(i,1)=c2(k,1);
	  Cloud(i,2)=c2(k,2);
	  k++;
	  if (k==c2.Cloud.Rows)i=Cloud.Rows;
	}
    }
  if (LineFramed)
    lineOrder();
}
void Point2DCloud::getLineValues(double gammaRho[2],double R[4], 
			  double sensorXYTheta[3])
{
  gammaRho[0]=Gamma-sensorXYTheta[2];
  while (gammaRho[0]>M_PI)gammaRho[0]-=2*M_PI;
  while (gammaRho[0]<-M_PI)gammaRho[0]+=2*M_PI;
  gammaRho[1]=(Center[1]-sensorXYTheta[1])*Normal[1]+
    (Center[0]-sensorXYTheta[0])*Normal[0];
  double d=(Center[1]-sensorXYTheta[1])*Normal[0]-
    (Center[0]-sensorXYTheta[0])*Normal[1];
 //d is the distance from the center of the line to closest point.   
  R[0]=SigmaGamma;;
  R[1]=-d*SigmaGamma;;
  R[3]=-(R[1]*d);
  R[3]+=SigmaRho;
  R[1]*=.5;
  R[2]=R[1];
}
void Point2DCloud::getEndpoints(Matrix & x)
{
  if (!LineFramed) 
    lineFrame();
  lineOrder();
  x.reallocate(4,1);
  x(0,0)=-Cloud(0,1)*Normal[1]+Rho*Normal[0];
  x(1,0)=Cloud(0,1)*Normal[0]+Rho*Normal[1];
  x(2,0)=-Cloud(Cloud.Rows-1,1)*Normal[1]+Rho*Normal[0];
  x(3,0)=Cloud(Cloud.Rows-1,1)*Normal[0]+Rho*Normal[1];
  return;
}
int Point2DCloud::extend( int start, double tightness,
			 int sense)
{
  if (start<0)return -1;
  if (!(start<Cloud.Rows))return -1;
  double t=Cloud(start,1)-(sense)*tightness;
  int s=findRow(t);
  if (sense==-1)
    {
      if (s<1)return -1;
      for (int i=s; i>0; i--)
	{
	  if ((Cloud(i,1)-Cloud(i-1,1))>tightness)
	    {
	      if (i<start)
		return i;
	      else return -1;
	    }
	}
    }
  else
    {
      if (s<1)return -1;
      if (s>Cloud.Rows-2)return -1;
      for (int i=s; i<Cloud.Rows; i++)
	{
	  if ((Cloud(i,1)-Cloud(i-1,1))>tightness)
	    {
	      if (i>start+1)
		return (i-1);
	      else return -1;
	    }
	}
    }
  return 0;
}
int Point2DCloud::getLine(Point2DCloud & c1, int start, double tightness,
			   int minCount,double minLength, double maxVar)
{
  if (3>minCount)return 0;
  if ((Cloud.Rows-start)<minCount)return 0;
  int bottom=start;
  int top=0;
  for (int i=start+1; i<Cloud.Rows; i++)
    {
      if ((Cloud(i,1)-Cloud(i-1,1))>tightness)
	{
	  top=0;
	  bottom=i;
	}
      else
	top++;
      if (!(top<minCount))i=Cloud.Rows;
    }
  if ((top<minCount))return 0;
  for (int i=bottom+top; i<Cloud.Rows; i++)
    {
      if ((Cloud(i,1)-Cloud(i-1,1))>tightness)
	{
	  i=Cloud.Rows;
	}
      else
	top++;
    }
  if ((Cloud(bottom+top-1,1)-Cloud(bottom,1)<minLength))
    return getLine(c1,bottom+top,tightness,minCount,minLength, maxVar);
  int oldRows=Cloud.Rows;
  Cloud.Element+=(3*bottom);
  Cloud.Rows=top;
  c1=*this;
  Cloud.Rows=oldRows;
  Cloud.Element-=(3*bottom);
  c1.fitLine();
  if (c1.SigmaRho>maxVar)
    return getLine(c1,bottom+top,tightness,minCount,minLength, maxVar);
  return bottom+top;
}
void Point2DCloud::remove(int start, int number)
{
  if(start<0)
    {
      number+=start;
      start=0;
    }
  int top=Cloud.Rows-number;
  for (int i=start; i<top; i++)
    {
      Cloud(i,0)=Cloud(i+number,0);
      Cloud(i,1)=Cloud(i+number,1);
      Cloud(i,2)=Cloud(i+number,2);
    }
  Cloud.Rows=top;
}
} // namespace Cure
