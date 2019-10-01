// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2004 John Folkesson
//    
#include "CircleCloud.hh"

using namespace std;
using namespace Cure;


CircleCloud::CircleCloud()
{
  CloudCount=0;
  Radius=0;
  Center[0]=0;
  Center[1]=0;
  VarRadius=0;
  SensorVar=.0001;
  for (int i=0;i<9;i++){
    Cov[i]=0;
    Sums[i]=0;
  }
}
void CircleCloud::operator = (const CircleCloud & c)
{
  Cloud=c.Cloud;
  SensorVar=c.SensorVar;
  CloudCount=c.CloudCount;
  Center[0]=c.Center[0];
  Center[1]=c.Center[1];
  Radius=c.Radius;
  VarRadius=c.VarRadius;
  for (int i=0;i<9;i++){
    Cov[i]=c.Cov[i];
    Sums[i]=c.Sums[i];
  }
}
LinkedArray * CircleCloud::prune(double minDistance)
{
  LinkedArray *la=&Cloud;
  if (!la->Next)return la;
  if (minDistance==0)
    {
      while(la->Next)la=la->Next;
    }
  else
    {
      if((*la)(0)<minDistance)
	{
	  for (int i=0; i<9; i++)Sums[i]-=la->Element[i+1];
	  while(la->Next)
	    {
	      if((*la)(0)<minDistance)la=la->Next;
	      else break;
	    }
	  if (la->Next)
	    {
	      Cloud.reallocate(la->Length);
	      for (int i=0; i<Cloud.Length; i++)Cloud(i)=(*la)(i);
	      la->cut();
	      delete la;
	      CloudCount--;
	      la=&Cloud;
	    }
	  else 
	    {
	      for (int i=0; i<9; i++)Sums[i]=0;;
	      Cloud.clear();
	      CloudCount=0;
	      return &Cloud;
	    }
	}
      while(la->Next)
	{
	  if((*la)(0)<minDistance)
	    {
	      LinkedArray *ta=la;
	      la=la->Next;
	      for (int i=0; i<9; i++)Sums[i]-=(*ta)(i+1);
	      ta->cut();
	      delete ta;
	      CloudCount--;
	    }
	  else la=la->Next;
	}
    }
  return la;
}

void CircleCloud::merge(const Matrix & c,double minDistance)
{
  double a[10];
  LinkedArray *la=prune(minDistance);
  for (int i=0; i<c.Rows; i++)
    {
      a[0]=c(i,0);
      a[1]=c(i,1);
      a[2]=c(i,2);
      a[3]=a[1]*a[1];
      a[4]=a[2]*a[2];
      a[5]=a[2]*a[1];
      a[6]=a[1]*a[3];
      a[7]=a[2]*a[4];
      a[8]=a[2]*a[3];
      a[9]=a[1]*a[4];
      la->add(a,10);
      for (int i=0; i<9; i++)Sums[i]+=a[i+1];
      CloudCount++;
      la=la->Next;
    }
  
}
int CircleCloud::fitCircle()
{
  if (CloudCount<4)return 1;
  double covXY[4], moments[9];
  double v[2];
  for (int i=0; i<9;i++)moments[i]=Sums[i]/CloudCount;
  covXY[0]=moments[2]-moments[0]*moments[0];
  covXY[1]=moments[4]-moments[0]*moments[1];
  covXY[2]=covXY[1];
  covXY[3]=moments[3]-moments[1]*moments[1];
  v[0]=-(moments[2]+moments[3]);
  v[1]=v[0]*moments[1];
  v[0]*=moments[0];
  v[0]+=moments[5]+moments[8];
  v[1]+=moments[6]+moments[7];
  double d=covXY[0]*covXY[3]-covXY[1]*covXY[1];
  if (d<1E-9)return 1;
  d*=2;
  Center[0]=covXY[3]*v[0]-covXY[1]*v[1];
  Center[1]=covXY[0]*v[1]-covXY[1]*v[0];
  Center[0]/=d;
  Center[1]/=d;
  VarRadius=moments[2]+moments[3]+(Center[0]-2*moments[0])*Center[0]+
    (Center[1]-2*moments[1])*Center[1];
  double temp[4];
  temp[0]=2*Center[0];
  temp[1]=2*Center[1];
  temp[2]=Center[0]*Center[0];
  temp[3]=Center[1]*Center[1];
  Radius=0;
  for (LinkedArray * la=&Cloud; la->Next; la=la->Next)
    Radius+=sqrt((*la)(3)+temp[2]-temp[0]*(*la)(1)+(*la)(4)+
		 temp[3]-temp[1]*(*la)(2));
  Radius/=CloudCount;
  VarRadius-=(Radius*Radius);
  VarRadius+=SensorVar/(CloudCount-3);
  Cov[8]=VarRadius;
  Cov[0]=VarRadius;
  Cov[4]=VarRadius;
  v[0]=Center[0]-moments[0];
  v[1]=Center[1]-moments[1];
  double a=sqrt(v[0]*v[0]+v[1]*v[1]);
  v[0]/=a;
  v[1]/=a;
  double b=v[1]*covXY[0]*v[1]+v[0]*covXY[3]*v[0]-2*v[1]*covXY[1]*v[0];
  a/=b;
  a*=VarRadius;
    Cov[0]+=(a*v[0]*v[0]);
  Cov[1]=a*v[1]*v[0];
  Cov[3]=Cov[1];
  Cov[2]=v[0]*a;
  Cov[6]=Cov[2];
  Cov[4]+=a*v[1]*v[1];
  Cov[5]=v[1]*a;
  Cov[7]=Cov[5];
  Cov[8]+=a;
  return 0;
}
int CircleCloud::fitRadius()
{
  if (CloudCount<4)return 1;
  double temp[4];
  temp[0]=2*Center[0];
  temp[1]=2*Center[1];
  temp[2]=Center[0]*Center[0];
  temp[3]=Center[1]*Center[1];
  Radius=0;
  VarRadius=0;
  for (LinkedArray * la=&Cloud; la->Next; la=la->Next)
    {
      double d=((*la)(3)+temp[2]-temp[0]*(*la)(1)+(*la)(4)+
		   temp[3]-temp[1]*(*la)(2));
      VarRadius+=d;
      Radius+=sqrt(d);
    }
  Radius/=CloudCount;
  VarRadius/=CloudCount;
  VarRadius-=(Radius*Radius);
  VarRadius+=SensorVar/(CloudCount-3);
  return 0;
}
