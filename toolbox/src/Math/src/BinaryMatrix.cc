//
// = AUTHOR(S)
//    John Folkesson
//    
// = COPYRIGHT
//    Copyright (c) 2005 John Folkesson
//

#include "BinaryMatrix.hh"

#ifndef DEPEND
#include <fstream>
#include <math.h>
#include <string.h>
#endif
#ifndef CUREBINARYMATRIXDEBUG
#define CUREBINARYMATRIXDEBUG 140
#endif

using namespace std;

namespace Cure {

BinaryMatrix::BinaryMatrix()
{
  AllocatedMatrix=0;
  AllocatedSize=0;
  AllocatedRows=0;
  AllocatedColumns=0;
  CurrentRowOffset=0;
  CurrentColOffset=0;  
  Rows=0;
  Columns=0;
  RowInc=0;
}
BinaryMatrix::BinaryMatrix(const BinaryMatrix& mat)
{
  AllocatedMatrix=0;
  AllocatedRows=0;
  AllocatedSize=0;
  AllocatedColumns=0;
  CurrentRowOffset=0;
  CurrentColOffset=0;  
  Rows=0;
  Columns=0;
  RowInc=0;
  *this=mat;
  return;
}
BinaryMatrix::BinaryMatrix(const long r, const long c,bool initvalue)
{
  AllocatedMatrix=0;
  AllocatedSize=0;
  AllocatedRows=0;
  AllocatedColumns=0;
  CurrentRowOffset=0;
  CurrentColOffset=0;  
  Rows=0;
  Columns=0;
  RowInc=0;
  reallocate(r,c, initvalue);
}
BinaryMatrix::~BinaryMatrix()
{
  if (AllocatedMatrix)delete [] AllocatedMatrix;
  AllocatedMatrix=0;
  AllocatedRows=0;
  AllocatedSize=0;
  AllocatedColumns=0;
}

void BinaryMatrix::setupElement(const long r, const long c,bool initvalue)
{
  Columns=c;
  if (c<0)Columns=r;
  CurrentRowOffset=0;
  CurrentColOffset=0;  
  if (r<1) 
    {
      Rows=0;
      RowInc=0;
      AllocatedRows=Rows;
      AllocatedColumns=Columns;
      return;
    }
  Rows=r;
  RowInc=Columns/32;
  if (Columns%32)RowInc++;
  unsigned long n=Rows*RowInc;  
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
	  AllocatedRows=Rows;
	  AllocatedColumns=Columns;
	  if (n>0){
	    if (initvalue)
	      memset(AllocatedMatrix, 0xFFFF, sizeof(unsigned long) * n);
	    else
	      memset(AllocatedMatrix, 0, sizeof(unsigned long) * n);
	  }
	  return;
	}
    }
  if (n>0)
    {
      AllocatedMatrix=new unsigned long[n];
      if (n>0){
	if (initvalue)
	  memset(AllocatedMatrix, 0xFFFF, sizeof(unsigned long) * n);
	else
	  memset(AllocatedMatrix, 0, sizeof(unsigned long) * n);
      }
      AllocatedRows=Rows;
      AllocatedSize=n;
      AllocatedColumns=Columns;
      return;
    }
}
void BinaryMatrix::grow(const long r,const long c, bool value)
{
  BinaryMatrix bm;
  copy(bm);
  AllocatedMatrix=0;
  AllocatedSize=0;
  RowInc=0;
  Rows=0;
  Columns=0;
  reallocate(r,c,value);
  int r2=Rows;
  int c2=Columns;
  if (Columns<bm.Columns)
    bm.offset(0,0,bm.Rows,Columns);
  if (Rows<bm.Rows)
    bm.offset(0,0,Rows, bm.Columns);
  offset(0,0,bm.Rows,bm.Columns);
 (*this)=bm;
  reset(r2,c2);
}
void BinaryMatrix::equalsNot(BinaryMatrix &mat)
{
    if ((Columns!=mat.Columns)||( Rows!=mat.Rows))
      reallocate(mat.Rows,mat.Columns);
    unsigned long top,last;
    unsigned long endcol=getIndex(top,last);
    if ((top==mat.getTop())){
      unsigned long ac=CurrentColOffset+top;
      ac=(ac>>5);
      unsigned long bc=mat.CurrentColOffset+top;
      bc=(bc>>5);
      unsigned long *a=&AllocatedMatrix[(CurrentRowOffset*RowInc)+ac];
      unsigned long b=((mat.CurrentRowOffset*mat.RowInc)+bc);
      for (long i=0; i<Rows; i++){
	for (long j=0; j<(long)top; j++) 
	  setBit(i,j,!mat(i,j));
	for (long j=0; j<(long)endcol; j++)
	  a[i*RowInc+j]=~(mat.AllocatedMatrix[b+i*mat.RowInc+j]);
      for (long j=last; j<Columns; j++) 
	setBit(i,j,!mat(i,j));
      }
    }
  else
    for (long i=0; i<Rows; i++)
      for (long j=0; j<Columns; j++) 
	setBit(i,j,!mat(i,j));
    return;
}

void BinaryMatrix::operator = (bool d) 
{
  unsigned long top,last;
  unsigned long endcol=getIndex(top,last);
  unsigned long ac=CurrentColOffset+top;
  ac=(ac>>5);
  unsigned long *a=&AllocatedMatrix[(CurrentRowOffset*RowInc)+ac];
  long ul=0;
  if (d)ul=0xFFFF;
  for (long i=0; i<Rows; i++){
    for (long j=0; j<(long)top; j++) 
      setBit(i,j,d);
    if (endcol>0)
      memset(a+i*RowInc,ul,(sizeof(unsigned long)*(endcol)));
    for (long j=last; j<Columns; j++) 
	  setBit(i,j,d);
  }
}
void BinaryMatrix::transpose(const BinaryMatrix b)
{
  transpose_(b);
}
void BinaryMatrix::transpose_(const BinaryMatrix & b )
{
  reallocate(b.Columns,b.Rows);
  for (long i=0; i<Rows; i++)
    {
      for (long j=0; j<Columns; j++)
	setBit(i,j,b(j,i));
    }
}
void BinaryMatrix::transpose( void )
{
  BinaryMatrix a(*this);
  transpose_(a);
} 
void  BinaryMatrix::save(const char *filename)
{
  std::fstream fsout;
  fsout.open(filename, ios::out);
  for (long i=0; i<Rows; i++)
    {
      for (long j=0; j<Columns; j++)
	{
	  fsout<<(*this)(i,j)<<" ";
	}
      fsout<<endl;
    }
  fsout.close();
}
void BinaryMatrix::print()
{
  cerr<<" Rows/cols/RowInc: "<<Rows<<" "<<Columns<<" "<<RowInc<<
    " AllocatedRows/Cols: "<<AllocatedRows<<" "<<AllocatedColumns<<" "<<
    " CurrentOffsets: "<<CurrentRowOffset<<" "<<CurrentColOffset<<endl;

  for (long i=0; i<Rows; i++)
    {
      for (long j=0; j<Columns; j++)
	cerr<<(*this)(i,j);
      cerr<<endl;
    }
}


