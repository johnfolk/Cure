// = RCSID
//    $Id: SparseMatrix.cc,v 1.2 2009/11/03 21:40:41 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
// = COPYRIGHT
//    Copyright (c) 2003 John Folkesson
//

#include "SparseMatrix.hh"
#include "LongArray.hh"

#ifndef DEPEND
//#include <fstream>
#include <math.h>
#include <string.h>
#endif
#ifndef CURESPARSEMATRIXDEBUG
#define CURESPARSEMATRIXDEBUG 140
#endif

using namespace std;

namespace Cure {




void SparseMatrix::setupElement(const int r, const int c)
{
  clear();
  if (r<1) m_Rows=0;
  else  m_Rows=r;
  if (c<0)m_Columns=m_Rows; 
  else  m_Columns=c;
  m_Row.grow(m_Rows);
  m_Column.grow(m_Columns);
}

SparseMatrix::SparseMatrix()
{
  m_Rows=0;
  m_Columns=0;
  m_NextIndex=0;
  m_Elements=0;
  m_NumberOfElements=0;
}
SparseMatrix::SparseMatrix(const SparseMatrix& mat)
{
  m_Rows=0;
  m_Columns=0;
  m_Elements=0;
  m_NumberOfElements=0;
  *this=mat;
  return;
}
SparseMatrix::SparseMatrix(const int r, const int c)
{
  m_Rows=0;
  m_Columns=0;
  m_Elements=0;
  m_NumberOfElements=0;
  reallocate(r,c);
}

SparseMatrix::~SparseMatrix()
{
  if (m_Elements){
    for (long i=0;i<m_NumberOfElements;i++)
      if (m_Allocated(0,i))
	delete [] m_Elements[i];
    delete []m_Elements;
  }
  m_Elements =0;
  m_NumberOfElements=0;
}
void SparseMatrix::grow(const int r,const int c)
{
  m_Row.grow(r);
  m_Column.grow(c);
  if ((r>=m_Rows)&&(c>=m_Columns))
    {
      m_Rows=r;
      m_Columns=c;
      return;
    }
  if (r<m_Rows){
    for (long i=0;i<c;i++){
      long k=m_Column.columns(i);
      for (long j=k-2;j>-1;j-=2){
	if (m_Column(i,j)>=r){
	  m_Column.removeCell(i,j);
	  m_Column.removeCell(i,j);
	}else break;
      }
    }
  }
  if (c<m_Columns){
    for (long i=0;i<c;i++){
      long k=m_Row.columns(i);
      for (long j=k-2;j>-1;j-=2){
	if (m_Row(i,j)>=c){
	  m_Row.removeCell(i,j);
	  m_Row.removeCell(i,j);
	}else break;
      }
    }
  } 
  m_Rows=r;
  m_Columns=c;
}

void SparseMatrix::operator = (const double d) 
{
  
  if (d!=0)
    {
      long top=m_Rows;
      if (m_Columns<m_Rows)top=m_Columns;
      for (long i=0; i<top; i++)
	setValue(i,i,d);
    }
}
void SparseMatrix::operator += ( SparseMatrix & mat) 
{
#if CURESPARSEMATRIXDEBUG >10
  if( m_Columns!=mat.m_Columns || m_Rows!=mat.m_Rows ) 
    {
      cerr << "\nSPARSEMATRIX ERROR Incompatible matrices for +=, not same size\n";
      return;
    }
#endif 
  for (long j=0;j<m_Rows;j++){
    long c=mat.m_Row.columns(j);
    for (long i=0;i<c;i+=2){
      addValue(j,mat.m_Row(j,i),mat.getValue(mat.m_Row(j,i+1)));
    }
  }
}

void SparseMatrix::operator -= (SparseMatrix & mat) 
{
#if CURESPARSEMATRIXDEBUG >10
  if( m_Columns!=mat.m_Columns || m_Rows!=mat.m_Rows ) 
    {
      cerr << "\nSPARSEMATRIX ERROR Incompatible matrices for -=, not same size\n";
      return;
    }
#endif 
    for (long j=0;j<m_Rows;j++){
    long c=mat.m_Row.columns(j);
    for (long i=0;i<c;i+=2){
      addValue(j,mat.m_Row(j,i),-mat.getValue(mat.m_Row(j,i+1)));
    }
  }
}



void SparseMatrix::arrayMult(SparseMatrix& b)
{
#if CURESPARSEMATRIXDEBUG >20
  if (((m_Rows!=b.m_Rows)||(m_Columns!=b.m_Columns)))
    {
      cerr << "\nSPARSEMATRIX ERROR Incompatible matrices for arrayMult\n";
      return;
    }
#endif
  for (long i=0; i<m_Rows; i++){
    long c=m_Row.columns(i);
    for (int j=0; j<c; j+=2)
      setValue(i,m_Row(i,j),getValue(m_Row(i,j+1)*b(i,m_Row(i,j))));
  }
}
void SparseMatrix::arrayDivide( SparseMatrix& b)
{
#if CURESPARSEMATRIXDEBUG >20
  if (((m_Rows!=b.m_Rows)||(m_Columns!=b.m_Columns)))
    {
      cerr << "\nSPARSEMATRIX ERROR Incompatible matrices for arrayMult\n";
      return;
    }
#endif
  for (long i=0; i<m_Rows; i++){
    long c=m_Row.columns(i);
    for (int j=0; j<c; j+=2)
      setValue(i,m_Row(i,j),getValue(m_Row(i,j+1)/b(i,m_Row(i,j))));
  }
}
void SparseMatrix::multTranspose_( SparseMatrix& a, 
				  SparseMatrix& b, int which)
{
  if (which==0){
    multiply_(a,b);
    return;
  }
  clear();
  if (which==1){
    reallocate(a.m_Columns,b.m_Columns);
    long i,r,k,k2,c,col;
    for (i=0;i<a.m_Columns;i++){
      r=a.m_Column.columns(i);
      if (r>0)
	for (k=0;k<r;k+=2){
	  col=a.m_Column(i,k);
	  c=b.m_Row.columns(col);
	  if (c>0){
	    for (k2=0;k2<c;k2+=2){
	      addValue(i,b.m_Row(col,k2),
		       (a.getValue(a.m_Column(i,k+1))*
			b.getValue(b.m_Row(col,k2+1))));
	    }
	    
	  }
	}
    }
    return;
  }
  if (which==3){
    reallocate(a.m_Columns,b.m_Rows);
    long i,r,k,k2,c,col;
    for (i=0;i<a.m_Columns;i++){
      r=a.m_Column.columns(i);
      if (r>0)
	for (k=0;k<r;k+=2){
	  col=a.m_Column(i,k);
	  c=b.m_Column.columns(col);
	  if (c>0){
	    for (k2=0;k2<c;k2+=2){
	      addValue(i,b.m_Column(col,k2),
		       (a.getValue(a.m_Column(i,k+1))*
			b.getValue(b.m_Column(col,k2+1))));
	    }
	    
	  }
	}
    }
    return;
  }
  if (which==2){
    reallocate(a.m_Rows,b.m_Rows);
    long i,r,k,k2,c,col;
    for (i=0;i<a.m_Rows;i++){
      r=a.m_Row.columns(i);
      if (r>0)
	for (k=0;k<r;k+=2){
	  col=a.m_Row(i,k);
	  c=b.m_Column.columns(col);
	  if (c>0){
	    for (k2=0;k2<c;k2+=2){
	      addValue(i,b.m_Column(col,k2),
		       (a.getValue(a.m_Row(i,k+1))*
			b.getValue(b.m_Column(col,k2+1))));
	    }
	    
	  }
	}
    } 
  }
}
void SparseMatrix::reorder(double threshold )
{
  if (m_NextIndex<2)return;
  m_TempL=(m_NextIndex);
  m_TempL=(m_TempL>>10);
  m_TempL2=(m_NextIndex&0x3FF);
  if (m_TempL2>0)m_TempL++;
  double **d=new double*[m_TempL];
  m_TempL2=(m_TempL<<10);
  d[0]=new double[m_TempL2];
  for (m_TempL2=1; m_TempL2<m_TempL;m_TempL2++)
    d[m_TempL2]=d[m_TempL2-1]+1024;
  m_TempL2=0;
  m_Column.clear();
  for (long j=0; j<m_Rows; j++){
    long k=m_Row.columns(j);
    for (long i=0;i<k;i+=2){
      double dd=getValue(m_Row(j,i+1));
      if ((dd>threshold)||(dd<-threshold)){
	d[0][m_TempL2]=dd;
	m_Row(j,i+1)=m_TempL2;
	m_Column.addPairOrdered(m_Row(j,i),j,m_TempL2);
	m_TempL2++;
      }else {
	m_Row.removeCell(j,i);
	m_Row.removeCell(j,i);
	k-=2;
      }
    }
  }
  m_NextIndex=m_TempL2;
  for (long j=0;j<m_NumberOfElements;j++)
    if (m_Allocated(0,j))
      delete []m_Elements[j];
  if (m_Elements)delete[]m_Elements;
  m_Elements=d;
  m_NumberOfElements=m_TempL;
  m_Allocated.grow(1,0);
  m_Allocated.grow(1,m_TempL,false);
  m_Allocated.setBit(0,0,true);
}
double SparseMatrix::determinant( void )
{      
  double det = 0.;
#if CURESPARSEMATRIXDEBUG >10
  if (m_Rows != m_Columns)
    {
      cerr << "ERROR SparseMatrix::determinant: non-square sparsematrix\n";
      return 0.0;
    }   
#endif 
  if (m_Rows == 1)
    return (*this)(0,0);              // trivial
  if (m_Rows == 2)                   // do
    {
      det  =  (*this)(0,0)*(*this)(1,1);
      det -=  (*this)(0,1)*(*this)(1,0);
      return det;
    }
  if (m_Rows == 3)
    {
      det  = (*this)(0,0)*(*this)(1,1)*(*this)(2,2);
      det += (*this)(0,1)*(*this)(1,2)*(*this)(2,0);
      det += (*this)(0,2)*(*this)(1,0)*(*this)(2,1);
      det -= (*this)(0,0)*(*this)(1,2)*(*this)(2,1);
      det -= (*this)(0,1)*(*this)(1,0)*(*this)(2,2);
      det -= (*this)(0,2)*(*this)(1,1)*(*this)(2,0);
      return det;
    }
  SparseMatrix lam(*this);
  lam.triangle();
  det=1;
  for (long i=0; i<m_Rows; i++)
    det*=lam(i,i);
  return det;
}

void SparseMatrix::transpose(const SparseMatrix b)
{
  (*this)=b;
  transpose();
}
void SparseMatrix::transpose_(const SparseMatrix & b )
{
  (*this)=b;
  transpose();
}
void SparseMatrix::transpose( void )
{
  LongArray tr(m_Row);
  m_Row=m_Column;
  m_Column=tr;
  m_Rows=m_Columns;
  m_Columns=m_Column.rows();
} 
void  SparseMatrix::save(const char *filename)
{
  std::fstream fsout;
  fsout.open(filename, ios::out);
  for (long i=0; i<m_Rows; i++)
    {
      for (long j=0; j<m_Columns; j++)
	{
	  fsout<<(*this)(i,j)<<" ";
	}
      fsout<<endl;
    }
  fsout.close();
}
void SparseMatrix::symetrize()
{
  if(m_Rows!=m_Columns)return;
  for (long i=0; i<m_Rows; i++)
    {
      long r=m_Row.columns(i);
      for (long k=r-2;k>-1;k-=2){
	long j=m_Row(i,k);
	if (j>i){
	  double *p=getPointer(i,j);
	  p[0]+=(*this)(j,i);
	  p[0]/=2;
	  setValue(j,i,p[0]);
	}else break;
      }
    }
}




double SparseMatrix::trace()
{
  double d=0;
  long top=m_Rows;
  if (top>m_Columns)top=m_Columns;
  for (m_TempL2=0; m_TempL2<top;m_TempL2++)
    d+=(*this)(m_TempL2,m_TempL2);
  return d;
}

void SparseMatrix::deleteRow(long r)
{
  if ((r<0)||(r>=m_Rows))
    {
#if CURESPARSEMATRIXDEBUG >20
      std::cerr<<"ERRROR SparseMatrix::deleteRow out of bounds\n";
#endif
      return;
    }
  long c=m_Row.columns(r);
  for (long j=0;j<c;j+=2){
    long k=m_Column.findPairOrdered(m_Row(r,j),r);
    if(k!=-1){
      m_Column.removeCell(m_Row(r,j),k);
      m_Column.removeCell(m_Row(r,j),k);
    }
  }
  m_Row.deleteRows(r);
  m_Rows--;
}
void SparseMatrix::deleteColumn(long c)
{
#if CURESPARSEMATRIXDEBUG >20
  if ((c<0)||(c>=m_Columns))
    {
      std::cerr<<"ERRROR SparseMatrix::deleteColumn out of bounds\n";
      return;
    }
#endif
  long r=m_Column.columns(c);
  for (long j=0;j<r;j+=2){
    long k=m_Row.findPairOrdered(m_Column(c,j),c);
    if(k!=-1){
      m_Row.removeCell(m_Column(c,j),k);
      m_Row.removeCell(m_Column(c,j),k);
    }
  }
  m_Column.deleteRows(c);
  m_Columns--;
}
void SparseMatrix::deleteColumns(long startcolumn, long numberofcolumns )
{
  if (numberofcolumns==0)return;
  if (numberofcolumns<0)return insertColumns(startcolumn,-numberofcolumns);
#if CURESPARSEMATRIXDEBUG >20
  if ((startcolumn<0)||
       ((startcolumn+numberofcolumns)>m_Columns))
    {
      std::cerr<<"ERRROR SparseMatrix::deletem_Columns out of bounds\n";
      return;
    }
  #endif
  numberofcolumns+=startcolumn;
  while (startcolumn<numberofcolumns){
    deleteColumn(startcolumn);
    numberofcolumns--;
  }
}
void SparseMatrix::deleteRows(long startrow, long numberofrows )
{
  if (numberofrows==0)return;
  if (numberofrows<0)return insertRows(startrow,-numberofrows);
#if CURESPARSEMATRIXDEBUG >20
  if ((startrow<0)||
       ((startrow+numberofrows)>m_Rows))
    {
      std::cerr<<"ERRROR SparseMatrix::deleteRows out of bounds\n";
      return;
    }
  #endif
  numberofrows+=startrow;
  while (startrow<numberofrows){
    deleteRow(startrow);
    numberofrows--;
  }
}
void SparseMatrix::insertColumn(long startcolumn){
  insertColumns(startcolumn,1);
}
void SparseMatrix::insertRow(long startRow)
{
  insertRows(startRow,1);
}
 

void SparseMatrix::insertColumns(long startcolumn, long numberofcolumns )
{ 
  if (numberofcolumns==0)return;
  if (numberofcolumns<0)return deleteColumns(startcolumn,-numberofcolumns);
#if CURESPARSEMATRIXDEBUG >20
  if ((startcolumn<0)||
      ((startcolumn)>m_Columns))
    {
      std::cerr<<"ERRROR SparseMatrix::insertColumns out of bounds\n";
      return;
    }
#endif

  for (long j=startcolumn;j<m_Columns;j++){
    long k=m_Column.columns(j);
    for (long i=0;i<k;i+=2){
      long c=m_Row.findPairOrdered(m_Column(j,i),j);
      if (c!=-1){
	m_Row(m_Column(j,i),c)+=numberofcolumns;
      }
    }
  }
  m_Column.insertRows(startcolumn,numberofcolumns);
  m_Columns+=numberofcolumns;
}

void SparseMatrix::insertRows(long startrow, long numberofrows )
{ 
  if (numberofrows==0)return;
  if (numberofrows<0)return deleteRows(startrow,-numberofrows);
#if CURESPARSEMATRIXDEBUG >20
  if ((startrow<0)||
      ((startrow)>m_Rows))
    {
      std::cerr<<"ERRROR SparseMatrix::insertRows out of bounds\n";
      return;
    }
#endif

  for (long j=startrow;j<m_Rows;j++){
    long k=m_Row.columns(j);
    for (long i=0;i<k;i+=2){
      long c=m_Column.findPairOrdered(m_Row(j,i),j);
      if (c!=-1){
	m_Column(m_Row(j,i),c)+=numberofrows;
      }
    }
  }
  m_Row.insertRows(startrow,numberofrows);
  m_Rows+=numberofrows;
}

void SparseMatrix::swapRows(long r1,long r2)
{
  long k=m_Row.columns(r1);
  for (long i=0;i<k;i+=2){
    long c=m_Column.findPairOrdered(m_Row(r1,i),r1);
    if (c!=-1){
      m_Column.removeCell(m_Row(r1,i),c);
      m_Column.removeCell(m_Row(r1,i),c);
      m_Column.addPairOrdered(m_Row(r1,i),r2,m_Row(r1,i+1));
    }
  }
   k=m_Row.columns(r2);
  for (long i=0;i<k;i+=2){
    long c=m_Column.findPairOrdered(m_Row(r2,i),r2);
    if (c!=-1){
      m_Column.removeCell(m_Row(r2,i),c);
      m_Column.removeCell(m_Row(r2,i),c);
      m_Column.addPairOrdered(m_Row(r2,i),r1,m_Row(r2,i+1));
    }
  }
  m_Row.swapRows(r1,r2);
}

void SparseMatrix::swapColumns(long r1,long r2)
{
  long k=m_Column.columns(r1);
  for (long i=0;i<k;i+=2){
    long c=m_Row.findPairOrdered(m_Column(r1,i),r1);
    if (c!=-1){
      m_Row.removeCell(m_Column(r1,i),c);
      m_Row.removeCell(m_Column(r1,i),c);
      m_Row.addPairOrdered(m_Column(r1,i),r2,m_Column(r1,i+1));
    }
  }
  k=m_Column.columns(r2);
  for (long i=0;i<k;i+=2){
    long c=m_Row.findPairOrdered(m_Column(r2,i),r1);
    if (c!=-1){
      m_Row.removeCell(m_Column(r2,i),c);
      m_Row.removeCell(m_Column(r2,i),c);
      m_Row.addPairOrdered(m_Column(r2,i),r1,m_Column(r2,i+1));
    }
  }
  m_Column.swapRows(r1,r2);
}




int SparseMatrix::moveRows(unsigned long r1,unsigned long r2, unsigned short num)
{
  if (r1==r2)return 0;
  if (r1>r2){
    if ((long)(r1+num)>m_Rows)return 1;
    long top=r2+num;
    if ((top)>(long) r1)return 1;
    while ((long)r2<top){
      swapRows(r2,r1);
      r1++;
      r2++;
    } 
    return 0;
  }
  return moveRows(r2,r1,num);
}


int SparseMatrix::moveColumns(unsigned long r1,unsigned long r2, unsigned short num)
{
  if (r1==r2)return 0;
  if (r1>r2){
    if ((long)(r1+num)>m_Columns)return 1;
    long top=r2+num;
    if ((top)>(long)r1)return 1;
    while ((long) r2<top){
      swapColumns(r2,r1);
      r1++;
      r2++;
    } 
    return 0;
  }
  return moveColumns(r2,r1,num);
}


unsigned long
SparseMatrix::tryBackSubstitute(Matrix &x, 
				const Matrix &rhs,
				double oneovermaxx,
				unsigned long stoprow, 
				long *permute)
{
  if (x.Rows!=m_Columns)return m_Columns;
  if (x.Columns!=rhs.Rows)return m_Columns;
  if (rhs.Columns!=m_Rows)return m_Columns;
  if (rhs.Rows<1)return stoprow;
  long matcol=m_Columns-1;
  while (matcol>=(long)stoprow)
    {
      long permcol=permute[matcol];
      double t=(*this)(matcol,permcol);
      for (unsigned long xmatcol=0;(long)xmatcol!=x.Columns;xmatcol++)
	x(permcol,xmatcol)=rhs(xmatcol,matcol);
      
      unsigned long topr=m_Row.columns(matcol);
      if (topr){
	long *mr=&m_Row(matcol,0);
	for (unsigned long j=0;j<topr;j+=2) {
	  long rcol=mr[j];
	  if (rcol!=permcol){
	    double d=getValue(mr[j+1]);
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
SparseMatrix::tryBackSubstitute(SparseMatrix &x, 
				SparseMatrix &rhs,
				double oneovermaxx,
				unsigned long stoprow, 
				long *permute)
{
  if (x.m_Rows!=m_Columns)return m_Columns;
  if (x.m_Columns!=rhs.m_Rows)return m_Columns;
  if (rhs.m_Columns!=m_Rows)return m_Columns;
  if (rhs.m_Rows<1)return stoprow;
  long matcol=m_Columns-1;
  while (matcol>=(long)stoprow)
    {
      long permcol=permute[matcol];
      double t=(*this)(matcol,permcol);
      for (unsigned long xmatcol=0;(long)xmatcol!=x.m_Columns;xmatcol++)
	x.setValue(permcol,xmatcol,rhs(xmatcol,matcol));
		   
      unsigned long topr=m_Row.columns(matcol);
      if (topr){
	long *mr=&m_Row(matcol,0);
	for (unsigned long j=0;j<topr;j+=2) {
	  long rcol=mr[j];
	  if (rcol!=permcol){
	    double d=getValue(mr[j+1]);
	    if (d!=0)
	      for (unsigned long xmatcol=0;(long)xmatcol!=x.m_Columns;xmatcol++)
		x.addValue(permcol,xmatcol,-d*x(rcol,xmatcol));
	  }
	}
      }
      for (unsigned long xmatcol=0;(long)xmatcol!=x.m_Columns;xmatcol++){
	double *p=x.getPointer(permcol,xmatcol); 
	if (p)
	  if (p[0]!=0){
	    double min=(p[0]*oneovermaxx);
	    if (min<0)min=-min;
	    if (min<1E-20)min=1E-20;
	    if (t>=0){
	      if(t<min)
		return matcol+1;
	    }else if (t>-min){
	      return matcol+1;
	    }
	    p[0]/=t;
	  }
      }
      matcol--;
    }
  return stoprow;
}
unsigned long
SparseMatrix::tryBackSubstitute(Matrix &x, 
				const Matrix &rhs,
				double oneovermaxx,
				unsigned long stoprow)
{
  if (x.Rows!=m_Columns)return m_Columns;
  if (x.Columns!=rhs.Rows)return m_Columns;
  if (rhs.Columns!=m_Rows)return m_Columns;
  if (rhs.Rows<1)return stoprow;
  long matcol=m_Columns-1;
  while (matcol>=(long)stoprow)
    {
      double t=(*this)(matcol,matcol);
      for (unsigned long xmatcol=0;(long)xmatcol!=x.Columns;xmatcol++)
	x(matcol,xmatcol)=rhs(xmatcol,matcol);
      
      unsigned long topr=m_Row.columns(matcol);
      if (topr){
	long *mr=&m_Row(matcol,0);
	for (unsigned long j=0;j<topr;j+=2) {
	  long rcol=mr[j];
	  if (rcol!=matcol){
	    double d=getValue(mr[j+1]);
	    if (d!=0)
	      for (unsigned long xmatcol=0;(long)xmatcol!=x.Columns;xmatcol++)
		x(matcol,xmatcol)-=d*x(rcol,xmatcol);
	  }
	}
      }
      for (unsigned long xmatcol=0;(long)xmatcol!=x.Columns;xmatcol++)
	if (x(matcol,xmatcol)!=0){
	  double min=(x(matcol,xmatcol)*oneovermaxx);
	  if (min<0)min=-min;
	  if (min<1E-20)min=1E-20;
	  if (t>=0){
	    if(t<min)
	      return matcol+1;
	  }else if (t>-min){
	    return matcol+1;
	  }
	  if (x(matcol,xmatcol)!=0)
	    x(matcol,xmatcol)/=t;
	}
      matcol--;
    }
  return stoprow;
}
void SparseMatrix::doQR(Matrix & q, long *permute, 
		  long startcol)
{
  long qrows=q.Rows;
  long qcols=q.Columns;
  Matrix rot(2),a(2,m_Columns);
  long top=m_Rows;
  if (top>m_Columns+1)top=m_Columns+1;
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
	bool fini[m_Columns];
	if (m_Columns>0)
	  memset(&fini,0,m_Columns*sizeof(bool));
	long row=k-1;
	unsigned long topr=m_Row.columns(row);
	if (topr){
	  long *mr=&m_Row(row,0);
	  for (unsigned long j=0;j<topr;j+=2) {
	    long rcol=mr[j];
	    fini[rcol]=true;
	    //long col=permute[rcol];
	    double e1=getValue(mr[j+1]);//(*this)(row,rcol);
	    double e2=(*this)(k,rcol);
	    double tempa=rot(0,0)*e1+
	      rot(0,1)*e2;
	    double tempb=rot(1,0)*e1+
	      rot(1,1)*e2;
	    setValue(k,rcol,tempb);
	    setValue(row,rcol,tempa);
	  }
	}
	row=k;
	topr=m_Row.columns(row);
	if (topr){
	  long *mr=&m_Row(row,0);
	  for (unsigned long j=0;j<topr;j+=2) {
	    long rcol=mr[j];
	    if (!fini[rcol]){
	      fini[rcol]=true;
	      //long col=permute[rcol];
	      double e1=(*this)(k-1,rcol);
	      double e2=getValue(mr[j+1]);//(*this)(k,rcol);
	      double tempa=rot(0,0)*e1+
		rot(0,1)*e2;
	      double tempb=rot(1,0)*e1+
		rot(1,1)*e2;
	      setValue(k-1,rcol,tempa);
	      setValue(row,rcol,tempb);
	    }
	  }
	}	  
	rot(1,0)=rot(0,1);
	rot(0,1)=-rot(1,0);
	q.offset((k-1),qrows,2); //columns 0..top-1
	q*=rot;
	q.offset(-(k-1),qrows,qcols);
	setToZero(k,kminus1);
      }
    } 
}
void SparseMatrix::doQR(Matrix & q, long startcol)
{
  long qrows=q.Rows;
  long qcols=q.Columns;
  Matrix rot(2),a(2,m_Columns);
  long top=m_Rows;
  if (top>m_Columns+1)top=m_Columns+1;
  for (long k=startcol+1; k<top; k++)
    {
      long kminus1=(k-1);
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
	bool fini[m_Columns];
	if (m_Columns>0)
	  memset(&fini,0,m_Columns*sizeof(bool));
	long row=k-1;
	unsigned long topr=m_Row.columns(row);
	if (topr){
	  long *mr=&m_Row(row,0);
	  for (unsigned long j=0;j<topr;j+=2) {
	    long rcol=mr[j];
	    fini[rcol]=true;
	    double e1=getValue(mr[j+1]);//(*this)(row,rcol);
	    double e2=(*this)(k,rcol);
	    double tempa=rot(0,0)*e1+
	      rot(0,1)*e2;
	    double tempb=rot(1,0)*e1+
	      rot(1,1)*e2;
	    setValue(k,rcol,tempb);
	    setValue(row,rcol,tempa);
	  }
	}
	row=k;
	topr=m_Row.columns(row);
	if (topr){
	  long *mr=&m_Row(row,0);
	  for (unsigned long j=0;j<topr;j+=2) {
	    long rcol=mr[j];
	    if (!fini[rcol]){
	      fini[rcol]=true;
	      double e1=(*this)(k-1,rcol);
	      double e2=getValue(mr[j+1]);//(*this)(k,rcol);
	      double tempa=rot(0,0)*e1+
		rot(0,1)*e2;
	      double tempb=rot(1,0)*e1+
		rot(1,1)*e2;
	      setValue(k-1,rcol,tempa);
	      setValue(row,rcol,tempb);
	    }
	  }
	}	  
	rot(1,0)=rot(0,1);
	rot(0,1)=-rot(1,0);
	q.offset((k-1),qrows,2); //columns 0..top-1
	q*=rot;
	q.offset(-(k-1),qrows,qcols);
	setToZero(k,kminus1);
      }
    } 
}
void SparseMatrix::doQR()
{
  Matrix rot(2),a(2,m_Columns);
  long top=m_Rows;
  if (top>m_Columns+1)top=m_Columns+1;
  for (long k=1; k<top; k++)
    {
      long kminus1=(k-1);
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
	bool fini[m_Columns];
	if (m_Columns>0)
	  memset(&fini,0,m_Columns*sizeof(bool));
	long row=k-1;
	unsigned long topr=m_Row.columns(row);
	if (topr){
	  long *mr=&m_Row(row,0);
	  for (unsigned long j=0;j<topr;j+=2) {
	    long rcol=mr[j];
	    fini[rcol]=true;
	    double e1=getValue(mr[j+1]);//(*this)(row,rcol);
	    double e2=(*this)(k,rcol);
	    double tempa=rot(0,0)*e1+
	      rot(0,1)*e2;
	    double tempb=rot(1,0)*e1+
	      rot(1,1)*e2;
	    setValue(k,rcol,tempb);
	    setValue(row,rcol,tempa);
	  }
	}
	row=k;
	topr=m_Row.columns(row);
	if (topr){
	  long *mr=&m_Row(row,0);
	  for (unsigned long j=0;j<topr;j+=2) {
	    long rcol=mr[j];
	    if (!fini[rcol]){
	      fini[rcol]=true;
	      double e1=(*this)(k-1,rcol);
	      double e2=getValue(mr[j+1]);//(*this)(k,rcol);
	      double tempa=rot(0,0)*e1+
		rot(0,1)*e2;
	      double tempb=rot(1,0)*e1+
		rot(1,1)*e2;
	      setValue(k-1,rcol,tempa);
	      setValue(row,rcol,tempb);
	    }
	  }
	}	  
	rot(1,0)=rot(0,1);
	rot(0,1)=-rot(1,0);
	setToZero(k,kminus1);
      }
    } 
}
void SparseMatrix::triangle(Matrix & q,long *permute, 
		      long startcol)
{
  long r=m_Rows;
  if (q.Columns!=r)
    q.reallocate(q.Rows,r);
  Matrix u(1,r);
  Matrix a(r,1);
  Matrix b(1,m_Columns);
  LongArray bnotzero;
  bnotzero.makeCell(0,m_Columns);//allocate enough elements
  bnotzero.clear();
  long m[r];//holds u not 0
  unsigned long topm=0;  //holds length of m
  //*first we hessenberg this sparsematrix
  //by householder reflections
  long top=m_Rows;
  if (top>(m_Columns+2)) top=(m_Columns+2);
  for (long k=startcol+2; k<top; k++) 
    {
      long col=permute[k-2];
      u(0,k-2)=0;
      double d=0;
      //sum squares of colum k-2 from row k
      topm=m_Column.columns(col);
      if (topm){
	long *mr=&m_Column(col,0);
	long sk=k-1;
	m[0]=sk;
	long j=1;
	for (unsigned long i=0;i<topm;i+=2)
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
	b.reallocateZero(1,m_Columns);
	bnotzero.clear();
	d=sqrt(d);
	for (unsigned long i=0;i<topm;i++) {
	  long row=m[i];
	  u(0,row)/=d;
	  for (int j=0;j<rr;j++)
	    a(j,0)+=q(j,row)*u(0,row);
	  unsigned long topr=m_Row.columns(row);
	  if (topr){
	    long *mr=&m_Row(row,0);
	    for (unsigned long j=0;j<topr;j+=2) {
	      long rcol=mr[j];
	      if (b(0,rcol)==0)
		bnotzero.addUnique(0,rcol);
	      b(0,rcol)+=u(0,row)*getValue(mr[j+1]);      
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
	      addValue(row,bcol,-u(0,row)*b(0,bcol));
	    }
	  }
	}
	
	for (unsigned long i=1; i<topm; i++){ 
	  setToZero(m[i],col);
	}	
      }//if d==0 then we  needed do anything.
    }
  //Now we can do the QR on the simplier sparsematrix
  doQR(q,permute,startcol);
}
void SparseMatrix::triangle(Matrix & q,long startcol)
{
  long r=m_Rows;
  if (q.Columns!=r)
    q.reallocate(q.Rows,r);
  Matrix u(1,r);
  Matrix a(r,1);
  Matrix b(1,m_Columns);
  LongArray bnotzero;
  bnotzero.makeCell(0,m_Columns);//allocate enough elements
  bnotzero.clear();
  long m[r];//holds u not 0
  unsigned long topm=0;  //holds length of m
  //*first we hessenberg this sparsematrix
  //by householder reflections
  long top=m_Rows;
  if (top>(m_Columns+2)) top=(m_Columns+2);
  for (long k=startcol+2; k<top; k++) 
    {
      long col=(k-2);
      u(0,k-2)=0;
      double d=0;
      //sum squares of colum k-2 from row k
      topm=m_Column.columns(col);
      if (topm){
	long *mr=&m_Column(col,0);
	long sk=k-1;
	m[0]=sk;
	long j=1;
	for (unsigned long i=0;i<topm;i+=2)
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
	b.reallocateZero(1,m_Columns);
	bnotzero.clear();
	d=sqrt(d);
	for (unsigned long i=0;i<topm;i++) {
	  long row=m[i];
	  u(0,row)/=d;
	  for (int j=0;j<rr;j++)
	    a(j,0)+=q(j,row)*u(0,row);
	  unsigned long topr=m_Row.columns(row);
	  if (topr){
	    long *mr=&m_Row(row,0);
	    for (unsigned long j=0;j<topr;j+=2) {
	      long rcol=mr[j];
	      if (b(0,rcol)==0)
		bnotzero.addUnique(0,rcol);
	      b(0,rcol)+=u(0,row)*getValue(mr[j+1]);      
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
	      addValue(row,bcol,-u(0,row)*b(0,bcol));
	    }
	  }
	}
	
	for (unsigned long i=1; i<topm; i++){ 
	  setToZero(m[i],col);
	}	
      }//if d==0 then we  needed do anything.
    }
  //Now we can do the QR on the simplier sparsematrix
  doQR(q,startcol);
}
void SparseMatrix::triangle()
{
  long r=m_Rows;
  Matrix u(1,r);
  Matrix a(r,1);
  Matrix b(1,m_Columns);
  LongArray bnotzero;
  bnotzero.makeCell(0,m_Columns);//allocate enough elements
  bnotzero.clear();
  long m[r];//holds u not 0
  unsigned long topm=0;  //holds length of m
  //*first we hessenberg this sparsematrix
  //by householder reflections
  long top=m_Rows;
  if (top>(m_Columns+2)) top=(m_Columns+2);
  for (long k=2; k<top; k++) 
    {
      long col=(k-2);
      u(0,k-2)=0;
      double d=0;
      //sum squares of colum k-2 from row k
      topm=m_Column.columns(col);
      if (topm){
	long *mr=&m_Column(col,0);
	long sk=k-1;
	m[0]=sk;
	long j=1;
	for (unsigned long i=0;i<topm;i+=2)
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
	b.reallocateZero(1,m_Columns);
	bnotzero.clear();
	d=sqrt(d);
	for (unsigned long i=0;i<topm;i++) {
	  long row=m[i];
	  u(0,row)/=d;
	  unsigned long topr=m_Row.columns(row);
	  if (topr){
	    long *mr=&m_Row(row,0);
	    for (unsigned long j=0;j<topr;j+=2) {
	      long rcol=mr[j];
	      if (b(0,rcol)==0)
		bnotzero.addUnique(0,rcol);
	      b(0,rcol)+=u(0,row)*getValue(mr[j+1]);      
	    }
	  }
	}
	for (unsigned long i=0;i<topm;i++) {
	  long row=m[i];
	  unsigned long topr=bnotzero.columns(0);
	  if (topr){
	    long *mb=&bnotzero(0,0);
	    for (unsigned long j=0;j<topr;j++) {
	      long bcol=mb[j];
	      addValue(row,bcol,-u(0,row)*b(0,bcol));
	    }
	  }
	}
	
	for (unsigned long i=1; i<topm; i++){ 
	  setToZero(m[i],col);
	}	
      }//if d==0 then we  needed do anything.
    }
  //Now we can do the QR on the simplier sparsematrix
  doQR();
}
void SparseMatrix::normalize()
{
  long c;
  double d;
  double *dd;
  for (long i=0;i<m_Rows;i++){
   c=m_Row.columns(i);
   d=0;
   for (long j=0;j<c;j+=2){
     dd=getPointer(m_Row(i,j+1));
     d+=dd[0]*dd[0];
   }
   d=sqrt(d);
   for (long j=0;j<c;j+=2){
     dd=getPointer(m_Row(i,j+1));
     dd[0]/=d;
   }
  }
}


int SparseMatrix::invert(Matrix &inv) 
{
  if (m_Rows!=m_Columns)return 1;
  inv.reallocateZero(m_Rows);
  Matrix q(m_Rows);
  SparseMatrix temp(*this);
  temp.triangle(q);
  //A=QR,  A inv  =I, R inv=Q^T
  return temp.tryBackSubstitute(inv,q,1E-20,0);
}


} // namespace Cure

