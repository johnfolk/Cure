//
// = LIBRARY
//
// = FILENAME
//    GenericData.cc
//
// = AUTHOR(S)
//    john Folkesson
//
// = COPYRIGHT
//    Copyright (c) 2005 John Folkesson
//
/*----------------------------------------------------------------------*/

#include "GenericData.hh"

namespace Cure {

GenericData::GenericData(unsigned short id)
{
  init();
  setID(id);
}

GenericData::GenericData(Timestamp &t, unsigned short id)
{
  init();
  Time=t;
  setID(id);
}

/**
 * Create a copy of a GenericData
 */
GenericData::GenericData(const GenericData &src)
{
  init();
  (*this) = src;
}

GenericData::~GenericData()
{}


void GenericData::init()
{
  m_Packed =false;
  m_ClassType=(GENERIC_TYPE);
  setSubType(0);
  setID(0);
  GenericType = 0;
}

GenericData& GenericData::operator=(const GenericData &src)
{
  Time=src.Time;
  setSubType(src.getSubType());
  setID(src.getID());
  GenericType = src.GenericType;
  Data=src.Data;
  ShortData=src.ShortData;
  return (*this);
}
void GenericData::operator=(PoseTree &tree){
  tree.save(ShortData,Data);
  setTime(tree.m_Time);
} 
int GenericData::getTree(PoseTree &tree){
  return tree.restore(ShortData,Data,Time);
}

void GenericData::setShortDataSize(int rows, int columns)
{
  ShortData.reallocate(rows,columns);
  ShortData=0;
  unsigned short t=ShortData.Rows*ShortData.Columns;
  setSubType(t);
}
void GenericData::forceShortDataSize(int rows, int columns)
{
  if ((ShortData.Rows!=rows)||(ShortData.Columns!=columns)){
    ShortData.grow(rows,columns);
    unsigned short t=ShortData.Rows*ShortData.Columns;
    setSubType(t);
  }
}

void GenericData::setSubType(unsigned short t)
{
  m_SubType = t; 
  if (t!=(ShortData.Rows*ShortData.Columns))
      ShortData.reallocate(1,t);
}
void GenericData::interpolate_(GenericData& a, GenericData& b, Timestamp t)
{
  if (a.Time==t){
    *this=a;
    return;
  }
  if (b.Time==t){
    *this=b;
     return;
  }
  if (b.Time==a.Time){
    *this=b;
     return;
  }
  if (!(a.GenericType&0x8000))    
    return TimestampedData::interpolate_(a,b,t);
  if (a.m_SubType!=b.m_SubType)return;
  if (a.Data.Rows!=b.Data.Rows)return;
  if (a.Data.Columns!=b.Data.Columns)return;
  if(a.Time<b.Time)
    {
      (*this)=b;
      Time=t;
      t-=a.Time;
      double w=t.getDouble();
      t=b.Time;
      t-=a.Time;
      w/=t.getDouble();
      Matrix adata=a.Data;
      Data*=(w);
      adata*=(1-w);
      Data+=adata;
    }
  else
    {
      (*this)=a;
      Time=t;
      t-=b.Time;
      double w=t.getDouble();
      t=a.Time;
      t-=b.Time;
      w/=t.getDouble();
      Matrix bdata=b.Data;
      Data*=(w);
      bdata*=(1-w);
      Data+=bdata;
    }
}
bool GenericData::getMatrix(const std::string & str, ShortMatrix & index,
		       Cure::Matrix & mat)
{
  if ((index.Rows<1)||(index.Columns<4))
  return TimestampedData::getMatrix(str,index,mat);
  if ((index(0,0)<0)||(index(0,1)<0))
    return TimestampedData::getMatrix(str,index,mat);
  if ((index(0,2)<0)||(index(0,3)<0))
    return TimestampedData::getMatrix(str,index,mat);
  int n=0;
  Cure::Matrix x;
  if ((str=="Data")||(str=="data")){
    n=Data.Rows*Data.Columns;
    x=Data;
  } else if ((str=="ShortData")||(str=="shortData")){
    n=ShortData.Rows*ShortData.Columns;
    x.reallocate(ShortData.Rows,ShortData.Columns);
    for (int i=0;i<x.Rows;i++)
      for (int j=0;j<x.Columns;j++){
	x(i,j)=(double)ShortData(i,j);
      }
  } else if ((str=="ShortDataRows")||(str=="shortDataRows")){
    n=1;
    x.reallocate(1,1);
    x(0,0)=(double)ShortData.Rows;   
  } else if (str=="GenericType"){
    n=1;
    x.reallocate(1,1);
    x(0,0)=(double)GenericType;
        }
 
 if (n>0){
    mat.reallocate(index(0,2),index(0,3));
    int r=index(0,0)+index(0,2);
    int c=index(0,1)+index(0,3);
    if ((x.Rows)<r)
      return false;
    if ((x.Columns)<c)
      return false;
    x.offset(index(0,0),index(0,1),index(0,2),index(0,3));
    mat=x;
    return true;
  }
 return TimestampedData::getMatrix(str,index,mat);
}
bool GenericData::setMatrix(const std::string & str, ShortMatrix & index,
		       Cure::Matrix & mat)
{
  if ((index.Rows<1)||(index.Columns<4))
  return TimestampedData::setMatrix(str,index,mat);
  if ((index(0,0)<0)||(index(0,1)<0))
    return TimestampedData::setMatrix(str,index,mat);
  if ((index(0,2)<0)||(index(0,3)<0))
    return TimestampedData::setMatrix(str,index,mat);
  if ((str=="Data")||(str=="data")){
    int n=Data.Rows*Data.Columns;
    int r=index(0,0)+index(0,2);
    int c=index(0,1)+index(0,3);
    int rr=Data.Rows; 
    int cc=Data.Columns; 
    if ((r*c>n)){
      if (rr<r)rr=r;
      if (cc<c)cc=c;
      Data.grow(rr,cc);
    }
    Data.offset(index(0,0),index(0,1),index(0,2),index(0,3));
    Data=mat;
    Data.offset(-index(0,0),-index(0,1),rr,cc);
  } else if ((str=="ShortData")||(str=="shortData")){
    int n=ShortData.Rows*ShortData.Columns;
    int r=index(0,0)+index(0,2);
    int c=index(0,1)+index(0,3);
    int rr=ShortData.Rows; 
    int cc=ShortData.Columns; 
    if ((r*c>n)){
      if (rr<r)rr=r;
      if (cc<c)cc=c;
      ShortData.grow(rr,cc);
    }
    ShortData.offset(index(0,0),index(0,1),index(0,2),index(0,3));
    for (int i=0;i<mat.Rows;i++)
      for (int j=0;j<mat.Columns;j++)
	ShortData(i,j)=(short)mat(i,j);
    ShortData.offset(-index(0,0),-index(0,1),rr,cc);      
  } else if ((str=="ShortDataRows")||(str=="shortDataRows")){
    if ((mat.Rows!=1)||(mat.Columns!=1))return false;
    forceShortDataSize((int)mat(0,0),ShortData.Columns);
  } else if ((str=="ShortDataColumns")||(str=="shortDataColumns")){
    if ((mat.Rows!=1)||(mat.Columns!=1))return false;
    forceShortDataSize(ShortData.Rows,(int)mat(0,0));
  } else if (str=="GenericType"){
    if ((mat.Rows!=1)||(mat.Columns!=1))return false;
    GenericType=(unsigned short)mat(0,0);
  }else return TimestampedData::setMatrix(str,index,mat);
  return true;
}

void GenericData::print()
{
  Time.print();
  std::cerr<< "GenericType=" << GenericType<< std::endl;
  std::cerr<<"Data= ";
    Data.print();
    std::cerr<<"ShortData= ";
    ShortData.print();
}

} // namespace Cure

std::ostream& 
operator << (std::ostream& os, const Cure::GenericData &cmd)
{
  os << "GenericType=" << cmd.GenericType<< std::endl;
  return os;
}