void BinaryMatrix::deleteRow(long r)
{
#if CUREBINARYMATRIXDEBUG >20
  if ((r<0)||(r>=Rows))
    {
      std::cerr<<"ERRROR BinaryMatrix::deleteRow out of bounds\n";
      return;
    }
#endif
  if (r==(Rows-1)){
    Rows--;
    return;
  }
  long newr=Rows-1;
  offset(r+1,0,Rows-r-1,Columns);
  BinaryMatrix cm;
  copy(cm);
  offset(-1,0,Rows,Columns);
  (*this)=cm;
  offset(-r,0,newr,Columns);
  cm.AllocatedMatrix=0;
}
void BinaryMatrix::deleteColumn(long c)
{
#if CUREBINARYMATRIXDEBUG >20
  if ((c<0)||(c>=Columns))
    {
      std::cerr<<"ERRROR BinaryMatrix::deleteColumn out of bounds\n";
      return;
    }
#endif
  if (c==(Columns-1)){
    Columns--;
    return;
  }
  long newc=Columns-1;
  offset(0,c+1,Rows,Columns-c-1);
  BinaryMatrix cm;
  copy(cm);
  offset(0,-1,Rows,Columns);
  move(cm);
  offset(0,-c,Rows,newc);
  cm.AllocatedMatrix=0;
}
void BinaryMatrix::deleteColumns(long startcolumn, long numberofcolumns )
{
  if (numberofcolumns==0)return;
  if (numberofcolumns<0){
    insertColumns(startcolumn,-numberofcolumns);
    return;
  }
#if CUREBINARYMATRIXDEBUG >20
  if ((startcolumn<0)||
      ((startcolumn+numberofcolumns)>Columns))
    {
      std::cerr<<"ERRROR BinaryMatrix::deleteColumns out of bounds\n";
      return;
    }
#endif
  long newc=Columns-numberofcolumns;
  if (startcolumn==newc){
      Columns-=numberofcolumns;
    return;
  }
  offset(0,startcolumn+numberofcolumns,Rows,newc-startcolumn);
  BinaryMatrix cm;
  copy(cm);
  offset(0,-numberofcolumns,Rows,Columns);
  move(cm);
  offset(0,-startcolumn,Rows,newc);
  cm.AllocatedMatrix=0;
}
void BinaryMatrix::deleteRows(long startrow, long numberofrows )
{
  if (numberofrows==0)return;
  if (numberofrows<0){
    insertRows(startrow,-numberofrows);
    return;
  }
#if CUREBINARYMATRIXDEBUG >20
  if ((startrow<0)||
      ((startrow+numberofrows)>Rows))
    {
      std::cerr<<"ERRROR BinaryMatrix::deleteRows out of bounds\n";
      return;
    }
#endif
  long newr=Rows-numberofrows;
  if (startrow==newr){
      Rows-=numberofrows;
    return;
  }
  offset(startrow+numberofrows,0,newr-startrow,Columns);
  BinaryMatrix cm;
  copy(cm);
  offset(-numberofrows,0,Rows,Columns);
  move(cm);
  offset(-startrow,0,newr,Columns);
  cm.AllocatedMatrix=0;
}
void BinaryMatrix::insertColumns(long startcolumn, long numberofcolumns ,bool value)
{ 
  if (numberofcolumns==0)return;
  if (numberofcolumns<0)return deleteColumns(startcolumn,-numberofcolumns);
#if CUREBINARYMATRIXDEBUG >20
  if ((startcolumn<0)||
       ((startcolumn)>Columns))
    {
      std::cerr<<"ERRROR BinaryMatrix::insertColumns out of bounds\n";
      return;
    }
#endif
  BinaryMatrix bm;
  copy(bm);
  AllocatedMatrix=0;
  AllocatedSize=0;
  RowInc=0;
  long c=Columns+numberofcolumns;
  long r=Rows;
  Rows=0;
  Columns=0;
  reallocate(r,c);
  offset(0,0,Rows,startcolumn);
  bm.offset(0,0,Rows,Columns);
  (*this)=bm;
  offset(0,startcolumn,Rows,numberofcolumns);
  (*this)=value;
  offset(0,numberofcolumns,Rows,c-startcolumn-numberofcolumns);
  bm.offset(0,startcolumn,Rows,Columns);
  (*this)=bm;
  offset(0,-(startcolumn+numberofcolumns),Rows,c);
}
void BinaryMatrix::insertRows(long startrow, long numberofrows, bool value)
{ 
  if (numberofrows==0)return;
  if (numberofrows<0)return;
#if CUREBINARYMATRIXDEBUG >20
  if ((startrow<0)||
       ((startrow)>Rows))
    {
      std::cerr<<"ERRROR BinaryMatrix::insertRowss out of bounds\n";
      return;
    }
#endif
  BinaryMatrix bm;
  copy(bm);
  AllocatedMatrix=0;
  AllocatedSize=0;
  RowInc=0;
  long r=Rows+numberofrows;
  Rows=0;
  long c=Columns;
  Columns=0;
  reallocate(r,c);
  offset(0,0,startrow,Columns);
  bm.offset(0,0,Rows,Columns);
  (*this)=bm;
  offset(startrow,0,numberofrows,Columns);
  (*this)=value;
  offset(numberofrows,0,r-startrow-numberofrows,Columns);
  bm.offset(startrow,0,Rows,Columns);
  (*this)=bm;
  offset(-(startrow+numberofrows),0,r,Columns);
}
void BinaryMatrix::swapRows(long r1,long r2)
{
  long r=Rows;
  offset(r1,0,1,Columns);
  BinaryMatrix bm(*this);
  offset(r2-r1,0,1,Columns);
  BinaryMatrix cm;
  copy(cm);
  offset(r1-r2,0,1,Columns);
  (*this)=cm;
  offset(r2-r1,0,1,Columns);
  (*this)=bm;
  offset(-r2,0,r,Columns);
  cm.AllocatedMatrix=0;
}
void BinaryMatrix::swapColumns(long c1,long c2)
{
  long c=Columns;
  offset(0,c1,Rows,1);
  BinaryMatrix bm(*this);
  offset(0,c2-c1,Rows,1);
  BinaryMatrix cm;
  copy(cm);
  offset(0,c1-c2,Rows,1);
  (*this)=cm;
  offset(0,c2-c1,Rows,1);
  (*this)=bm;
  offset(0,-c2,Rows,c);
  cm.AllocatedMatrix=0;
}
void BinaryMatrix::rotate(int angle)
{
  BinaryMatrix bm;
  copy(bm);
  Rows=0;
  Columns=0;
  AllocatedSize=0;
  AllocatedMatrix=0;
  Rows=0;
  Columns=0;
  while (angle>180)angle-=360;
  while (angle<=-180)angle+=360;
  
  if (angle==0){
    (*this)=bm;
    return;
  }
  if (angle==90)
    {
      reallocate(bm.Columns,bm.Rows);
      for (long i=0; i<Rows; i++)
	for (long j=0; j<Columns;j++)
	  setBit(i,j,(bm(j,(Rows-i-1))));
      return;
    }
  if (angle==-90)
    {
      reallocate(bm.Columns,bm.Rows);
      for (long i=0; i<Rows; i++)
	for (long j=0; j<Columns;j++)
	  setBit(i,j,(bm((Columns-j-1),i)));
      return;
    }
  if (angle==180)
    {
      reallocate(bm.Rows,bm.Columns);
      for (long i=0; i<Rows; i++)
	for (long j=0; j<Columns;j++)
	  setBit(i,j,(bm((Rows-i-1),(Columns-j-1))));
      return;
    }
  double a=((M_PI_4*(double)angle)/45.0);
  double origin[2];
  origin[0]=0; 
  origin[1]=0; 
  rotate(bm,a,origin,true,3);
}

