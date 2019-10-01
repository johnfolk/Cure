//
// = AUTHOR(S)
//    John Folkesson
//    
// = COPYRIGHT
//    Copyright (c) 2005 John Folkesson
//

#include "ShortMatrix.hh"
#include "Matrix.hh"

#ifndef DEPEND
#include <math.h>
#include <string.h>
#endif
#ifndef CURESHORTMATRIXDEBUG
#define CURESHORTMATRIXDEBUG 140
#endif

using namespace std;

namespace Cure {

ShortMatrix::ShortMatrix()
{
  AllocatedMatrix=0;
  AllocatedSize=0;
  Rows=0;
  Columns=0;
  RowInc=0;
  Element=0;
}
ShortMatrix::ShortMatrix(const ShortMatrix& mat)
{
  AllocatedMatrix=0;
  AllocatedSize=0;
  Rows=0;
  Columns=0;
  Element=0;
  *this=mat;
  return;
}
ShortMatrix::ShortMatrix(const int r, const int c)
{
  AllocatedMatrix=0;
  AllocatedSize=0;
  Rows=0;
  Columns=0;
  RowInc=0;
  Element=0;
  reallocate(r,c);
}

ShortMatrix::~ShortMatrix()
{
  if (AllocatedMatrix)delete [] AllocatedMatrix;
  AllocatedMatrix=0;
  AllocatedSize=0;
}
void ShortMatrix::grow(const int r,const int c)
{
  ShortMatrix m;
  copy(m);
  Rows=0;
  RowInc=0;
  Columns=0;
  Element=0;
  AllocatedMatrix=0;
  AllocatedSize=0;
  RowInc=0;
  reallocate(r,c);
  int r2=Rows;
  int c2=Columns;
  if (c2<m.Columns)
    m.offset(0,0,m.Rows,c2);
  if (r2<m.Rows)
    m.offset(0,0,r2, m.Columns);
  offset(0,0,m.Rows,m.Columns);
  (*this)=m;
  reset(r2,c2);
}

void ShortMatrix::setupElement(const int r, const int c)
  {
  Columns=c;
  if (c<0)Columns=r;
  if (r<1)
    {
      Rows=0;
      Element=AllocatedMatrix;
      RowInc=0;
      RowInc=Columns;
      return;
    }
  Rows=r;
  RowInc=Columns;
  if (Columns==0){
    Element=AllocatedMatrix;
    return;  
  }
  long n=Rows*Columns;
  if (AllocatedMatrix)
    {
      if (AllocatedSize<n)
	{
	  delete [] AllocatedMatrix;      
	  AllocatedMatrix=0;
	  AllocatedSize=0;
	}
      else
	{
	  Element=AllocatedMatrix;
	  return;
	}
    }
  if (n>0)
    {
      AllocatedMatrix=new short[n];
      memset(AllocatedMatrix, 0, sizeof(short) * n);
      AllocatedSize=n;
      Element=AllocatedMatrix;
      return;
    }
  Element=0;
}
void ShortMatrix::operator = (const short d) 
{
  long n=Rows*Columns;
  if (RowInc==Columns)
    memset(Element, 0, sizeof(short) * n);
  else
    {
      long irow=0;
      for (long i=0; i<Rows; i++,irow+=RowInc)
	for (long j=0; j<Columns; j++)
	  Element[irow+j]=0;
    }
  if (d!=0)
    {
      long top=Rows;
      long irow=0;
      if (Columns<Rows)top=Columns;
      for (long i=0; i<top; i++,irow+=RowInc)
	Element[irow+i]=d;
    }
}
void ShortMatrix::operator += (const ShortMatrix & mat) 
{
#if CURESHORTMATRIXDEBUG >10
  if( Columns!=mat.Columns || Rows!=mat.Rows ) 
    {
      cerr << "\nSHORTMATRIX ERROR Incompatible matrices for +=, not same size\n";
      return;
    }
#endif 
  for (int i=0; i<Rows; i++)
    for (int j=0; j<Columns;j++)
      (*this)(i,j)+=mat(i,j);
}

void ShortMatrix::operator -= (const ShortMatrix & mat) 
{
#if CURESHORTMATRIXDEBUG >10
  if( Columns!=mat.Columns || Rows!=mat.Rows ) 
    {
      cerr << "\nSHORTMATRIX ERROR Incompatible matrices for -=, not same size\n";
      return;
    }
#endif 
  for (int i=0; i<Rows; i++)
    for (int j=0; j<Columns;j++)
      (*this)(i,j)-=mat(i,j);
}

void ShortMatrix::operator *=(const ShortMatrix &mat)
{
#if CURESHORTMATRIXDEBUG >10
  if ((Columns!=mat.Columns)||(Columns!=mat.Rows))
    {
      cerr << "\nSHORTMATRIX ERROR Incompatible matrices for *=, result not same size\n";
      return;
    }
#endif 
  ShortMatrix r;
  copy(r);
  AllocatedSize=0;
  AllocatedMatrix=0;
  Rows=0;
  Columns=0;
  reallocate(r.Rows,r.Columns);
  for (int i=0; i<Rows; i++)
    for (int j=0; j<Columns;j++)
      for (int k=0; k<mat.Rows;k++)
	(*this)(i,j)+=r(i,k)*mat(k,j);
}
void ShortMatrix::operator =(const Matrix &mat)
{
#if CURESHORTMATRIXDEBUG >10
  if ((Columns!=mat.Columns)||(Columns!=mat.Rows))
    {
      cerr << "\nSHORTMATRIX ERROR Incompatible matrices for *=, result not same size\n";
      return;
    }
#endif 
  reallocate(mat.Rows,mat.Columns);
  for (int i=0; i<Rows; i++)
    for (int j=0; j<Columns;j++)
      {
	double d=mat(i,j);
	if (d<0)d-=.5;
	else d+=.5;
   	(*this)(i,j)=(short)((double)(d));
      }
}
void ShortMatrix::addProduct_(const ShortMatrix& a, const ShortMatrix& b)
{
#if CURESHORTMATRIXDEBUG >10
  if (((Rows!=a.Rows)||(Columns!=b.Columns))||
      (b.Rows!=a.Columns))
    {
      cerr << "\nSHORTMATRIX ERROR Incompatible matrices for addProduct\n";
      return;
    }
#endif
  for (int i=0; i<Rows; i++)
    for (int j=0; j<Columns;j++)
      for (int k=0; k<b.Rows;k++)
	(*this)(i,j)+=a(i,k)*b(k,j);
}
void ShortMatrix::minusMult_(const ShortMatrix& a, const ShortMatrix& b)
{
#if CURESHORTMATRIXDEBUG >10
  if (b.Rows!=a.Columns)
    {
      cerr << "\nSHORTMATRIX ERROR Incompatible matrices for minusMult\n";
      return;
    }
#endif
  reallocate(a.Rows,b.Columns);
  (*this)=0;
  for (int i=0; i<Rows; i++)
    for (int j=0; j<Columns;j++)
      for (int k=0; k<b.Rows;k++)
	(*this)(i,j)-=a(i,k)*b(k,j);
}
void ShortMatrix::subtractProduct_(const ShortMatrix& a, const ShortMatrix& b)
{
#if CURESHORTMATRIXDEBUG >10
  if (((Rows!=a.Rows)||(Columns!=b.Columns))||(b.Rows!=a.Columns))
    {
      cerr << "\nSHORTMATRIX ERROR Incompatible matrices for subtractProduct\n";
      return;
    }
#endif
  for (int i=0; i<Rows; i++)
    for (int j=0; j<Columns;j++)
      for (int k=0; k<b.Rows;k++)
	(*this)(i,j)-=a(i,k)*b(k,j);
}

void ShortMatrix::arrayMult(const ShortMatrix& b)
{
#if CURESHORTMATRIXDEBUG >20
  if (((Rows!=b.Rows)||(Columns!=b.Columns)))
    {
      cerr << "\nSHORTMATRIX ERROR Incompatible matrices for arrayMult\n";
      return;
    }
#endif
  long irow=0;
  for (int i=0; i<Rows; i++,irow+=RowInc)
    for (int j=0; j<Columns; j++)
      Element[irow+j]*=b(i,j);
}
void ShortMatrix::arrayDivide(const ShortMatrix& b)
{
#if CURESHORTMATRIXDEBUG >20
  if (((Rows!=b.Rows)||(Columns!=b.Columns)))
    {
      cerr << "\nSHORTMATRIX ERROR Incompatible matrices for arrayMult\n";
      return;
    }
#endif
  long irow=0;
  for (int i=0; i<Rows; i++,irow+=RowInc)
    for (int j=0; j<Columns; j++)
      Element[irow+j]/=b(i,j);
}
void ShortMatrix::multTranspose_(const ShortMatrix& a, const ShortMatrix& b,const int which)
{
  long k=a.Columns;
  long offset_1=a.RowInc;
  long offset_2=b.RowInc;
  long inc_1=1; 
  long inc_2=1;

  long newRows=a.Rows;
  long newCols=b.Columns;
  if (which%2) newRows=a.Columns;
  if (which>1)newCols=b.Rows;
  reallocate(newRows,newCols);
  if (which%2)
    {
      k=a.Rows;
      inc_1=offset_1;
      offset_1=1;
    }
 #if CURESHORTMATRIXDEBUG >10
  if (which>1)
    {
      if (k!=b.Columns)
	{
	  cerr<<"\nERROR ShortMatrix::multTranspose_() INCOMPATABLE b.columns\n";
	  return;
	}
      inc_2=offset_2;
      offset_2=1;;
  
   }
  else if (k!=b.Rows)
    {
      cerr<<"\nERROR ShortMatrix::multTranspose_() INCOMPATABLE b.Rows\n";
      return;
    }
#endif
  k*=inc_1;
  long top=Rows*RowInc;
  long irow1=0;
  for (long irow=0; irow<top;irow+=RowInc, irow1+=offset_1)
    {
      long jcol=0; 
      for (long j=0; j<Columns; j++, jcol+=inc_2)
	{
	  Element[irow+j]=0;
	  long hrow=0;
	  for (long h=0; h<k;h+=inc_1,hrow+=offset_2)
	    Element[irow+j]+=a.Element[irow1+h]*b.Element[hrow+jcol];
	}
    }
}
void  ShortMatrix::operator *= (const short d)
{
  long top=Rows*RowInc;
  for (long i=0; i<top; i+=RowInc)
    for (long j=0;j<Columns;j++)
      Element[i+j]*=d;
}
void  ShortMatrix::operator += (const short d)
{
  long top=Rows;
  if (top>Columns)top=Columns;
  top*=RowInc;
  for (long i=0; i<top; i+=(RowInc+1))
      Element[i]+=d;
}
void  ShortMatrix::operator /= (const short d)
{
  long top=Rows*RowInc;
  if (d==0) {
#if CURESHORTMATRIXDEBUG >10
    cerr << "Tried to divide ShortMatrix::operator/= 0\n";
#endif
    return; 
  }
  for (long i=0; i<top; i+=RowInc)
    for (long j=0;j<Columns;j++)
      Element[i+j]/=d;
}
void ShortMatrix::transpose(const ShortMatrix b)
{
  long irow=0;
  reallocate(b.Columns,b.Rows);
  for (long i=0; i<Rows; i++,irow+=RowInc)
    {
      long jrow=0;
      for (long j=0; j<Columns; j++, jrow+=b.RowInc)
	Element[irow+j]=b.Element[jrow+i];
    }
}
void ShortMatrix::transpose_(const ShortMatrix & b )
{
  long irow=0;
  reallocate(b.Columns,b.Rows);
  for (long i=0; i<Rows; i++,irow+=RowInc)
    {
      long jrow=0;
      for (long j=0; j<Columns; j++, jrow+=b.RowInc)
	Element[irow+j]=b.Element[jrow+i];
    }
}
void ShortMatrix::transpose( void )
{
  ShortMatrix a(*this);
  transpose_(a);
} 

void  ShortMatrix::save(const char *filename)
{
  std::fstream fsout;
  fsout.open(filename, ios::out);
  long irow=0;
  for (long i=0; i<Rows; i++,irow+=RowInc)
    {
      for (long j=0; j<Columns; j++)
	{
	  fsout<<Element[irow+j]<<" ";
	}
      fsout<<endl;
    }
  fsout.close();
}
void ShortMatrix::symetrize()
{
  if(Rows!=Columns)return;
  long irow=0;
  for (long i=0; i<Rows; i++,irow+=RowInc)
    {
      long jrow=irow+RowInc;
      for (long j=i+1; j<Columns; j++,jrow+=RowInc)
	{
	  Element[irow+j]+=Element[jrow+i];
	  Element[irow+j]/=2;
	  Element[jrow+i]=Element[irow+j];
	}
    }
}
void ShortMatrix::addProduct_(const ShortMatrix& b,const short d)
{
  long top=RowInc*Rows;
  long irow2=0;
  for (long irow=0; irow<top;irow+=RowInc, irow2+=b.RowInc)
    for (long j=0; j<Columns; j++)
      Element[irow+j]+=(d*b.Element[irow2+j]);
}
void ShortMatrix::subtractProduct_(const ShortMatrix& b,const short d)
{
  long top=RowInc*Rows;
  long irow2=0;
  for (long irow=0; irow<top;irow+=RowInc, irow2+=b.RowInc)
    for (long j=0; j<Columns; j++)
      Element[irow+j]-=(d*b.Element[irow2+j]);
}
void ShortMatrix::minus()
{
  long top=RowInc*Rows;
  for (long irow=0; irow<top;irow+=RowInc)
    for (long j=0; j<Columns; j++)
      Element[irow+j]=-Element[irow+j];
}
void ShortMatrix::print()
{
  cerr<<AllocatedSize<<" Rows/cols/RowInc: "<<Rows<<" "<<Columns<<" "<<RowInc<<endl;
  long irow=0;
  for (long i=0; i<Rows; i++,irow+=RowInc)
    {
      for (long j=0; j<Columns; j++)
	cerr<<Element[irow+j]<<" ";
      cerr<<endl;
    }
}


long ShortMatrix::trace() const
{
  long d=0;
  long top=Rows;
  if (top>Columns)top=Columns;
  for (long i=0; i<top;i++)
    d+=Element[i*(RowInc+1)];
  return d;
}

ShortMatrix ShortMatrix::operator + (const ShortMatrix& mat) const
{
  ShortMatrix res(Rows,Columns);  
#if CURESHORTMATRIXDEBUG >20
  if( Columns!=mat.Columns || Rows!=mat.Rows ) 
    {
      cerr << "\nERROR Incompatible matrices for +\n";
      return( res );
    }
#endif 
  res.add_(*this,mat);
  return( res );
}
ShortMatrix ShortMatrix::operator - (const ShortMatrix &mat)const 
{

  ShortMatrix res(Rows,Columns);  
#if CURESHORTMATRIXDEBUG >20
  if( Columns!=mat.Columns || Rows!=mat.Rows ) 
    {
      cerr << "\nERROR Incompatible matrices for - \n";
      return( res );
    }
#endif 
  res.subtract_(*this,mat);
  return(res );
}

ShortMatrix ShortMatrix::operator * (const ShortMatrix &mat)const
{
  ShortMatrix res(Rows,mat.Columns);  
#if CURESHORTMATRIXDEBUG >20
  if (Columns!=mat.Rows)
    {
      cerr<<"\nERROR ShortMatrix:: + INCOMPATABLE\n";
      return res;
    }
#endif 
  res.multiply_(*this,mat);
  return (res);
}
void ShortMatrix::deleteRow(int r)
{
  if ((r<0)||(r>=Rows))
    {
#if CURESHORTMATRIXDEBUG >20
      std::cerr<<"ERRROR ShortMatrix::deleteRow out of bounds\n";
#endif
      return;
    }
  if (r==(Rows-1)){
    Rows--;
    return;
  }
  int newr=Rows-1;
  offset(r+1,0,Rows-r-1,Columns);
  ShortMatrix cm;
  copy(cm);
  offset(-1,0,Rows,Columns);
  move(cm);
  offset(-r,0,newr,Columns);
  cm.AllocatedMatrix=0;
}
void ShortMatrix::deleteColumn(int c)
{
#if CURESHORTMATRIXDEBUG >20
  if ((c<0)||(c>=Columns))
    {
      std::cerr<<"ERRROR ShortMatrix::deleteColumn out of bounds\n";
      return;
    }
#endif
  if (c==(Columns-1)){
    Columns--;
    return;
  }
  int newc=Columns-1;
  offset(0,c+1,Rows,Columns-c-1);
  ShortMatrix cm;
  copy(cm);
  offset(0,-1,Rows,Columns);
  move(cm);
  offset(0,-c,Rows,newc);
  cm.AllocatedMatrix=0;
}
void ShortMatrix::deleteColumns(int startcolumn, int numberofcolumns )
{
  if (numberofcolumns==0)return;
  if (numberofcolumns<0)return insertColumns(startcolumn,-numberofcolumns);
#if CURESHORTMATRIXDEBUG >20
  if ((startcolumn<0)||
       ((startcolumn+numberofcolumns)>Columns))
    {
      std::cerr<<"ERRROR ShortMatrix::deleteColumns out of bounds\n";
      return;
    }
  #endif
  int newc=Columns-numberofcolumns;
  if (startcolumn==newc){
      Columns-=numberofcolumns;
    return;
  }
  offset(0,startcolumn+numberofcolumns,Rows,newc-startcolumn);
  ShortMatrix cm;
  copy(cm);
  offset(0,-numberofcolumns,Rows,Columns);
  move(cm);
  offset(0,-startcolumn,Rows,newc);
  cm.AllocatedMatrix=0;
}


void ShortMatrix::insertColumns(int startcolumn, int numberofcolumns )
{ 
  if (numberofcolumns==0)return;
  if (numberofcolumns<0)return deleteColumns(startcolumn,-numberofcolumns);
#if CURESHORTMATRIXDEBUG >20
  if ((startcolumn<0)||
      ((startcolumn)>Columns))
    {
      std::cerr<<"ERRROR ShortMatrix::insertColumns out of bounds\n";
      return;
    }
#endif
  ShortMatrix bm(*this);
  int c=Columns+numberofcolumns;
  reallocate(Rows,c);
  offset(0,0,Rows,startcolumn);
  bm.offset(0,0,Rows,Columns);
  (*this)=bm;
  offset(0,startcolumn+numberofcolumns,Rows,c-startcolumn-numberofcolumns);
  bm.offset(0,startcolumn,Rows,Columns);
  (*this)=bm;
  offset(0,-(startcolumn+numberofcolumns),Rows,c);
}

void ShortMatrix::insertRows(int startrow, int numberofrows )
{ 

  if (numberofrows==0)return;
  if (numberofrows<0)return;
  if (startrow>=Rows){
    int r=Rows+numberofrows;
    if (AllocatedMatrix)
      {
	long n=r*RowInc; 
	if ((AllocatedMatrix==Element)&&(AllocatedSize>=n))
	  {
	    reset(r,Columns);
	    return;
	  }
      }
    grow(r,Columns);
    return;
  }
#if CURESHORTMATRIXDEBUG >20
  if ((startrow<0)||
      ((startrow)>Rows))
    {
      std::cerr<<"ERRROR ShortMatrix::insertRows out of bounds"<<startrow<<" "<<Rows<<"\n";
      return;
    }
#endif
  int r=Rows+numberofrows;
  if (AllocatedMatrix)
    {
      long n=r*RowInc; 
     if ((AllocatedMatrix==Element)&&(AllocatedSize>=n))
	{
	  ShortMatrix bm;	  
	  copy(bm);	 
	  offset(startrow+numberofrows,0,r-startrow-numberofrows,Columns);
	  bm.offset(startrow,0,Rows,Columns);	 
	  move(bm);
	  reset(r,Columns);  
	  bm.AllocatedMatrix=0;
	  return;
	}
    }
  ShortMatrix bm(*this);
  reallocate(r,Columns);
  offset(0,0,startrow,Columns);
  bm.offset(0,0,Rows,Columns);
  (*this)=bm;
  offset(startrow+numberofrows,0,r-startrow-numberofrows,Columns);
  bm.offset(startrow,0,Rows,Columns);
  (*this)=bm;
  offset(-(startrow+numberofrows),0,r,Columns);
}
void ShortMatrix::swapRows(int r1,int r2)
{
  int r=Rows;
  offset(r1,0,1,Columns);
  ShortMatrix bm(*this);
  offset(r2-r1,0,1,Columns);
  ShortMatrix cm;
  copy(cm);
  offset(r1-r2,0,1,Columns);
  (*this)=cm;
  offset(r2-r1,0,1,Columns);
  (*this)=bm;
  offset(-r2,0,r,Columns);
  cm.AllocatedMatrix=0;
}
void ShortMatrix::swapColumns(int c1,int c2)
{
  int c=Columns;
  offset(0,c1,Rows,1);
  ShortMatrix bm(*this);
  offset(0,c2-c1,Rows,1);
  ShortMatrix cm;
  copy(cm);
  offset(0,c1-c2,Rows,1);
  (*this)=cm;
  offset(0,c2-c1,Rows,1);
  (*this)=bm;
  offset(0,-c2,Rows,c);
  cm.AllocatedMatrix=0;
}

void ShortMatrix::threshold(short pthreshold)
{
  long top=RowInc*Rows;
  for (long i=0; i<top;i+=RowInc)
    for (int j=0;j<Columns; j++)
      if ((Element[i+j]<pthreshold)&&(Element[i+j]>-pthreshold))
	  Element[i+j]=0;
}
void ShortMatrix::append(short d, int col)
{
  if (col==0)
    {
      bool test=true;
      if (AllocatedMatrix)
	{
	  long an=AllocatedMatrix+AllocatedSize-Element;
	  long n=((Rows+1)*(Columns));
	  if (an>=n)
	    test=false;
	}
      if (test)grow(Rows+1,Columns);
      else Rows++;
    }
  if (col>=Columns)grow(Rows,Columns);
  (*this)(Rows-1,col)=d;
}
int ShortMatrix::fitToArc( double &radius, double center[2],
			    double startangle,
			   double maxdeviation)
{
  int r=Rows-1;
  if (r<0)return 0;
  double n[2],x0[2],dx[2];
  n[0]=-sin(startangle);
  n[1]=cos(startangle);
  x0[0]=(*this)(0,0);
  x0[1]=(*this)(0,1);
  while (r>-1){
    if (r==0){
      center[0]=x0[0];
      center[1]=x0[1];
      radius=0;
      return 1;
    }
    dx[0]=x0[0]-(*this)(r,0);
    dx[1]=x0[1]-(*this)(r,1);
    double d=dx[0]*dx[0]+dx[1]*dx[1];
    double g=2*(n[0]*dx[0]+n[1]*dx[1]);
    bool test=true;
    if ((g<1E-3)&&(g>-1E-3))
      {
	radius=1E50;
	center[0]=x0[0];
	center[1]=x0[1];
	for (int i=0; i<r-1; i++)
	  {
	    dx[0]=(*this)(i+1,0)-center[0];
	    dx[1]=(*this)(i+1,1)-center[1];
	    d=dx[0]*n[0]+dx[1]*n[1];
	    g=dx[0]*n[1]-dx[1]*n[0];
	    if ((g<0)||((d>maxdeviation)||(d<-maxdeviation)))
	      {
		test=false;
		r--;
		i=r;
	      }
	 
	  }
	if (test)return -(r+1);
      }else{
	radius=d/g;
	center[0]=x0[0]-radius*n[0];
	center[1]=x0[1]-radius*n[1];
	for (int i=0; i<r-1; i++)
	  {
	    dx[0]=(*this)(i+1,0)-center[0];
	    dx[1]=(*this)(i+1,1)-center[1];
	    d=dx[0]*dx[0]+dx[1]*dx[1];
	    d=sqrt(d)-radius;
	    dx[0]=(*this)(i+1,0)-x0[0];
	    dx[1]=(*this)(i+1,1)-x0[1];
	    g=dx[0]*n[1]-dx[1]*n[0];
	    if ((g<0)||((d>maxdeviation)||(d<-maxdeviation)))
	      {
		test=false;
		r--;
		i=r;
	      }
	  }
	if (test)return (r+1);
      }
    
  }
  return 0;
}


int ShortMatrix::moveRows(unsigned long r1,unsigned long r2, unsigned short num)
{
  int r=Rows;
  if (r1==r2)return 0;
  if (r1>r2){
    if ((long)(r1+num)>Rows)return 1;
    offset(r1,0,num,Columns);
    ShortMatrix bm(*this);
    offset(r2-r1,0,r1-r2,Columns);
    for (int i=Rows-1;i>-1;i--)
      memmove(Element+RowInc*(num+i),Element+RowInc*i,
	      Columns*sizeof(short));
    offset(0,0,num,Columns);
    (*this)=bm;
    offset(-r2,0,r,Columns);
    return 0;
  }
  if ((long)(r2+num)>Rows)return 1;
  offset(r1,0,num,Columns);
  ShortMatrix bm(*this);
  offset(num,0,r2-r1,Columns);
  for (int i=0;i<Rows;i++)
    memmove(Element-RowInc*(num-i),Element+RowInc*i,Columns*sizeof(short));
  offset(r2-r1-num,0,num,Columns);
  (*this)=bm;
  offset(-r2,0,r,Columns);
  return 0;
}


int ShortMatrix::moveColumns(unsigned long c1,unsigned long c2, unsigned short num)
{
  int c=Columns;
  if (c1==c2)return 0;
  if (c1>c2){
    if ((long)(c1+num)>Columns)return 1;
    offset(0,c1,Rows,num);
    ShortMatrix bm(*this);
    offset(0,c2-c1,Rows,c1-c2);
    ShortMatrix cm(*this);
    offset(0,0,Rows,num);
    (*this)=bm;
    offset(0,num,Rows,c1-c2);
    (*this)=cm;
    offset(0,-c2-num,Rows,c);
    return 0;
  }
  if ((long)(c2+num)>Columns)return 1;
  offset(0,c1,Rows,num);
  ShortMatrix bm(*this);
  offset(0,num,Rows,c2-c1);
  ShortMatrix cm(*this);
  offset(0,c2-c1-num,Rows,num);
  (*this)=bm;
  offset(0,c1-c2,Rows,c2-c1);
  (*this)=cm;
  offset(0,-c1,Rows,c);
  return 0;
}



} // namespace Cure
