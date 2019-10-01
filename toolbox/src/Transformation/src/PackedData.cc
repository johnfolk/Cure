// = RCSID
//    $Id: PackedData.cc ,v 1.1 2004/03/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2004 John Folkesson
//    

#include "PackedData.hh"

#ifndef DEPEND
#include <iostream>
#include <string.h>  // memcpy
#endif

using namespace std;
namespace Cure {

PackedData::PackedData(){
  init();
};

PackedData::PackedData(PackedData& p){ 
  init();
  *this=(p);
}

PackedData::PackedData(TimestampedData& p){     
  init();
  *this=(p);
}

void PackedData::init()
{  
  ShortData = 0;
  ShortDataSize = 0;
  Data = 0;
  DataSize = 0;
  m_Packed = true;
  m_ClassType=(TIMESTAMP_TYPE);
  setID(0);
  Version = 0;
}

PackedData::~PackedData()
{
  if (Data)delete []Data;
  Data = 0;
  DataSize = 0;

  if (ShortData)delete []ShortData;
  ShortData = 0;
  ShortDataSize = 0;
}

unsigned long
PackedData::setDataSize(unsigned long len) 
{
  if (DataSize == len) return DataSize;
  if (Data) delete [] Data;
  DataSize = len;
  if (DataSize){
    Data = new double[len];
    memset(Data, 0, DataSize*sizeof(double));
  } else Data=0;
  return DataSize;
}

bool PackedData::getMatrix(const std::string & str, ShortMatrix & index,
			 Matrix &mat)
{
  TimestampedData *td=makeUnpack();
  bool ret=td->getMatrix(str,index,mat);
  delete td;
  return ret;
}
bool PackedData::setMatrix(const std::string & str, ShortMatrix & index,
			 Matrix &mat)
{
  TimestampedData *td=makeUnpack();
  bool ret=td->setMatrix(str,index,mat);
  *this=*td;
  delete td;
  return ret;
}

int PackedData::operator == (const PackedData& p)const
{
  if (!(p.Time == Time))return 0;
  if (!(p.isPacked() == isPacked())) return 0;
  if (!(p.getClassType() == getClassType())) return 0;
  if (!(p.getSubType() == getSubType())) return 0;
  if (!(p.getID() == getID())) return 0;

  unsigned long top=getDataSize();
  if (!(top==p.getDataSize()))return 0;
  for (unsigned long i=0; i<top; i++)
    if (Data[i]!=p.Data[i])return 0;
    top=getShortDataSize();
    if (!(top==p.getShortDataSize()))return 0;
    for (unsigned long i=0; i<top; i++)
      if (ShortData[i]!=p.ShortData[i])return 0;
    return 1;
}

void PackedData::operator = (const PackedData& p)  
{
   Time=p.Time;

   if ( !(getClassType() == p.getClassType()) ) {
     std::cerr << "PackedData::operator Cannot assign classType "
               << int(getClassType()) << " to " 
               << int(p.getClassType()) << std::endl;
     return;
   }
   setID(p.getID());
   setShortDataSize(p.getShortDataSize());
   setDataSize(p.getDataSize());
   setSubType(p.getSubType());

   // Copy only min(ShortDataLen, p.ShortDataLen) shorts
   unsigned long top = getShortDataSize();
   if (top > p.getShortDataSize()) top = p.getShortDataSize();
   memcpy(ShortData,p.ShortData, top * sizeof(short));

   // Copy only min(DataLen, p.DataLen) doubles
   top = getDataSize();
   if (top > p.getDataSize()) top = p.getDataSize();
   memcpy(Data, p.Data, top * sizeof(double));
}

void PackedData::print()
{
  TimestampedData::print();
  using namespace std;
  std::cerr << "Short size " << getShortDataSize()
            << " Double size " << getDataSize() << endl;
}

} // namespace Cure
