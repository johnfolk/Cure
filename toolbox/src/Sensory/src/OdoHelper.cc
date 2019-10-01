
#include "OdoHelper.hh"
using namespace std;
using namespace Cure;
OdoHelper::OdoHelper()
{
    J.reallocate(3);
    S.reallocate(3);
    W.reallocate(3);
    X.reallocate(3,1);
    DB.reallocate(3,1);
    S=0;
    X=0;
    DB=0;
    B[0]=0;
    B[1]=1;
    B[2]=1;
    W=1;
    P=W;
    DX=X;
    A=S;
    start=0;
}
OdoHelper::OdoHelper(double initialBias[3],Cure::Matrix& xErrorWeight,
	    Cure::Matrix& smallBiasWeight)
{
    J.reallocate(3);
    S.reallocate(3);
    W.reallocate(3);
    X.reallocate(3,1);
    DB.reallocate(3,1);
    S=0;
    X=0;
    DB=0;
    start=0;
    setInitialBias(initialBias);
    W=1;
    P=W;
    setXErrorWeight(xErrorWeight);    
    setSmallBiasWeight(smallBiasWeight);
    DX=X;
    A=S;
}
void OdoHelper::setXErrorWeight(Cure::Matrix &w)
{
  if ((w.Rows==3)&&(w.Columns==3))
    W=w;
}
void OdoHelper::setSmallBiasWeight(Cure::Matrix &p)
{
    if (p.Rows<3)
      P.Rows=p.Rows;
    else return;
    if (p.Columns<3)
      P.Columns=p.Columns;
    else {
      P.Rows=3;
      return;
    }
    P=p;
    P.Rows=3;
    P.Columns=3;
}

void OdoHelper::odometerData(Cure::Transformation3D & odoPose)
{
  if (start){
    cumOdo=odoPose;
    start=0;
  }
  else
    {
      Transformation3D incOdo=odoPose;
      incOdo-=cumOdo;
      cumOdo=odoPose;
      double ds=incOdo.Position(0);
      ds*=ds;
      double d=incOdo.Position(1);
      d*=d;
      d+=ds;
      ds=sqrt(d);
      if (incOdo.Position(0)<0)ds=-ds;
      X(0,0)+=B[2]*incOdo.getTheta()+B[0]*B[1]*ds;
      J(0,0)=B[1]*ds;
      J(0,1)=B[0]*ds;
      J(0,2)=incOdo.getTheta();
  
      double c=incOdo.Trig[0];
      double s=incOdo.Trig[1];
      double temp[2];
      temp[0]=incOdo.Position(0)*c-incOdo.Position(1)*s;
      temp[1]=incOdo.Position(0)*s+incOdo.Position(1)*c;
      
      X(1,0)+=B[1]*temp[0];
      J(1,0)=-(S(0,0)*temp[1]);
      J(1,1)=-(S(0,1)*temp[1]);
      J(1,2)=-(S(0,2)*temp[1]);
      J(1,1)+=temp[0];
      
      X(2,0)+=B[1]*temp[1];
      J(2,0)=S(0,0)*temp[0];
      J(2,1)=S(0,1)*temp[0];
      J(2,2)=S(0,2)*(temp[0]);
      J(2,1)+=temp[1];  
      S+=J;
    }
}
void OdoHelper::setTruePose(Cure::Transformation3D & robotPose)
{
  Transformation3D incrobo=robotPose;
  incrobo-=TrueStart;
  DX(0,0)=incrobo.getTheta();
  DX(1,0)=incrobo.Position(0);
  DX(2,0)=incrobo.Position(1);
  DX-=X;
  A=S;
  TrueStart=robotPose;
  X=0;
  S=0;
  start=1;
}
void OdoHelper::getBias(double b[3],int n)
{
  //DB=(A^T*W*A+P)^-1*A^T*DX  
  if (!start)return;
  Matrix atemp;
  b[0]=B[0];
  b[1]=B[1];
  b[2]=B[2];
  if (n==3)atemp=A;
  else if (n==2)
    {
      A.Columns=2;
      atemp=A;
      A.Columns=3;
      atemp(0,1)+=A(0,2);
      atemp(1,1)+=A(1,2);
      atemp(2,1)+=A(2,2);
    }
  else if (n==1)
    {
      A.Columns=1;
      atemp=A;
      A.Columns=3;
    }
  else 
    return;
  Matrix m;
  m.multTranspose_(atemp,W,1);
  m=(m*atemp);
  P.Rows=n;
  P.Columns=n;
  m+=P;
  m.invert();
  DB.Rows=n;
  DB.multTranspose_(atemp,DX,1);
  DB=(m*DB);
  for(int i=0; i<n;i++)b[i]+=DB(i,0);
  DB=0;
  DB.Rows=3;
  P.Rows=3;
  P.Columns=3;
}





/*

int main(int argc, char * argv[])
{

  cerr<<"CalibrateOdo ";
  if (argc < 4) {
    printf("Example of usage: "
           "./CalibrateOdo odo_in.dat  calibrated_odo_out.dat \n");
    return -1;
  }
  AddressBank bank(4);
  FileAddress odofile;
  odofile.openReadFile(argv[1]);
  FileAddress outputfile1(&bank);
  outputfile1.openWriteFile(argv[2]);
 
  

  Pose3D podo[2];
  odofile.read(podo[0]);
  int next=1;
 
  pose3D dodo, pcum, pstart;
  pstart=podo[0];
  while (!odofile.read(podo[next]))
    {
      int latest=next;
      next++;
      if (next>1)next=0;
      dodo.subtract(podo[latest],podo[next]);
      double ds=dodo.Postion*dodo*position;
      ds=sqrt(ds);
      dtheta=dodo.getTheta();
      
    }
 
  return 0;

}
void OdoHelper::addBias(Cure::Transformation3D & odoPose)
{
  if (start){
    cumOdo=odoPose;
    start=0;
  }
  else
    {
      Transformation3D incOdo=odoPose;
      incOdo-=cumOdo;
      cumOdo=odoPose;
      double ds=incOdo.Position(0);
      double d=incOdo.Position(1)/incOdo.Position(0);
      d*=d;
      d+=1;
      ds*=sqrt(d);
      X(0,0)+=B[2]*incOdo.getTheta()+B[0]*B[1]*ds;
      double c=incOdo.Trig[0];
      double s=incOdo.Trig[1];
      double temp[2];
      temp[0]=incOdo.Position(0)*c-incOdo.Position(1)*s;
      temp[1]=incOdo.Position(0)*s+incOdo.Position(1)*c;      
      X(1,0)+=B[1]*temp[0];
      X(2,0)+=B[1]*temp[1];
      odoPose.setXYTheta(X.Element);
    }
}
*/