long BinaryMatrix::trimRows(bool which)
{
  long ret=0;
  long i=0;
  long r=Rows;
  long c=Columns;
  if (which){
    offset(0,0,1,c);
    while (count()==c){
	i++;
	if (i<r)
	  offset(1,0,1,c);
	else break;
      }
      offset(-i,0,r,c);
 
      deleteRows(0,i);

      r=Rows;
      ret=i;
      i=0;
      if (Rows==0)return ret;
      offset(Rows-1,0,1,Columns);
      while (count()==c){
	i++;
	if (i<r)
	  offset(-1,0,1,c);
	else break;
      }
      offset(-r+1+i,0,r-i,c);
      return ret;
  }
  else{
    offset(0,0,1,c);
      while (count()==0){
	i++;
	if (i<r)
	  offset(1,0,1,c);
	else break;
      }
      offset(-i,0,r,c);
 
      deleteRows(0,i);

      r=Rows;
      ret=i;
      i=0;
      if (Rows==0)return ret;
      offset(Rows-1,0,1,c);
      while (count()==0){
	i++;
	if (i<r)
	  offset(-1,0,1,Columns);
	else break;
      }
      offset(-r+1+i,0,r-i,c);
      return ret;
  }
}
long BinaryMatrix::trimColumns(bool which)
{
  long ret=0;
  long i=0;
  long r=Rows;
  long c=Columns;
  if (which){
    offset(0,0,r,1);
    while (count()==r){
	i++;
	if (i<c)
	  offset(0,1,r,1);
	else break;
      }
      offset(0,-i,r,c);
      deleteColumns(0,i);
      c=Columns;
      ret=i;
      i=0;
      if (c==0)return ret;
      offset(0,Columns-1,r,1);
      while (count()==r){
	i++;
	if (i<c)
	  offset(0,-1,r,1);
	else break;
      }
      offset(0,-c+1+i,r,c-i);
      return ret;
  }
  else{
    offset(0,0,r,1);
    while (count()==0){
	i++;
	if (i<c)
	  offset(0,1,r,1);
	else break;
      }
      offset(0,-i,r,c);
      deleteColumns(0,i);
      c=Columns;
      ret=i;
      i=0;
      if (c==0)return ret;
      offset(0,Columns-1,r,1);
      while (count()==0){
	i++;
	if (i<c)
	  offset(0,-1,r,1);
	else break;
      }
      offset(0,-c+1+i,r,c-i);
      return ret;
  }
}
void BinaryMatrix::rotate(BinaryMatrix &bm, const double a,
			   double origin[2], const bool which, 
			   const unsigned short extent)
{
  double c=cos(a);
  double s=-sin(a);
  if ((c<1E-10)&&(c>-1E-10))c=0;
  if ((s<1E-10)&&(s>-1E-10))s=0;
  double x=((double)bm.Rows/2.0);
  double y=((double)bm.Columns/2.0);
  double c1x=(-x)*c+y*s;
  double c1y=(-x)*s-y*c;
  double c2x=(x)*c+y*s;
  double c2y=(x)*s-y*c;
  double c3x=(x)*c-(y)*s;
  double c3y=(x)*s+(y)*c;
  double c4x=(-x)*c-(y)*s;
  double c4y=(-x)*s+(y)*c;
  double minx=c1x;
  double miny=c1y;
  if (minx>c2x)minx=c2x;
  if (minx>c3x)minx=c3x;
  if (minx>c4x)minx=c4x;

  if (miny>c2y)miny=c2y;
  if (miny>c3y)miny=c3y;
  if (miny>c4y)miny=c4y;
  double dr=-2.0*minx;
  double dc=-2.0*miny;
  long ir=(long)(dr);
  long ic=(long)(dc);
  if (dr>(double)ir)ir++;
  if (dc>(double)ic)ic++;
  double ox=-c*(x)+s*(y)-minx;
  double dox=origin[0]-ox;
  long iox=(long)dox;
  if (dox>0)
    dox-=iox;
  else 
    dox-=(iox-1);
  minx-=dox;
  origin[0]=-c*(x)+s*(y)-minx;
  ir++;
  double oy=-s*(x)-c*(y)-miny;
  double doy=origin[1]-oy;
  long ioy=(long)doy;
  if (doy>0)
    doy-=ioy;
  else 
    doy-=(ioy-1);
  miny-=doy;
  origin[1]=-s*(x)-c*(y)-miny;
  ic++;
  (*this)=(!(which));  
  reallocate(ir,ic,(!(which)));
  for (long i=0; i<bm.Rows; i++)
    for (long j=0; j<bm.Columns; j++)
      if (bm(i,j)==which)
	{
	  double di=i;
	  double dj=j;
	  double newx=c*(di-x)-s*(dj-y)-minx;
	  double newy=s*(di-x)+c*(dj-y)-miny;
	  long newi=(long)newx;
	  long newj=(long)newy;
	  if ((newi<Rows)&&(newj<Columns))
	    setBit(newi,newj,which);
	  if (extent==3)
	    {
	      //di-=.5,dj+=.5
	      newx-=((c/2)+(s/2));
	      newy-=((s/2)-(c/2));
	      newi=(long)(newx);
	      newj=(long)(newy);
	      if ((newi<Rows)&&(newj<Columns))
		setBit(newi,newj,which);
	      //dj-=1
	      newx+=s;
	      newy-=c;
	      newi=(long)(newx);
	      newj=(long)(newy);
	      if ((newi<Rows)&&(newj<Columns))	
		setBit(newi,newj,which);
	      //di+=1
	      newx+=c;
	      newy+=s;
	      newi=(long)(newx);
	      newj=(long)(newy);
	      if ((newi<Rows)&&(newj<Columns))
		setBit(newi,newj,which);
	      //dj+=1;
	      newx-=s;
	      newy+=c;
	      newi=(long)(newx);
	      newj=(long)(newy);
	      if ((newi<Rows)&&(newj<Columns))	
		setBit(newi,newj,which);
	    }
	  else if (extent==2)
	    {
	      //di-=.5,dj+=.5
	      newx-=((c/2)+(s/2));
	      newy-=((s/2)-(c/2));
	      if (i>0)
		{
		  bool test=false;
		  if (j<(bm.Columns-1))
		    {
		      if (bm(i-1,j+1)==which)
			test=true;
		    }
		  if (!test)
		    if (bm(i-1,j)==which)
		      test=true;
		  if (test)
		    {
		      newi=(long)(newx);
		      newj=(long)(newy);
		      if ((newi<Rows)&&(newj<Columns))
			setBit(newi,newj,which);
		    }
		}
	      //dj-=1
	      newx+=s;
	      newy-=c;
	      if (i>0)
		{
		  bool test=false;
		  if (j>0)
		    {
		      if (bm(i-1,j-1)==which)
			test=true;
		      if (!test)
			if (bm(i,j-1)==which)
			  test=true;
		    }
		  if (!test)
		    if (bm(i-1,j)==which)
		      test=true;
		  if (test)
		    {
		      newi=(long)(newx);
		      newj=(long)(newy);
		      if ((newi<Rows)&&(newj<Columns))
			setBit(newi,newj,which);
		    }
		}
	      //di+=1
	      newx+=c;
	      newy+=s;
	      if (j>0)
		{
		  if (bm(i,j-1)==which)
		    {
		      newi=(long)(newx);
		      newj=(long)(newy);
		      if ((newi<Rows)&&(newj<Columns))
			setBit(newi,newj,which);
		    }
		}
	    }
	}
}
long BinaryMatrix::count()
{
  unsigned long top,last;
    unsigned long endcol=getIndex(top,last);
    long r=0;
    unsigned long ac=CurrentColOffset+top;
    ac=(ac>>5);
    unsigned long *a=&AllocatedMatrix[(CurrentRowOffset*RowInc)+ac];
    for (long i=0; i<Rows; i++){
      for (long j=0; j<(long)top; j++) 
	if ((*this)(i,j))r++;
	for (long j=0; j<(long)endcol; j++)
	  {
	    unsigned long t=a[i*RowInc+j];
	    while (t>0)
	      {
		if(t&1)r++;
		t=t>>1;
	      }
	  }	
	for (long j=last; j<Columns; j++) 
	  if ((*this)(i,j))r++;
      }
    return r;
}
long BinaryMatrix::match(BinaryMatrix &mat)
{
 #if CUREBINARYMATRIXDEBUG  >20
    if ((Columns!=mat.Columns)||(Rows!=mat.Rows))
      {
	std::cerr<<"\nERROR BinaryMatrix::match INCOMPATABLE\n";
	return -1;
      }
#endif 
    unsigned long top,last;
    long r=0;
    unsigned long endcol=getIndex(top,last);
    if ((top==mat.getTop())){
      unsigned long ac=CurrentColOffset+top;
      ac=(ac>>5);
      unsigned long *a=&AllocatedMatrix[(CurrentRowOffset*RowInc)+ac];
      ac=mat.CurrentColOffset+top;
      ac=(ac>>5);
      unsigned long b=((mat.CurrentRowOffset*mat.RowInc)+ac);
      for (long i=0; i<Rows; i++){
	for (long j=0; j<(long)top; j++) 
	  if (((*this)(i,j)^(mat(i,j))))r++;
	for (long j=0; j<(long)endcol; j++)
	  {
	    unsigned long t=(a[i*RowInc+j]^
			     (mat.AllocatedMatrix[b+i*mat.RowInc+j]));
	    while (t>0)
	      {
		if(t&1)r++;
		t=t>>1;
	      }
	  }
	for (long j=last; j<Columns; j++) 
	  if ((*this)(i,j)^(mat(i,j)))r++;
      }
    }
    else
      for (long i=0; i<Rows; i++)
	for (long j=0; j<Columns; j++) 
	  if ((*this)(i,j)^(mat(i,j)))r++;
    
    return r;

}
bool BinaryMatrix::logicAnd(BinaryMatrix &mat)
{
 #if CUREBINARYMATRIXDEBUG  >20
    if ((Columns!=mat.Columns)||(Rows!=mat.Rows))
      {
	std::cerr<<"\nERROR BinaryMatrix::match INCOMPATABLE\n";
	return -1;
      }
#endif 
    unsigned long top,last;
    long r=0;
    unsigned long endcol=getIndex(top,last);
    if ((top==mat.getTop())){
      unsigned long ac=CurrentColOffset+top;
      ac=(ac>>5);
      unsigned long *a=&AllocatedMatrix[(CurrentRowOffset*RowInc)+ac];
      ac=mat.CurrentColOffset+top;
      ac=(ac>>5);
      unsigned long b=((mat.CurrentRowOffset*mat.RowInc)+ac);
      for (long i=0; i<Rows; i++){
	for (long j=0; j<(long)top; j++) 
	  if (((*this)(i,j)&(mat(i,j))))return true;;
	for (long j=0; j<(long)endcol; j++)
	  {
	    unsigned long t=(a[i*RowInc+j]&
			     (mat.AllocatedMatrix[b+i*mat.RowInc+j]));
	    if (t>0)
	      {
		return true;
	      }
	  }
	for (long j=last; j<Columns; j++) 
	  if ((*this)(i,j)&(mat(i,j)))return true;
      }
    }
    else
      for (long i=0; i<Rows; i++)
	for (long j=0; j<Columns; j++) 
	  if ((*this)(i,j)&(mat(i,j)))return true;;
    
    return r;

}

