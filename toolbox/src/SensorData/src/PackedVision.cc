// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2005 John Folkesson
//    

#include "PackedVision.hh"

using namespace std;

namespace Cure {

const int N_DATA_OVERHEAD = 0;
const int VISION_SHORT_OVERHEAD = 6;
int PackedVision::VERSION = 1;

PackedVision::PackedVision(){
  init();
}

PackedVision::PackedVision(PackedVision& p){ 
  PackedVision();
  *this=(p);
}

PackedVision::PackedVision(VisionData& p){ 
  PackedVision();
  *this=(p);
}

void PackedVision::init()
{
  m_Packed =true;
  Data = 0;
  DataSize = N_DATA_OVERHEAD;
  ShortDataSize = 0;
  m_ClassType=(VISION_TYPE);
  setSubType(0);
  setID(0);
  Version = VERSION;
 }

PackedVision::~PackedVision()
{
  if (Data)delete []Data;
  Data = 0;
  DataSize = 0;
  if (ShortData)delete []ShortData;
  ShortData = 0;
  ShortDataSize = 0;
}
void
PackedVision::operator=(TimestampedData& p){
  VisionData *s=p.narrowVisionData();
  if (s) {
    operator=(*s);
  } else {
    PackedData *pp=p.narrowPackedVision();
    if (pp) {
      // Need to allocate space for the double data before we perform
      // this operation
      setDataSize(pp->getDataSize());
      PackedData::operator=(*pp);
    } else {
      std::cerr << "WARNING: PackedVision::operator= logic error!!\n";
    }
  }
}

void PackedVision::setSubType(unsigned short type)
{
  if (m_SubType == 0 || type != m_SubType) {
    m_SubType = type;
    ShortDataSize = VISION_SHORT_OVERHEAD;;
  }
  // Allocate space for shorts
  if (ShortData) delete []ShortData;
  ShortData = new unsigned short[ShortDataSize];    
  memset(ShortData, 0, ShortDataSize*sizeof(unsigned short));
} 
int PackedVision::setShortDataSize(unsigned long len)
{
  if (ShortDataSize!=len){
    unsigned short *s=ShortData;
    ShortData = new unsigned short[len];    
    if (s) {
      if (ShortDataSize>len)ShortDataSize=len;
      memcpy(ShortData,s,ShortDataSize*sizeof(unsigned short));
      delete []s;
    }
  }
  ShortDataSize = (len); 
  return 0;
}
void PackedVision::operator = (const VisionData& p)
{
  Time=p.Time;
  setSubType(p.getSubType());
  setID(p.getID());
  ShortData[0]=p.SensorType;
  ShortData[1]=p.SensorID;
  unsigned long t=(unsigned long)p.m_Image;
  ShortData[2]=(0xFFFF&t);
  t=(t>>16);
  ShortData[3]=(0xFFFF&t);
  ShortData[4]=p.m_CameraType;
  t=p.m_FrameNumber;  
  ShortData[5]=(0xFFFF&t);
  t=(t>>16);
  ShortData[6]=(0xFFFF&t);
}

void PackedVision::unpack(VisionData& p)
{
  p.setSubType(getSubType());
  p.setID(getID());
  p.Time=Time;
  if (!Data)return;
  if (!ShortData)return;
  p.SensorType=ShortData[0];
  p.SensorID=ShortData[1];

  unsigned long t=ShortData[2];
  t=(t<<16);
  t+=ShortData[3];
  p.m_Image=(CureImage*)t;
  p.m_CameraType=ShortData[4];
  t=ShortData[5];
  t=(t<<16);
  t+=ShortData[6];
  p.m_FrameNumber=t;
}

} // namespace Cure
