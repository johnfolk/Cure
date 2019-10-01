// = RCSID
//    $Id: Transformation3D.cc ,v 1.1 2004/03/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2004 John Folkesson
//    
#include "Transformation3D.hh"
#include "MatrixStuff.hh"

namespace Cure {
  int Transformation3D::setCovCoordinates(Matrix & coordinates,
					  unsigned short covType)
  {
    unsigned short k= calcRows(covType);
    if (coordinates.Rows!=k) return 1;
    if (coordinates.Columns!=1)return 1;
    if (coordinates.RowInc==1){
      setCovCoordinates(coordinates.Element,covType);
      return 0;
    }
    double x[k];
    for (int i=0; i<k; i++)x[i]=coordinates(i,0);
    setCovCoordinates(x,covType);
    return 0;
    
  }

  void Transformation3D::setCovCoordinates(const double *coordinates,
					   unsigned short covType)
  {
    covType=(covType&0x3F);
    int dim=0;
    if (covType)
      {
	double x[6];
	getCoordinates(x);
	int k=0;
	while (covType)
	  {
	    if (covType&1)
	      {
		x[k]=coordinates[dim];
		dim++;
	      }
	    k++;
	    covType=(covType>>1);
	  }
	*this=x;
      }
  }
  void Transformation3D::getCovCoordinates(Matrix & coordinates,
					   unsigned short covType) const
  {
    coordinates.reallocate(calcRows(covType),1);
    if (coordinates.RowInc==1){
      getCovCoordinates(coordinates.Element,covType);
      return;
    }
    double x[coordinates.Rows];
    getCovCoordinates(x,covType);
    for (int i=0; i<coordinates.Rows; i++)coordinates(i,0)=x[i];
}

void Transformation3D::getCovCoordinates(double *coordinates,
					 unsigned short covType) const
{
  covType=(covType&0x3F);
  int dim=0;
  if (covType)
    {
      double x[6];
      getCoordinates(x);
      int k=0;
      while (covType)
	{
	  if (covType&1)
	    {
	      coordinates[dim]=x[k];
	      dim++;
	    }
	  k++;
	  covType=(covType>>1);
	}
    }
}


void Transformation3D::transform(const double fromx[3],double tox[3])
{
  Vector3D temp(fromx);
  temp-=Position;
  rotate(temp.X,tox);
}

void Transformation3D::invTransform(const double fromx[3],double tox[3])
{
  Vector3D temp(fromx);
  invRotate(temp.X,tox);
  tox[0]+=Position(0);
  tox[1]+=Position(1);
  tox[2]+=Position(2);
}
void Transformation3D::print()
{
  Position.print();
  Rotation3D::print();
}

void Transformation3D::transform2D(const double fromx[2],double tox[2])
{  
  double temp_x[2];
  temp_x[0]=fromx[0]-Position(0);
  temp_x[1]=fromx[1]-Position(1);
  rotate2D(temp_x,tox);
}
void Transformation3D::invTransform2D(const double fromx[2],double tox[2])
{  
  invRotate2D(fromx,tox);
  tox[0]+=Position(0);
  tox[1]+=Position(1);
}
Transformation3D Transformation3D::operator + (Transformation3D& t)const
{
  Transformation3D res(*this);
  res.Transformation3D::operator+=(t);
  return(res);
}
Transformation3D Transformation3D::operator - (Transformation3D& t) const
{
  Transformation3D res(*this);
  res.Transformation3D::operator-=(t);
  return(res);
}

Transformation3D  Transformation3D::inverse()
{
  Transformation3D res;
  res.Transformation3D::operator-=(*this);
  return(res);    
}
//Rc*xc=Rb*(Ra*(x-xa)-xb) -> xc=xa+Rainv*xb
void Transformation3D::doAplusB(Transformation3D& a, Transformation3D& b,
				Matrix& jacobian,
				int rows, int cols, int deleteExtra)
{
  jacobian.reallocate(6,12);
  jacobian.offset(0,3);
  jacobian=1;
  jacobian.offset(9,3,3);
  jacobian=0;
  jacobian.offset(27,3,3);
  jacobian=0;
  jacobian.offset(6,3,3);
  jacobian=0;
  jacobian.reset(6,12);
  int which_rows=rows>>3;
  int which_cols=cols>>3;
  int tempi=which_cols>>6;
  tempi=(tempi<<3);
  which_cols=which_cols&7;
  which_cols+=tempi;
  Vector3D apos=a.Position;
  Vector3D mpos;
  a.invRotate(b.Position,mpos);  
  Matrix bx(3,1);
  b.Position.getXYZ(bx.Element);
  Matrix aR(3);
  a.getR(aR.Element);
  if (cols&0x38){
    Matrix dxR(3);
    Matrix dR[3];
    a.getJRxRinv(dR);
    dxR.offset(0,0,3,1);
    if (cols&0x8)dxR.multTranspose_(dR[0],bx,1);
    dxR.offset(0,1,3,1);
    if (cols&0x10)dxR.multTranspose_(dR[1],bx,1);
    dxR.offset(0,1,3,1);
    if (cols&0x20)dxR.multTranspose_(dR[2],bx,1);
    dxR.reset(3,3);
    jacobian.offset(0,3,3,3);
    jacobian.multTranspose_(aR,dxR,1);
    jacobian.offset(0,-3,6,12);
  }
  Matrix jac;
  Rotation3D::doAplusB(a,b, jac,which_rows, which_cols);
  Position=mpos;
  Position+=apos;
  int h=1;
  for (int j=0;j<3;j++,h=h<<1)
  {
    if (rows&h)
    {
      int k=0x40;
      for(int i=6; i<9;i++,k=k<<1)
	if (cols&k) jacobian(j,i)=aR(i-6,j);
    }
  }
  for (int j=0; j<3; j++,h=h<<1)
  {
    if (rows&h)
    {
      int k=8;
      for (int i=0;i<3; i++,k=k<<1)
      {
	if (cols&k)jacobian(j+3,i+3)=jac(j,i);
	if (cols&(k<<6))jacobian(j+3,i+9)=jac(j,3+i);
      }
    }
  }
  if (deleteExtra)
  {
    int k=32;
    for (int i=5;i>-1; i--,k=k>>1)
      if (!(rows&k))jacobian.deleteRow(i);
    k=2048;
    for (int i=11;i>-1; i--,k=k>>1)
      if (!(cols&k))jacobian.deleteColumn(i);
  }
}
//Rc*xc=Rbinv*(Ra*(x-xa))+xb -> xc=xa-Rainv*Rb*xb
void Transformation3D::doAminusB(Transformation3D& a, Transformation3D& b,
				Matrix& jacobian,
				int rows, int cols, int deleteExtra)
{
  jacobian.reallocate(6,12);
  jacobian.offset(0,3);
  jacobian=1;
  jacobian.offset(36,3,3);
  jacobian=0;
  jacobian.offset(6,3,3);
  jacobian=0;
  jacobian.reset(6,12);
  int which_rows=rows>>3;
  int which_cols=cols>>3;
  int tempi=which_cols>>6;
  tempi=(tempi<<3);
  which_cols=which_cols&7;
  which_cols+=tempi;
  Matrix bR(3);
  b.getR(bR.Element);
  Matrix aR(3);
  a.getR(aR.Element);
  Vector3D apos=a.Position;
  Matrix cR(3);
  cR.multTranspose_(bR,aR,1);
  Matrix dR[3];
  a.getJRxRinv(dR);
  Matrix bx(3,1);
  Matrix rbx(3,1);
  b.Position.getXYZ(bx.Element);
  bx.minus();
  rbx.multiply(bR,bx);
  if (cols&0x38){
    Matrix dxR(3);
    a.getJRxRinv(dR);
    dxR.offset(0,0,3,1);
    if (cols&0x8)dxR.multTranspose_(dR[0],rbx,1);
    dxR.offset(0,1,3,1);
    if (cols&0x10)dxR.multTranspose_(dR[1],rbx,1);
    dxR.offset(0,1,3,1);
    if (cols&0x20)dxR.multTranspose_(dR[2],rbx,1);
    dxR.reset(3,3);
    jacobian.offset(0,3,3,3);
    jacobian.multTranspose_(aR,dxR,1);
    jacobian.offset(0,-3,5,12);
  }
  if (cols&0xE00){
    Matrix dxR(3);
    b.getJRxRinv(dR);
    dxR.offset(0,0,3,1);
    if (cols&0x200)dxR.multiply_(dR[0],rbx);
    dxR.offset(0,1,3,1);
    if (cols&0x400)dxR.multiply_(dR[1],rbx);
    dxR.offset(0,1,3,1);
    if (cols&0x800)dxR.multiply_(dR[2],rbx);
    dxR.reset(3,3);
    jacobian.offset(0,9,3,3);
    jacobian.multTranspose_(aR,dxR,1);
    jacobian.offset(0,-9,6,12);
  }
  Matrix jac;
  Rotation3D::doAminusB(a,b, jac,which_rows, which_cols);
  Position=(b.Position*cR);
  Position*=-1;
  Position+=apos;
  int h=1;
  for (int j=0;j<3;j++,h=h<<1)
  {
    if (rows&h)
    {
      int k=0x40;
      for(int i=6; i<9;i++,k=k<<1)
	if (cols&k) jacobian(j,i)=-cR(i-6,j);
    }
  }
  for (int j=0; j<3; j++,h=h<<1)
  {
    if (rows&h)
    {
      int k=8;
      for (int i=0;i<3; i++,k=k<<1)
      {
	if (cols&k)jacobian(j+3,i+3)=jac(j,i);
	if (cols&(k<<6))jacobian(j+3,i+9)=jac(j,3+i);
      }
    }
  }
  if (deleteExtra)
  {
    int k=32;
    for (int i=5;i>-1; i--,k=k>>1)
      if (!(rows&k))jacobian.deleteRow(i);
    k=2048;
    for (int i=11;i>-1; i--,k=k>>1)
      if (!(cols&k))jacobian.deleteColumn(i);
  }
}
//Rc*xc=Rb*(Rainv*x+xa-xb) -> xc=Ra*(xb-xa)
void Transformation3D::dominusAplusB(Transformation3D& a, Transformation3D& b,
				Matrix& jacobian,
				int rows, int cols, int deleteExtra)
{
  jacobian.reallocate(6,12);
  jacobian.offset(9,3,3);
  jacobian=0;
  jacobian.offset(27,3,3);
  jacobian=0;
  jacobian.offset(6,3,3);
  jacobian=0;
  jacobian.reset(6,12);
  int which_rows=rows>>3;
  int which_cols=cols>>3;
  int tempi=which_cols>>6;
  tempi=(tempi<<3);
  which_cols=which_cols&7;
  which_cols+=tempi;
  Matrix aR(3);
  a.getR(aR.Element);
  Vector3D cpos=(b.Position-a.Position);
  Matrix dR[3];
  Matrix jaca(3);
  Matrix rbx(3,1);
  cpos=cpos.leftMultBy(aR); 
  if (cols&0x38){
    a.getJRxRinv(dR);
    cpos.getXYZ(rbx.Element);
    jacobian.offset(0,3,3,1);
    if (cols&0x8)jacobian.multiply_(dR[0],rbx);
    jacobian.offset(0,1,3,1);
    if (cols&0x10)jacobian.multiply_(dR[1],rbx);
    jacobian.offset(0,1,3,1);
    if (cols&0x20)jacobian.multiply_(dR[2],rbx);
    jacobian.offset(0,-5,6,12);
  }
  Matrix jac;
  Rotation3D::dominusAplusB(a,b, jac,which_rows, which_cols);
  Position=cpos;
  int h=1;
  for (int j=0;j<3;j++,h=h<<1)
  {
    if (rows&h)
    {
      int k=1;
      for(int i=0; i<3;i++,k=k<<1)
	if (cols&k) jacobian(j,i)=-aR(j,i);
      k=0x40;     
      for(int i=6; i<9;i++,k=k<<1)
	if (cols&k) jacobian(j,i)=aR(j,i-6);
    }
  }
  for (int j=0; j<3; j++,h=h<<1)
  {
    if (rows&h)
    {
      int k=8;
      for (int i=0;i<3; i++,k=k<<1)
	{
	  if (cols&k)jacobian(j+3,i+3)=jac(j,i);
	  if (cols&(k<<6))jacobian(j+3,i+9)=jac(j,3+i);
	}
    }
  }
  if (deleteExtra)
  {
    int k=32;
    for (int i=5;i>-1; i--,k=k>>1)
      if (!(rows&k))jacobian.deleteRow(i);
    k=2048;
    for (int i=11;i>-1; i--,k=k>>1)
      if (!(cols&k))jacobian.deleteColumn(i);
  }
}
//Rc*xc=Rbinv*(Rainv*x+xa)+xb -> xc=-Ra*(xa+Rb*xb)
void Transformation3D::dominusAminusB(Transformation3D& a, Transformation3D& b,
				      Matrix& jacobian,
				      int rows, int cols, int deleteExtra)
{
  jacobian.reallocate(6,12);
   jacobian.offset(36,3,3);
  jacobian=0;
  jacobian.offset(6,3,3);
  jacobian=0;
  jacobian.reset(6,12);
  int which_rows=rows>>3;
  int which_cols=cols>>3;
  int tempi=which_cols>>6;
  tempi=(tempi<<3);
  which_cols=which_cols&7;
  which_cols+=tempi;
  Matrix aR(3);
  a.getR(aR.Element);
  Matrix bR(3);
  b.getR(bR.Element);
  Matrix cRt(3);
  cRt.multiply_(aR,bR);
  Vector3D cpos=b.Position;
  cpos*=-1;
  cpos=cpos.leftMultBy(bR);

  Matrix rbx1(3,1);
  cpos.getXYZ(rbx1.Element);
  cpos-=a.Position;
  cpos=cpos.leftMultBy(aR);//****Added 4/5/06
  Matrix dR[3];
  if (cols&0x38){
    Matrix rbx(3,1);
    cpos.getXYZ(rbx.Element);
    a.getJRxRinv(dR);
    jacobian.offset(0,3,3,1);
    if (cols&0x8)jacobian.multiply_(dR[0],rbx);
    jacobian.offset(0,1,3,1);
    if (cols&0x10)jacobian.multiply_(dR[1],rbx);
    jacobian.offset(0,1,3,1);
    if (cols&0x20)jacobian.multiply_(dR[2],rbx);
    jacobian.offset(0,-5,6,12);
  }
  if (cols&0xE00){
    b.getJRxRinv(dR);
    Matrix dxR(3);
    dxR.offset(0,0,3,1);
    if (cols&0x200)dxR.multiply_(dR[0],rbx1);
    dxR.offset(0,1,3,1);
    if (cols&0x400)dxR.multiply_(dR[1],rbx1);
    dxR.offset(0,1,3,1);
    if (cols&0x800)dxR.multiply_(dR[2],rbx1);
    dxR.reset(3,3);
    jacobian.offset(0,9,3,3);
    jacobian.multiply_(aR,dxR);
    jacobian.offset(0,-9,6,12);
  }
  Matrix jac;
  Rotation3D::dominusAminusB(a,b, jac,which_rows, which_cols);
  Position=cpos;
  int h=1;
  for (int j=0;j<3;j++,h=h<<1)
  {
    if (rows&h)
    {
      int k=1;
      for(int i=0; i<3;i++,k=k<<1)
	if (cols&k) jacobian(j,i)=-aR(j,i);
      k=0x40;
      for(int i=6; i<9;i++,k=k<<1)
	if (cols&k) jacobian(j,i)=-cRt(j,i-6);
    }
  }

  for (int j=0; j<3; j++,h=h<<1)
  {
    if (rows&h)
    {
      int k=8;
      for (int i=0;i<3; i++,k=k<<1)
      {
	if (cols&k)jacobian(j+3,i+3)=jac(j,i);
	if (cols&(k<<6))jacobian(j+3,i+9)=jac(j,3+i);
      }
    }
  }
  if (deleteExtra)
  {
    int k=32;
    for (int i=5;i>-1; i--,k=k>>1)
      if (!(rows&k))jacobian.deleteRow(i);
    k=2048;
    for (int i=11;i>-1; i--,k=k>>1)
      if (!(cols&k))jacobian.deleteColumn(i);
  }
}

} // namespace Cure

std::ostream& operator<<(std::ostream& os, const Cure::Transformation3D& t)
{
  os << "Pos:" << t.Position << " Rot:" << (Cure::Rotation3D)t;
  return os;
}