void BinaryMatrix::growInto(BinaryMatrix &mat,double radius,bool value)
{
  mat.reallocate(Rows,Columns,!value);
  for (long i=0; i<Rows; i++)
    for (long j=0; j<Columns; j++)
      if ((*this)(i,j)==value){
	if (adjacentBit(i,j,4,!value))
	  mat.fillDisk(i,j,radius,value);
	else mat.setBit(i,j,value);
      }
}
bool BinaryMatrix::adjacentBit(long r,long c,short neighbors,bool value)
{
  if (inRange(r-1,c))
    if ((*this)(r-1,c)==value)return true;
  if (inRange(r+1,c))
    if ((*this)(r+1,c)==value)return true;
  if (inRange(r,c-1))
    if ((*this)(r,c-1)==value)return true;
  if (inRange(r,c+1))
    if ((*this)(r,c+1)==value)return true;
  if (neighbors==4)return false;
  if (inRange(r-1,c-1))
    if ((*this)(r-1,c-1)==value)return true;
  if (inRange(r+1,c+1))
    if ((*this)(r+1,c+1)==value)return true;
  if (inRange(r+1,c-1))
    if ((*this)(r+1,c-1)==value)return true;
  if (inRange(r-1,c+1))
    if ((*this)(r-1,c+1)==value)return true;
  return false;
}
void BinaryMatrix::fillDisk(long r,long c,double radius, bool value)
{
  double test1=radius;
  double test2=test1*test1;
  long topi=(long)(test1);
  for (long i=0; i<(long)topi; i++)
    {
      double dc=(test2-i*i);
      if (dc<0)return;
      dc=sqrt(dc);
      dc+=.5;
      long topc=(long)dc; 
      if ((i+r)<Rows)
	{
	  for (long j=0; j<(long)topc; j++){
	    if (inRange(i+r,j+c))
	      setBit(i+r,j+c,value);
	    else j=topc;
	  }
	  for (long j=0; j<(long)topc; j++){
	    if (inRange(i+r,c-j))
	      setBit(i+r,c-j,value);
	    else j=topc;
	  }
	}
      if ((r-i)>=0)
	{
	  for (long j=0; j<(long)topc; j++){
	    if (inRange(r-i,j+c))
	      setBit(r-i,j+c,value);
	    else j=topc;
	  }
	  for (long j=0; j<(long)topc; j++){
	    if (inRange(r-i,c-j))
	      setBit(r-i,c-j,value);
	    else j=topc;
	  }
	}
    }
}
double  BinaryMatrix::path(short r1,short c1,short r2,short c2,
			Cure::ShortMatrix &path, double searchlimit)
{
  if (!inRange(r1,c1))return -1;
  if (!inRange(r2,c2))return -1;
  if ((r1==r2)&&(c1==c2)){
    path.reallocate(1,2);
    path(0,0)=r1;
    path(0,1)=c1;
    return 0;
  }
  Cure::ShortMatrix cells1, cells2;
  Cure::Matrix celldis1,celldis2;
  
  searchlimit/=2;
  long lim1=(long)(Rows*Columns/2);
  lim1+=4;
  long lim2=lim1;
  cells1.reallocate(lim1,2);
  cells2.reallocate(lim2,2);
  cells1(0,0)=r1;
  cells1(0,1)=c1;
  cells2(0,0)=r2;
  cells2(0,1)=c2;
  cells1.offset(0,0,1,2);
  cells2.offset(0,0,1,2);
  celldis1.reallocate(Rows,Columns);
  celldis2.reallocate(Rows,Columns);
  celldis1(r1,c1)=1;
  celldis2(r2,c2)=1;
  ShortMatrix index(1,2);
  long next1=0;
  double mindis=1E50;
  int b1=pathSearchstep(cells1, next1, celldis1, 
			celldis2, index,mindis);
  int next2=0;
  int b2=pathSearchstep(cells2, next2, celldis2,
			celldis1, index, mindis);
  if (mindis<searchlimit)searchlimit=mindis/2.0;
  while ((b1!=0)&&(b2!=0)){
    next1++;
    if (next1<cells1.Rows){
      double dist=(celldis1(cells1(next1,0),cells1(next1,1)));
      if (mindis<(2.0*searchlimit))searchlimit=mindis/2.0;
      while (dist>searchlimit){
	next1++;
	if (next1<cells1.Rows)
	  dist=(celldis1(cells1(next1,0),cells1(next1,1)));
	else dist=0;
      }
    }
    b1=pathSearchstep(cells1,  next1,celldis1,
			celldis2, index, mindis);
    if (cells1.Rows==lim1){
      cells1.grow(lim1+100,2);
      cells1.offset(0,0,lim1,2);
      lim1+=100;
    }
    next2++;
    if (next2<cells2.Rows){
      double dist=(celldis2(cells2(next2,0),cells2(next2,1)));
      if (mindis<(2.0*searchlimit))searchlimit=mindis/2.0;
      while (dist>searchlimit){
	next2++;
	if (next2<cells2.Rows)
	  dist=(celldis2(cells2(next2,0),cells2(next2,1)));
	else  dist=0;
      }
    }
    b2=pathSearchstep(cells2,  next2,celldis2,
		      celldis1, index,mindis);
    if (cells2.Rows==lim2){
      cells2.grow(lim2+100,2);
      cells2.offset(0,0,lim2,2);
      lim2+=100;
    }
  }
  if (mindis>1E40)
    return -1;
  ShortMatrix path1;
  path1.reallocate((int)(mindis+1),2);
  path1.offset(0,0,1,2);
  path1(0,0)=index(0,0);
  path1(0,1)=index(0,1);
  int r=index(0,0);
  int c=index(0,1);
  bool test=true;
  if ((r==r1)&&(c==c1))test=false;
  while (test){
    double d=celldis1(r,c);
    int rt=r-1;
    int ct=c-1;
    if (inRange(rt,ct))
      if (celldis1(rt,ct)>0)
	if (d>celldis1(rt,ct)){
	  r=rt;
	  c=ct;
	  d=celldis1(rt,ct);
	}
    rt++;
    if (inRange(rt,ct))
      if (celldis1(rt,ct)>0)
	if (d>celldis1(rt,ct)){
	  r=rt;
	  c=ct;
	  d=celldis1(rt,ct);
	}
    rt++;
    if (inRange(rt,ct))
      if (celldis1(rt,ct)>0)
	if (d>celldis1(rt,ct)){
	  r=rt;
	  c=ct;
	  d=celldis1(rt,ct);
	}
    ct++;
    if (inRange(rt,ct))
      if (celldis1(rt,ct)>0)
	if (d>celldis1(rt,ct)){
	  r=rt;
	  c=ct;
	  d=celldis1(rt,ct);
	}
    ct++;
    if (inRange(rt,ct))
      if (celldis1(rt,ct)>0)
	if (d>celldis1(rt,ct)){
	  r=rt;
	  c=ct;
	  d=celldis1(rt,ct);
	}
    rt--;
    if (inRange(rt,ct))
      if (celldis1(rt,ct)>0)
	if (d>celldis1(rt,ct)){
	  r=rt;
	  c=ct;
	  d=celldis1(rt,ct);
	}
    rt--;
    if (inRange(rt,ct))
      if (celldis1(rt,ct)>0)
	if (d>celldis1(rt,ct)){
	  r=rt;
	  c=ct;
	  d=celldis1(rt,ct);
	}
    ct--;
    if (inRange(rt,ct))
      if (celldis1(rt,ct)>0)
	if (d>celldis1(rt,ct)){
	  r=rt;
	  c=ct;
	  d=celldis1(rt,ct);
	}
    path1.append(r,0);
    path1.append(c,1);
    if ((r==r1)&&(c==c1))test=false;
  }
  ShortMatrix path2;
  path2.reallocate((int)(mindis+1),2);
  path2.offset(0,0,1,2);
  path2(0,0)=index(0,0);
  path2(0,1)=index(0,1);
  r=index(0,0);
  c=index(0,1);
  test=true;
  if ((r==r2)&&(c==c2))test=false;
  while (test){
    double d=celldis2(r,c);
    int rt=r-1;
    int ct=c-1;
    if (inRange(rt,ct))
      if (celldis2(rt,ct)>0)
  	if (d>celldis2(rt,ct)){
	  r=rt;
	  c=ct;
	  d=celldis2(rt,ct);
	}
    rt++;
    if (inRange(rt,ct))
    if (celldis2(rt,ct)>0)
	if (d>celldis2(rt,ct)){
	  r=rt;
	  c=ct;
	  d=celldis2(rt,ct);
	}
    rt++;
    if (inRange(rt,ct))
      if (celldis2(rt,ct)>0)
	if (d>celldis2(rt,ct)){
	  r=rt;
	  c=ct;
	  d=celldis2(rt,ct);
	}
    ct++;
    if (inRange(rt,ct))
      if (celldis2(rt,ct)>0)
	if (d>celldis2(rt,ct)){
	  r=rt;
	  c=ct;
	  d=celldis2(rt,ct);
	}
    ct++;
    if (inRange(rt,ct))
      if (celldis2(rt,ct)>0)
	if (d>celldis2(rt,ct)){
	  r=rt;
	  c=ct;
	  d=celldis2(rt,ct);
	}
    rt--;
    if (inRange(rt,ct))
      if (celldis2(rt,ct)>0)
	if (d>celldis2(rt,ct)){
	  r=rt;
	  c=ct;
	  d=celldis2(rt,ct);
	}
    rt--;
    if (inRange(rt,ct))
      if (celldis2(rt,ct)>0)
	if (d>celldis2(rt,ct)){
	  r=rt;
	  c=ct;
	  d=celldis2(rt,ct);
	}
    ct--;
    if (inRange(rt,ct))
      if (celldis2(rt,ct)>0)
	if (d>celldis2(rt,ct)){
	  r=rt;
	  c=ct;
	  d=celldis2(rt,ct);
	}
    path2.append(r,0);
    path2.append(c,1);
    if ((r==r2)&&(c==c2))test=false;
  }
  r=path1.Rows-1;
  path.reallocate(path2.Rows+path1.Rows-1,2);
  for (int i=0; i<path1.Rows; i++,r--){
    path(i,0)=path1(r,0);
    path(i,1)=path1(r,1);
  }
  r=1;
  for (int i=path1.Rows;r<path2.Rows; i++,r++){
    path(i,0)=path2(r,0);
    path(i,1)=path2(r,1);
  }
  return (mindis-2);
}

int BinaryMatrix::pathSearchstep(Cure::ShortMatrix &cells, int next, 
				 Cure::Matrix &celldis, 
				 Cure::Matrix &celldis2,
				 ShortMatrix &minindex,
				 double &mindist)
{
  if (next>=cells.Rows)return 0;
  int r=cells(next,0);
  int c=cells(next,1);
  double dis=celldis(r,c)+1.0;
  r--;
  for (int it=0; it<8; it++){
    if (inRange(r,c)){
      if (celldis(r,c)==0){
	if (!(*this)(r,c)){
	  celldis(r,c)=dis;
	  cells.append(r,0);
	  cells.append(c,1);
	  if (celldis2(r,c)>0){
	    double dis=celldis2(r,c)+celldis(r,c);
	    if (dis<mindist){
	      mindist=dis;
	      minindex(0,0)=r;
	      minindex(0,1)=c;
	    }
	  }
	  checkdis(celldis,r,c,celldis2,minindex,mindist);
	}
      }
    }
    if (it==0)r+=2;
    else if(it==1){
      r--;
      c--;
    }
    else if (it==2) c+=2;
    else if (it==3) {
      r--;
      dis+=(M_SQRT2-1.0);
    }
    else if (it==4)r+=2;
    else if(it==5)c-=2;
    else r-=2;
  } 
  return 1;
}

