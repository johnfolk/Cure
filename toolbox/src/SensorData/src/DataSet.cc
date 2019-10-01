// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2005 John Folkesson
//    
#include "DataSet.hh"
#include "CureDebug.hh"
using namespace Cure;

void DataSet::setSetSize(unsigned short len)
{
  if (SetSize<len)
    {
      SmartData *s=Data;
      Data=new SmartData[len];
      for (int i=0; i<SetSize; i++)
	Data[i]=s[i];
      if (s)
	delete[]s;
    }
  SetSize=len;
}

void DataSet::interpolate_(TimestampedData& a, TimestampedData& b, 
			     Timestamp t)
{
  DataSet *spa=a.narrowDataSet();
  DataSet *spb=b.narrowDataSet();
  if ((spa)&&(spb))return interpolate_(*spa,*spb,t);
}
void DataSet::interpolate_(DataSet& a, DataSet& b, 
			     Timestamp t)
{
  if (a.SetSize!=b.SetSize)return;
  if (a.SetSize!=SetSize)return;
  for (int i=0; i<SetSize; i++)
    Data[i].interpolate_(a.Data[i],b.Data[i],t);
}

 bool DataSet::getMatrix(const std::string & str, ShortMatrix & index,
				Matrix &mat)
{
  ShortMatrix offsets=index; 
  if ((offsets.Rows<1)||(offsets.Columns<2))
    return TimestampedData::getMatrix(str,index,mat);
  if (offsets(0,0)>=SetSize)
    return TimestampedData::getMatrix(str,index,mat);
  short ind=offsets(0,0);
  int r=offsets.Rows;
  int c=offsets.Columns;
  offsets.offset(0,1,r,c-1);
  bool ret=true;
  if (ind>-1){
    TimestampedData *td=getTPointer(ind);
    if (td){
      ret=td->getMatrix(str,offsets,mat);
    }else ret=false;
  } 
 else{
   Matrix temp;
    if (SetSize>0)
      if (getTPointer(0)->getMatrix(str,offsets,temp)){
	int col=temp.Columns;
	mat.reallocateZero(SetSize,col);
	if (temp.Rows==0)return true;
	temp.offset(0,0,1,col);
	mat.offset(0,0,1,col);
	mat=temp;
	for (int i=1;i<SetSize;i++)
	  {
	    mat.offset(1,0,1,col);
	    Matrix temp2;
	    ret=(ret&&getTPointer(i)->getMatrix(str,offsets,temp2));
	    if (temp2.Rows==0)return ret;
	    if (temp2.Columns<col)return ret;
	    temp2.offset(0,0,1,col);
	    mat=temp2;
	  }
	// mat.offset(-SetSize+1,0,SetSize,col);
	mat.offset(-SetSize+1,0,SetSize,col);
      }
 }
  return ret;
}
 bool DataSet::setMatrix(const std::string & str, ShortMatrix & index,
				Matrix &mat)
{
  ShortMatrix offsets=index; 
  if ((offsets.Rows<1)||(offsets.Columns<2))
    return TimestampedData::setMatrix(str,index,mat);
  if (offsets(0,0)>=SetSize)
    return TimestampedData::setMatrix(str,index,mat);
  short ind=offsets(0,0);
  int r=offsets.Rows;
  int c=offsets.Columns;
  offsets.offset(0,1,r,c-1);
  bool ret=true;
  if (ind>-1){
    if (ind<SetSize)
      ret=getTPointer(ind)->setMatrix(str,offsets,mat);
    else return false;
  } 
 else{
   ret=false;
   Matrix temp;
    if (SetSize>0)
      if (getTPointer(0)->getMatrix(str,offsets,temp)){
	int col=temp.Columns;
	if (temp.Rows==0)return true;
	if (1!=temp.Rows)return false;
	setSetSize(mat.Rows);
	mat.offset(0,0,1,col);
	ret=true;
	if (getTPointer(0)->setMatrix(str,offsets,mat)){
	mat.offset(1,0,0,0);
	for (int i=1;i<SetSize;i++)
	  {
	    mat.offset(0,0,1,col);
	    ret=(ret&&getTPointer(i)->setMatrix(str,offsets,mat));
	    mat.offset(1,0,0,0);
	  }
	mat.offset(-SetSize,0,SetSize,col);
	}
      }
  }
  return ret;
}
