// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2007 John Folkesson
//    

#include "PackedTimestampedString.hh"

using namespace std;

namespace Cure {

const int N_SHORTS_OVERHEAD = 2;

int PackedTimestampedString::VERSION = 1;

PackedTimestampedString::PackedTimestampedString(){
  init();
}

PackedTimestampedString::PackedTimestampedString(PackedTimestampedString& p){ 
  PackedTimestampedString();
  *this=(p);
}

PackedTimestampedString::PackedTimestampedString(TimestampedString& p){ 
  PackedTimestampedString();
  *this=(p);
}

void PackedTimestampedString::init()
{
  m_Packed =true;
  Data = 0;
  DataSize = 0;
  ShortData = 0;
  ShortDataSize = 0;
  m_ClassType=(STRING_TYPE);
  setSubType(0);
  setID(0);
  Version = VERSION;
 }

PackedTimestampedString::~PackedTimestampedString()
{
  if (Data)delete []Data;
  Data = 0;
  DataSize = 0;

  if (ShortData)delete []ShortData;
  ShortData = 0;
  ShortDataSize = 0;
}

void
PackedTimestampedString::operator=(TimestampedData& p){
  TimestampedString *s=p.narrowTimestampedString();
  if (s) {
    operator=(*s);
  } else {
    PackedData *pp=p.narrowPackedTimestampedString();
    if (pp) {
      PackedData::operator=(*pp);
    } else {
      std::cerr << "WARNING: PackedTimestampedString::operator= logic error!!\n";
    }
  }
}

void PackedTimestampedString::setSubType(unsigned short t)
{
  if (m_SubType == 0 || t != m_SubType) {
    m_SubType = t;
    
    // Calculate how many Shorts we need
    unsigned long sdz = (t);
    sdz/=2;
    sdz += N_SHORTS_OVERHEAD;
    setShortDataSize(sdz);
  } 
}
int PackedTimestampedString::setShortDataSize(unsigned long len)
{
    if (ShortDataSize==len)return 0;
    m_SubType =(len-N_SHORTS_OVERHEAD)*2;
    if (ShortData) delete []ShortData;
    ShortDataSize=len;
    if (ShortDataSize){
      ShortData = new unsigned short[len];
      memset(ShortData, 0, ShortDataSize*sizeof(unsigned short));
    }else ShortData=0;
    return 1;
}

void PackedTimestampedString::operator= (const TimestampedString& p)
{
  Time=p.Time;
  unsigned short len=p.m_String.length();
  setSubType(len);
  setID(p.getID());
  ShortData[0]=p.m_StringType;
  char *chr=(char *)(ShortData+1);
  chr[len]=0;
  chr[len-1]=0;
  for (int i=0;i<len;i++)
    chr[i]=p.m_String[i];
  if (Data) delete []Data;
  Data=0;
}

void PackedTimestampedString::unpack(TimestampedString& p)
{
  p.setID(getID());
  p.Time=Time;
  if (!ShortData)return;
  p.m_StringType=ShortData[0];
  char *chr=(char *)(ShortData+1);
  p=chr;;
}

} // namespace Cure