double BinaryMatrix::nearest(int r,int c,ShortMatrix &indexes, 
			     double direction[2], double maxdistance,
			     double mindistance)
{
  if (!inRange(r,c))return -1;
  double test1=maxdistance;
  double test2=mindistance;
  long topi=(long)(test1+1.0);
  long bottomi=(long)(test2);
  indexes.reallocate(0,2);
  double x=r;
  double y=c;
  double dx=direction[0];
  double dy=direction[1];
  double sx=x+direction[1];
  double sy=y-direction[0];
  for (short j=0; j<5; j++){
    x=sx;
    y=sy;
    for (long i=bottomi; i<(long)topi; i++){
      short rx=(short)((double)(x+.5));
      short cy=(short)((double)(y+.5));
      if (inRange(rx,cy)){
	if ((*this)(rx,cy)){
	  if (test1>((double)i)){
	    test1=(double)(i);
	    topi=(int)(test1+1.0);
	    indexes.reallocate(1,2);
	    indexes(0,0)=rx;
	    indexes(0,1)=cy;
	  }
	}
      }
      x+=dx;
      y+=dy;
    }
    sx-=direction[1]/2;
    sy+=direction[0]/2;
  }
  return test1;
}
double BinaryMatrix::nearest(int r,int c,ShortMatrix &indexes, 
			     double maxdistance,
			     double mindistance)
{
  double resolution=M_SQRT2;
  double test1=maxdistance;
  double test2=test1*test1;
  double test3=(test1+resolution);
  double test4=test3*test3;
  double test5=mindistance*mindistance;
  int topi=(int)(test3+2);
  int topj=topi/32+2;
  unsigned long *a=0;
  unsigned short bitcol=bitColumn(r,c,&a);
  unsigned short abitcol=32-bitcol;
  indexes.reallocate(0,2);
  ShortMatrix temp(50,2);
  temp.reset(0,2);
  for (int i=0; i<(long)topi; i++)
    {
      if ((i+r)<Rows)
	if (a[i*RowInc])
	  {
	    for (int k=0; k<32; k++)
	      {
		int jin=c+k-bitcol;
		if (((jin>=0)&&(jin<Columns))&&((*this)(i+r,jin)))
		  {
		    double d=(k-bitcol)*(k-bitcol);
		    d+=(i*i);
		    if ((d<=(test4))&(d>test5)){
		      temp.append(i,0);
		      temp.append(k-bitcol,1);
		      if (d<test2){
			test2=d;
			test1=sqrt(test2);
			test3=(test1+resolution);
			test4=test3*test3;
		      }
		    }
		  }
	      }
	  }
      if ((i>0)&&((r-i)>=0))
	if (a[-i*RowInc])
	  {
	    for (int k=0; k<32; k++)
	      {
		int jin=c+k-bitcol;
		if (((jin>=0)&&(jin<Columns))&&((*this)(r-i,jin)))
		  {
		    double d=(k-bitcol)*(k-bitcol);
		    d+=(i*i);
		    if ((d<=(test4))&(d>test5)){
		      temp.append(-i,0);
		      temp.append(k-bitcol,1);
		      if (d<test2){
			test2=d;
			test1=sqrt(test2);
			test3=(test1+resolution);
			test4=test3*test3;
		      }
		    }
		  }
	      }
	  }
      topi=(int)(test3+2);
      topj=topi/32+2;
          for (int j=1; j<(long)topj; j++)
	{
	  if ((i+r)<Rows)
	    {
	      short jin=(((j-1)*32)+abitcol);
	      if ((jin+c)<Columns)
		if (a[i*RowInc+j])
		  {
		    for (int k=0; k<32; k++)
		      {
			if ((((c+k+jin)>-1)&&((c+k+jin)<Columns))
			    &&((*this)(i+r,c+k+jin)))
			  {
			   double d=(k+jin)*(k+jin);
			   d+=(i*i);
			   if ((d<=(test4))&(d>test5)){
			     temp.append(i,0);
			     temp.append(k+jin,1);
			     if (d<test2){
			       test2=d;
			       test1=sqrt(test2);
			       test3=(test1+resolution);
			       test4=test3*test3;
			     }
			   }
			  }
		     }
		  }
	      jin=-((j*32)+bitcol);
	      if ((jin+c+32)>0)
		if (a[i*RowInc-j])
		  {
		   for (int k=0; k<32; k++)
		     {
		       if ((((c+k+jin)<Columns)&&((c+k+jin)>=0))
			   &&((*this)(i+r,c+k+jin)))
			 {
			   double d=(k+jin)*(k+jin);
			   d+=(i*i);
			   if ((d<=(test4))&(d>test5)){
			     temp.append(i,0);
			     temp.append(k+jin,1);
			     if (d<test2){
			       test2=d;
			       test1=sqrt(test2);
			       test3=(test1+resolution);
			       test4=test3*test3;
			     }
			   }
			   
			 }
		     }
		  }
	    }
	  if ((i>0)&&((r-i)>=0))
	    {
	      short jin=(((j-1)*32)+abitcol);
	      if ((jin+c)<Columns)
		if (a[-i*RowInc+j])
		  {
		    for (int k=0; k<32; k++)
		     {
		       if ((((c+k+jin)>=0)&&
			    (((c+k+jin)<Columns)))&&
			   ((*this)(r-i,c+k+jin)))
			 {
			   double d=(k+jin)*(k+jin);
			   d+=(i*i);
			   if ((d<=(test4))&(d>test5)){
			     temp.append(-i,0);
			     temp.append(k+jin,1);
			     if (d<test2){
			       test2=d;
			       test1=sqrt(test2);
			       test3=(test1+resolution);
			       test4=test3*test3;
			     }
			   }
			 }
		     }
		  }
	      jin=-((j*32)+bitcol);
	      if ((jin+c+32)>0)
		if (a[-i*RowInc-j])
		  {
		    for (int k=0; k<32; k++)
		      {
			if ((((c+k+jin)>=0)&&
			     (((c+k+jin)<Columns)))
			    &&((*this)(r-i,c+k+jin)))
			  {
			    double d=(k+jin)*(k+jin);
			    d+=(i*i);
			    if ((d<=(test4))&(d>test5)){
			      temp.append(-i,0);
			      temp.append(k+jin,1);
			      if (d<test2){
				test2=d;
				test1=sqrt(test2);
				test3=(test1+resolution);
			       test4=test3*test3;
			      }
			    }
			  }
		      }
		  }
	    }
	  topi=(int)(test3+2);
	  topj=topi/32+2;
	}
    }
  for (int i=0; i<temp.Rows; i++)
    {
      int k=temp(i,0);
      int jin=temp(i,1);
      double d=(jin)*(jin);
      d+=(k*k);
      if ((d>(test4))||(d<test5))
	{
	  temp.deleteRow(i);
	  i--;
	}
    }
  if (temp.Rows>1000){
    indexes=temp;
    for (int i=0; i<indexes.Rows; i++)
      {
	indexes(i,0)+=r;
	indexes(i,1)+=c;
      }
    return test1;
  }
while(temp.Rows)
  {
    ShortMatrix clusters(6,2);
    clusters.reset(0,2);
    clusters.append(temp(0,0),0);
    clusters.append(temp(0,1),1);
    temp.deleteRow(0);
    while (cluster(clusters,temp)){}
    int k=clusters(0,0);
    int jin=clusters(0,1);
    int i=0;
    double d=(jin)*(jin);
    d+=(k*k);
    for (int j=1; j<clusters.Rows; j++)
      {
	int k2=clusters(j,0);
	int jin2=clusters(j,1);
	double d2=((jin2*jin2)+(k2*k2)); 
	if (d2<d){
	  i=j;
	  d=d2;
	}
      }
    indexes.append(clusters(i,0)+r,0);
    indexes.append(clusters(i,1)+c,1);
  }
return test1; 
 
}
void  BinaryMatrix::boundary(BinaryMatrix & boundmat,short adjacency)
{
  BinaryMatrix temp(Rows+2,Columns+2);
  temp.offset(1,1,Rows,Columns);
  temp=(*this);
  ~(temp);
  boundmat=(*this);
  temp.offset(-1,0,Rows,Columns);
  boundmat&=temp;
  temp.offset(2,0,Rows,Columns);
  boundmat.orAnd(temp,*this);
  temp.offset(-1,-1,Rows,Columns);
  boundmat.orAnd(temp,*this);
  temp.offset(0,2,Rows,Columns);
  boundmat.orAnd(temp,*this);
  if (adjacency==4)return;
  temp.offset(-1,0,Rows,Columns);
  boundmat.orAnd(temp,*this);
  temp.offset(2,0,Rows,Columns);
  boundmat.orAnd(temp,*this);
  temp.offset(0,-2,Rows,Columns);
  boundmat.orAnd(temp,*this);
  temp.offset(0,2,Rows,Columns);
  boundmat.orAnd(temp,*this);  
}	
void  BinaryMatrix::orExor(BinaryMatrix &a,BinaryMatrix &b)
{
#if CUREBINARYMATRIXDEBUG  >20
    if ((Columns!=a.Columns)||(Rows!=a.Rows))
      {
	std::cerr<<"\nERROR BinaryMatrix::orExorINCOMPATABLE\n";
	return;
      }
    if ((Columns!=b.Columns)||(Rows!=b.Rows))
      {
	std::cerr<<"\nERROR BinaryMatrix::orExor INCOMPATABLE\n";
	return;
      }
#endif 
    unsigned long top,last;
    unsigned long endcol=getIndex(top,last);
    unsigned long ac=CurrentColOffset+top;
    ac=(ac>>5);
    unsigned long *m=&AllocatedMatrix[(CurrentRowOffset*RowInc)+ac];
    for (long i=0; i<Rows; i++){
      for (long j=0; j<(long)top; j++) 
	setBit(i,j,((*this)(i,j)|(a(i,j)^(b(i,j)))));
	for (long j=0; j<(long)endcol; j++)
	  {
	    long jin=32*j+top;
	    unsigned long t=0;
	    for (long k=0; k<32; k++)
	      { 
		t=(t<<1);
		long h=(a(i,jin+31-k)^b(i,jin+31-k));
		t=(t|h);
	      }
	    m[i*RowInc+j]=(m[i*RowInc+j]|t);
	  }
	for (long j=last; j<Columns; j++) 
	  setBit(i,j,((*this)(i,j)|(a(i,j)^b(i,j))));	
    }
}	
    
