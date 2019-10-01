// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2005 John Folkesson
//    

#include "PackedGrid.hh"

using namespace std;

namespace Cure {

const int N_SHORTS_OVERHEAD = 7;

int PackedGrid::VERSION = 1;

PackedGrid::PackedGrid(){
  init();
}

PackedGrid::PackedGrid(PackedGrid& p){ 
  PackedGrid();
  *this=(p);
}

PackedGrid::PackedGrid(GridData& p){ 
  PackedGrid();
  *this=(p);
}

void PackedGrid::init()
{
  m_Packed =true;

  setDataSize(4);
  ShortData = 0;
  ShortDataSize = 0;
  m_ClassType=(GRID_TYPE);
  setSubType(1);
  setID(0);
  Version = VERSION;
 }

PackedGrid::~PackedGrid()
{
  if (Data)delete []Data;
  Data = 0;
  DataSize = 0;

  if (ShortData)delete []ShortData;
  ShortData = 0;
  ShortDataSize = 0;
}

void
PackedGrid::operator=(TimestampedData& p){
  GridData *s=p.narrowGridData();
  if (s) {
    operator=(*s);
  } else {
    PackedData *pp=p.narrowPackedGrid();
    if (pp) {
      PackedData::operator=(*pp);
    } else {
      std::cerr << "WARNING: PackedGrid::operator= logic error!!\n";
    }
  }
}

void PackedGrid::setSubType(unsigned short t)
{
  m_SubType = t;
}
int PackedGrid::setShortDataSize(unsigned long len)
{
   if (ShortDataSize==len)return 0;
   ShortDataSize = len;  
   if (ShortData) delete [] ShortData;
   if (ShortDataSize){
     ShortData = new unsigned short[len];
     memset(ShortData, 0, ShortDataSize*sizeof(unsigned short));
   } else ShortData=0; 
   return 0;
}
void PackedGrid::operator = (const GridData& p)
{
  Time=p.Time;
  setSubType(p.getSubType());
  setID(p.getID());
  Data[0]=p.CellWidth;
  Data[1]=p.Offset[0];
  Data[2]=p.Offset[1];
  Data[3]=p.Offset[2];
  unsigned long len=p.Grid.Rows*p.Grid.Columns;
  int a=len%16;
  if (a>0) a=1;
  else a=0;
  len/=16;
  len+=a;
  len+=N_SHORTS_OVERHEAD;
  setShortDataSize(len);
  
  ShortData[0]=p.Grid.Rows;
  ShortData[1]=p.Grid.Columns;
  ShortData[2]=p.SensorType;
  ShortData[3]=(unsigned short)((0xFFFF)&(p.Birthday.Seconds>>16));
  ShortData[4]=(unsigned short)((0xFFFF)&(p.Birthday.Seconds));
  ShortData[5]=(unsigned short)((0xFFFF)&(p.Birthday.Microsec>>16));
  ShortData[6]=(unsigned short)((0xFFFF)&(p.Birthday.Microsec));
  
  int k=N_SHORTS_OVERHEAD;
  unsigned short t=0;
  int count=0;
  
  for (int i=0; i<p.Grid.Rows; i++)
    for (int j=0; j<p.Grid.Columns; j++)
      {
	t=(t<<1);
	if (p.Grid(i,j))
	  t=(t|1);
	count++;
	if (count==16)
	  {
	    ShortData[k]=t;
	    k++;
	    t=0;
	    count=0;
	  }
      }
}

void PackedGrid::unpack(GridData& p)
{

  p.setSubType(getSubType());
  p.setID(getID());
  p.Time=Time;
  p.CellWidth=Data[0];
  p.Offset[0]=Data[1];
  p.Offset[1]=Data[2];
  p.Offset[2]=Data[3];
  if (!ShortData)return;
  p.Grid.grow(ShortData[0],ShortData[1]);
  p.SensorType=ShortData[2];
  long sec=ShortData[3];
  sec=(sec<<16);
  sec=(sec|(long)ShortData[4]);
  p.Birthday.Seconds=sec;
  sec=ShortData[5];
  sec=(sec<<16);
  sec=(sec|(long)ShortData[6]);
  p.Birthday.Microsec=sec;
 
  int i=0;
  int j=0;
  for (int k=N_SHORTS_OVERHEAD; k<(int)ShortDataSize-1; k++)
    {
      unsigned short t=ShortData[k];
      for (int count=0; count<16; count++)
	{
	  bool v=(t&0x8000);
	  t=(t<<1);
	  p.Grid.setBit(i,j,v);
	  j++;
	  if (j==p.Grid.Columns){
	    j=0;
	    i++;
	  }
	}
    }
  unsigned short t=ShortData[ShortDataSize-1];
  unsigned long len=p.Grid.Rows*p.Grid.Columns;
  int a=len%16;
  i=p.Grid.Rows-1;
  j=p.Grid.Columns-1;
  for (int k=0; k<a; k++)
    {
      bool v=(t&0x1);
      t=(t>>1);
      p.Grid.setBit(i,j,v);
      j--;
      if (j==-1){
	j=p.Grid.Columns-1;
	i--;
      }
    }
}

} // namespace Cure
