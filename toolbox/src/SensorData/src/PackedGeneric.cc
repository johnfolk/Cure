// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2005 John Folkesson
//    

#include "PackedGeneric.hh"

using namespace std;

namespace Cure {



int PackedGeneric::VERSION = 1;

PackedGeneric::PackedGeneric(){
  init();
}

PackedGeneric::PackedGeneric(PackedGeneric& p){ 
  PackedGeneric();
  *this=(p);
}

PackedGeneric::PackedGeneric(GenericData& p){ 
  PackedGeneric();
  *this=(p);
}

void PackedGeneric::init()
{
  m_Packed =true;
  Data = 0;
  DataSize = 0;
  ShortData = 0;
  ShortDataSize = 0;
  m_ClassType=(GENERIC_TYPE);
  setSubType(0);
  setID(0);
  Version = VERSION;
 }

PackedGeneric::~PackedGeneric()
{
  if (Data)delete []Data;
  Data = 0;
  DataSize = 0;

  if (ShortData)delete []ShortData;
  ShortData = 0;
  ShortDataSize = 0;
}

void
PackedGeneric::operator=(TimestampedData& p){
  GenericData *s=p.narrowGenericData();
  if (s) {
    operator=(*s);
  } else {
    PackedData *pp=p.narrowPackedGeneric();
    if (pp) {
      // Need to allocate space for the double data before we perform
      // this operation
      setDataSize(pp->getDataSize());

      PackedData::operator=(*pp);
    } else {
      std::cerr << "WARNING: PackedGeneric::operator= logic error!!\n";
    }
  }
}


void PackedGeneric::operator = (const GenericData& p)
{
  Time=p.Time;

  setSubType(p.getSubType());
  long typ=p.ShortData.Rows*p.ShortData.Columns;
  if (m_SubType!=typ)
    setSubType(typ);
  setID(p.getID());

  ShortData[0]=p.ShortData.Rows;
  ShortData[1]=p.ShortData.Columns;
  ShortData[2]=p.Data.Rows;
  ShortData[3]=p.Data.Columns;
  ShortData[4]=p.GenericType;
  int k=5;

  for (int i=0; i<p.ShortData.Rows; i++)
    for (int j=0; j<p.ShortData.Columns; j++)
      {
	ShortData[k]=p.ShortData(i,j);
	k++;
      }
  unsigned long dsize=(p.Data.Rows*p.Data.Columns);
  if (dsize!=DataSize) {
    if (Data) delete []Data;
    if (dsize) Data = new double[dsize];
    else Data=0;
    DataSize=dsize;
  }
  k=0;
  for (int i=0; i<p.Data.Rows; i++)
    for (int j=0; j<p.Data.Columns; j++)
      {
	Data[k]=p.Data(i,j);
	k++;
      }
}

void PackedGeneric::unpack(GenericData& p)
{

  p.setSubType(getSubType());
  p.setID(getID());
  p.Time=Time;
  if (!ShortData)return;
  p.ShortData.reallocate(ShortData[0],ShortData[1]);
  p.Data.reallocate(ShortData[2],ShortData[3]);
  p.GenericType=ShortData[4];
  int k=5;
  for (int i=0; i<p.ShortData.Rows; i++)
    for (int j=0; j<p.ShortData.Columns; j++)
      {
	p.ShortData(i,j)=ShortData[k];
	k++;
      }
  if (!Data)return;
  k=0;
  for (int i=0; i<p.Data.Rows; i++)
    for (int j=0; j<p.Data.Columns; j++)
      {
	p.Data(i,j)=Data[k];
	k++;
      }
  
}

} // namespace Cure