void  BinaryMatrix::orAnd(BinaryMatrix &a,BinaryMatrix &b)
{
#if CUREBINARYMATRIXDEBUG  >20
    if ((Columns!=a.Columns)||(Rows!=a.Rows))
      {
	std::cerr<<"\nERROR BinaryMatrix::orExorINCOMPATABLE\n";
	return;
      }
    if ((Columns!=b.Columns)||(Rows!=b.Rows))
      {
	std::cerr<<"\nERROR BinaryMatrix::orExor INCOMPATABLE\n";
	return;
      }
#endif 
    unsigned long top,last;
    unsigned long endcol=getIndex(top,last);
    unsigned long ac=CurrentColOffset+top;
    ac=(ac>>5);
    unsigned long *m=&AllocatedMatrix[(CurrentRowOffset*RowInc)+ac];
    for (long i=0; i<Rows; i++){
      for (long j=0; j<(long)top; j++) 
	setBit(i,j,((*this)(i,j)|(a(i,j)&(b(i,j)))));
      for (long j=0; j<(long)endcol; j++)
	{
	  long jin=32*j+top;
	  unsigned long t=0;
	  for (long k=0; k<32; k++)
	    { 
	      t=(t<<1);
	      long h=(a(i,jin+31-k)&b(i,jin+31-k));
	      t=(t|h);
	      }
	    m[i*RowInc+j]=(m[i*RowInc+j]|t);
	  }
	for (long j=last; j<Columns; j++) 
	  setBit(i,j,((*this)(i,j)|(a(i,j)&b(i,j))));	
    }
}	
int BinaryMatrix::cluster(int r,int c)
{
  // BinaryMatrix visited;
  int t=clusterGrowth(*this,false, r,c);
  //  this|=visited;
  return t;
}
 
int BinaryMatrix::cluster(ShortMatrix &clusters,ShortMatrix &temp)
{
  for (int i=0; i<temp.Rows; i++)
    {
      short ii=temp(i,0);
      short ij=temp(i,1);
      for (int j=0; j<clusters.Rows; j++)
	{
	  short di=(ii-clusters(j,0));
	  if ((di<2)&&(di>-2))
	    {
	      short dj=(ij-clusters(j,1));
	      if ((dj<2)&&(dj>-2))
		{
		  clusters.append(ii,0);
		  clusters.append(ij,1);
		  temp.deleteRow(i);
		  return 1;
		}
	      
	    }
	}
    }
  return 0;
}
void BinaryMatrix::setPolygon(ShortMatrix &verticies)
{
  if (verticies.Rows<3)return;
  BinaryMatrix visitup(Rows,Columns);
  BinaryMatrix visitdown(Rows,Columns);
  double xend=verticies(0,0)+.5;
  double yend=verticies(0,1)+.5;
  double n[2];
  for (long i=0; i<verticies.Rows; i++)
    {
      
      short r=verticies(i,0);
      short c=verticies(i,1);
      double xstart=r+.5;
      double ystart=c+.5;
      if (i==(verticies.Rows-1))
	{
	  xend=verticies(0,0)+.5;
	  yend=verticies(0,1)+.5;
      	}
      else 
	{
	  xend=verticies(i+1,0)+.5;
	  yend=verticies(i+1,1)+.5;
	}
      n[0]=xend-xstart;
      n[1]=yend-ystart;
      double d=n[0]*n[0]+n[1]*n[1];
      d=sqrt(d);
      short step=1;
      if (n[0]<0)step=-1;
      else if (n[0]==0)
	step=0;
      if (inRange(r,c)){
	if (step>0) {
	  if(inRange(r,c-1)) visitup.setBit(r,c-1,true);
	  if(inRange(r+1,c)) visitup.setBit(r+1,c,true);
	  visitup.setBit(r,c,true);
	}
	else if (step<0){
	  if(inRange(r,c+1)) visitdown.setBit(r,c+1,true);
	  if(inRange(r-1,c)) visitdown.setBit(r-1,c,true);
	  visitdown.setBit(r,c,true);
	}
	else {
	  if(inRange(r,c-1)) visitup.setBit(r,c-1,true);
	  if(inRange(r,c+1)) visitdown.setBit(r,c+1,true);
	  visitup.setBit(r,c,true);
	  visitdown.setBit(r,c,true);
	}
      }
      if (d>.5){
	n[0]/=d;
	n[1]/=d;
	bool test=true;
	double t=0;
	d-=1;
	while (test)
	  {
	    t+=1;
	    double x=xstart+n[0]*t;
	    double y=ystart+n[1]*t;
	    r=(int)x;
	    c=(int)y;
	    if (inRange(r,c)){
	      if (step>0) {
		if(inRange(r,c-1)) visitup.setBit(r,c-1,true);
		if(inRange(r+1,c)) visitup.setBit(r+1,c,true);
		visitup.setBit(r,c,true);
	      }
	      else if (step<0){
		if(inRange(r,c+1)) visitdown.setBit(r,c+1,true);
		if(inRange(r-1,c)) visitdown.setBit(r-1,c,true);
		visitdown.setBit(r,c,true);
	      }
	      else {
		if(inRange(r,c+1)) visitdown.setBit(r,c+1,true);
		if(inRange(r,c-1)) visitup.setBit(r,c-1,true);
		visitup.setBit(r,c,true);
		visitdown.setBit(r,c,true);
	      }
	      }
	    if (t>d)test=false;
	  }
      }
    }
  xend=verticies(0,0)+.5;
  yend=verticies(0,1)+.5;
  for (long i=0; i<verticies.Rows; i++)
    {
      
      short r=verticies(i,0);
      short c=verticies(i,1);
      double xstart=r+.5;
      double ystart=c+.5;
      if (i==(verticies.Rows-1))
	{
	  xend=verticies(0,0)+.5;
	  yend=verticies(0,1)+.5;
      	}
      else 
	{
	  xend=verticies(i+1,0)+.5;
	  yend=verticies(i+1,1)+.5;
	}
      n[0]=xend-xstart;
      n[1]=yend-ystart;
      double d=n[0]*n[0]+n[1]*n[1];
      d=sqrt(d);
      int step=1;
      if (n[0]<0)step=-1;
      else if (n[0]==0)
	step=0;
      bool clearrow=true;
      short rc=c;
      while (clearrow)
	{
	  if (inRange(r,rc)){
	    if (checkSet(r,rc,true))clearrow=false;	  	  
	    if ((step>0)&& (visitdown(r,rc)))clearrow=false;
	    else if ((step<0)&& (visitup(r,rc)))clearrow=false;
	    else if (step==0)clearrow=false;
	  }
	  else clearrow=false;
	  rc+=step;
	}

      if (d>.5){
	n[0]/=d;
	n[1]/=d;
	bool test=true;
	double t=0;
	d-=1;
	while (test)
	  {
	    t+=1;
	    double x=xstart+n[0]*t;
	    double y=ystart+n[1]*t;
	    r=(int)x;
	    c=(int)y;
	    clearrow=true;
	    rc=c;
	    while (clearrow)
	      {
		if (inRange(r,rc)){
		  if (checkSet(r,rc,true))clearrow=false;	  	  
		  if ((step>0) &&(visitdown(r,rc)))clearrow=false;
		  else if ((step<0) &&(visitup(r,rc)))clearrow=false;
		  else if (step==0)clearrow=false;
		}
		else clearrow=false;
		rc+=step;
	      }
	    if (t>d)test=false;
	  }
      }
    }

}

