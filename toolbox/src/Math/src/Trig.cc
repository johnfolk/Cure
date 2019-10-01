// = AUTHOR(S)
//    John Folkesson
//    
//    March 11, 2004
//
//    Copyright (c) 2004 John Folkesson
//    
#include <math.h>
#include "Trig.hh"
using namespace Cure;
Trig::Trig()
{
  HalfToRads=M_PI/(360);
  for (int i=0; i<720;i++)
    {
      double atemp;
      atemp=(double)i*HalfToRads;
      CosAngle[i]=cos(atemp);
    }
  CosAngle[180]=0;
  CosAngle[540]=0;
  for (int i=0; i<97;i++)
    TanAngle[i]=sinOfHalf(i)/cosOfHalf(i);
}

int Trig::intAsin(double newx)
{
  double x[2];
  x[0]=newx;
  x[1]=(1-(newx*newx));
  if (x[1]<0)
    {
      if (newx<-1)return 270;
      else return 450;
    }
  x[1]=sqrt(x[1]);
  return intAtan(x);
}
int Trig::intAtan(double newx[2])
{

  int quad[2];
  double x[2];
  if (newx[0]>0)
    {
      quad[0]=1;
      x[0]=newx[0];
    }
  else
    {
      quad[0]=0;
      x[0]=-newx[0];
    }
  if (newx[1]>0)
    {
      quad[1]=1;
      x[1]=newx[1];
    }
  else
    {
      quad[1]=0;
      x[1]=-newx[1];
    }
  double d;
  int complement;
  if (x[0]>x[1])
    {
      d=x[1]/x[0];
      complement=0;
    }
  else
    {
      d=x[0]/x[1];
      complement=1;
    }
  int b=64;
  if (TanAngle[b]>d)
    b-=32;
  else
    b+=32;
  if (TanAngle[b]>d)
    b-=16;
  else
    b+=16;
  if (TanAngle[b]>d)
    b-=8;
  else
    b+=8;
  if (TanAngle[b]>d)
    b-=4;
  else
    b+=4;
  if (TanAngle[b]>d)
    b-=2;
  else
    b+=2;
  if (TanAngle[b]>d)
    b--;
  else
    b++;
  if (TanAngle[b]<d)
    b++;
  b/=2;
  if (complement)b=90-b;
  if (quad[0])
    {
      if(!quad[1])
	b= 360-b;
    }
  else
    if(quad[1])
      b= 180-b;
    else
      b=180+b;
  if (b>359)b-=360;
  if (b<0)b+=360;
  return b;
}

