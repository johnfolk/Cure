
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2004 John Folkesson
//    

#include "MeasurementSet.hh"

namespace Cure {

MeasurementSet::MeasurementSet(unsigned short id){
  init();
  setID(id);
}
  
MeasurementSet::MeasurementSet(Timestamp &t,unsigned short id){
  init();
  Time=t;
  setID(id);
}

MeasurementSet::MeasurementSet(const MeasurementSet &src){
  init();
  (*this) = src;
}

MeasurementSet::~MeasurementSet(){
  if (Measurements) delete[] Measurements;
  Measurements=0;
  MeasurementsSize=0;
}

void 
MeasurementSet::init()
{
  m_Packed =false;
  m_ClassType=(MEASUREMENTSET_TYPE);
  setSubType(0);
  setID(0);
  Measurements=0;
  MeasurementsSize=0;
}
  
MeasurementSet& 
MeasurementSet::operator=(const MeasurementSet &src)
{
  setTime(src.getTime());  
  setID(src.getID());  
  setNumberOfElements(src.getNumberOfElements()); // same as setSubType
  unsigned short n = getNumberOfElements();
  for (unsigned short i=0; i<n;i++)Measurements[i]=src.Measurements[i];
  return (*this);
}

void 
MeasurementSet::operator = (TimestampedData& p)
{
  MeasurementSet *ss=p.narrowMeasurementSet();
  if (ss)operator=(*ss);
  else TimestampedData::operator=(p);
}


void 
MeasurementSet::setSubType(unsigned short t)
{
  if (getSubType()==t)return;
  if (MeasurementsSize<t)
    {
      unsigned short s=MeasurementsSize;
      Measurement *m=Measurements;
      if (t>0){
        Measurements=new Measurement[t];
        MeasurementsSize=t;
      }
      else {
        Measurements=0;  
        MeasurementsSize=0;  
      }
      if (m){
	if (s>t)s=t;
	if (s>getSubType())s=getSubType();
	for (unsigned short  i=0; i<s; i++)
	  Measurements[i]=m[i];
	delete[] m;
      }
    }  
  m_SubType = t;
}

void 
MeasurementSet::setupMeasurements()
{
  for (int i=1; i<getNumberOfElements(); i++)
    {
      Measurements[i]=Measurements[0];
    }
}
 bool MeasurementSet::getMatrix(const std::string & str, ShortMatrix & index,
				Matrix &mat)
{
  if ((index.Rows<1)||(index.Columns<5))
  return TimestampedData::getMatrix(str,index,mat);
  if ((index(0,0)<0)||(index(0,1)<0))
    return TimestampedData::getMatrix(str,index,mat);
  if ((index(0,2)<0)||(index(0,3)<0))
    return TimestampedData::getMatrix(str,index,mat);
  if (index(0,4)>=getNumberOfElements())
    return TimestampedData::getMatrix(str,index,mat);
  if (index(0,4)==-1){
    if (str=="SensorID"){
      mat.grow(getNumberOfElements(),1);
      for (int i=0;i<mat.Rows;i++)
	mat(i,0)=Measurements[i].SensorID;
      return true;
    }
    mat.grow(0,index(0,3));
    for (int i=0;i<getNumberOfElements();i++){
      Measurements[i].getMatrix(str,index,mat);
    }
    return true;
  }
  if (Measurements[(index(0,4))].getMatrix(str,index,mat))return true;
  return TimestampedData::getMatrix(str,index,mat);
}

bool MeasurementSet::setMatrix(const std::string & str, ShortMatrix & index,
				Matrix &mat)
{
  if ((index.Rows<1)||(index.Columns<5))
  return TimestampedData::setMatrix(str,index,mat);
  if ((index(0,0)<0)||(index(0,1)<0))
    return TimestampedData::setMatrix(str,index,mat);
  if ((index(0,2)<0)||(index(0,3)<0))
    return TimestampedData::setMatrix(str,index,mat);
  if (index(0,4)>=getNumberOfElements())
    return TimestampedData::setMatrix(str,index,mat);
  if (index(0,4)==-1){
    if (str=="SensorID"){
      if ((mat.Rows<1)||mat.Columns!=1)return false;  
      setNumberOfElements(mat.Rows);
      setSensorID((short)mat(0,0));
      for (int i=0;i<mat.Rows;i++)
	Measurements[i].SensorID=(unsigned short)mat(i,0);
    }
    int r=mat.Rows;
    if (r<getNumberOfElements())
       setNumberOfElements(r);
    mat.offset(0,0,0,0);
    for (int i=0;i<getNumberOfElements();i++){
      if (!Measurements[i].setMatrix(str,index,mat)){
	mat.offset(-i,0,r,index(0,3));
	return false;
      }
    }
    mat.offset(-(getNumberOfElements()),0,r,index(0,3));
    return true;
  }
  if (Measurements[(index(0,4))].setMatrix(str,index,mat))return true;
  return TimestampedData::setMatrix(str,index,mat);
}
void
MeasurementSet::print()
{
  std::cerr << "MeasurmentSet (size=" << MeasurementsSize << ") "<<getSensorType()<<" "<<getSensorID()<<" ";
  TimestampedData::print();
}

} // namespace Cure