void BinaryMatrix::clearPolygon(ShortMatrix &verticies)
{
  // Dorian's version

  if(verticies.Rows<3)return;
  BinaryMatrix visitup(Rows,Columns);
  BinaryMatrix visitdown(Rows,Columns);
  ShortMatrix lastLine, thisLine;
  
  for(int i = 0; i < verticies.Rows; i++){
    double xstart, ystart, xend, yend, xprev, yprev, n[2], m[2];
    double xaux = 0.0;
    double yaux = 0.0;
    int r=verticies(i,0);
    int c=verticies(i,1);
    int next = (i == verticies.Rows-1? 0: i+1);
    if(verticies(next,0) == r && verticies(next,1) == c) continue;
    
    int prev = i;
    while(verticies(prev,0)==verticies(i,0) &&
	  verticies(prev,1)==verticies(i,1)){
      prev = (prev == 0? verticies.Rows-1: prev-1);
    }
    
    // Prevent from getting two impossible edges
    if(verticies(next,0)==verticies(prev,0)
    	&& verticies(next,1)==verticies(prev,1)) continue;

    xprev = verticies(prev,0)+.5;
    yprev = verticies(prev,1)+.5;
    xstart = (double)r+.5;
    ystart = (double)c+.5;
    xend = verticies(next,0)+.5;
    yend = verticies(next,1)+.5;

    n[0]=xend-xstart;
    n[1]=yend-ystart;
    m[0]=xstart-xprev;
    m[1]=ystart-yprev;

    bool V0, V1, H0, H1, SV0, SV1, SH0, SH1;
    SV0 = fabs(m[1]) < 1e-6; // 1: straight vertical
    SV1 = fabs(n[1]) < 1e-6;
	SH0 = fabs(m[0]) < 1e-6; // 1: straight horizontal
    SH1 = fabs(n[0]) < 1e-6;
    V0 = (m[0] >= 0); // 1: goes down in grid
    V1 = (n[0] >= 0);
    H0 = (m[1] >= 0); // 1: goes right in grid
    H1 = (n[1] >= 0);
    
    bool paintwhenconflict;
    bool opposite_lines = ((SV0 || SV1) && (SH0 || SH1))
    	|| (V0!=V1 && (H0!=H1 || SH0 || SH1))
    	|| (H0!=H1 && (SV0 || SV1));
    {
		double t0 = atan2(-xprev+xstart, yprev-ystart);
		double t1 = atan2(-xend+xstart, yend-ystart);
		
		paintwhenconflict = false;
		if(!SV0 && !SH0){
			if(H0&&V0)
				paintwhenconflict = (t1 <= 0) || (t1 > t0);
			else if(H0&&!V0)
				paintwhenconflict = (t1 == 0) || ((t1 < 0)&&(t1>t0));
			else if(!H0&&!V0)
				paintwhenconflict = (t1 == 0) || ((t1 < 0)&&(t1>t0));
			else if(!H0&&V0)
				paintwhenconflict = (t1 <= 0) || (t1 > t0);
		}else if(SV0 && !SH0){
			if(!V0)
				paintwhenconflict = (t1 < 0) && (t1 > t0);
			else
				paintwhenconflict = (t1 < 0) || (t1 > t0);
		}else if(!SV0 && SH0 && H0){
			paintwhenconflict = (t1 < 0) && (t1 > M_PI*2.0);
		}
	}
    
    bool right = n[1] >= 0;
    int step=1;
    if(fabs(n[0])<1e-6) step = 0;
    else if(n[0]<0) step = -1;

    // horizontal lines are special
    if(step == 0){
      int j2;
      if(right){
	// we need the next line (so first vertice from next)
	j2 = next;
	while(verticies(next,0)==verticies(j2,0)
	      &&verticies(next,1)==verticies(j2,1)){
	  j2 = (j2==verticies.Rows-1? 0: j2+1);
	}
      }else{
	// we need the previous edge
	j2 = prev;
      }
      xaux = verticies(j2,0)+.5;
      yaux = verticies(j2,1)+.5;
    }
    
    double d=n[0]*n[0]+n[1]*n[1];
    d=sqrt(d);
  
    n[0]/=d;
    n[1]/=d;
    double t = 0;
    
	// We have to compare this line with the previous line to prevent 
	// pixels from different lines from crossing when they should not
	if(thisLine.Rows > 0) lastLine = thisLine;
	thisLine.reallocate(0,0);
	
	int lastr = (int)xend;
	int lastc = (int)yend;

    bool needloop = false; // flag to control de do loop
    int rc = c;
	do { // to ensure the last pixel is reached
		bool force = false;
	    while(t <= d){
	      double x=xstart+n[0]*t;
	      double y=ystart+n[1]*t;
	      t+=1;
	      if(!force){
		      r=(int)x;
		      rc=(int)y;
		  }
	      
	      thisLine.reallocate(thisLine.Rows+1, 2);
	      thisLine(thisLine.Rows-1, 0) = r;
	      thisLine(thisLine.Rows-1, 1) = rc;
	      
	      if (inRange(r,rc)){			
			// We correct the position of this line if it crosses the 
			// previous one
			if(opposite_lines && lastLine.Rows > 0 && step != 0){
				bool fix = true;
				bool any = false;
				int best = -1;
				for(int j = 0; fix && j < lastLine.Rows; j++){
					if(lastLine(j,0) == r){
						any = true;
						if(right){ // currect pixel must be on the right
							if(rc >= lastLine(j,1)){
								fix = false;
							}else if(best == -1 || lastLine(j,1) < best){
								best = lastLine(j,1);
							}
						}else{ // current pixel must be on the left
							if(rc <= lastLine(j,1)){
								fix = false;
							}else if(best == -1 || lastLine(j,1) > best){
								best = lastLine(j,1);
							}
						}
					}
				}
				if(fix && any) rc = best;
			}
			
		if (step>0){
		  if(visitdown(r,rc)){
		    visitup.setBit(r,rc,paintwhenconflict);
		    visitdown.setBit(r,rc,!paintwhenconflict);
		  }else
		    visitup.setBit(r,rc, true);

		}else if (step<0){
		  if(visitup(r,rc)){
		    visitup.setBit(r,rc,paintwhenconflict);
		    visitdown.setBit(r,rc,!paintwhenconflict);
		  }else
		    visitdown.setBit(r,rc, true);
		}					
	      }
	    }
	    if(step == 0){
	    	int thisc = (right? rc+1: c+1);
	      t = atan2(xaux-xend, yaux-yend);
	      if(right && (t < 0) && inRange(r,thisc)){
			if(!visitup(r,thisc) && !visitdown(r,thisc-1)){
				visitdown.setBit(r,thisc, true);
				visitup.setBit(r,thisc,false);
			}
	      }else if(!right && (t > 0) && inRange(r,thisc)){
			if(!visitup(r,thisc) && !visitdown(r,thisc-1)){
		    	visitdown.setBit(r,thisc, true);
		    	visitup.setBit(r,thisc, false);
		    }
	      }
	    } // while(t <= d)
	    
	    if(!needloop && (lastr != r || lastc != rc)){
	    	force = true; // make the while take our values
	    	r = lastr; // our values
	    	rc = lastc;
	    	t = d; // make sure the while runs only once
	    	needloop = true; // make sure the do does its job
	    }else
	    	needloop = false;
	}while(needloop); // do 
  } // for each verticie

  for(int i = 0; i < verticies.Rows; i++){
    double xstart, ystart, xend, yend, n[2];
    int r=verticies(i,0);
    int c=verticies(i,1);
    int next = (i == verticies.Rows-1? 0: i+1);
    if(verticies(next,0) == r && verticies(next,1) == c) continue;

    xstart = (double)r+.5;
    ystart = (double)c+.5;
    xend = verticies(next,0)+.5;
    yend = verticies(next,1)+.5;
    n[0]=xend-xstart;
    n[1]=yend-ystart;

    int step=1;
    if(fabs(n[0])<1e-6) step = 0;
    else if(n[0]<0) step = -1;

    double d=n[0]*n[0]+n[1]*n[1];
    d=sqrt(d);
    
    n[0]/=d;
    n[1]/=d;
    double t = 0;
    while(t <= d){
      double x=xstart+n[0]*t;
      double y=ystart+n[1]*t;
      t+=1;
      r=(int)x;
      c=(int)y;
      if(inRange(r,c)) setBit(r,c, false);
    
      if(step >= 0 && !visitdown(r,c)){
	c++;
	while(inRange(r,c) && !visitdown(r,c)){
	  setBit(r,c,false);
	  c++;
	}
      }
    }
  } 
}
void BinaryMatrix::setArc(double center[2], 
			  double radius, 
			  double startangle, double endangle, bool value, double thickness)
{
  while (thickness>0)
    {
      setArc(center,radius+thickness,startangle,endangle,value,0);
      thickness-=.33;
    }
  double dt=1/radius;
  double t=startangle;
  while (t<=endangle){
    double x=radius*cos(t)+center[0]+.5;
    double y=radius*sin(t)+center[1]+.5;
    long r=(int)x;
    long c=(int)y;
    if (inRange(r,c))
      setBit(r,c,value);
    t+=dt;
  }
}
void BinaryMatrix::setHarc(double center[3], 
			   double radius, 
			   double startangle, double endangle)
{
  if ((Rows==0)||(Columns==0))return;
  double c2=center[2]*center[2];
  double r2=radius*radius-c2;
  if (r2<0)return;
  while (startangle>endangle)endangle+=2*M_PI;
  while (startangle<(endangle-2*M_PI))endangle-=2*M_PI;
  double t=(endangle-startangle)/2;
  if (t*radius<.0001)return;
  if (t>1.57){
    return;
  }
  double m2=tan(t);
  m2=m2*m2;
  double ct=(startangle+endangle)/2;  
  double p=radius*cos(t); 
  double p2=p*p; 
  double q2=m2*p2-c2;
  double q=sqrt(q2); 
  radius=sqrt(r2);
  double t2=atan2(q,p);  
  startangle=ct-t2;
  endangle=ct+t2;

  setArc(center,radius,startangle,endangle);
  return;
}
void BinaryMatrix::setSector(double center[2], 
			       double radius, 
			       double startangle, double endangle)
{
  if ((Rows==0)||(Columns==0))return;
  double minrad=radius/2;
   while (minrad>0){
     if (minrad<5)minrad=0;
     double inc=.25/radius;
     for (double a=startangle; a<endangle; a+=inc)
       setRay(center, radius,a, true,minrad);
     setRay(center, radius,endangle, true,minrad);
     radius/=2;
     minrad=radius/2;
   }
  return;
}
void BinaryMatrix::clearSector(double center[2], 
			       double radius, 
			       double startangle, double endangle)
{
  if ((Rows==0)||(Columns==0))return;
  double minrad=radius/2;
   while (minrad>0){
     if (minrad<5)minrad=0;
     double inc=.25/radius;
     for (double a=startangle; a<endangle; a+=inc)
       setRay(center, radius,a, false,minrad);
     setRay(center, radius,endangle, false,minrad);
     radius/=2;
     minrad=radius/2;
   }
  return;
}

void BinaryMatrix::setRay(double center[2], 
			    double radius, 
			    double angle, bool value, double minradius)
{
  double s=sin(angle);
  double cs=cos(angle);
  minradius+=.5;
  long t=(int)minradius;
  double x=center[0]+.5;
  double y=center[1]+.5;
  long r=(int)x;
  long c=(int)y;
  radius+=.5;
  while (t<=(int)radius){
    if (inRange(r,c))
      setBit(r,c,value);
    else return;
    x+=cs;
    y+=s;
    r=(long)x;
    c=(long)y;
    t++;
 }
}

