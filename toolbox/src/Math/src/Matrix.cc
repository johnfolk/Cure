// = RCSID
//    $Id: Matrix.cc,v 1.46 2010/05/25 08:51:35 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
// = COPYRIGHT
//    Copyright (c) 2003 John Folkesson
//

#include "Matrix.hh"
#include "MatrixStuff.hh"
#include "LongArray.hh"

#ifndef DEPEND
//#include <fstream>
#include <math.h>
#include <string.h>
#endif
#ifndef CUREMATRIXDEBUG
#define CUREMATRIXDEBUG 140
#endif

using namespace std;
using namespace Cure::MatrixStuff;

namespace Cure {

Matrix::Matrix()
{
  AllocatedMatrix=0;
  AllocatedSize=0;
  Rows=0;
  Columns=0;
  RowInc=0;
  Element=0;
}
Matrix::Matrix(const Matrix& mat)
{
  AllocatedMatrix=0;
  AllocatedSize=0;
  Rows=0;
  Columns=0;
  Element=0;
  RowInc=0;
  *this=mat;
  return;
}
Matrix::Matrix(const int r, const int c)
{
  AllocatedMatrix=0;
  AllocatedSize=0;
  Rows=0;
  Columns=0;
  RowInc=0;
  Element=0;
  reallocate(r,c);
}

Matrix::Matrix(double *mat, const int r, const int c,
	       const int row_offset)
{
  AllocatedMatrix=0;
  AllocatedSize=0;
  Element=mat; 
  Rows=r;
  if (c<0)Columns=r;
  else Columns=c;
  if (row_offset<0)RowInc=Columns;
  else RowInc=row_offset;
}

Matrix::Matrix(Matrix &src,
               int rowOffset, int columnOffset, 
               int r, int c)
{
  AllocatedMatrix=0;
  AllocatedSize=0;
  Rows=0;
  Columns=0;
  RowInc=0;
  Element=0;

  if (rowOffset + r > src.Rows ||
      columnOffset + c > src.Columns) {
    std::cerr << "Matrix Cannot copy submatrix outside matrix ("
              << src.Rows << " "
              << src.Columns << " "
              << rowOffset << " "
              << columnOffset << " "
              << r << " "
              << c << std::endl;
    return;
  }

  int oldRows = src.Rows;
  int oldCols = src.Columns;

  src.offset(rowOffset, columnOffset, r, c);
  *this = src;
  src.reset(oldRows, oldCols);
}

Matrix::~Matrix()
{
  if (AllocatedMatrix)delete [] AllocatedMatrix;
  AllocatedMatrix=0;
  AllocatedSize=0;
}
void Matrix::grow(const int r,const int c)
{
  Matrix m;
  copy(m);
  Element=0;
  AllocatedMatrix=0;
  AllocatedSize=0;
  RowInc=0;
  Rows=0;
  Columns=0;
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

void Matrix::setupElement(const int r, const int c)
  {
  Columns=c;
  if (c<0)Columns=r;
  if (r<1)
    {
      Rows=0;
      Element=AllocatedMatrix;
      RowInc=0;
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
	  if (n>0)
	    memset(AllocatedMatrix, 0, sizeof(double) * n);
	  Element=AllocatedMatrix;
	  return;
	}
    }
  if (n>0)
    {
      AllocatedMatrix=new double[n];
      memset(AllocatedMatrix, 0, sizeof(double) * n);
      AllocatedSize=n;
      Element=AllocatedMatrix;
      return;
    }
  Element=0;
}
void Matrix::operator = (const double d) 
{
  long n=Rows*Columns;
  if ((n>0)&&(RowInc==Columns))
    memset(Element, 0, sizeof(double) * n);
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
void Matrix::operator += (const Matrix & mat) 
{
#if CUREMATRIXDEBUG >10
  if( Columns!=mat.Columns || Rows!=mat.Rows ) 
    {
      cerr << "\nMATRIX ERROR Incompatible matrices for +=, not same size\n";
      return;
    }
#endif 
  matPlusEqual(Element,mat.Element,Rows,Columns, RowInc,mat.RowInc);
}

void Matrix::operator -= (const Matrix & mat) 
{
#if CUREMATRIXDEBUG >10
  if( Columns!=mat.Columns || Rows!=mat.Rows ) 
    {
      cerr << "\nMATRIX ERROR Incompatible matrices for -=, not same size\n";
      return;
    }
#endif 
  matMinusEqual(Element,mat.Element,Rows,Columns, 
		  RowInc,mat.RowInc);
}

void Matrix::operator *=(const Matrix &mat)
{
#if CUREMATRIXDEBUG >10
  if ((Columns!=mat.Columns)||(Columns!=mat.Rows))
    {
      cerr << "\nMATRIX ERROR Incompatible matrices for *=, result not same size\n";
      return;
    }
#endif 
  double res[Rows*Columns];
  matEqual(res,Element,Rows,Columns, Columns,RowInc);
  matMult(Element,res,mat.Element, Rows,Columns,
	   mat.Columns,RowInc,Columns,mat.RowInc);
}
void Matrix::addProduct_(const Matrix& a, const Matrix& b)
{
#if CUREMATRIXDEBUG >10
  if (((Rows!=a.Rows)||(Columns!=b.Columns))||
      (b.Rows!=a.Columns))
    {
      cerr << "\nMATRIX ERROR Incompatible matrices for addProduct\n";
      return;
    }
#endif
  matAddProd(Element,a.Element,b.Element, 
	       Rows,a.Columns,Columns,
	       RowInc,a.RowInc,b.RowInc);
}
void Matrix::minusMult_(const Matrix& a, const Matrix& b)
{
#if CUREMATRIXDEBUG >10
  if (b.Rows!=a.Columns)
    {
      cerr << "\nMATRIX ERROR Incompatible matrices for minusMult\n";
      return;
    }
#endif
  reallocate(a.Rows,b.Columns);
  matMinusMult(Element,a.Element,b.Element, 
		 Rows,a.Columns,Columns,
		 RowInc,a.RowInc,b.RowInc);
}
void Matrix::subtractProduct_(const Matrix& a, const Matrix& b)
{
#if CUREMATRIXDEBUG >10
  if (((Rows!=a.Rows)||(Columns!=b.Columns))||(b.Rows!=a.Columns))
    {
      cerr << "\nMATRIX ERROR Incompatible matrices for subtractProduct\n";
      return;
    }
#endif
  matSubProd(Element,a.Element,b.Element, 
	       Rows,a.Columns,Columns,
	       RowInc,a.RowInc,b.RowInc);
}

void Matrix::arrayMult(const Matrix& b)
{
#if CUREMATRIXDEBUG >20
  if (((Rows!=b.Rows)||(Columns!=b.Columns)))
    {
      cerr << "\nMATRIX ERROR Incompatible matrices for arrayMult\n";
      return;
    }
#endif
  long irow=0;
  for (int i=0; i<Rows; i++,irow+=RowInc)
    for (int j=0; j<Columns; j++)
      Element[irow+j]*=b(i,j);
}
void Matrix::arrayDivide(const Matrix& b)
{
#if CUREMATRIXDEBUG >20
  if (((Rows!=b.Rows)||(Columns!=b.Columns)))
    {
      cerr << "\nMATRIX ERROR Incompatible matrices for arrayMult\n";
      return;
    }
#endif
  long irow=0;
  for (int i=0; i<Rows; i++,irow+=RowInc)
    for (int j=0; j<Columns; j++)
      Element[irow+j]/=b(i,j);
}
void Matrix::multTranspose_(const Matrix& a, const Matrix& b,const int which)
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
 #if CUREMATRIXDEBUG >10
  if (which>1)
    {
      if (k!=b.Columns)
	{
	  cerr<<"\nERROR Matrix::multTranspose_() INCOMPATABLE b.columns\n";
	  return;
	}
      inc_2=offset_2;
      offset_2=1;;
  
   }
  else if (k!=b.Rows)
    {
      cerr<<"\nERROR Matrix::multTranspose_() INCOMPATABLE b.Rows\n";
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
void  Matrix::operator *= (const double d)
{
  long top=Rows*RowInc;
  for (long i=0; i<top; i+=RowInc)
    for (long j=0;j<Columns;j++)
      Element[i+j]*=d;
}
void  Matrix::operator += (const double d)
{
  long top=Rows;
  if (top>Columns)top=Columns;
  top*=RowInc;
  for (long i=0; i<top; i+=(RowInc+1))
      Element[i]+=d;
}
void  Matrix::operator /= (const double d)
{
  long top=Rows*RowInc;
  if (d==0) {
#if CUREMATRIXDEBUG >10
    cerr << "Tried to divide Matrix::operator/= 0\n";
#endif
    return; 
  }
  for (long i=0; i<top; i+=RowInc)
    for (long j=0;j<Columns;j++)
      Element[i+j]/=d;
}
double Matrix::determinant( void ) const 
{      
  long i,j;
  double det = 0.;
#if CUREMATRIXDEBUG >10
  if (Rows != Columns)
    {
      cerr << "ERROR Matrix::determinant: non-square matrix\n";
      return 0.0;
    }   
#endif 
  if (Rows == 1)
    return Element[0];              // trivial
  if (Rows == 2)                   // do
    {
      det  =  Element[0] * Element[RowInc+1];
      det -=  Element[1] * Element[RowInc];
      return det;
    }
  if (Rows == 3)
    {
      det  = Element[0] * Element[RowInc+1] * Element[2*RowInc+2];
      det += Element[1] * Element[RowInc+2] * Element[2*RowInc];
      det += Element[2] * Element[RowInc] * Element[2*RowInc+1];
      det -= Element[0] * Element[RowInc+2] * Element[2*RowInc+1];
      det -= Element[1] * Element[RowInc] * Element[2*RowInc+2];
      det -= Element[2] * Element[RowInc+1] * Element[2*RowInc];
      return det;
    }
  // Summation by recursive computation of matrices
  if (Rows <7){
    //  double a[(Rows-1)*(Rows-1)],b[4];
    Matrix tmp1(Rows-1),tmp2(2);
    tmp2.Element[0] = Element[0];
    for( i = 1; i < Rows; i++)
      for( j = 1; j < Columns; j++)
	{
	  tmp2.Element[1] = Element[j];
	  tmp2.Element[2] = Element[i*RowInc];
	  tmp2.Element[3] = Element[i*RowInc+j];
	  tmp1.Element[(i-1)*(Rows-1)+j-1] =  tmp2.determinant();
	}
    
    det = 1.0;
    for( i = 0; i < Rows-2; i++)
      det /= Element[0];
    det *= tmp1.determinant();
    return det;
  }
  Matrix lam, u;
  eigenFactor(lam,u);
  det=1;
  for (int i=0; i<Rows; i++)
    det*=lam(i,i);
  return det;
}

Matrix Matrix::getSubmatrix(int rowOffset, int columnOffset, 
                            int r, int c)
{
  if (rowOffset + r > Rows ||
      columnOffset + c > Columns) {
    std::cerr << "Matrix Cannot getSubmatrix outside matrix ("
              << Rows << " "
              << Columns << " "
              << rowOffset << " "
              << columnOffset << " "
              << r << " "
              << c << std::endl;
    return Matrix();
  }

  int oldRows = Rows;
  int oldCols = Columns;

  offset(rowOffset, columnOffset, r, c);
  Matrix tmp;
  tmp = *this;
  reset(oldRows, oldCols);
  return tmp;
}

int Matrix::setToSubmatrix(Matrix &src,
                         int rowOffset, int columnOffset, 
                           int r, int c)
{
  if (rowOffset + r > src.Rows ||
      columnOffset + c > src.Columns) {
    std::cerr << "Matrix Cannot setToSubmatrix outside matrix ("
              << src.Rows << " "
              << src.Columns << " "
              << rowOffset << " "
              << columnOffset << " "
              << r << " "
              << c << std::endl;
    return 1;
  }

  int oldRows = src.Rows;
  int oldCols = src.Columns;

  src.offset(rowOffset, columnOffset, r, c);
  AllocatedMatrix=0;
  AllocatedSize=0;
  Rows=0;
  Columns=0;
  RowInc=0;
  Element=0;
  *this = src;
  src.reset(oldRows, oldCols);
  return 0;
}

void Matrix::transpose(const Matrix b)
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
void Matrix::transpose_(const Matrix & b )
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
void Matrix::transpose( void )
{
  Matrix a(*this);
  transpose_(a);
} 
int Matrix::invert() 
{
  long irow=0;
  long r=Rows*Columns;
  if (!r)return 0;
  double resm[r];
  int indx[Rows];
  double  b[Rows];
  if (luDecomposition(indx))return 1;
  for(long j = 0 ; j < Rows ; j++ ) 
    {
      for (long i=0;i<Rows;i++)b[i] = 0;
      b[j] = 1;
      luBackSubst( indx, b );
      irow=0;
      for( long i = 0 ; i < Rows ; i++,irow+=Columns )
	resm[irow+j] = b[i];
    }
  irow=0;
  for(long i = 0 ; i < r ; i+=Columns,irow+=RowInc)
    for( long j = 0 ; j < Columns ; j++ )
      Element[irow+j] = resm[i+j];
  return 0;
}

// Routines used by the LU decomposition
#define TINY 1.0e-20;

int Matrix::luDecomposition(int* v)
{ 
  double vv[Rows];
  long i,imax=0,j,k;
  long n = Rows;
  double big,dum,sum,temp, d;
  for( i = 0 ; i < Rows ; i++ )
    v[i] = 0;
#if CUREMATRIXDEBUG >10
  if( Columns != Rows ) 
    {
      cerr << "ERROR Matrix::luDecomposition Cannot LU decompose a non-square matrix" << endl;
      return 2;
    }
#endif
  d=1.0;
  for (i=0;i<n;i++) 
    {   // is it singular?
      big=0.0; 
      for (j=0;j<n;j++)
	{
	  if (Element[i*RowInc+j]>0)
	    temp=Element[i*RowInc+j];
	  else
	    temp=-Element[i*RowInc+j];
	  if (temp > big) 
	      big=temp;
	}
      if (big == 0.0)  
	{
#if CUREMATRIXDEBUG >150
	  cerr << "Matrix Singular matrix in routine LUDCMP\n";
#endif
	  return 1;
	}
      vv[i]=1.0/big;
    }
  for (j=0;j<n;j++) 
    {
      for (i=0;i<j;i++) 
	{
	  sum=Element[RowInc*i+j];
	  for (k=0;k<i;k++) 
	    sum -= Element[RowInc*i+k] * Element[RowInc*k+j];
	  Element[i*RowInc+j]=sum;
	}
      big=0.0;
      for (i=j;i<n;i++) 
	{
	  sum=Element[RowInc*i+j];
	  for (k=0;k<j;k++)
	    sum -=Element[RowInc*i+k]*Element[RowInc*k+j];
	  Element[RowInc*i+j]=sum;
	  if (sum>0)temp=sum;
	  else temp=-sum;
	  if ( (dum=vv[i] * temp) >= big) 
	    {
	      big=dum;
	      imax=i;
	    }
	}
      if (j != imax) 
	{
	  for (k=0;k<n;k++) 
	    {
	      dum=Element[RowInc*imax+k];
	      Element[RowInc*imax+k]=Element[RowInc*j+k];
	      Element[RowInc*j+k]=dum;
	    }
	  d = -d;
	  vv[imax]=vv[j];
	}
      v[j]=imax;
      if (Element[RowInc*j+j] == 0.0) 
	Element[j*RowInc+j]=TINY;
      if (j != (n-1)) 
	{
	  dum=1.0/(Element[RowInc*j+j]);
	  for (i=j+1;i<n;i++) 
	    Element[RowInc*i+j] *= dum;
      }
    }
  return 0;
}

#undef TINY
void Matrix::luBackSubst(int* indx, double *b)
{ 
  long i,ii=-1,ip,j;
  double sum;
  for (i=0;i<Rows;i++) 
    {
      ip=indx[i];
      sum=b[ip];
      b[ip]=b[i];
    if (ii != -1)
      for (j=ii;j<i;j++) 
        sum -= Element[RowInc*i+j]*b[j];
    else if (sum)
        ii=i;
    b[i]=sum;
  }
  for (i=Rows-1;i>=0;i--) 
    {
      sum=b[i];
      for (j=i+1;j<Rows;j++) 
	sum -=Element[RowInc*i+j]*b[j];
      b[i]=sum/Element[i*(RowInc+1)];
    }
}
void  Matrix::save(const char *filename)
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
void Matrix::symetrize()
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
void Matrix::addProduct_(const Matrix& b,const double d)
{
  long top=RowInc*Rows;
  long irow2=0;
  for (long irow=0; irow<top;irow+=RowInc, irow2+=b.RowInc)
    for (long j=0; j<Columns; j++)
      Element[irow+j]+=(d*b.Element[irow2+j]);
}
void Matrix::subtractProduct_(const Matrix& b,const double d)
{
  long top=RowInc*Rows;
  long irow2=0;
  for (long irow=0; irow<top;irow+=RowInc, irow2+=b.RowInc)
    for (long j=0; j<Columns; j++)
      Element[irow+j]-=(d*b.Element[irow2+j]);
}
void Matrix::minus(const Matrix & a)
{
  unsigned long irow=0; 
  unsigned long airow=0; 
  reallocate(a.Rows,a.Columns);
  for (int i=0;i<a.Rows;i++,irow+=RowInc, airow+=a.RowInc)
    for (int j=0;j<a.Columns;j++)
      Element[irow+j]=-a.Element[airow+j];
}
void Matrix::minus()
{
  long top=RowInc*Rows;
  for (long irow=0; irow<top;irow+=RowInc)
    for (long j=0; j<Columns; j++)
      Element[irow+j]=-Element[irow+j];
}
void Matrix::print()
{
  cerr<<" Rows/cols/RowInc: "<<Rows<<" "<<Columns<<" "<<RowInc<<endl;
  long irow=0;
  for (long i=0; i<Rows; i++,irow+=RowInc)
    {
      for (long j=0; j<Columns; j++)
	cerr<<Element[irow+j]<<" ";
      cerr<<endl;
    }
}

void Matrix::makeOrthonormal()
{
  Matrix j_norm(Rows,Rows);
  double *norm=j_norm.Element;
  j_norm.multTranspose_(*this,*this,2);
  for (long i=0; i<Rows; i++)
    {
      double d=sqrt(norm[i*(Rows+1)]);
      long top=i*RowInc+Columns;
      if (d!=1)
	for (long j=(top-Columns); j<top; j++)
	  Element[j]/=d;
    }
  j_norm.multTranspose_(*this,*this,2);
  long krow=0;
  for (long k=0; k<Rows-1;k++,krow+=RowInc)
    {
      long irow=krow+RowInc;
      for (long i=k+1; i<Rows; i++,irow+=RowInc)
	{
	  double d=norm[k*Rows+i];
	  if (d!=0)
	    {
	      double d2=sqrt(1-d*d);
	      long kind=krow;
	      long top=irow+Columns;
	      for (long j=irow; j<top; j++,kind++)
		{
		  Element[j]-=d*Element[kind];
		  Element[j]/=(d2);
		}
	    }
	}
      j_norm.multTranspose_(*this,*this,2);
    }
}


int Matrix::svDecomposition(double *w, double *v) 
{
  int   flag,i,its,j,jj,k,l=0,nm=0;
  double c,f,h,s,x,y,z,temp;
  double anorm=0.0,g=0.0,scale=0.0;
  double rv1[Columns];
  
  if (Rows < Columns)
    {
      cerr << "FATAL ERROR: SVDCMP: You must augment matrix with extra zero Rows" << endl;
      cerr << "             or transpose the matrix before SVD decomposition" << endl;
      return 1;
    }
  // Householders reduction to bi-diagonal form
  for (i=0;i<Columns;i++) 
    {
      l= i+1;
      rv1[i]= scale * g;
      g = s = scale = 0.0;
      if (i < Rows) 
	{
	  for (k=i;k<Rows;k++)
	    { 
	      temp=Element[k*RowInc+i];
	      if (temp>0)scale += temp;
	      else scale -= temp;
	    }	
	  if (scale) 
	    {
	      for (k=i;k<Rows;k++) 
		{
		  Element[k*RowInc+i] /= scale;
		  s += Element[k*RowInc+i]*Element[k*RowInc+i];
		}
	      f=Element[i*(1+RowInc)];
	      if (f>0)g=-sqrt(s);
	      else g=sqrt(s);
	      h=f*g-s;
	      Element[i*(1+RowInc)]=f-g;
	      if (i != (Columns-1)) 
		{
		  for (j=l;j<Columns;j++) 
		    {
		      for (s=0.0,k=i;k<Rows;k++) 
			s += Element[k*RowInc+i]*Element[k*RowInc+j];
		      f=s/h;
		      for (k=i;k<Rows;k++) 
			Element[k*RowInc+j] += f*Element[k*RowInc+i];
		    }
		}
	      for (k=i;k<Rows;k++) 
		Element[k*RowInc+i] *= scale;
	    }
	}
      w[i]=scale*g;
      g=s=scale=0.0;
      if (i < Rows && i != (Columns-1)) 
	{
	  for (k=l;k<Columns;k++) 
	    {
	      temp=Element[i*RowInc+k];
	      if (temp>0)scale+=temp;
	      else scale-=temp;
	    }
	  if (scale) 
	    {
	      for (k=l;k<Columns;k++) 
		{
		  Element[i*RowInc+k] /= scale;
		  s += Element[i*RowInc+k]*Element[i*RowInc+k];
		}
	      f=Element[i*RowInc+l];
	      if (f>0)g=-sqrt(s);
	      else g=sqrt(s);
	    h=f*g-s;
	    Element[i*RowInc+l]=f-g;
	    for (k=l;k<Columns;k++) 
	      rv1[k]=Element[i*RowInc+k]/h;
	    if (i != (Rows-1)) 
	      {
		for (j=l;j<Rows;j++) 
		  {
		    for (s=0.0,k=l;k<Columns;k++) 
		      s += Element[j*RowInc+k]*Element[i*RowInc+k];
		    for (k=l;k<Columns;k++) 
		      Element[j*RowInc+k] += s*rv1[k];
		  }
	      }
	    for (k=l;k<Columns;k++) 
	      Element[i*RowInc+k] *= scale;
	    }
	}
      if (w[i]>0)temp=w[i];
      else temp=-w[i];
      if (rv1[i]>0)temp+=rv1[i];
      else temp-=rv1[i];
      if (temp>anorm)anorm=temp;
    }
  for (i=Columns-1;i>=0;i--) 
    { // Accumulation of right hand transformation
      if (i < (Columns-1)) 
	{
	  if (g) 
	    {
	      for (j=l;j<Columns;j++)
		v[j*Columns+i]=(Element[i*RowInc+j]/Element[i*RowInc+l])/g;
	      for (j=l;j<Columns;j++) {
		for (s=0.0,k=l;k<Columns;k++) 
		  s += Element[i*RowInc+k]*v[k*Columns+j];
		for (k=l;k<Columns;k++) 
		  v[k*Columns+j] += s*v[k*Columns+i];
	      }
	    }
	  for (j=l;j<Columns;j++)  
	    {
	      v[i*Columns+j]=0.0;
	      v[j*Columns+i]=0.0;
	    }
	}
      v[(i+1)*Columns]=1.0;
      g=rv1[i];
      l=i;
    }

  for (i=Columns-1;i>=0;i--) 
    { // Accumulation of left hand transformation
      l=i+1;
      g=w[i];
    if (i < (Columns-1))
      for (j=l;j<Columns;j++) 
	Element[i*RowInc+j]=0.0;
    if (g) 
      {
	g=1.0/g;
	if (i != (Columns-1)) 
	  {
	    for (j=l;j<Columns;j++) 
	      {
		for (s=0.0,k=l;k<Rows;k++) 
		  s += Element[k*RowInc+i]*Element[k*RowInc+j];
		f=(s/Element[i*(1+RowInc)])*g;
		for (k=i;k<Rows;k++) 
		  Element[k*RowInc+j] += f*Element[k*RowInc+i];
	      }
	  }
	for (j=i;j<Rows;j++) 
	  Element[j*RowInc+i] *= g;
      } 
    else 
      {
	for (j=i;j<Rows;j++) 
	  Element[j*RowInc+i]=0.0;
      }
    Element[i*(1+RowInc)] += 1.;
    }
  // diagonalisation of the bidiagonal form
  for (k=Columns-1;k>=0;k--) {         // loop over singular values
    for (its=1;its<=300;its++) 
      {
	flag=1;
	for (l=k;l>=0;l--) 
	  {   // test for splitting
	    nm=l-1;
	    if (rv1[l]>0)temp=rv1[l];
	    else temp=-rv1[l];
	    if (temp+anorm == anorm) {
	      flag=0;
	      break;
	    }
	    if (w[nm]>0)temp=w[nm];
	    else temp=-w[nm];
	    if ((temp+anorm) == anorm) 
	      break;
	  }
	  if (flag) 
	    {
	      c=0.0;
	      s=1.0;
	      for (i=l;i<=k;i++) 
		{
		  f=s*rv1[i];
		  if (f>0)temp=f;
		  else temp=-f;
		  if (temp+anorm != anorm) 
		    {
		      g=w[i];
		      h=sqrt(f*f+g*g);
		      w[i]=h;
		      h=1.0/h;
		      c=g*h;
		      s=(-f*h);
		      for (j=0;j<Rows;j++) 
			{
			  y=Element[j*RowInc+nm];
			  z=Element[j*RowInc+i];
			  Element[j*RowInc+nm]=y*c+z*s;
			  Element[j*RowInc+i]=z*c-y*s;
			}
		    }
		}
	    }
	  z=w[k];
	  if (l == k) 
	    {                // convergence ?
	      if (z < 0.0) 
		{           // singular value should positive
		  w[k] = -z;
		  for (j=0;j<Columns;j++) 
		    v[j*Columns+k]=-v[j*Columns+k];
		}
	      break;
	    }
	  if (its == 300)
	    {
	      cerr << "FATAL ERROR: ";
	      cerr << "No convergence in 300 SVDCMP iterations";
	      return 1;
	    }
	  x=w[l];
	  nm=k-1;
	  y=w[nm];
	  g=rv1[nm];
	  h=rv1[k];
	  f=((y-z)*(y+z)+(g-h)*(g+h))/(2.0*h*y);
	  g=sqrt(f*f+1);
	  if (g>0)temp=g;
	  else temp=-g;
	  if (f<0)temp=-temp;
	  f=((x-z)*(x+z)+h*((y/(f+temp))-h))/x;
	  c=s=1.0;
	  for (j=l;j<=nm;j++) 
	    {
	      i=j+1;
	      g=rv1[i];
	      y=w[i];
	      h=s*g;
	      g=c*g;
	      z=sqrt(f*f+h*h);
	      rv1[j]=z;
	      c=f/z;
	      s=h/z;
	      f=x*c+g*s;
	      g=g*c-x*s;
	      h=y*s;
	      y=y*c;
	      for (jj=0;jj<Columns;jj++) 
		{
		  x=v[jj*Columns+j];
		  z=v[jj*Columns+i];
		  v[jj*Columns+j]=x*c+z*s;
		  v[jj*Columns+i]=z*c-x*s;
		}
	      z=sqrt(f*f+h*h);
	      w[j]=z;
	      if (z) 
		{
		  z=1.0/z;
		  c=f*z;
		  s=h*z;
		}
	      f=(c*g)+(s*y);
	      x=(c*y)-(s*g);
	      for (jj=0;jj<Rows;jj++) 
		{
		  y=Element[jj*RowInc+j];
		  z=Element[jj*RowInc+i];
		  Element[jj*RowInc+j]=y*c+z*s;
		  Element[jj*RowInc+i]=z*c-y*s;
		}
	    }
	  rv1[l]=0.0;
	  rv1[k]=f;
	  w[k]=x;
	}
    }
  return 0;
}
int Matrix::rank() const 
{      
  // Find eigenvalues and test for small eigenvalues
  Matrix u(Rows,Columns);
  double v[Rows*Columns],w[Columns];
  u = *this;
  u.svDecomposition(w,v);
  double l1 = 0.00000001 * w[0];
  int    r = 1;
  for( int i = 1 ; i < Columns ; i++ )
    if( w[i] > l1 )
      r++;
  if( w[0] < 0.0001 )
    return( --r );
  else
    return( r );
}
int Matrix::singularValues(double *w) const
{
  Matrix u(Rows,Columns);
  double v[Rows*Columns];
  u = *this;
  return u.svDecomposition(w,v);
}
double Matrix::trace() const
{
  double d=0;
  long top=Rows;
  if (top>Columns)top=Columns;
  for (long i=0; i<top;i++)
    d+=Element[i*(RowInc+1)];
  return d;
}

Matrix Matrix::operator + (const Matrix& mat) const
{
  Matrix res(Rows,Columns);  
#if CUREMATRIXDEBUG >20
  if( Columns!=mat.Columns || Rows!=mat.Rows ) 
    {
      cerr << "\nERROR Incompatible matrices for +\n";
      return( res );
    }
#endif 
  matAdd(res.Element,Element,mat.Element,
	  Rows,Columns,Columns,RowInc,mat.RowInc);
  return( res );
}
Matrix Matrix::operator - (const Matrix &mat)const 
{

  Matrix res(Rows,Columns);  
#if CUREMATRIXDEBUG >20
  if( Columns!=mat.Columns || Rows!=mat.Rows ) 
    {
      cerr << "\nERROR Incompatible matrices for - \n";
      return( res );
    }
#endif 
  matSubtract(res.Element,Element,mat.Element,
	       Rows,Columns,Columns,RowInc,mat.RowInc);  
  return(res );
}
Matrix Matrix::operator * (const Matrix &mat)const
{
  Matrix res(Rows,mat.Columns);  
#if CUREMATRIXDEBUG >20
  if (Columns!=mat.Rows)
    {
      cerr<<"\nERROR Matrix:: + INCOMPATABLE\n";
      return res;
    }
#endif 
  matMult(res.Element,Element,mat.Element, 
	   Rows,Columns,mat.Columns,
	   mat.Columns,RowInc,mat.RowInc); 
  return (res);
}
void Matrix::deleteRow(int r)
{
  if ((r<0)||(r>=Rows))
    {
#if CUREMATRIXDEBUG >20
      std::cerr<<"ERRROR Matrix::deleteRow out of bounds\n";
#endif
      return;
    }
  if (r==(Rows-1)){
    Rows--;
    return;
  }
  long irow=(r+1)*RowInc;
  if (Columns>0)
    for(long i=r+1;i<Rows;i++,irow+=RowInc)
      memcpy(Element+irow-RowInc,Element+irow,
	     Columns*sizeof(double));
  Rows--;
}
void Matrix::deleteColumn(int c)
{
#if CUREMATRIXDEBUG >20
  if ((c<0)||(c>=Columns))
    {
      std::cerr<<"ERRROR Matrix::deleteColumn out of bounds\n";
      return;
    }
#endif
  if (c==(Columns-1)){
    Columns--;
    return;
  }
  long irow=0;
  if (Columns>(1+c))
    for(long i=0;i<Rows;i++,irow+=RowInc)
      memmove(Element+c+irow,Element+irow+c+1,(Columns-1-c)*sizeof(double));
  Columns--;
}
void Matrix::deleteColumns(int startcolumn, int numberofcolumns )
{
  if (numberofcolumns==0)return;
  if (numberofcolumns<0)return insertColumns(startcolumn,-numberofcolumns);
#if CUREMATRIXDEBUG >20
  if ((startcolumn<0)||
       ((startcolumn+numberofcolumns)>Columns))
    {
      std::cerr<<"ERRROR Matrix::deleteColumns out of bounds\n";
      return;
    }
  #endif
  long newc=Columns-numberofcolumns;
  if (startcolumn==newc){
      Columns-=numberofcolumns;
    return;
  }
  long irow=0;
  if ((Columns-numberofcolumns-startcolumn)>0)
    for(long i=0;i<Rows;i++,irow+=RowInc)
      memmove(Element+irow+startcolumn,
	      Element+irow+startcolumn+numberofcolumns,
	      (Columns-numberofcolumns-startcolumn)*sizeof(double));
  Columns=newc;
}
void Matrix::insertColumn(int startcolumn){
  insertColumns(startcolumn,1);
}
void Matrix::insertRow(int startRow)
{
  if (startRow>=Rows){
    grow(startRow+1,Columns);
    return;
  } else if (startRow<0) return;
  
  //  grow(Rows+1,Columns);
  //for (int i=Rows-1;i>startRow;i++)
  //swapRows(i,i-1);
  Matrix m;
  copy(m);
  Element=0;
  AllocatedMatrix=0;
  AllocatedSize=0;
  RowInc=0;
  Rows=0;
  Columns=0;
  reallocate(m.Rows+1,m.Columns);
  int r2=Rows;
  offset(0,0,startRow,m.Columns);
  m.offset(0,0,startRow,m.Columns);
  (*this)=m;
  offset(startRow+1,0,r2-startRow-1,m.Columns);
  m.offset(startRow,0,r2-startRow-1,m.Columns);
  (*this)=m;
  offset(-startRow-1,0,r2,m.Columns);
}
 

void Matrix::insertColumns(int startcolumn, int numberofcolumns )
{ 
  if (numberofcolumns==0)return;
  if (numberofcolumns<0)return deleteColumns(startcolumn,-numberofcolumns);
#if CUREMATRIXDEBUG >20
  if ((startcolumn<0)||
      ((startcolumn)>Columns))
    {
      std::cerr<<"ERRROR Matrix::insertColumns out of bounds\n";
      return;
    }
#endif
  Matrix bm(*this);
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


int Matrix::moveRows(unsigned long r1,unsigned long r2, unsigned short num)
{
  int r=Rows;
  if (r1==r2)return 0;
  if (r1>r2){
    if ((long)(r1+num)>Rows)return 1;
    offset(r1,0,num,Columns);
    Matrix bm(*this);
    offset(r2-r1,0,r1-r2,Columns);
    if (Columns>0)
      for (int i=Rows-1;i>-1;i--)
	memmove(Element+RowInc*(num+i),Element+RowInc*i,
		Columns*sizeof(double));
    offset(0,0,num,Columns);
    (*this)=bm;
    offset(-r2,0,r,Columns);
    return 0;
  }
  if ((long)(r2+num)>Rows)return 1;
  offset(r1,0,num,Columns);
  Matrix bm(*this);
  offset(num,0,r2-r1,Columns);
  if (Columns>0)
    for (int i=0;i<Rows;i++)
      memmove(Element-RowInc*(num-i),Element+RowInc*i,Columns*sizeof(double));
  offset(r2-r1-num,0,num,Columns);
  (*this)=bm;
  offset(-r2,0,r,Columns);
  return 0;
}


int Matrix::moveColumns(unsigned long c1,unsigned long c2, unsigned short num)
{
  int c=Columns;
  if (c1==c2)return 0;
  if (c1>c2){
    if ((long)(c1+num)>Columns)return 1;
    offset(0,c1,Rows,num);
    Matrix bm(*this);

    offset(0,c2-c1,Rows,c1-c2);
   if (Columns>0)
     for (long i=0;i<Rows;i++)
       memmove(Element+i*RowInc+num,Element+i*RowInc,Columns*sizeof(double));
    offset(0,0,Rows,num);

    (*this)=bm;
    offset(0,-c2,Rows,c);
    return 0;
  }
  if ((long)(c2+num)>Columns)return 1;
  offset(0,c1,Rows,num);
  Matrix bm(*this);

  offset(0,0,Rows,c2-c1);
  if (Columns>0)
    for (long i=0;i<Rows;i++)
      memmove(Element+i*RowInc,Element+i*RowInc+num,Columns*sizeof(double));
  offset(0,c2-c1,Rows,num);
  
  (*this)=bm;
  offset(0,-c2,Rows,c);
  return 0;
}


void Matrix::swapRows(int r1,int r2)
{
  int r=Rows;
  offset(r1,0,1,Columns);
  Matrix bm(*this);
  offset(r2-r1,0,1,Columns);
  Matrix cm;
  copy(cm);
  offset(r1-r2,0,1,Columns);
  (*this)=cm;
  offset(r2-r1,0,1,Columns);
  (*this)=bm;
  offset(-r2,0,r,Columns);
  cm.AllocatedMatrix=0;
}
void Matrix::swapColumns(int c1,int c2)
{
  int c=Columns;
  offset(0,c1,Rows,1);
  Matrix bm(*this);
  offset(0,c2-c1,Rows,1);
  Matrix cm;
  copy(cm);
  offset(0,c1-c2,Rows,1);
  (*this)=cm;
  offset(0,c2-c1,Rows,1);
  (*this)=bm;
  offset(0,-c2,Rows,c);
  cm.AllocatedMatrix=0;
}
int Matrix::eigenFactor(Matrix &uppertri,Matrix &u) const
{
#if CUREMATRIXDEBUG >20
  if (Rows != Columns)
    {
      cerr << "\nERROR Matrix::eigenFactor: non-square matrix\n";
      return 1;
    }   
#endif 
  uppertri=(*this);
  u.reallocate(Rows);
  if (Rows==0)return 0;  
  if (Rows==1){
    u=1;
    return 0;  
  }
  long irow=0;
  bool test=true;
  long leading=Rows-1;
  uppertri.hessenberg(u);
  Matrix q(Rows);
  long cnt=5*Rows;
  while (test)
    {
      uppertri.Rows=leading+1;
      q=1;
      q.Rows=leading+1;
      q.Columns=leading+1;
      double shift=-uppertri(leading,leading);
      uppertri+=shift;
      uppertri.factorQR(q);
      double tst=-uppertri(leading,leading);
      if ((tst<1E-3)&&(tst>-1E-3))
	{
	  leading--;
	  if (leading<0)
	    leading=0;
	}

      uppertri.Columns=uppertri.Rows;
      uppertri*=q;
      shift=-shift;
      uppertri+=shift; 
      uppertri.Rows=Rows;
      uppertri.Columns=uppertri.Rows;
      q.Rows=Rows;
      q.Columns=Columns;
      u*=q;
      if (leading==0)test=false;
      cnt--;
      if (cnt==0)test=false;
    }
  irow=RowInc;
  for (long i=1; i<Rows; i++,irow+=RowInc+1)
    uppertri.Element[irow]=0; 
  return 0;
}
int Matrix::symmetricEigen(Matrix &lambda,Matrix &ev, int  iterations )
{
#if CUREMATRIXDEBUG >20
  if (Rows != Columns)
    {
      cerr << "\nERRORMatrix::symmetricEigen: non-square matrix\n";
      return 1;
    }   
#endif 
  lambda=(*this);
  ev.reallocate(Rows);
  if (Rows==0)return 0;  
  if (Rows==1){
    ev=1;
    return 0;  
  }
  if (Rows==2){
    double evec[4],lam[2];
    if (RowInc==2){
      if (eigen2X2(Element,evec,lam)==1)return 1;
    } else {
      double temp[4];
      temp[0]=Element[0];
      temp[1]=Element[1];
      temp[2]=Element[RowInc];
      temp[3]=Element[RowInc+1];
      if (eigen2X2(temp,evec,lam)==1)return 1;
    }
    lambda=lam[0];
    lambda(1,1)=lam[1];
    ev(0,0)=evec[0];
    ev(1,0)=evec[1];
    ev(0,1)=evec[2];
    ev(1,1)=evec[3];
    return 0;
  }
  lambda.symmetricHessenberg(ev);
  bool test=true;
  long leading=Rows-1;
  Matrix q(Rows);
  Matrix qcum(Rows);
  qcum=1;
  long cnt=5*Rows;
  while (test)
    {
      lambda.Rows=leading+1;
      q=1;  
      q.Rows=leading+1;
      q.Columns=leading+1;
      double shift=-lambda(leading,leading);
      lambda+=shift;
      lambda.symmetricQR(q,qcum);
      
      double tst=-lambda(leading,leading);
      if ((tst<1E-3)&&(tst>-1E-3))
	{
	  leading--;
	  if (leading<0)
	    leading=0;
	}
      lambda.Columns=lambda.Rows;
      long lr=lambda.Rows-1;
      if (lr>1)
	{
	  Matrix a(1,3),b(1);
	  a(0,0)=lambda(0,0);
	  a(0,1)=lambda(0,1);
	  a(0,2)=lambda(0,2);
	  q.offset(0,0,3,1);
	  b.multiply_(a,q);
	  lambda(0,0)=b(0,0);
	  q.Element++;
	  b.multiply_(a,q);
	  lambda(0,1)=b(0,0);
	  q.reset(lr+1,lr+1);
	  lambda(0,2)=0;
	  for (long k=1; k<lr-1; k++)
	    {
	      a(0,0)=lambda(k,k);
	      a(0,1)=lambda(k,k+1);
	      a(0,2)=lambda(k,k+2);
	      lambda(k,k-1)=lambda(k-1,k);
	      q.offset(k,k,3,1);
	      b.multiply_(a,q);
	      lambda(k,k)=b(0,0);
	      q.Element++;
	      b.multiply_(a,q);
	      lambda(k,k+1)=b(0,0);
	      q.reset(lr+1,lr+1);
	      lambda(k,k+2)=0;
	    }
	  a.Columns=2;
	  a(0,0)=lambda(lr-1,lr-1);
	  a(0,1)=lambda(lr-1,lr);
	  lambda(lr-1,lr-2)=lambda(lr-2,lr-1);
	  q.offset(lr-1,lr-1,2,1);
	  b.multiply_(a,q);
	  lambda(lr-1,lr-1)=b(0,0);
	  q.Element++;
	  b.multiply_(a,q);
	  lambda(lr-1,lr)=b(0,0);
	  q.reset(lr+1,lr+1);
	  a.Columns=1;
	  a(0,0)=lambda(lr,lr);
	  lambda(lr,lr-1)=lambda(lr-1,lr);
	  q.offset(lr,lr,1,1);
	  b.multiply_(a,q);
	  lambda(lr,lr)=b(0,0);
	  q.reset(lr+1,lr+1);
	}
      else lambda*=q;
      shift=-shift;
      lambda+=shift; 
      lambda.Rows=Rows;
      lambda.Columns=lambda.Rows;
      if (leading==0)test=false;
      cnt--;
      if (cnt==0)test=false;
    }
  ev.Rows=1;
  qcum.Rows=1;
  ev=qcum;
  ev.offset(1,0,Rows-1,Rows);
  qcum.reset(Rows,Rows);
  ev*=qcum;
  ev.reset(Rows,Rows);
  cnt=5*Rows-cnt;
  for (long i=1; i<Rows-1; i++)
    {
      lambda(i,i-1)=0;
      lambda(i-1,i)=0;
      lambda(i-1,i+1)=0;
    }
  lambda(Rows-1,Rows-2)=0;
  lambda(Rows-2,Rows-1)=0;
  for (int i=0; i<Rows-1; i++)
    {
      double temp=lambda(i,i);
      if (temp<0)temp=-temp;
      int maxi=i;
      for (int j=i+1; j<Rows; j++)
	{
	  double temp2=lambda(j,j);
	  if (temp2<0)temp2=-temp2;
	  if(temp<temp2)
	    {
	      temp=temp2;
	      maxi=j;
	    }
	}
      if (maxi!=i)
	{
	  double temp3=lambda(i,i);
	  ev.swapColumns(i,maxi);
	  lambda(i,i)=lambda(maxi,maxi);
	  lambda(maxi,maxi)=temp3;
	}
    }
  ev.orthogonalize();
  int zerorows=-1;
  if (lambda(0,0)==0)return 0;
  double d=lambda(Rows-1,Rows-1)/(1E-15*lambda(0,0));

  if (d<0)d=-d;
  if (d<1)
    {
      zerorows=Rows-1;
      for (int i=1; i<Rows-1; i++){
	d=lambda(i,i)/(1E-15*lambda(0,0));
	if (d<0)d=-d;
	if (d<1)
	  {
	    zerorows=i;
	    i=Rows;
	  }
	ev.offset(0,zerorows,Rows,Rows-zerorows);
	q=ev;
	ev.reset(Rows,Rows);
      }    
    }
  for (long i=0; i<iterations;i++)
    eigenize(ev);
  if (zerorows>-1)
    {
      ev.offset(0,zerorows,Rows,Rows-zerorows);
      ev=q;
      ev.reset(Rows,Rows);
      ev.orthogonalize();
    }
  return 0;
}
int Matrix::symmetricEigen(Matrix &lambda )
{
#if CUREMATRIXDEBUG >10
  if (Rows != Columns)
    {
      cerr << "\nERRORMatrix::symmetricEigen: non-square matrix\n";
      return 1;
    }   
#endif 
  lambda=(*this);
  if (Rows==0)return 0;  
  if (Rows==1){
    return 0;  
  }
  lambda.symmetricHessenberg();
  bool test=true;
  long leading=Rows-1;
  Matrix q(Rows);
  long cnt=5*Rows;
  while (test)
    {
      lambda.Rows=leading+1;
      q=1;  
      q.Rows=leading+1;
      q.Columns=leading+1;
      double shift=-lambda(leading,leading);
      lambda+=shift;
      lambda.symmetricQR(q);
      
      double tst=-lambda(leading,leading);
      if ((tst<1E-3)&&(tst>-1E-3))
	{
	  leading--;
	  if (leading<0)
	    leading=0;
	}
      lambda.Columns=lambda.Rows;
      long lr=lambda.Rows-1;
      if (lr>1)
	{
	  Matrix a(1,3),b(1);
	  a(0,0)=lambda(0,0);
	  a(0,1)=lambda(0,1);
	  a(0,2)=lambda(0,2);
	  q.offset(0,0,3,1);
	  b.multiply_(a,q);
	  lambda(0,0)=b(0,0);
	  q.Element++;
	  b.multiply_(a,q);
	  lambda(0,1)=b(0,0);
	  q.reset(lr+1,lr+1);
	  lambda(0,2)=0;
	  for (long k=1; k<lr-1; k++)
	    {
	      a(0,0)=lambda(k,k);
	      a(0,1)=lambda(k,k+1);
	      a(0,2)=lambda(k,k+2);
	      lambda(k,k-1)=lambda(k-1,k);
	      q.offset(k,k,3,1);
	      b.multiply_(a,q);
	      lambda(k,k)=b(0,0);
	      q.Element++;
	      b.multiply_(a,q);
	      lambda(k,k+1)=b(0,0);
	      q.reset(lr+1,lr+1);
	      lambda(k,k+2)=0;
	    }
	  a.Columns=2;
	  a(0,0)=lambda(lr-1,lr-1);
	  a(0,1)=lambda(lr-1,lr);
	  lambda(lr-1,lr-2)=lambda(lr-2,lr-1);
	  q.offset(lr-1,lr-1,2,1);
	  b.multiply_(a,q);
	  lambda(lr-1,lr-1)=b(0,0);
	  q.Element++;
	  b.multiply_(a,q);
	  lambda(lr-1,lr)=b(0,0);
	  q.reset(lr+1,lr+1);
	  a.Columns=1;
	  a(0,0)=lambda(lr,lr);
	  lambda(lr,lr-1)=lambda(lr-1,lr);
	  q.offset(lr,lr,1,1);
	  b.multiply_(a,q);
	  lambda(lr,lr)=b(0,0);
	  q.reset(lr+1,lr+1);
	}
      else lambda*=q;
      shift=-shift;
      lambda+=shift; 
      lambda.Rows=Rows;
      lambda.Columns=lambda.Rows;
      if (leading==0)test=false;
      cnt--;
      if (cnt==0)test=false;
    }
  cnt=5*Rows-cnt;
  for (long i=1; i<Rows-1; i++)
    {
      lambda(i,i-1)=0;
      lambda(i-1,i)=0;
      lambda(i-1,i+1)=0;
    }
  lambda(Rows-1,Rows-2)=0;
  lambda(Rows-2,Rows-1)=0;
 
  for(int i=0; i<Rows-1;i++)
    {
      double temp=lambda(i,i);
      if (temp<0)temp=-temp;
      for (int k=i+1; k<Rows; k++)
	{
	  double temp2=lambda(k,k);
	  if (temp2<0)temp2=-temp2;
	  if (temp<temp2)
	    {
	      temp=temp2;
	      temp2=lambda(k,k);
	      lambda(k,k)=lambda(i,i);
	      lambda(i,i)=temp2;
	    }
	}
    }
  return 0;
}
void Matrix::factorQR(Matrix & q)
{
  long r=Rows;
  long c=Columns;
  Matrix rot(2),a(2,Columns);
  for (long k=1; k<Rows; k++)
    {
      double t=(*this)(k-1,k-1);
      rot(1,0)=-(*this)(k,k-1);
      rot(1,1)=t*t+rot(1,0)*rot(1,0);
      if (rot(1,1)!=0){
	rot(1,1)=1/sqrt(rot(1,1));
	rot(1,0)*=rot(1,1);
	rot(1,1)*=t;
      }
      else {
	rot(1,1)=1;
	rot(1,0)=0;
      }
      rot(0,0)=rot(1,1);
      rot(0,1)=-rot(1,0);
      offset(((RowInc+1)*(k-1)),2,c-k+1);
      a.multiply_(rot,*this);
      *this=a;
      reset(r,c);
      rot(1,0)=rot(0,1);
      rot(0,1)=-rot(1,0);
      q.offset((k-1),k+1,2);
      q*=rot;
      q.reset(Rows,Rows);
      (*this)(k,k-1)=0;
    } 
}
void Matrix::symmetricQR(Matrix & q,Matrix & qcum)
{
  long r=Rows;
  long c=Columns;
  long qc=q.Columns;
  long qr=q.Rows;
  q.Columns=2;
  q.Rows=2;
  long qcm=qcum.Columns;
  long qrm=qcum.Rows;
  qcum.Columns=2;
  qcum.Rows=2;
  Matrix rot(2),a(2,3);
  double *e=rot.Element;
  for (long k=1; k<Rows; k++)
    {
      double t=(*this)(k-1,k-1);
      e[2]=-(*this)(k,k-1);
      e[3]=t*t+e[2]*e[2];
      if (e[3]!=0){
	e[3]=1/sqrt(e[3]);
	e[2]*=e[3];
	e[3]*=t;  
      }
      else {
	e[2]=0;
	e[3]=1;
      }
      e[0]=e[3];
      e[1]=-e[2];     
      if (k==c-1)
	{
	  offset((k-1),2);
	  a.Columns=2;
	}
      else offset((k-1),(k-1),2,3);
      a.multiply_(rot,*this);
      *this=a;
      reset(r,c);
      e[2]=e[1];
      e[1]=-e[2];
      //      q.offset((k-1),k+1,2);
      q*=rot;
      qcum*=rot;
      q.Element++;
      q.Rows++;
      qcum.Element++;
      qcum.Rows++;
      //q.reset(qr,qc);
      (*this)(k,k-1)=0;
    } 
  q.reset(qr,qc);
  qcum.reset(qrm,qcm);
}
void Matrix::symmetricQR(Matrix & q)
{
  long r=Rows;
  long c=Columns;
  long qc=q.Columns;
  long qr=q.Rows;
  q.Columns=2;
  q.Rows=2;
  Matrix rot(2),a(2,3);
  double *e=rot.Element;
  for (long k=1; k<Rows; k++)
    {
      double t=(*this)(k-1,k-1);
      e[2]=-(*this)(k,k-1);
      e[3]=t*t+e[2]*e[2];
      if (e[3]!=0){
	e[3]=1/sqrt(e[3]);
	e[2]*=e[3];
	e[3]*=t;  
      }
      else {
	e[2]=0;
	e[3]=1;
      }
      e[0]=e[3];
      e[1]=-e[2];     
      if (k==c-1)
	{
	  offset((k-1),2);
	  a.Columns=2;
	}
      else offset((k-1),(k-1),2,3);
      a.multiply_(rot,*this);
      *this=a;
      reset(r,c);
      e[2]=e[1];
      e[1]=-e[2];
      //      q.offset((k-1),k+1,2);
      q*=rot;
      q.Element++;
      q.Rows++;
      (*this)(k,k-1)=0;
    } 
  q.reset(qr,qc);
}
void Matrix::decompQR(Matrix & q){
  q.reallocate(Rows);
  Matrix u(1,Rows);
  Matrix a(Rows,1);
  Matrix b(1,Rows);
  q=1;
  long irowinit=(RowInc*2);
  //*first we hessenberg this matrix
  //by householder reflections
  for (long k=2; k<Rows; k++) 
    {
      u(0,k-2)=0;
      long irow=irowinit;//(RowInc*k)+k-2;
      double d=0;
      double t=Element[irow-RowInc];
      for (long i=k; i<Rows; i++,irow+=RowInc)
	{
	  u(0,i)=Element[irow];
	  d+=u(0,i)*u(0,i);
	}
      if (d>0){
	u(0,k-1)=t+sqrt(t*t+d);
	d+=u(0,k-1)*u(0,k-1);
	d/=2;
	u/=sqrt(d);
	a.multTranspose_(q,u,2);
	q.subtractProduct_(a,u);
	// a.multTranspose_(*this,u,2);
	// (*this).subtractProduct_(a,u);
	b.multiply_(u,*this);
	u.Rows=Rows;
	u.Columns=1;
	u.RowInc=1;
	(*this).subtractProduct_(u,b);
	u.Columns=Rows;
	u.Rows=1;
	u.RowInc=Rows;
      }//if d==0 then we needed do anything.
      irow=irowinit;
      for (long i=k; i<Rows; i++,irow+=RowInc)
	Element[irow]=0;
      irowinit+=RowInc+1;
    }
  //Now we can do the QR on the simplier matrix
  factorQR(q);
}

long Matrix::solveToPermuted(Matrix &x, long *permute,
			     Matrix &rhs, double oneovermaxx,
			     double minsquarediagonal,
			     double mininformation,
			     long startrow,
			     unsigned long stoprow)
{
  long c=Columns;
  long r=Rows;
  long w=rhs.Rows;
  x.reallocateZero(c,w);
  //flags that we have check the covariance of a row
  unsigned char checkedcov[c]; 
  for (long i=0;i<c;i++){
    checkedcov[i]=0;
  }
  if (rhs.Columns!=r)return -1;
  if (c==0) return -1; 
  if (rhs.Rows<1)return -1; 
  if (c>r)return -1;
  long dim=c;
  if (startrow>-1)
    if (dim>startrow)
      dim=startrow;
  triangle(rhs,permute);
  offset(0,0,dim,c);
  rhs.offset(0,0,w,dim);
  for (long i=dim-1;i>-1;i--)
    if (checkedcov[i]==0){
      double d=(*this)(i,permute[i]);
      d*=d;
      
      for (int j=0;j<i;j++)d+=((*this)(j,permute[i])*(*this)(j,permute[i]));
      if ((d<mininformation)){
	if ((dim-1)!=(long)i){ 
	  long k=permute[i];
	  memmove(permute+i, permute+i+1,(dim-1-i)*sizeof(long));
	  permute[dim-1]=k;
	  memmove(checkedcov+i, checkedcov+i+1,(dim-1-i)*sizeof(unsigned char));
	  doQR(rhs,permute,i);
	}
	checkedcov[dim-1]=1;
	d=(*this)(dim-1,dim-1);
	d*=d;
	if (d<minsquarediagonal){
	  //these are the easy to find numerical problem dimensions.
	  //Here we have low information and high covariance in this dimension
	  dim--;  
	  offset(0,0,dim,c);
	  rhs.offset(0,0,w,dim);
	}
	i++;
      }
    } 
  unsigned long stp=tryBackSubstitute(x,rhs,oneovermaxx,stoprow,dim,permute);  
  while(stoprow<stp){
    unsigned long done=stp;
    if (checkedcov[stp-1]){
      double d=(*this)(stp-1,permute[stp-1]);
      double d2=d*d;
      if (d2<minsquarediagonal){
	offset(0,0,r,c);
	rhs.offset(0,0,w,c);
	//x.offset(0,0,c,w);
	if (dim>(long)stp){ 
	  //moveColumns(stp-1,dim-1); 
	  //x.moveRows(stp-1,dim-1);
	  long k=permute[stp-1];
	  memmove(permute+stp-1, permute+stp,(dim-stp)*sizeof(long));
	  memmove(checkedcov+stp-1, checkedcov+stp,(dim-stp)*sizeof(char));
	  permute[dim-1]=k;
	  doQR(rhs,permute,stp-1);
	}
	dim--;
	done=dim;
	for (int j=0;j<w;j++)
	  x(permute[dim],j)=0;
      }else{
	for (int j=0;j<w;j++){
	  x(permute[stp-1],j)=rhs(j,dim-1); 
	  for (int k=stp;k<dim;k++)
	    x(permute[stp-1],j)-=(*this)(stp-1,permute[k])*x(permute[k],j);
	  x(permute[stp-1],j)/=d;
	  done=stp-1;
	}
      }
      offset(0,0,done,c);
      rhs.offset(0,0,w,done);
      stp=tryBackSubstitute(x,rhs,oneovermaxx,stoprow,dim,permute);  
    }else{
      checkedcov[stp-1]=1;
      offset(0,0,r,c);
      rhs.offset(0,0,w,c);
      if (dim>(long)stp){ 
	long k=permute[stp-1];
	memmove(permute+stp-1, permute+stp,(dim-stp)*sizeof(long));
	memmove(checkedcov+stp-1, checkedcov+stp,(dim-stp)*sizeof(char));
	permute[dim-1]=k;
	doQR(rhs,permute,stp-1);
      }
      double d=(*this)(dim-1,permute[dim-1]);
      double d2=d*d;
      if (d2<minsquarediagonal){
	dim--;
	done=dim;
	for (int j=0;j<w;j++)
	  x(permute[dim],j)=0;
      }else {
	//here the covariance is tight dispite the big x result
	// we keep the column.  This is just a big adjustment.
	for (int j=0;j<w;j++)
	  x(permute[dim-1],j)=rhs(j,dim-1)/d; 
	done=dim-1;
      }
      offset(0,0,done,c);
      rhs.offset(0,0,w,done);
      stp=tryBackSubstitute(x,rhs,oneovermaxx,stoprow,dim,permute);  
    }
  }
  offset(0,0,r,c);
  rhs.offset(0,0,w,r);
  return dim;  

}
long Matrix::solveTo(Matrix &x, long *columns,
		    Matrix &rhs, double oneovermaxx,
		     double minsquarediagonal,
		     double mininformation,
		     unsigned long stoprow)
{
  long c=Columns;
  long r=Rows;
  long w=rhs.Rows;
  x.reallocateZero(c,w);
  //flags that we have check the covariance of a row
  unsigned char checkedcov[c]; 
  for (long i=0;i<c;i++){
    columns[i]=i;
    checkedcov[i]=0;
  }
  if (rhs.Columns!=r)return -1;
  if (c==0) return -1; 
  if (rhs.Rows<1)return -1; 
  if (c>r)return -1;
  long dim=c;
  triangle(rhs);
  offset(0,0,dim,dim);
  rhs.offset(0,0,w,dim);
  for (long i=dim-1;i>-1;i--)
    if (checkedcov[i]==0){
      double d=(*this)(i,i);
      d*=d;
      
      for (int j=0;j<i;j++)d+=((*this)(j,i)*(*this)(j,i));
      if ((d<mininformation)){
	if ((dim-1)!=(long)i){ 
	  moveColumns(i,dim-1); 
	  long k=columns[i];
	  memmove(columns+i, columns+i+1,(dim-1-i)*sizeof(long));
	  columns[dim-1]=k;
	  memmove(checkedcov+i, checkedcov+i+1,(dim-1-i)*sizeof(unsigned char));

	  doQR(rhs,i);
	}
	checkedcov[dim-1]=1;
	d=(*this)(dim-1,dim-1);
	d*=d;
	if (d<minsquarediagonal){
	  //these are the easy to find numerical problem dimensions.
	  //Here we have low information and high covariance in this dimension
	  dim--;  
	  offset(0,0,dim,dim);
	  rhs.offset(0,0,w,dim);
	  x.offset(0,0,dim,w);
	}
	i++;
      }
    } 
  unsigned long stp=tryBackSubstitute(x,rhs,oneovermaxx,stoprow);
  
  while(stoprow<stp){
    unsigned long done=stp;
    if (checkedcov[stp-1]){
      double d=(*this)(stp-1,stp-1);
      double d2=d*d;
      if (d2<minsquarediagonal){
	offset(0,0,r,c);
	rhs.offset(0,0,w,c);
	x.offset(0,0,c,w);
	if (dim>(long)stp){ 
	  moveColumns(stp-1,dim-1); 
	  x.moveRows(stp-1,dim-1);
	  long k=columns[stp-1];
	  memmove(columns+stp-1, columns+stp,(dim-stp)*sizeof(long));
	  memmove(checkedcov+stp-1, checkedcov+stp,(dim-stp)*sizeof(char));
	  columns[dim-1]=k;
	  doQR(rhs,stp-1);
	}
	dim--;
	done=dim;
	for (int j=0;j<w;j++)
	  x(dim,j)=0;
      }else{
	for (int j=0;j<w;j++){
	  x(stp-1,j)=rhs(j,dim-1); 
	  for (int k=stp;k<dim;k++)
	    x(stp-1,j)-=(*this)(stp-1,k)*x(k,j);
	  x(stp-1,j)/=d;
	  done=stp-1;
	}
      }
      offset(0,0,done,dim);
      rhs.offset(0,0,w,done);
      x.offset(0,0,dim,w);
      stp=tryBackSubstitute(x,rhs,oneovermaxx,stoprow);
    }else{
      checkedcov[stp-1]=1;
      offset(0,0,r,c);
      rhs.offset(0,0,w,c);
      x.offset(0,0,c,w);
      if (dim>(long)stp){ 
	moveColumns(stp-1,dim-1); 
	x.moveRows(stp-1,dim-1);
	long k=columns[stp-1];
	memmove(columns+stp-1, columns+stp,(dim-stp)*sizeof(long));
	memmove(checkedcov+stp-1, checkedcov+stp,(dim-stp)*sizeof(char));
	columns[dim-1]=k;
	doQR(rhs,stp-1);
      }
      double d=(*this)(dim-1,dim-1);
      double d2=d*d;
      if (d2<minsquarediagonal){
	dim--;
	done=dim;
	for (int j=0;j<w;j++)
	  x(dim,j)=0;
      }else {
	//here the covariance is tight dispite the big x result
	// we keep the column.  This is just a big adjustment.
	for (int j=0;j<w;j++)
	  x(dim-1,j)=rhs(j,dim-1)/d; 
	done=dim-1;
      }
      offset(0,0,done,dim);
      rhs.offset(0,0,w,done);
      x.offset(0,0,dim,w);
      stp=tryBackSubstitute(x,rhs,oneovermaxx,stoprow);
    }
  }
  offset(0,0,r,c);
  rhs.offset(0,0,w,r);
  x.offset(0,0,c,w);
  return dim;  
}

unsigned long
Matrix::tryBackSubstitute(Matrix &x, 
			  const Matrix &rhs,
			  double oneovermaxx,
			  unsigned long stoprow, 
			  unsigned long startrow,
			  long *permute)
{
  if (x.Rows!=Columns)return Columns;
  if (x.Columns!=rhs.Rows)return Columns;
  if (rhs.Columns!=Rows)return Columns;
  if (rhs.Rows<1)return stoprow;
  if (startrow>(unsigned long)Rows)startrow=Rows;
  long matcol=startrow-1;
  if (startrow>(unsigned long)Columns)matcol=Columns-1;
  long startcol=startrow;
  if (startcol>Columns)startcol=Columns;
  while (matcol>=(long)stoprow)
    {
      double t=(*this)(matcol,permute[matcol]);
      int start=matcol+1;
      for (unsigned long xmatcol=0;(long)xmatcol!=x.Columns;xmatcol++)
	x(permute[matcol],xmatcol)=rhs(xmatcol,matcol);
      for (long i=start;i<startcol;i++){
	double d=(*this)(matcol,permute[i]);
	if (d!=0)
	  for (unsigned long xmatcol=0;(long)xmatcol!=x.Columns;xmatcol++)
	    x(permute[matcol],xmatcol)-=d*x(permute[i],xmatcol);
      }
      for (unsigned long xmatcol=0;(long)xmatcol!=x.Columns;xmatcol++)
	if (x(permute[matcol],xmatcol)!=0){
	  double min=(x(permute[matcol],xmatcol)*oneovermaxx);
	  if (min<0)min=-min;
	  if (min<1E-20)min=1E-20;
	  if (t>=0){
	    if(t<min)
	      return matcol+1;
	  }else if (t>-min){
	    return matcol+1;
	  }
	  if (x(permute[matcol],xmatcol)!=0)
	    x(permute[matcol],xmatcol)/=t;
	}
      matcol--;
    }
  return stoprow;
}

// solve xt(this)=rhs^T  
long
Matrix::tryTransposedBackSubstitute(Matrix &xt, 
				    const Matrix &rhs,
				    double oneovermaxx,
				    LongArray &columns,
				    long *permute)
{
  if (xt.Rows!=rhs.Columns)return -1;
  if (xt.Columns!=Rows)return -1;
  if (rhs.Rows!=Columns)return -1;
  if (rhs.Columns<1)return Rows-1;
  // start with  permcol=permute(0) as col of this, and row of rhs, 
  // and 0 as col of xt 
  long matcol=0;
  while (matcol<(long)Rows)
    {
      long permcol=permute[matcol];
      double t=(*this)(matcol,permcol); //the 'diagonal' element
      for (unsigned long xmatcol=0;(long)xmatcol!=xt.Rows;xmatcol++)
	xt(xmatcol,matcol)=rhs(permcol,xmatcol);
      
      unsigned long topr=columns.columns(permcol);
      if (topr){
	long *mr=&columns(permcol,0);
	for (unsigned long j=0;j<topr;j++) {
	  long rcol=mr[j];
	  if (rcol!=matcol){
	    double d=(*this)(rcol,permcol);
	    if (d!=0)
	      for (unsigned long xmatcol=0;(long)xmatcol!=xt.Rows;xmatcol++)
		xt(xmatcol,matcol)-=d*xt(xmatcol,rcol);
	  }
	}
      }
      for (unsigned long xmatcol=0;(long)xmatcol!=xt.Rows;xmatcol++)
	if (xt(xmatcol,matcol)!=0){
	  double min=(xt(xmatcol,matcol)*oneovermaxx);
	  if (min<0)min=-min;
	  if (min<1E-20)min=1E-20;
	  if (t>=0){
	    if(t<min)
	      return matcol-1;
	  }else if (t>-min){
	    return matcol-1;
	  }
	  xt(xmatcol,matcol)/=t;
	}
      matcol++;
    }
  return matcol-1;
}
unsigned long
Matrix::tryBackSubstitute(Matrix &x, 
			  const Matrix &rhs,
			  double oneovermaxx,
			  unsigned long stoprow, 
			  LongArray &rows,
			  long *permute)
{
  if (x.Rows!=Columns)return Columns;
  if (x.Columns!=rhs.Rows)return Columns;
  if (rhs.Columns!=Rows)return Columns;
  if (rhs.Rows<1)return stoprow;
  long matcol=Columns-1;
  while (matcol>=(long)stoprow)
    {
      long permcol=permute[matcol];
      double t=(*this)(matcol,permcol);
      for (unsigned long xmatcol=0;(long)xmatcol!=x.Columns;xmatcol++)
	x(permcol,xmatcol)=rhs(xmatcol,matcol);
      
      unsigned long topr=rows.columns(matcol);
      if (topr){
	long *mr=&rows(matcol,0);
	for (unsigned long j=0;j<topr;j++) {
	  long rcol=mr[j];
	  if (rcol!=permcol){
	    double d=(*this)(matcol,rcol);
	    if (d!=0)
	      for (unsigned long xmatcol=0;(long)xmatcol!=x.Columns;xmatcol++)
		x(permcol,xmatcol)-=d*x(rcol,xmatcol);
	  }
	}
      }
      for (unsigned long xmatcol=0;(long)xmatcol!=x.Columns;xmatcol++)
	if (x(permcol,xmatcol)!=0){
	  double min=(x(permcol,xmatcol)*oneovermaxx);
	  if (min<0)min=-min;
	  if (min<1E-20)min=1E-20;
	  if (t>=0){
	    if(t<min)
	      return matcol+1;
	  }else if (t>-min){
	    return matcol+1;
	  }
	  if (x(permcol,xmatcol)!=0)
	    x(permcol,xmatcol)/=t;
	}
      matcol--;
    }
  return stoprow;
}
unsigned long
Matrix::tryBackSubstitute(Matrix &x, 
		       const Matrix &rhs,
		       double oneovermaxx,
		       unsigned long stoprow)
{
  if (x.Rows!=Columns)return Columns;
  if (x.Columns!=rhs.Rows)return Columns;
  if (rhs.Columns!=Rows)return Columns;
  if (rhs.Rows<1)return stoprow;
  long matcol=Rows-1;
  if (Rows>Columns)matcol=Columns-1;
  while (matcol>=(long)stoprow)
    {
      unsigned long xmatcol=0;
      while ((long)xmatcol!=x.Columns){
	double d=rhs(xmatcol,matcol);
	int start=matcol+1;
	for (int i=start; i<Columns; i++)
	  {
	    d-=(*this)(matcol,i)*
	      x(i,xmatcol);
	  }
     	if (d!=0){
	  double min=d*oneovermaxx;
	  if (min<0)min=-min;
	  if (min<1E-20)min=1E-20;
	  double t=(*this)(matcol,matcol);
	  if (t>=0){
	    if(t<min){
	      return matcol+1;
	    } 
	  }else if (t>-min){
	    return matcol+1;
	  }
	  if (d!=0)
	    d/=t;
	}
	x(matcol,xmatcol)=d;
	xmatcol++;
      }
      matcol--;
    }
  return stoprow;
}

long
Matrix::backSubstitute(Matrix &x, 
		       const Matrix &rhs,
		       double oneovermaxx,
		       unsigned long stoprow)
{
  long ret=0;
  x.reallocateZero(Columns,rhs.Rows);
  if (rhs.Columns!=Rows)return Columns;
  if (rhs.Rows<1)return (long)stoprow;
  long matcol=Columns-1; 
  while (matcol>=(long)stoprow)
    {
      unsigned short xmatcol=0;
      while (xmatcol!=x.Columns){
	double d=rhs(xmatcol,matcol);
	int start=matcol+1;
	for (int i=start; i<Columns; i++)
	  {
	    d-=(*this)(matcol,i)*
	      x(i,xmatcol);
	  }
     	if (d!=0){
	  double min=d*oneovermaxx;
	  if (min<0)min=-min;
	  if (min<1E-20)min=1E-20;
	  double t=(*this)(matcol,matcol);
	  if (t>=0){
	    if(t<min){
	      if (ret==0)ret=matcol+1;
	      d*=(t/min);
	      t=min;
	    }
	  } else if (t>-min){
	    if (ret==0)ret=matcol+1;
	    d*=(-t/min);
	    t=-min;
	  }
	  if (d!=0)
	    d/=t;
	}
	x(matcol,xmatcol)=d;
	xmatcol++;
      }
      matcol--;
    }
  if (ret==0)ret=matcol+1;
  return ret;
}

void Matrix::triangle(Matrix & q, long startcol){
  long r=Rows;
  if (q.Columns!=r)
    q.reallocate(q.Rows,r);
  Matrix u(1,r);
  Matrix a(r,1);
  Matrix b(1,Columns);

  long irowinit=(RowInc*(startcol+2))+startcol;  //(RowInc*(startcol+2));
  //*first we hessenberg this matrix
  //by householder reflections
  long top=Rows;
  //*************************
  if (top>(Columns+2)) top=(Columns+2);
  for (long k=startcol+2; k<top; k++) 
    {
      u(0,k-2)=0;
      long irow=irowinit;//(RowInc*k)+k-2;
      double d=0;
      long count=0;
      for (long i=k; i<Rows; i++,irow+=RowInc)
	{
	  u(0,i)=Element[irow];
	  if (u(0,i)!=0)count++;
	  d+=u(0,i)*u(0,i);
	}
      if (d>0){
	double t=Element[irowinit-RowInc];   //rows 1..top-2 cols 0...top-3
	u(0,k-1)=t+sqrt(t*t+d);
	d+=u(0,k-1)*u(0,k-1);
	d/=2;
	count=(count<<1);
	if(count>(Rows-k)){
	  u.offset(0,k-2,1,r-k+2);
	  q.offset(0,k-2,q.Rows,u.Columns);
	  u/=sqrt(d);
	  a.multTranspose_(q,u,2);
	  q.subtractProduct_(a,u);
	  q.offset(0,2-k,q.Rows,r);
	  // a.multTranspose_(*this,u,2);
	  // (*this).subtractProduct_(a,u);
	  offset(k-2,0,r-k+2,Columns);
	  b.multiply_(u,*this);
	  
	  u.Rows=u.Columns;
	  u.Columns=1;
	  u.RowInc=1;
	  (*this).subtractProduct_(u,b);
	  offset(2-k,0,r,Columns);
	
	  u.Columns=u.Rows;
	  u.Rows=1;
	  u.RowInc=r;
	  u.offset(0,2-k,1,r);
	}else { //if u is more than half empty we can skip many loops
	  int rr=q.Rows;
	  a.reallocateZero(rr,1);
	  b.reallocateZero(1,Columns);
	  d=sqrt(d);
	  for(long i=k-2;i<r;i++)
	    if (u(0,i)!=0)
	    {
	      u(0,i)/=d;
	      for (int j=0;j<rr;j++)
		a(j,0)+=q(j,i)*u(0,i);
	      for (int j=0;j<Columns;j++)
		b(0,j)+=u(0,i)*(*this)(i,j);
	    }
	  for(long i=k-2;i<r;i++)
	    if (u(0,i)!=0)
	      {
		for (int j=0;j<rr;j++)
		  q(j,i)-=a(j,0)*u(0,i);
		for (int j=0;j<Columns;j++)
		  (*this)(i,j)-=u(0,i)*b(0,j);
	      }
	}
	irow=irowinit;
	for (long i=k; i<Rows; i++,irow+=RowInc)
	  Element[irow]=0;
      }//if d==0 then we needed do anything.
      irowinit+=RowInc+1;
    }
  //Now we can do the QR on the simplier matrix
  doQR(q,startcol);
}
void Matrix::makeSparseLists(LongArray & columns, 
			    LongArray & rows,
			    long num)
{
  columns.grow(0);
  rows.grow(0);
  if (num<10)num=10;
  long top=Columns-1;
  columns.makeCell(top,num);
  for (long i=0;i<top;i++)
    columns.makeCell(i,num);
  top=Rows-1;
  rows.makeCell(top,num);
  for (long i=0;i<top;i++)
    rows.makeCell(i,num);
  columns.clear();  
  rows.clear();
  long irow=0;
  for (long i=0;i<Rows;i++,irow+=RowInc)
    for (long j=0;j<Columns;j++)
      if (Element[irow+j]!=0){
	rows.add(i,j);
	columns.add(j,i);
      }
}
		      
void Matrix::triangle(Matrix & q,long *permute, 
		      LongArray & columns, 
		      LongArray & rows,
		      long startcol)
{
  long r=Rows;
  if (q.Columns!=r)
    q.reallocate(q.Rows,r);
  Matrix u(1,r);
  Matrix a(r,1);
  Matrix b(1,Columns);
  LongArray bnotzero;
  bnotzero.makeCell(0,Columns);//allocate enough elements
  bnotzero.clear();
  long m[r];//holds u not 0
  unsigned long topm=0;  //holds length of m
  //*first we hessenberg this matrix
  //by householder reflections
  long top=Rows;
  if (top>(Columns+2)) top=(Columns+2);
  for (long k=startcol+2; k<top; k++) 
    {
      long col=permute[k-2];
      u(0,k-2)=0;
      double d=0;
      //sum squares of colum k-2 from row k
      topm=columns.columns(col);
      if (topm){
	long *mr=&columns(col,0);
	long sk=k-1;
	m[0]=sk;
	long j=1;
	for (unsigned long i=0;i<topm;i++)
	  if (mr[i]>sk){
	    m[j]=mr[i];
	    j++;
	  }
	topm=j;
	for (unsigned long i=1;i<topm;i++) {
	  long row=m[i];
	  u(0,row)=(*this)(row,col);
	  d+=u(0,row)*u(0,row);
	}
      } 
      if (d!=0){
	double t=(*this)(k-1,col);
	u(0,k-1)=t+sqrt(t*t+d);
	d+=u(0,k-1)*u(0,k-1);
	d/=2;
	int rr=q.Rows;
	a.reallocateZero(rr,1);
	b.reallocateZero(1,Columns);
	bnotzero.clear();
	d=sqrt(d);
	for (unsigned long i=0;i<topm;i++) {
	  long row=m[i];
	  u(0,row)/=d;
	  for (int j=0;j<rr;j++)
	    a(j,0)+=q(j,row)*u(0,row);
	  unsigned long topr=rows.columns(row);
	  if (topr){
	    long *mr=&rows(row,0);
	    for (unsigned long j=0;j<topr;j++) {
	      long rcol=mr[j];
	      if (b(0,rcol)==0)
		bnotzero.addUnique(0,rcol);
	      b(0,rcol)+=u(0,row)*(*this)(row,rcol);      
	    }
	  }
	}
	for (unsigned long i=0;i<topm;i++) {
	  long row=m[i];
	  for (int j=0;j<rr;j++)
	    if (a(j,0)!=0)
	      q(j,row)-=a(j,0)*u(0,row);
	  unsigned long topr=bnotzero.columns(0);
	  if (topr){
	    long *mb=&bnotzero(0,0);
	    for (unsigned long j=0;j<topr;j++) {
	      long bcol=mb[j];
	      //if ((*this)(row,bcol)==0){
		rows.addUnique(row,bcol);
		columns.addUnique(bcol,row);
		//}
	      (*this)(row,bcol)-=u(0,row)*b(0,bcol);
	    }
	  }
	}
	
	for (unsigned long i=1; i<topm; i++){
	  long row=m[i];
	  (*this)(row,col)=0;
	  columns.remove(col,row);
	  rows.remove(row,col);
	}
	
      }//if d==0 then we  needed do anything.
    }
  //Now we can do the QR on the simplier matrix
  // makeSparseLists(columns,rows); 
  doQR(q,permute,columns,rows,startcol);
   // doQR(q,permute,startcol);
  
}
void Matrix::doQR(Matrix & q, long *permute, 
		  LongArray & columns, 
		  LongArray &rows,
		  long startcol)
{
  //  long r=Rows;
  //long c=Columns;
  long qrows=q.Rows;
  long qcols=q.Columns;
  Matrix rot(2),a(2,Columns);
  long top=Rows;
  if (top>Columns+1)top=Columns+1;
  for (long k=startcol+1; k<top; k++)
    {
      long kminus1=permute[k-1];
      rot(1,0)=-(*this)(k,kminus1);  //rows 1..top-1 cols 0..top-2
      if (rot(1,0)!=0){
	double t=(*this)(k-1,kminus1); //rows 0..top-2 cols 0..top-2
	rot(1,1)=t*t+rot(1,0)*rot(1,0);
	if (rot(1,1)!=0){
	  rot(1,1)=1/sqrt(rot(1,1));
	  rot(1,0)*=rot(1,1);
	  rot(1,1)*=t;
	}
	else {
	  rot(1,1)=1;
	  rot(1,0)=0;
	}
	rot(0,0)=rot(1,1);
	rot(0,1)=-rot(1,0);
	bool fini[Columns];
	if (Columns>0)
	  memset(&fini,0,Columns*sizeof(bool));
	long row=k-1;
	unsigned long topr=rows.columns(row);
	if (topr){
	  long *mr=&rows(row,0);
	  for (unsigned long j=0;j<topr;j++) {
	    long rcol=mr[j];
	    fini[rcol]=true;
	    //long col=permute[rcol];
	    double e1=(*this)(k-1,rcol);
	    double e2=(*this)(k,rcol);
	    double tempa=rot(0,0)*e1+
	      rot(0,1)*e2;
	    double tempb=rot(1,0)*e1+
	      rot(1,1)*e2;
	    //if (e2==0){
	      rows.addUnique(k,rcol);
	      columns.addUnique(rcol,k);
	      // } else if (e1==0){
	      rows.addUnique(k-1,rcol);
	      columns.addUnique(rcol,k-1);
	      // }
	    (*this)(k-1,rcol)=tempa;
	    (*this)(k,rcol)=tempb;
	  }
	}
	row=k;
	topr=rows.columns(row);
	if (topr){
	  long *mr=&rows(row,0);
	  for (unsigned long j=0;j<topr;j++) {
	    long rcol=mr[j];
	    if (!fini[rcol]){
	      fini[rcol]=true;
	      //long col=permute[rcol];
	      double e1=(*this)(k-1,rcol);
	      double e2=(*this)(k,rcol);
	      double tempa=rot(0,0)*e1+
		rot(0,1)*e2;
	      double tempb=rot(1,0)*e1+
		rot(1,1)*e2;
	      //if (e1==0){
		rows.addUnique(k,rcol);
		columns.addUnique(rcol,k);
		// }else if (e1==0){
		rows.addUnique(k-1,rcol);
		columns.addUnique(rcol,k-1);
		// }
	      (*this)(k-1,rcol)=tempa;
	      (*this)(k,rcol)=tempb;
	    }
	  }
	}	  
	rot(1,0)=rot(0,1);
	rot(0,1)=-rot(1,0);
	q.offset((k-1),qrows,2); //columns 0..top-1
	q*=rot;
	q.offset(-(k-1),qrows,qcols);
	(*this)(k,kminus1)=0;   //rows 1..top-1 cols 0..top-2
	columns.remove(kminus1,k);
	rows.remove(k,kminus1);
      }
    } 
}

void Matrix::triangle(Matrix & q,long *permute, long startcol)
{
  long r=Rows;
  if (q.Columns!=r)
    q.reallocate(q.Rows,r);
  Matrix u(1,r);
  Matrix a(r,1);
  Matrix b(1,Columns);

  //  long irowinit=(RowInc*(startcol+2))+permute[startcol];  

  //*first we hessenberg this matrix
  //by householder reflections
  long top=Rows;
  if (top>(Columns+2)) top=(Columns+2);
  for (long k=startcol+2; k<top; k++) 
    {
      long irowinit=(RowInc*(k))+permute[k-2];  
      //long kcol=permute[k-2];
      u(0,k-2)=0;
      long irow=irowinit;//(RowInc*k)+k-2;
      double d=0;
      long count=0;
      for (long i=k; i<Rows; i++,irow+=RowInc)
	{
	  u(0,i)=Element[irow];
	  if (u(0,i)!=0){
	    count++;
	    d+=u(0,i)*u(0,i);
	  }
	}
      if (count){
	double t=Element[irowinit-RowInc];   //rows 1..top-2 cols 0...top-3
	u(0,k-1)=t+sqrt(t*t+d);
	d+=u(0,k-1)*u(0,k-1);
	d/=2;
	count=(count<<1);
	if(count>(Rows-k)){
	  u.offset(0,k-2,1,r-k+2);
	  q.offset(0,k-2,q.Rows,u.Columns);
	  double sqd=sqrt(d);
	  u/=sqd;
	  a.multTranspose_(q,u,2);
	  q.subtractProduct_(a,u);
	  q.offset(0,2-k,q.Rows,r);
	  offset(k-2,0,r-k+2,Columns);
	  b.multiply_(u,*this);
	  
	  u.Rows=u.Columns;
	  u.Columns=1;
	  u.RowInc=1;
	  (*this).subtractProduct_(u,b);
	  offset(2-k,0,r,Columns);
	  
	  u.Columns=u.Rows;
	  u.Rows=1;
	  u.RowInc=r;
	  u.offset(0,2-k,1,r);
	}else { //if u is more than half empty we can skip many loops
	  int rr=q.Rows;
	  a.reallocateZero(rr,1);
	  b.reallocateZero(1,Columns);
	  d=sqrt(d);
	  for(long i=k-2;i<r;i++)
	    if (u(0,i)!=0)
	    {
	      u(0,i)/=d;
	      for (int j=0;j<rr;j++)
		a(j,0)+=q(j,i)*u(0,i);
	      for (long j=0;j<Columns;j++)
		b(0,j)+=u(0,i)*(*this)(i,j);
	    }
	  for(long i=k-2;i<r;i++)
	    if (u(0,i)!=0)
	      {
		for (int j=0;j<rr;j++)
		  if (a(j,0)!=0)
		    q(j,i)-=a(j,0)*u(0,i);
		  for (int j=0;j<Columns;j++)
		    if (b(0,j)!=0)
		      (*this)(i,j)-=u(0,i)*b(0,j);
	      }
	}
	irow=irowinit;
	for (long i=k; i<Rows; i++,irow+=RowInc)
	  Element[irow]=0;
      }//if d==0 then we needed do anything.
    }
  //Now we can do the QR on the simplier matrix
  doQR(q,permute,startcol);
}
void Matrix::doQR(Matrix & q, long *permute, long startcol)
{
  //  long r=Rows;
  long c=Columns;
  long qrows=q.Rows;
  long qcols=q.Columns;
  Matrix rot(2),a(2,Columns);
  long top=Rows;
  if (top>Columns+1)top=Columns+1;
  for (long k=startcol+1; k<top; k++)
    {
      long kminus1=permute[k-1];
      rot(1,0)=-(*this)(k,kminus1);  //rows 1..top-1 cols 0..top-2
      if (rot(1,0)!=0){
	double t=(*this)(k-1,kminus1); //rows 0..top-2 cols 0..top-2
	rot(1,1)=t*t+rot(1,0)*rot(1,0);
	if (rot(1,1)!=0){
	  rot(1,1)=1/sqrt(rot(1,1));
	  rot(1,0)*=rot(1,1);
	  rot(1,1)*=t;
	}
	else {
	  rot(1,1)=1;
	  rot(1,0)=0;
	}
	rot(0,0)=rot(1,1);
	rot(0,1)=-rot(1,0);
	/*
	  offset(((RowInc+1)*(k-1)),2,c-k+1);  //rows 0..top-1 cols 0..Columns-1 
	  a.multiply_(rot,*this);
	  *this=a;
	  offset(-((RowInc+1)*(k-1)),r,c);
	*/
	for (long j=k-1; j<c;j++){
	  long col=permute[j];
	  double e1=(*this)(k-1,col);
	  double e2=(*this)(k,col);
	  if ((e1!=0)||(e2!=0)){
	    double tempa=rot(0,0)*e1+
	      rot(0,1)*e2;
	    double tempb=rot(1,0)*e1+
	      rot(1,1)*e2;
	    (*this)(k-1,col)=tempa;
	    (*this)(k,col)=tempb;
	  }
	}
	rot(1,0)=rot(0,1);
	rot(0,1)=-rot(1,0);
	q.offset((k-1),qrows,2); //columns 0..top-1
	q*=rot;
	q.offset(-(k-1),qrows,qcols);
	(*this)(k,kminus1)=0;   //rows 1..top-1 cols 0..top-2
      }
    } 
}
void Matrix::doQR(Matrix & q,long startcol)
{
  long r=Rows;
  long c=Columns;
  long qrows=q.Rows;
  long qcols=q.Columns;
  Matrix rot(2),a(2,Columns);
  long top=Rows;
  if (top>Columns+1)top=Columns+1;
  for (long k=startcol+1; k<top; k++)
    {
      rot(1,0)=-(*this)(k,k-1);  //rows 1..top-1 cols 0..top-2
      if (rot(1,0)!=0){
	double t=(*this)(k-1,k-1); //rows 0..top-2 cols 0..top-2
	rot(1,1)=t*t+rot(1,0)*rot(1,0);
	if (rot(1,1)!=0){
	  rot(1,1)=1/sqrt(rot(1,1));
	  rot(1,0)*=rot(1,1);
	  rot(1,1)*=t;
	}
	else {
	  rot(1,1)=1;
	  rot(1,0)=0;
	}
	rot(0,0)=rot(1,1);
	rot(0,1)=-rot(1,0);
	offset(((RowInc+1)*(k-1)),2,c-k+1);  //rows 0..top-1 cols 0..Columns-1 
	a.multiply_(rot,*this);
	*this=a;
	offset(-((RowInc+1)*(k-1)),r,c);
	//reset(r,c);
	rot(1,0)=rot(0,1);
	rot(0,1)=-rot(1,0);
	q.offset((k-1),qrows,2); //columns 0..top-1
	q*=rot;
	q.offset(-(k-1),qrows,qcols);
	//q.reset(Rows,Rows);
	(*this)(k,k-1)=0;   //rows 1..top-1 cols 0..top-2
      }
    } 
}

void Matrix::decompQR(){
  Matrix u(1,Rows);
  Matrix b(1,Rows);
  long irowinit=(RowInc*2);
  for (long k=2; k<Rows; k++) 
    {
      u(0,k-2)=0;
      long irow=irowinit;//(RowInc*k)+k-2;
      double d=0;
      double t=Element[irow-RowInc];
      for (long i=k; i<Rows; i++,irow+=RowInc)
	{
	  u(0,i)=Element[irow];
	  d+=u(0,i)*u(0,i);
	}
      if (d>0){
      u(0,k-1)=t+sqrt(t*t+d);
      d+=u(0,k-1)*u(0,k-1);
      d/=2;
      u/=sqrt(d);
      b.multiply_(u,*this);
      u.Rows=Rows;
      u.Columns=1;
      u.RowInc=1;
      (*this).subtractProduct_(u,b);
      u.Columns=Rows;
      u.Rows=1;
      u.RowInc=Rows;
      }
      irow=irowinit;
      for (long i=k; i<Rows; i++,irow+=RowInc)
	Element[irow]=0;
      irowinit+=RowInc+1;
    }
  Matrix q(Rows);
  factorQR(q);
}
void Matrix::hessenberg(Matrix & h)
{
  h.reallocate(Rows);
  Matrix u(1,Rows);
  Matrix a(Rows,1);
  Matrix b(1,Rows);
  h=1;
  long irowinit=(RowInc*2);
  for (long k=2; k<Rows; k++) 
    {
      u(0,k-2)=0;
      long irow=irowinit;//(RowInc*k)+k-2;
      double d=0;
      double t=Element[irow-RowInc];
      for (long i=k; i<Rows; i++,irow+=RowInc)
	{
	  u(0,i)=Element[irow];
	  d+=u(0,i)*u(0,i);
	}
      if (d>0){
      u(0,k-1)=t+sqrt(t*t+d);
      d+=u(0,k-1)*u(0,k-1);
      d/=2;
      u/=sqrt(d);
      a.multTranspose_(h,u,2);
      h.subtractProduct_(a,u);
      a.multTranspose_(*this,u,2);
      (*this).subtractProduct_(a,u);
      b.multiply_(u,*this);
      u.Rows=Rows;
      u.Columns=1;
      u.RowInc=1;
      (*this).subtractProduct_(u,b);
      u.Columns=Rows;
      u.Rows=1;
      u.RowInc=Rows;
      }
      irow=irowinit;
      for (long i=k; i<Rows; i++,irow+=RowInc)
	Element[irow]=0;
      irowinit+=RowInc+1;
    }
  
}

void Matrix::symmetricHessenberg(Matrix & h)
{
  h.reallocate(Rows);
  Matrix u(1,Rows);
  Matrix a(Rows,1);
  Matrix b(1,Rows);
  h=1;
  long r=Rows;
  long irowinit=(RowInc*2);
  for (long k=2; k<r; k++) 
    {
      u(0,k-2)=0;
      long irow=irowinit;//(RowInc*k)+k-2;
      double d=0;
      double t=Element[irow-RowInc];
      for (long i=k; i<Rows; i++,irow+=RowInc)
	{
	  u(0,i)=Element[irow];
	  d+=u(0,i)*u(0,i);
	}
      if (d>0){
	double tt=t;
	if (t>0)
	  tt+=sqrt(t*t+d);
	else
	  tt-=sqrt(t*t+d);
	u(0,k-1)=tt;
	double dem=(d+tt*tt);
	dem/=2;
	u.offset(k-1,1,r-k+1);
	u/=sqrt(dem);
	(*this)(k-1,k-2)-=(tt*(d+tt*t))/dem;
	(*this)(k-2,k-1)=(*this)(k-1,k-2);
	offset(k-1,r-k+1);
	a.multTranspose_(*this,u,2);
	(*this).subtractProduct_(a,u);
	b.multiply_(u,*this);
	u.Rows=u.Columns;
	u.Columns=1;
	u.RowInc=1;
	(*this).subtractProduct_(u,b);
	u.reset(1,r);
	u.RowInc=r;
	reset(r,r);
	u.reset(1,r);
	a.multTranspose_(h,u,2);
	h.subtractProduct_(a,u);
      }
      irow=irowinit;
      long krow=(k-2)*RowInc;
      for (long i=k; i<Rows; i++,irow+=RowInc)
	{
	  Element[irow]=0;
	  Element[krow+i]=0;
	}
      irowinit+=RowInc+1;
    }
}
void Matrix::symmetricHessenberg()
{
  Matrix u(1,Rows);
  Matrix a(Rows,1);
  Matrix b(1,Rows);
  long r=Rows;
  long irowinit=(RowInc*2);
  for (long k=2; k<r; k++) 
    {
      u(0,k-2)=0;
      long irow=irowinit;
      double d=0;
      double t=Element[irow-RowInc];
      for (long i=k; i<Rows; i++,irow+=RowInc)
	{
	  u(0,i)=Element[irow];
	  d+=u(0,i)*u(0,i);
	}
      if (d>0){
	double tt=t+sqrt(t*t+d);
	u(0,k-1)=tt;
	double dem=(d+tt*tt);
	dem/=2;
	u.offset(k-1,1,r-k+1);
	u/=sqrt(dem);
	(*this)(k-1,k-2)-=(tt*(d+tt*t))/dem;
	(*this)(k-2,k-1)=(*this)(k-1,k-2);
	offset(k-1,r-k+1);
	a.multTranspose_(*this,u,2);
	(*this).subtractProduct_(a,u);
	b.multiply_(u,*this);
	u.Rows=u.Columns;
	u.Columns=1;
	u.RowInc=1;
	(*this).subtractProduct_(u,b);
	u.reset(1,r);
	u.RowInc=r;
	reset(r,r);
      }
      irow=irowinit;
      long krow=(k-2)*RowInc;
      for (long i=k; i<Rows; i++,irow+=RowInc)
	{
	  Element[irow]=0;
	  Element[krow+i]=0;
	}
      irowinit+=RowInc+1;
    }  
}
void Matrix::eigenize(Matrix &ev, Matrix *lambda)
{
 
  Matrix temp(Columns,ev.Columns); 
  temp.multiply_((*this),ev);
  ev.multiply_((*this),temp);
  ev.orthogonalize();
  if (!lambda)return;  
  temp.multiply_((*this),ev);
  lambda->multTranspose(ev,temp,1);
  for (long i=0; i<lambda->Rows;i++)
    for (long j=i+1; j<lambda->Columns; j++)
      {
	(*lambda)(i,j)=0;
	(*lambda)(j,i)=0;
      }
}

void Matrix::orthogonalize()
{
  Matrix v(Element,Rows,1,RowInc);
  Element++;
  long c=Columns;
  Columns=1;
  Matrix d(1);
  for (long k=0; k<c-1; k++)
    {
      v.normalize();
      for (long i=k+1; i<c; i++)
	{
	  d.multTranspose_(v,*this,1);
	  long jrow=0;
	  for (long j=0; j<Rows; j++,jrow+=RowInc)
	    Element[jrow]-=d.Element[0]*v.Element[jrow];
	  Element++;
	}
      v.Element++;
      Element=v.Element+1;
    }
  
  Element-=(c);
  d.multTranspose_(v,*this,1);
  long jrow=0;
  for (long j=0; j<Rows; j++,jrow+=RowInc)
    v.Element[jrow]-=d.Element[0]*Element[jrow];
  
  v.normalize();
  Columns=c;

}
void Matrix::threshold(double pthreshold)
{
  long top=RowInc*Rows;
  for (long i=0; i<top;i+=RowInc)
    for (int j=0;j<Columns; j++)
      if ((Element[i+j]<pthreshold)&&(Element[i+j]>-pthreshold))
	  Element[i+j]=0;
}
void Matrix::normalize()
{
  long c=Columns;
  Columns=1;
  Matrix d(1);
  for (long k=0; k<c; k++)
    {
      d.multTranspose_(*this,*this,1);
      double s=sqrt(d(0,0));
      long jrow=0;
      if(s!=0)
	for (long j=0; j<Rows; j++,jrow+=RowInc)
	  Element[jrow]/=s;
      Element++;
    }
  Columns=c;
  Element-=(c);
}
int  Matrix::convertInformationT2L(Cure::Matrix &pe,
				    Cure::Matrix &rn)
{
  if (Rows!=Columns)return 1;
  int rtype=1;  
  if ((rn.Rows!=Rows)||(rn.Rows!=rn.Columns)){
    rn.reallocateZero(Rows);
    rtype=0;
  }

  if ((pe.Rows==Rows)&&(pe.Rows==pe.Columns)){
    if (rtype){
      Matrix an(*this);
      an+=rn;
      an+=pe;
      if (an.invert()){
	Cure::Matrix lam,ev,v(Rows);
	an.symmetricEigen(lam,ev);
	double min=1E-12;
	for (int i=0;i<an.Rows;i++)
	  {
	    double d=1E12;;
	    if (lam(i,i)>min)
	      d=1/lam(i,i);
	    for (int j=0;j<ev.Rows;j++)
	      v(i,j)=ev(j,i)*d;
	  }
	an.multiply_(ev,v);
      }
      an*=pe;
      Matrix tp(*this);
      this->multTranspose_(an,tp,1);      
      //this->addProduct_(tp,an);
      //(*this)/=2.0;
      symetrize();
      rn=pe;
      rn-=(*this);
      rn.subtractProduct_(pe,an);
      rn.symetrize();
    }else{
      (*this)+=pe;
      symetrize();
    }
  }
  return 0;
}

} // namespace Cure

std::ostream& operator<<(std::ostream& os, const Cure::Matrix& m)
{
  long irow=0;
  os << "[";
  for (long i=0; i<m.Rows; i++,irow+=m.RowInc) {
    for (long j=0; j<m.Columns; j++)
      os << m.Element[irow+j]<<" ";
    if (i<(m.Rows-1)) os<<endl;
  }
  os << "]; %Rows/cols/RowInc:" 
     << m.Rows << " " 
     << m.Columns << " " 
     << m.RowInc << endl;
  return os;
}
