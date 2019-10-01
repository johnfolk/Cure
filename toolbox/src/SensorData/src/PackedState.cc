// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2006 John Folkesson
//    

#include "PackedState.hh"

using namespace std;

namespace Cure {

const int N_SHORTS_OVERHEAD = 7;

int PackedState::VERSION = 1;

PackedState::PackedState(){
  init();
}

PackedState::PackedState(PackedState& p){ 
  PackedState();
  *this=(p);
}

PackedState::PackedState(StateData& p){ 
  PackedState();
  *this=(p);
}

void PackedState::init()
{
  m_Packed =true;
  Data = 0;
  DataSize = 0;
  ShortData = 0;
  ShortDataSize = 0;
  m_ClassType=(STATE_TYPE);
  setSubType(0);
  setID(0);
  Version = VERSION;
 }

PackedState::~PackedState()
{
  if (Data)delete []Data;
  Data = 0;
  DataSize = 0;

  if (ShortData)delete []ShortData;
  ShortData = 0;
  ShortDataSize = 0;
}

void
PackedState::operator=(TimestampedData& p){
  StateData *s=p.narrowStateData();
  if (s) {
    operator=(*s);
  } else {
    PackedData *pp=p.narrowPackedState();
    if (pp) {
      // Need to allocate space for the double data before we perform
      // this operation
      setDataSize(pp->getDataSize());

      PackedData::operator=(*pp);
    } else {
      std::cerr << "WARNING: PackedState::operator= logic error!!\n";
    }
  }
}

void PackedState::setSubType(unsigned short t)
{
  if (m_SubType == 0 || t != m_SubType) {
    m_SubType = t;
    
    // Calculate how many Shorts we need
    unsigned long sdz = (t);
    sdz += N_SHORTS_OVERHEAD;
    setShortDataSize(sdz);
  }
}

int PackedState::setShortDataSize(unsigned long len)
{
    if (ShortDataSize==len)return 0;
    m_SubType =len-N_SHORTS_OVERHEAD;
    if (ShortData) delete []ShortData;
    ShortDataSize=len;
    if (ShortDataSize){
      ShortData = new unsigned short[len];
      memset(ShortData, 0, ShortDataSize*sizeof(unsigned short));
    } else ShortData=0;
    return 1;
}


void PackedState::operator = (const StateData& p)
{
  Time=p.Time;
  setSubType(p.StateID.Columns*p.StateID.Rows);
  setID(p.getID());
  ShortData[0]=p.State.Rows;
  ShortData[1]=p.m_Flags;
  ShortData[2]=p.StateType;
  ShortData[3]=p.Covariance.Rows;
  ShortData[4]=p.Jacobian.Rows;
  ShortData[5]=p.Jacobian.Columns;
  ShortData[6]=p.StateID.Columns;
  int k=N_SHORTS_OVERHEAD;

  for (long i=0; i<p.StateID.Rows; i++)
    for (long j=0;j<p.StateID.Columns;j++)
      {
	ShortData[k]=p.StateID(i,j);
	k++;
      }
  unsigned long dsize=(p.Jacobian.Rows*p.Jacobian.Columns);
  dsize+=p.State.Rows;
  dsize+=p.Covariance.Rows*p.Covariance.Rows;
  if (dsize!=DataSize) {
    if (Data) delete []Data;
    if (dsize) Data = new double[dsize];
    else Data=0;
    DataSize=dsize;
  }
  k=0;
  for (int i=0; i<p.State.Rows; i++)
    {
	Data[k]=p.State(i,0);
	k++;
    }
  for (int i=0; i<p.Covariance.Rows; i++)
    for (int j=0; j<p.Covariance.Rows; j++)
      {
	Data[k]=p.Covariance(i,j);
	k++;
      }
  for (int i=0; i<p.Jacobian.Rows; i++)
    for (int j=0; j<p.Jacobian.Columns; j++)
      {
	Data[k]=p.Jacobian(i,j);
	k++;
      }
}

void PackedState::unpack(StateData& p)
{

  unsigned short c=(unsigned short)ShortData[6];
  if (c>0)
    p.setIDDim(m_SubType/c,c);
  else p.setSubType(0);
  p.setID(getID());
  p.Time=Time;
  if (!Data)return;
  if (!ShortData)return;
  p.setDim(ShortData[0]);
  p.setCovarianceDim(ShortData[3]);
  p.setJacobianDim(ShortData[4],ShortData[5]);
  p.setFlags(ShortData[1]);
  p.StateType=ShortData[2];
  int k=N_SHORTS_OVERHEAD;
  for (int i=0; i<p.StateID.Rows; i++)
    for (unsigned int j=0;j<c;j++)
    {
      p.StateID(i,j)=ShortData[k];
      k++;
    }
  k=0;
  for (int i=0; i<p.State.Rows; i++)
    {
      p.State(i,0)=Data[k];
      k++;
    }
  for (int i=0; i<p.Covariance.Rows; i++)
    for (int j=0; j<p.Covariance.Rows; j++)
      {
	p.Covariance(i,j)=Data[k];
	k++;
      }
  for (int i=0; i<p.Jacobian.Rows; i++)
    for (int j=0; j<p.Jacobian.Columns; j++)
      {
	p.Jacobian(i,j)=Data[k];
	k++;
      }  
}

} // namespace Cure
