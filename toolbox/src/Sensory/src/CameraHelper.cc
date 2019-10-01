// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2004 John Folkesson
//    

#include "CameraHelper.hh"

using namespace Cure;

void CameraHelper::setNumberOfPoints(int numPoses,int nMatches){
  NumberMatches=nMatches;
  NumberPoses=numPoses;
  if(T)delete [] T;
  T=new Transformation3D[NumberPoses];
  Pix.reallocate(NumberPoses,NumberMatches*2);
  Pix=0;
}

void CameraHelper::getCameraOffset(Cure::Transformation3D &result){
  calcPostOffset();
  result=PostCameraOffset;
}
void  CameraHelper::recordPixels(Cure::Transformation3D &robotpose, int *pixels)
{ 
  NextPose++;
  if (NextPose>NumberPoses-1)NextPose=NumberPoses-1;
  T[NextPose]=robotpose; 
  for (int i=0; i<NumberMatches; i++)
    {
      Pix(NextPose,2*i)=(double)pixels[2*i];
      Pix(NextPose,2*i+1)=(double)pixels[2*i+1];
    }
}
void CameraHelper::calcPostOffset()
{
  int n=NumberPoses-1;
  int m=NumberMatches;
  Transformation3D c[n];

  Transformation3D toff,roffinv;
  toff.Position=PrioriCameraOffset.Position;

  // c[0] will be the transformation from camera position 0 to 1 without 
  // rotation wrt robot  (ie assuming the camera is not rotated relative to 
  // the robot)
  for (int i=0; i<n; i++)
    {
      c[i]-=toff;
      c[i]-=T[0];
      c[i]+=T[i+1];
      c[i]+=toff;
    }
  Matrix rcinv(3);
  PrioriCameraOffset.getR(rcinv.Element);
  rcinv.transpose();
  roffinv.setR(rcinv.Element);
  Matrix jac[3];
  roffinv.getJacobianR(jac);
  
  Vector3D rotv[m*(NumberPoses)], jv[(3*m*(NumberPoses))],v[(m*(NumberPoses))];
  /** 
* pix_x = (u - Center_u)
 * pix_y = (v - Center_v )
 * So that 
 *     |    pix_x   |
 * V = |  focal_length | is the 3D vector towards the point in the Camera 
 *     |    -pix_y   | Frame. The camera axis is along Y direction. 
 *  
*/

  for (int k=0; k<m;k++)
    for (int i=0; i<(NumberPoses); i++)
      {
	v[(k*NumberPoses)+i](0)=-(ImageCenter[0]-Pix(i,2*(k)));
	v[(k*NumberPoses)+i](1)=FocalLength;
	v[(k*NumberPoses)+i](2)=-(Pix(i,(2*k)+1)-ImageCenter[1]);
	rotv[(k*NumberPoses)+i]=v[(k*NumberPoses)+i].leftMultBy(rcinv);
	for (int j=0; j<3;j++)
	  jv[3*((k*NumberPoses)+i)+j]=  v[(k*NumberPoses)+i].leftMultBy(jac[j]);
      }
  for (int k=0; k<m;k++)
    for (int i=0; i<(n); i++)
      {
	int j=i+1;
	j+=k*(NumberPoses);
	  c[i].invRotate(rotv[j],rotv[j]);
	  rotv[j]*=-1;
	  j*=3;
	  c[i].invRotate(jv[j],jv[j]);
	  jv[j]*=-1;  
	  j++;
	  c[i].invRotate(jv[j],jv[j]);
	  jv[j]*=-1;
	  j++;
	  c[i].invRotate(jv[j],jv[j]);
	  jv[j]*=-1;
      }
  Matrix a(3*(n*m),(m*NumberPoses)+3);
  a.Columns=(m*NumberPoses);
  a=0;
  Matrix x(3*n*m,1);
  Matrix s((m*NumberPoses),1);
  for (int k=0; k<m; k++)
    for (int i=0; i<n; i++)
      for (int j=0; j<3;j++)
	{
	  a(3*((k*n)+i)+j,(k*NumberPoses))=rotv[(k*NumberPoses)](j);
	  a(3*((k*n)+i)+j,(k*NumberPoses)+i+1)=rotv[(k*NumberPoses)+i+1](j);
	  x(3*((k*n)+i)+j,0)=c[i].Position(j);
	}
  Matrix ata((m*NumberPoses));
  ata.multTranspose_(a,a,1);
  ata.invert();
  s.multTranspose_(a,x,1);
  s=(ata*s);
  /*
    a(s+ds)+(s(0)jv[0]+s(1)jv[3])*dEuler[0]+(s(0)jv[1]+s(1)jv[4])*dEuler[1]+
    (s(0)jv[2]+s(1)jv[5])*dEuler[2]=x(0..2)
    and then same with s(1)->s(2) jv[3+i]->jv[6+i] x(0..2)->x(3..5)
    m*(ds,dEuler)^T=dx
  */
  x-=(a*s);
  a.Columns=m*(NumberPoses)+3;  
  for (int k=0; k<m; k++)
    for (int i=0; i<(NumberPoses); i++)
      for (int j=0; j<3; j++)
	jv[3*((k*NumberPoses)+i)+j]*=s((k*NumberPoses)+i,0);
/*  
jv[0]*=s(0,0);
  jv[1]*=s(0,0);
  jv[2]*=s(0,0);

  jv[3]*=s(1,0);
  jv[4]*=s(1,0);
  jv[5]*=s(1,0);

  jv[6]*=s(2,0);
  jv[7]*=s(2,0);
  jv[8]*=s(2,0);
  */

  for (int k=0; k<m; k++)
    for (int i=0; i<(n); i++)
      for (int j=0; j<3; j++)
	jv[3*((k*NumberPoses)+i+1)+j]+=jv[3*((k*NumberPoses))+j];

  for (int k=0; k<m; k++)
    for (int i=0; i<n; i++)
      for (int h=0; h<3;h++)
	for (int j=0; j<3;j++)
	  {
	    a(3*((k*n)+i)+h,m*(NumberPoses)+j)=
	      jv[3*((k*NumberPoses)+i+1)+j](h);
	  }
  /*
    Matrix a(3*(n*m),m*(n+1)+3);
  Matrix ata(m*(n+1));
  ata.multTranspose_(a,a,1);
  ata.invert();
  s.multTranspose_(a,x,1);
  s=(ata*s);
  a*(ds,de)=x
  (ds,de)=(a^Ta)^-1 a^T x
  */
  ata.multTranspose_(a,a,1);
  ata.invert();
  s.multTranspose_(a,x,1);
  s=(ata*s);  
  roffinv.getAngles(x.Element);
  for (int i=0; i<3;i++)x(i,0)+=s(NumberPoses*m+i,0);
  roffinv.setAngles(x.Element);
  roffinv.getR(rcinv.Element);
  rcinv.transpose();
  PostCameraOffset.setR(rcinv.Element);
  PostCameraOffset.Position=PrioriCameraOffset.Position;
  NextPose=-1;
}