void BinaryMatrix::setHsection(double center[3], 
			       double radius, 
				 double startangle, double endangle)
{
  double c2=center[2]*center[2];
  double r2=radius*radius-c2;
  if (r2<0)return;
  while (startangle>endangle)endangle+=2*M_PI;
  while (startangle<(endangle-2*M_PI))endangle-=2*M_PI;
  double t=(endangle-startangle)/2;
  if (t>1){
    clearHsection(center,radius,startangle, startangle+t/2);
    clearHsection(center,radius,startangle+t/2,endangle);
  }
   if (t*radius<.0001)return;  
  double m2=tan(t); //m*p=sqrt(x2+y2) on inner curves
  m2=m2*m2;
  double ct=(startangle+endangle)/2;  
  double theta=0;
  double rhomax=sqrt(r2);
  double inc=.2/rhomax;
  if (inc<.005)inc=.005;
  double top=t+(inc*.9);
  
  while(theta<(top))
    {
      if (theta>t)theta=t;
      double tantheta=tan(theta);
      tantheta=tantheta*tantheta;
      tantheta+=1;
      tantheta*=c2;
      rhomax=radius*radius-tantheta;
      if (rhomax<0)return;
      rhomax=sqrt(rhomax);
      double rhomin=tantheta/m2;
      rhomin=sqrt(rhomax);
      double a=ct+theta;
      setRay(center, rhomax,a, true,rhomin);      
      a=ct-theta;
      setRay(center,rhomax,a, true,rhomin); 
      theta+=inc;
    }
  
  return;
}
void BinaryMatrix::clearHsection(double center[3], 
			       double radius, 
				 double startangle, double endangle)
{
  double c2=center[2]*center[2];
  double r2=radius*radius-c2;
  if (r2<0)return;
  while (startangle>endangle)endangle+=2*M_PI;
  while (startangle<(endangle-2*M_PI))endangle-=2*M_PI;
  double t=(endangle-startangle)/2;
  if (t>1){
    clearHsection(center,radius,startangle, startangle+t/2);
    clearHsection(center,radius,startangle+t/2,endangle);
  }
   if (t*radius<.0001)return;  
  double m2=tan(t); //m*p=sqrt(x2+y2) on inner curves
  m2=m2*m2;
  double ct=(startangle+endangle)/2;  
  double theta=0;
  double rhomax=sqrt(r2);
  double inc=.2/rhomax;
  if (inc<.005)inc=.005;
  double top=t+(inc*.9);
  
  while(theta<(top))
    {
      if (theta>t)theta=t;
      double tantheta=tan(theta);
      tantheta=tantheta*tantheta;
      tantheta+=1;
      tantheta*=c2;
      rhomax=radius*radius-tantheta;
      if (rhomax<0)return;
      rhomax=sqrt(rhomax);
      double rhomin=tantheta/m2;
      rhomin=sqrt(rhomax);
      double a=ct+theta;
      setRay(center, rhomax,a, false,rhomin);      
      a=ct-theta;
      setRay(center,rhomax,a, false,rhomin); 
      theta+=inc;
    }
  
  return;
}
int BinaryMatrix::distanceSearchstep(Cure::ShortMatrix &cells, int next, 
			   BinaryMatrix &visited,
			   Cure::BinaryMatrix &boundary, 
			   Cure::Matrix &celldis)
{
  if (next>=cells.Rows)return 0;
  int  bound=-1; 
  int r=cells(next,0);
  int c=cells(next,1);
  double dis=celldis(r,c)+1.0;
  r--;
  for (int it=0; it<8; it++){
    if (inRange(r,c)){
      if (!visited(r,c)){
	if (!(*this)(r,c)){
	  celldis(r,c)=dis;
	  cells.append(r,0);
	  cells.append(c,1);
	  checkdis(celldis,r,c);
	  visited.setBit(r,c,true);
	} else {
	  bound=1;
	  boundary.setBit(r, c,true);
	}
      }
    }
    if (it==0)r+=2;
    else if(it==1){
      r--;
      c--;
    }
    else if (it==2) c+=2;
    else if (it==3) {
      r--;
      dis+=(M_SQRT2-1.0);
    }
    else if (it==4)r+=2;
    else if(it==5)c-=2;
    else r-=2;
  } 
  return bound;
}
int BinaryMatrix::clusterGrowth( BinaryMatrix &visited, bool eightneighbor, 
				  int r, int c, double distancelimit)
{
  int top=4;
  if (eightneighbor)top=8;
  ShortMatrix cells;
  int lim=10*Rows;
  cells.reallocate(lim,2);
  cells.offset(0,0,1,2);
  cells(0,0)=r;
  cells(0,1)=c;
  double center[2];
  center[0]=(double)r;
  center[1]=(double)c;
  visited.reallocate(Rows,Columns);
  if (distancelimit>0){
    visited.setArc(center,distancelimit,-M_PI-.1,M_PI+.1,true,1.6);
  }
  int i=0;
  while (clusterGrowth(cells, visited,top)){
    if (i+cells.Rows==lim){
      cells.grow(lim+1,2);
      cells.offset(0,0,lim-i,2);
      lim+=1;
    }
    i++;
  }
 
  if (distancelimit>0){
    visited.setArc(center,distancelimit,-M_PI-.1,M_PI+.1,false,1.6);
  }
  return i;
}
double BinaryMatrix::distanceSearch(Cure::ShortMatrix &cells,
				    double distancelimit, 
				    BinaryMatrix &neighborhood,
				    Cure::BinaryMatrix &boundary, 
				    short mindistance[2],
				    Cure::Matrix &celldis)
{
  //   int sizer=Rows;
  //  int sizec=Columns;
  neighborhood.reallocate(Rows,Columns);
  neighborhood.setBit(cells(0,0),cells(0,1),true);
  int lim=(int)(distancelimit*distancelimit/2);
  lim+=8;
  int cl=cells.Columns;

  cells.grow(lim,cl);
  cells.offset(0,0,1,cl);
  celldis.reallocate(Rows,Columns);
  celldis=0;
  boundary.reallocate(Rows,Columns);
  int next=0;
  double mindist=-1;
  int b=distanceSearchstep(cells, next, neighborhood,boundary,celldis);
  double dist=0;
  while (b!=0){
    next++;
    if (next<cells.Rows){
      dist=(celldis(cells(next,0),cells(next,1)));
      while (dist>distancelimit){
	next++;
	if (next<cells.Rows)
	  dist=(celldis(cells(next,0),cells(next,1)));
	else {
	  b=0;
	  dist=0;
	}
      }
    }
    b=distanceSearchstep(cells,  next, neighborhood,boundary,celldis);
    if (cells.Rows==lim){
      cells.grow(lim+100,cl);
      cells.offset(0,0,lim,cl);
      lim+=100;
    }
    if (b>0)
      {
	if (next<cells.Rows)
	  {
	    dist=(celldis(cells(next,0),cells(next,1)));
	    if ((mindist==-1)||(mindist>dist))
	      {
		mindist=dist;
		mindistance[0]=cells(next,0);     
		mindistance[1]=cells(next,1);     
	      }
	  }
      }
  }
  return mindist;
}

bool BinaryMatrix::testArc(Cure::Matrix &velocity,
			   long r,long c,double startangle)
{
  double v=velocity(0,0);
  if (v<0){
    startangle+=M_PI;
    v=-v;
  }  
  if (velocity(0,2)<=0)return false;
  double d=velocity(0,1)*velocity(0,2);
  if ((d>-.001)&&(d<.001))
    {
      //line
      double n[2];
      n[0]=cos(startangle);
      n[1]=sin(startangle);
      d=v*velocity(0,2);
      double s=0;
      double x=r;;
      double y=c;
      while (s<d){
	if (inRange(r,c))
	  {
	    if ((*this)(r,c))return true;
	  }
	else return false;
	s+=1;
	x+=n[0];
	y+=n[1];
	r=(int)(x+.5);
	c=(int)(y+.5);
      }
      return false;
    }
  double radius=v/velocity(0,1);
  
  double dt=1;
  if (radius<0)dt=-1;
  if (radius>1)dt=1/radius;
  if (radius<-1)dt=1/radius;
  double center[2];
  double n[2];
  n[0]=-sin(startangle);
  n[1]=cos(startangle);
  double x=r;
  double y=c;
  center[0]=x+n[0]*radius;
  center[1]=y+n[1]*radius;
  double t=startangle;
  if (d>0){
    d+=t;
    while(t<d){
      if (inRange(r,c))
	{
	  if ((*this)(r,c))return true;
	}
      else return false;
      t+=dt;
      n[0]=-sin(t);
      n[1]=cos(t);
      x=center[0]+n[0]*radius;
      y+=center[1]+n[1]*radius;
      r=(long)(x+.5);
      c=(long)(y+.5);
    }
  }else  if (d<0){
    d+=t;
    while(t>d){
      if (inRange(r,c))
	{
	  if ((*this)(r,c))return true;
	}
      else return false;
      t+=dt;
      n[0]=-sin(t);
      n[1]=cos(t);
      x=center[0]+n[0]*radius;
      y+=center[1]+n[1]*radius;
      r=(long)(x+.5);
      c=(long)(y+.5);
    }
  }
  return false;
}

bool BinaryMatrix::addOrdered(Cure::BinaryMatrix &sig)
{
  long row=0;
  if (findRow(row,sig)){
    return false;
  }
  long r=sig.Rows;   
  if (r!=1)sig.offset(0,0,1,sig.Columns);
  if (row==Rows)grow(Rows+1,Columns);
  else insertRows(row,1);
  long rw=Rows;
  offset(row,0,1,Columns);
  (*this)=sig;
  offset(-row,0,rw,Columns);
  if (r!=1) sig.offset(0,0,r,sig.Columns);
  return true;
}

bool  BinaryMatrix::findRow(long &row,Cure::BinaryMatrix &sig)
{

  if (sig.Rows<1)sig.grow(1,Columns,false);
  if (Rows==0){
    row=0;
    return false;
  }
  if (sig.Columns<Columns){
    sig.grow(sig.Rows,Columns,false);
  }
  else if (sig.Columns>Columns){
    grow(Rows,sig.Columns,false);
  }

 unsigned long k=(Rows>>1);
 unsigned long step=k;
 short test=0;
  while (step)
    {
      if ((long)k>=Rows)k=Rows-1;
      test=rowTest(k,sig);
      if (test==1){
	k+=(step);
      }else if (test==-1){
	if (k==0)break;
	else if (k<step)k=step;
	k-=(step);
      }else if (test==0){
	row=k;
	return true;
      }else {
	std::cerr<<"ERROR in BinaryMatrix::findRow ???????????\n";
	row=-1;
	return false;
      }
      step=(step>>1);
    }
  if ((long)k<Rows){
    test=rowTest(k,sig);
    if (test!=-1){  
      if (test==1){
	k++;
      } else if (test==0){
	row=k;
	return true;
      } else {
	std::cerr<<"ERROR in BinaryMatrix::findRow ???????????\n";
	row=-1;
	return false;
      }
    }
  }else if ((long)k>Rows)k=Rows;
  row=(long)k;
  return false;
}

}
