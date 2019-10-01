// = RCSID
//    $Id: DeadReckoner.cc ,v 1.1 2004/03/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2004 John Folkesson
//    
#include "DeadReckoner.hh"
#include "DataSet.hh"
#include "DebugAddress.hh"

namespace Cure {
  
DeadReckoner::DeadReckoner( long inputbuffer)
  :DataFilter(2, 6, true),
   Cumulator(true),
   m_SlotInA(inputbuffer),
   m_SlotInB( inputbuffer), 
   m_RobotRotate(2*inputbuffer), 
   m_DeadIncremental(2*inputbuffer),
   m_DeadCumulated( 2*inputbuffer)
{
  FilterName = "DeadReckoner"; 
  Poses[3].setSubType(0x8000);
  OutSet.setSetSize(3);
  OutSet(0)=Poses[2];
  OutSet(1)=Poses[3];
  OutSet(2)=Poses[4];
  m_SlotInA.setName("DeadReckoning.SlotInA");
  m_SlotInB.setName("DeadReckoning.SlotInB");
  m_DeadIncremental.setName("DeadReckoning.DeadIncremental");
  m_DeadCumulated.setName("DeadReckoning.DeadCumulated");
  m_RobotRotate.setName("DeadReckoning.RobotRotate");
  m_UseRotate=false;
  m_UseBInput=false;
  m_ModelA=0;
  m_ModelB=0;
  m_Fuser=0;
  m_Period=.05;
  m_BufferTime=1.2;
  m_Start=1;
  PushOut=false;
  FilterName = "DeadReckoner";
  Cumulator.in(0)->pull(&m_DeadIncremental);
  Cumulator.in(1)->pull(&m_DeadCumulated);
}

DeadReckoner::DeadReckoner(PoseErrorModel *modelA,
			   Timestamp & period, 
			   PoseErrorModel *modelB, FuseFilter *fuser, 
			   bool userotate, 
			   long inputbuffer,
			   long outputbuffer)
  :DataFilter( 2, 6, true),
   Cumulator(true),
   m_SlotInA(inputbuffer),
   m_SlotInB( inputbuffer), 
   m_RobotRotate(outputbuffer),
   m_DeadIncremental(outputbuffer),
   m_DeadCumulated( outputbuffer)
{
  m_SlotInA.setName("DeadReckoning.SlotInA");
  m_SlotInB.setName("DeadReckoning.SlotInB");
  m_DeadIncremental.setName("DeadReckoning.DeadIncremental");
  m_DeadCumulated.setName("DeadReckoning.DeadCumulated");
  m_RobotRotate.setName("DeadReckoning.RobotRotate");
  m_UseBInput=false;
  m_ModelA=0;
  m_ModelB=0;
  m_Fuser=0;
  m_Period=period;
  m_BufferTime=1.2;
  m_Start=1;
  FilterName = "DeadReckoner"; 
  Poses[3].setSubType(0x8000);
  PushOut=false;
  OutSet.setSetSize(3);
  OutSet(0)=Poses[2];
  OutSet(1)=Poses[3];
  OutSet(2)=Poses[4];
  setFuser(fuser, userotate); 
  setErrorModel(modelA, 0);
  setErrorModel(modelB, 1);
  Cumulator.in(0)->pull(&m_DeadIncremental);
  Cumulator.in(1)->pull(&m_DeadCumulated);
}
void DeadReckoner::setErrorModel(PoseErrorModel *m, int which)
{
  if (!m)return;
  if (which){
    m_ModelB=m;
    if (m_Fuser) {
      m_Fuser->in(2)->pull(m_ModelB->out(0)); 
      m_Fuser->in(3)->pull(m_ModelB->out(1));
    }
    m_UseBInput=true;
  }
  else{
    m_ModelA=m;
    if (m_Fuser) {
	m_ModelA->out(0)->push(m_Fuser->in(0));
	m_Fuser->in(1)->pull(m_ModelA->out(1)); 
      }
      else{
	m_ModelA->out(0)->push(&m_DeadIncremental);
	m_ModelA->out(1)->push(&m_DeadCumulated);
      }
  }
}
void DeadReckoner::setFuser(FuseFilter *m, bool userotate)
{
  if (!m)return;
  m_UseRotate=userotate;
  m_Fuser=m;
  if (m_ModelB) {
    m_Fuser->in(2)->pull(m_ModelB->out(0)); 
    m_Fuser->in(3)->pull(m_ModelB->out(1));
  }
  if (m_ModelA) {
    m_ModelA->out(0)->disconnect(&m_DeadIncremental);
    m_ModelA->out(1)->disconnect(&m_DeadCumulated);
    m_ModelA->out(0)->push(m_Fuser->in(0));
    m_Fuser->in(1)->pull(m_ModelA->out(1)); 
    m_Fuser->in(0)->pull(m_ModelA->out(0)); 
  }
  if (m_UseRotate)
    m_Fuser->out(0)->push(&m_RobotRotate);
  m_Fuser->out(1)->push(&m_DeadIncremental);
  m_Fuser->out(2)->push(&m_DeadCumulated);
}
unsigned short DeadReckoner::calc()
{
  if (m_Start&7)return 0;
  if (m_ATime<m_NextTime)return 0;
  if (m_UseBInput) 
    if (m_BTime<m_NextTime)return 0;
  Pose3D pA,pB;
  unsigned short r=m_SlotInA.read(pA,m_NextTime);
  if (r&TIMESTAMP_TOO_OLD)
    {
      m_NextTime=pA.Time;
      r=(r&~(TIMESTAMP_TOO_OLD|TIMESTAMP_ERROR));
    }
  if (r)return r;
 
  if (m_UseBInput) {
    r=m_SlotInB.read(pB,m_NextTime);
    if (r&TIMESTAMP_TOO_OLD)
      {
	m_NextTime=pB.Time;
	r=(r&~(TIMESTAMP_TOO_OLD|TIMESTAMP_ERROR));
	if (r)return r;
	r=m_SlotInA.read(pA,m_NextTime);
      }
    if (r)return r;
    m_ModelB->in(0)->write(pB);
  }
  m_ModelA->in(0)->write(pA);
  m_LastTime=pA.Time;
  m_NextTime+=m_Period;
  if (m_Start){
    if (m_Start&8) {
      m_Start=0;

      return 0;
    }
  }
  if (m_UseRotate)
    m_RobotRotate.read(Poses[2]);
  m_DeadIncremental.read(Poses[3]); 
  m_DeadCumulated.read(Poses[4]); 
  OutSet(0)=Poses[2];
  OutSet(1)=Poses[3];
  OutSet(2)=Poses[4];
  Transformation3D trans(Poses[4]);
  trans+=Poses[2];
  trans-=pA;
  Poses[6]=trans;
  Poses[6].Time=Poses[4].Time;
  Poses[7]=Poses[4];
  Poses[7]+=Poses[2];
  PushOut=true;
  return 0;
}
unsigned short DeadReckoner::setOutputs()
{
  if (PushOut)
    for (short i=Ins; i<(Ins+Outs);i++)
      Ports[i].pushData(*data(i));
  PushOut=false;
  return 0;
}

unsigned short DeadReckoner::getInputs(TimestampedData & inputData,int port)
{
  if (!(port<Ins)){
    std::cerr << "\nDataFilter \"" << FilterName <<"\" warning  "
              << "Trying to get input port number "<<port
	      <<" I only have "<<(Ins)<<" input ports.";
    return (RESOURCE_ERROR|ADDRESS_INVALID);
  }
  if (port>-1)
    if (typeCheck(port,&inputData)){
      std::cerr<<"DataFilter \"" << FilterName << "\""
	       << " got an invalid type written to it. \n";
      
      inputData.print();
      return  (RESOURCE_ERROR|TYPE_ERROR);
    }
  if (m_Start&7){
    if (!m_UseBInput) m_Start=(m_Start|4);
    if ((m_Start&2)&&(m_Start&3)){
      m_NextTime+=m_Period;
      m_Start=8;
    }
  }
  if(port==0){
    m_SlotInA.write(inputData);
    m_ATime=inputData.Time;
    if (m_Start&7){
      m_Start=(m_Start|2);
      if (inputData.Time>m_NextTime)
	m_NextTime=inputData.Time;;
    } 
  }
  else if(port==1){
    m_SlotInB.write(inputData);
    m_BTime=inputData.Time;
    if (m_Start&7){
      m_Start=(m_Start|4);
      if (inputData.Time>m_NextTime)
	m_NextTime=inputData.Time;;
    }
  }
  else if(port==-1){
    if (inputData.Time>m_LastTime)
      m_NextTime=inputData.Time;
  }
  Timestamp t=inputData.Time;
  t-=m_NextTime;
  if (t>m_BufferTime)
    m_NextTime=inputData.Time;
  return 0;   
}
unsigned short DeadReckoner::typeCheck(int port,TimestampedData * inputData)
{
  
  unsigned short r=0;
  if (inputData)
    {
      if (inputData->getClassType()==POSE3D_TYPE)
	{
	  if (inputData->isPacked())
	    {
	      inputData->narrowPackedData()->unpack(*data(port));
	    }
	  else *data(port)=*inputData;   
	}
      else if (inputData->narrowSmartData())
	{
	  SmartData *sd=inputData->narrowSmartData();
	  if (sd->getTPointer()->getClassType()==POSE3D_TYPE)
	    {
	      sd->setEqual(*data(port));
	    }
	}    
      else {  
	std::cerr << "\nPoseFilter \"" << FilterName <<"\" warning  "
		  << "Wrong type of input to  "<<port<<":\n";
	return TYPE_ERROR;
      }
    }

  if (!Checked[port])return r;
  Checked[port]--;
  if ((port==0)||(port==1))
    {
      if (((Poses[port].getSubType())&(0x8000))){
	std::cerr << "\nFilter \"" << FilterName <<"\" warning  "
		  << "Must pull cumulated Pose Data to port "<<port;
	Poses[port].print();
	Poses[port].setSubType((Poses[port].getSubType()&0x7FFF));
	Poses[port].zero();
      } 
      else return 0;
    }
  return TYPE_ERROR;
}
unsigned short DeadReckoner::read(TimestampedData& p,const int port, 
				  const Timestamp t,
				  const int interpolate)
{
  if (port==5)
    {
      DataSet *ds=p.narrowDataSet();
      if ((!ds)||(ds->SetSize<3)){
	std::cerr << "\nDataFilter \"" << FilterName <<"\" warning  "
		  <<"read to out (3) must use a DataSet of size >2";
	return TYPE_ERROR;
      }     
      if (t>Poses[3].Time)return  (TIMESTAMP_ERROR|NO_DATA_AVAILABLE); 
      Pose3D tp;
      tp.Time=p.Time;
      unsigned short r=Cumulator.out(0)->read(tp,t);
      if (t==0)
	p.Time=tp.Time;
      if (r) return r;
      (*ds)(1)=tp;
      m_DeadCumulated.read((*ds)(2),t,interpolate);
      if (m_UseRotate)
	m_RobotRotate.read((*ds)(0),t,interpolate);
      else (*ds)(0).Time=tp.Time;
      return 0;
    }
  if(port==6){
    if (t==0){
      p=Poses[6];
      return 0;
    }  
    Pose3D pA, pD, pR;
    unsigned short r=m_DeadCumulated.read(pD,t);
    if (m_UseRotate){
      r=(r|m_RobotRotate.read(pR,t));
      pD+=pR;
    }    
    r=(r|m_SlotInA.read(pA,pD.Time));
    if (r&TIMESTAMP_TOO_OLD)return  (TIMESTAMP_ERROR|TIMESTAMP_TOO_OLD); 
    else if (r&NO_DATA_AVAILABLE)return  (TIMESTAMP_ERROR|NO_DATA_AVAILABLE); 
    else if (r)return RESOURCE_ERROR;
    pD-=pA;
    p=pD;
    return 0;
  }
  if(port==2)
    return m_RobotRotate.read(p,t,interpolate);
  if(port==3)
    return m_DeadIncremental.read(p,t,interpolate);
  if(port==4)
    {
      return m_DeadCumulated.read(p,t,interpolate);
    }
  if(port==7)
    {
      Pose3D pA, pD, pR;   
      unsigned short r=m_DeadCumulated.read(pD);
      if (m_UseRotate){
	r=(r|m_RobotRotate.read(pR));
	pD+=pR;
      }    
      r=(r|m_SlotInA.read(pA,pD.Time));
      if (r&TIMESTAMP_TOO_OLD)return  (TIMESTAMP_ERROR|TIMESTAMP_TOO_OLD); 
      else if (r&NO_DATA_AVAILABLE)return  (TIMESTAMP_ERROR|NO_DATA_AVAILABLE);
      else if (r)return RESOURCE_ERROR;
      pD-=pA;
      r=(m_SlotInA.read(pA,t,interpolate));
      pD+=pA;
      pD.Time=pA.Time;
      p=pD;
      return 0;
    }
  std::cerr << "\nDataFilter \"" << FilterName <<"\" warning  "
	    << "Trying to read port number "<<port
	    <<" I only have "<<(Ins+Outs)<<"  ports.";
  return ADDRESS_INVALID;
}



} // namespace Cure
