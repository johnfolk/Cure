// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2004 John Folkesson
//    

#include "PackedMeasurementSet.hh"

using namespace std;

namespace Cure {

const int N_SHORTS_PER_MEAS = 12;
const int N_SHORTS_OVERHEAD = 0;

int PackedMeasurementSet::VERSION = 4;

PackedMeasurementSet::PackedMeasurementSet(){
  init();
}

PackedMeasurementSet::PackedMeasurementSet(PackedMeasurementSet& p){ 
  PackedMeasurementSet();
  *this=(p);
}

PackedMeasurementSet::PackedMeasurementSet(MeasurementSet& p){ 
  PackedMeasurementSet();
  *this=(p);
}

void PackedMeasurementSet::init()
{
  m_Packed =true;
  Data = 0;
  DataSize = 0;
  ShortData = 0;
  ShortDataSize = 0;
  m_ClassType=(MEASUREMENTSET_TYPE);
  setSubType(0);
  setID(0);
  Version = VERSION;
 }

PackedMeasurementSet::~PackedMeasurementSet()
{
  if (Data)delete []Data;
  Data = 0;
  DataSize = 0;

  if (ShortData)delete []ShortData;
  ShortData = 0;
  ShortDataSize = 0;
}

void
PackedMeasurementSet::operator=(TimestampedData& p){
  MeasurementSet *s=p.narrowMeasurementSet();
  if (s) {
    operator=(*s);
  } else {
    PackedData *pp=p.narrowPackedMeasurementSet();
    if (pp) {
      // Need to allocate space for the double data before we perform
      // this operation
      PackedData::operator=(*pp);
    } else {
      std::cerr << "WARNING: PackedMeasurementSet::operator= logic error!!\n";
    }
  }
}
int PackedMeasurementSet::fixShortData(int vers)
{
  if (vers==1){
    if (ShortDataSize<2)return 1;
    if (ShortDataSize==2){
      return 1;
    }else{
      unsigned long sdz=ShortDataSize-2;
      if (sdz%8)return 1;
      long n=(sdz>>3);
      sdz*=10*n;
      sdz+=2;
      unsigned short *sd=new unsigned short[sdz];
      unsigned long ind=2;
      for (unsigned long i=2;i<sdz;i+=10,ind+=8)
	{
	  sd[i]=ShortData[ind];
	  sd[i+1]=0;
	  sd[i+2]=0;
	  memcpy(sd+i+3,ShortData+ind+1,7*sizeof(short));
	}
      delete []ShortData;
      ShortData=sd;
      ShortDataSize=sdz;
      m_SubType =(ShortDataSize-2)/10;
      unsigned long t=(ShortDataSize-2)%10;
      if (t)return 1;
      vers=2;
    }
  }
  if (vers==2){
    if (ShortDataSize<2)return 1;
    unsigned long datasize=(unsigned short)ShortData[0];
    datasize=(datasize<<16);
    datasize+=(unsigned short)ShortData[1];
    setDataSize(datasize);
    if (ShortDataSize==2){
      return 1;
    }else{
      unsigned short *sd=new unsigned short[ShortDataSize-2];
      memcpy(sd,ShortData+2,(ShortDataSize-2)*sizeof(short));
      delete []ShortData;
      ShortData=sd;
      ShortDataSize-=2;
      m_SubType =(ShortDataSize)/10;
      unsigned long t=(ShortDataSize)%10;
      if (t)return 1;
      vers=3;
    }
  }
  if (vers==3){
    unsigned long n=m_SubType;  
    unsigned long sdz=12*n;      
    if (ShortDataSize<10*n)return 1;
    unsigned short *sd=new unsigned short[sdz];
    unsigned long ind=0;
    for (unsigned long i=0;i<sdz;i+=12,ind+=10)
      {
	memcpy(sd+i,ShortData+ind,10*sizeof(short));
	sd[i+10]=0;
	sd[i+11]=0;
      }
    delete []ShortData;
    ShortData=sd;
    ShortDataSize=sdz;
    vers=4;
  }
  if (vers==Version)return 0;
  return 1;
}

void PackedMeasurementSet::setSubType(unsigned short t)
{
  if (m_SubType == 0 || t != m_SubType) {
    m_SubType = t;
    
    // Calculate how many Shorts we need at a min for this
    unsigned long sdz= t;
    sdz *= N_SHORTS_PER_MEAS;
    sdz += N_SHORTS_OVERHEAD;
    if (ShortDataSize<sdz){
      // Allocate space for shorts
      unsigned short *sd=ShortData;
      ShortData = new unsigned short[sdz];    
      memset(ShortData+ShortDataSize, 0, 
	     (sdz-ShortDataSize)*sizeof(unsigned short));
      if (sd){
	memcpy(ShortData,sd,ShortDataSize*sizeof(unsigned short));
	delete []sd;
      }
    } 
  }
}

int PackedMeasurementSet::setShortDataSize(unsigned long len)
{
  if (ShortDataSize==len)return 0;
  if (ShortData) delete []ShortData;
  unsigned long sdz= m_SubType;
  sdz *= N_SHORTS_PER_MEAS;
  sdz += N_SHORTS_OVERHEAD;
  if (len<sdz)len=sdz;
  ShortDataSize=len;
  if (ShortDataSize){
    ShortData = new unsigned short[len];
    memset(ShortData, 0, ShortDataSize*sizeof(unsigned short));
  } else ShortData=0;
  return 0;
}

void PackedMeasurementSet::operator = (const MeasurementSet& p)
{
  Time=p.Time;  
  unsigned short n = getNumberOfElements();
  unsigned long sdz=(n*N_SHORTS_PER_MEAS)+N_SHORTS_OVERHEAD;
  for(int i=0;i<n;i++){
    sdz+=p(i).SW.Rows*p(i).SW.Columns;
  }
  setSubType(p.getSubType());
  //  m_SubType=0;
  setShortDataSize(sdz);
  // m_SubType=p.getSubType();
  setID(p.getID());
  int si = N_SHORTS_OVERHEAD;
  unsigned long dsize=0;
  for(int i = 0; i<n; i++) {
    //mtype,stype,sid,keyH,keyL,Z.Rows,Box.R/C,W.Rows,W.Columns,V.Rows
    Measurement &m=p.Measurements[i];
    
    ShortData[si+0] = m.MeasurementType;
    
    ShortData[si+1] = m.SensorType;
    
    ShortData[si+2] = m.SensorID;
    
    ShortData[si+3] = (unsigned short)(m.Key>>16);
    ShortData[si+4] = (unsigned short)(m.Key&0xFFFF);
    
    // Store dim of Z and inc double counter
    ShortData[si+5]=m.Z.Rows;
    dsize+=m.Z.Rows;
    
    // Store dim of Boudning box and inc double counter for box
    ShortData[si+6]=m.BoundingBox.Rows;
    dsize+=m.BoundingBox.Rows*m.BoundingBox.Rows;
    
    // Store dim of W and inc double counter
    ShortData[si+7]=m.W.Rows;
    ShortData[si+8]=m.W.Columns;
    dsize+=m.W.Rows*m.W.Columns;

    
    // Store dim of V and thus covV and inc double counter for V and CovV
    ShortData[si+9]=m.V.Rows;
    dsize+=m.V.Rows;
    dsize+=m.V.Rows*m.V.Rows;

    // Store dim of W and inc double counter
    ShortData[si+10]=m.SW.Rows;
    ShortData[si+11]=m.SW.Columns;
    si += N_SHORTS_PER_MEAS;
    long rinc=0;
    for(long r=0;r<m.SW.Rows;r++,si+=m.SW.Columns,rinc+=m.SW.RowInc)
      memcpy(&ShortData[si],
	     (unsigned short*)&m.SW.Element[rinc],
	     m.SW.Columns*sizeof(unsigned short));
  } 
  if (dsize!=DataSize) 
    setDataSize(dsize);
  int index=0;
  for(int i=0;i<n; i++) {
    Measurement &m=p.Measurements[i];
    int sz=m.Z.Rows*m.Z.Columns;
    if (sz)memcpy(Data+index,m.Z.Element, sz*sizeof(double)); 
    index+=sz;
    sz=m.BoundingBox.Rows*m.BoundingBox.Rows;
    if (sz)memcpy(Data+index,m.BoundingBox.Element, sz*sizeof(double)); 
    index+=sz;
    sz=m.W.Rows*m.W.Columns;
    long rinc=0;
    if (sz){
      for(long ind=0;ind<m.W.Rows;ind++,index+=m.W.Columns,rinc+=m.W.RowInc)
	memcpy(Data+index,m.W.Element+rinc, m.W.Columns*sizeof(double)); 
    }  
    sz=m.V.Rows;
    if (sz)memcpy(Data+index,m.V.Element, sz*sizeof(double)); 
    index+=sz;
    sz=m.CovV.Rows*m.CovV.Columns;
    if (sz)memcpy(Data+index,m.CovV.Element, sz*sizeof(double)); 
    index+=sz;
  }
}

void PackedMeasurementSet::unpack(MeasurementSet& p)
{  
  p.setNumberOfElements(getNumberOfElements());
  p.setID(getID());
  p.Time=Time;
  if (!ShortData)return;
  unsigned short n = p.getNumberOfElements();
  int si = N_SHORTS_OVERHEAD;
  int index=0;  
  for(unsigned short i = 0; i < n; i++)
    {
      Measurement &m=p.Measurements[i];
      
      m.MeasurementType=ShortData[si];
      
      m.SensorType = ShortData[si+1];

      m.SensorID = ShortData[si+2];

      m.Key=ShortData[si+3];
      m.Key=(m.Key<<16);
      m.Key+=ShortData[si+4];
      
      m.Z.reallocate(ShortData[si+5],1);
      int sz=ShortData[si+5];
      
      if (sz)
	memcpy(m.Z.Element,Data+index, sz*sizeof(double)); 
      index+=sz;

      m.BoundingBox.reallocate(ShortData[si+6]);
      sz=ShortData[si+6]*ShortData[si+6];
      if (sz) memcpy(m.BoundingBox.Element,Data+index, sz*sizeof(double)); 
      index+=sz;

      sz=ShortData[si+7]*ShortData[si+8];
      m.W.reallocate(ShortData[si+7],ShortData[si+8]);
      if (sz){
	long rind=0;
	for(int rw=0; rw<m.W.Rows; rw++, index+=m.W.Columns, rind+=m.W.RowInc){
	  memcpy(m.W.Element+rind, Data+index,  
		 m.W.Columns*sizeof(double)); 
	}
      }
      sz=ShortData[si+9];
      m.V.reallocate(sz,1);
      if (sz)memcpy(m.V.Element,Data+index, sz*sizeof(double)); 
      index+=sz;
      m.CovV.reallocate(sz);
      sz*=sz;
      if (sz)memcpy(m.CovV.Element,Data+index, sz*sizeof(double)); 
      index+=sz;

      m.SW.reallocate(ShortData[si+10],ShortData[si+11]);
      si += N_SHORTS_PER_MEAS;
      long rinc=0;
      for(long r=0;r<m.SW.Rows;r++,rinc+=m.SW.RowInc,si+=m.SW.Columns)
	memcpy((unsigned short*)&m.SW.Element[rinc],
	       &ShortData[si],
	       m.SW.Columns*sizeof(unsigned short));
     
    }
}

} // namespace Cure
