// = RCSID
//    $Id: SLAMFilter.cc ,v 1.1 2004/03/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2004 John Folkesson
//    


#include "SLAMFilter.hh"
#include "ArithmaticFilter.hh"
#include "DataSlotAddress.hh"
#include "MeasurementSet.hh"
#include "SmartData.hh"
#include "CureDebug.hh"

namespace Cure {

SLAMFilter::SLAMFilter(short numin, short numout,
		       short numberOfSensors,
		       short msetSize)
  :DataFilter(numin, numout, true), 
   m_Cumulator(false)    
{
  AllwaysPredictSensor=0;
  SensorIndex=0;
  LastSensorTime=0;
  if (numout<4)  {
    std::cerr << "\nFilter \"" << FilterName <<"\" warning  "
              << "Needs at least "<<(4)<<" Outputs "
	      <<" I only have "<<(Outs)<<" output ports.\n";  
    std::cerr<<" You will soon get a seg fault\n";
  }
  if (numin<4+numberOfSensors){
    std::cerr << "\nFilter \"" << FilterName <<"\" warning  "
              << "Needs at least "<<(4+numberOfSensors)<<" Ins "
	      <<" I only have "<<(Ins)<<" input ports.\n";  
    numberOfSensors=Ins-4;
  }
  if (numberOfSensors<1){
    std::cerr<<FilterName<<" must have at least 1 sensor\n";
    numberOfSensors=1;
  }
  NumberOfSensors=numberOfSensors;
  AllwaysPredictSensor=new bool[NumberOfSensors];
  SensorIndex=new int[NumberOfSensors];
  RobotSensorPose=new SensorPose[NumberOfSensors];
  LastSensorTime=new Timestamp[NumberOfSensors];
  for (short i=0; i<NumberOfSensors;i++){
    Ports[4+i].setDataNeeded(false);
    AllwaysPredictSensor[i]=false;
    SensorIndex[i]=-1;
  }
  MSet.setNumberOfElements(msetSize);
  MSet.Time = 0.0;

  Data=new SmartData[Ins+Outs+1];
  Data++;
  Pose3D p;
  Data[0]=p;
  Data[1]=p;
  Data[2]=p;
  Data[3]=MSet;
  for (short i=0; i<NumberOfSensors;i++)
    Data[4+i]=p;
  Data[Ins]=p;
  Data[Ins+1]=p;
  Data[Ins+2]=p;
  Data[Ins+3]=p;
  LastTime=0.0;
  FilterName = "SLAMFilter";
  m_Cumulator.FilterName = "SLAMFilter.PoseCumulator";
  in(1)->setSequenced(LastTime);  
  m_Cumulator.in(0)->pull(&Ports[1]);
  m_Cumulator.in(1)->pull(&Ports[2]);
  Ports[-1].setInterpolate(false);
  Ports[3].setInterpolate(false);
  Ports[-1].setDataNeeded(false);
  Ports[3].setDataNeeded(false);
}


SLAMFilter::~SLAMFilter()
{    
  if(AllwaysPredictSensor)
    delete []AllwaysPredictSensor;
 AllwaysPredictSensor=0;
  if(SensorIndex)
    delete []SensorIndex;
  SensorIndex=0;
  if(RobotSensorPose)
    delete []RobotSensorPose;
  RobotSensorPose=0;
  if (LastSensorTime)
    delete []LastSensorTime;
  LastSensorTime=0;
  if (Data) {
    Data--;
    delete [] Data;
    Data = 0;
  }
}

int
SLAMFilter::getSensorIndex()
{
  if (MSet.getNumberOfElements() == 0) {
    // No measurements means that we have nothing to update with and
    // that we cannot get the feature key
    //But we might as well do predict so use the first index
   return SensorIndex[0]; 
  }
  int st = MSet.Measurements[0].SensorType;
  int id = MSet.Measurements[0].SensorID;

  CureCERR(50) << "Looking for sensor st=" << st << " id=" << id << std::endl;

  for (int i=0;i<NumberOfSensors;i++) {
    CureCERR(50) << "Comparing with st=" << RobotSensorPose[i].m_SensorType
                 << " id=" << RobotSensorPose[i].m_SensorID
                 << std::endl;
    if (st==RobotSensorPose[i].m_SensorType &&
        id==RobotSensorPose[i].m_SensorID) {
      return SensorIndex[i]; 
    }
  }
  CureCERR(20) << "WARNING: Could not look up sensor type for st="
	       << st 
               << " (sick is " << Cure::SensorData::SENSORTYPE_SICK <<") "
               << " id=" << id << std::endl;
  return -1;
}

unsigned short SLAMFilter::read(TimestampedData& p,const int port, 
				const Timestamp t,
				const int interpolate)
{
  if (port==1){
    int i=interpolate;
    Timestamp t1=t;
    Ports[port].setSequenced(t1,i);
    return Ports[port].pullData(p,t);
  }
  if (port==2){
    return Ports[port].pullData(p,t);
  }
  if (!(port<Ins+Outs)){
    std::cerr << "\nDataFilter \"" << FilterName <<"\" warning  "
              << "Trying to read port number "<<port
	      <<" I only have "<<(Ins+Outs)<<"  ports.";
    return (RESOURCE_ERROR|ADDRESS_INVALID);
  }
  if (port<Ins){
    std::cerr << "\nDataFilter \"" << FilterName <<"\" warning  "
              << "Trying to read and input port number "<<port
	      <<" I have "<<(Ins)<<" input ports.";
    return (RESOURCE_ERROR|ADDRESS_INVALID);
  }
  setEqual(port,p);
  return 0;
}

unsigned short SLAMFilter::write(TimestampedData& td,const int port)
{
  if ((port==1)||(port==2)){
    std::cerr << "\nFilter \"" << FilterName <<"\" warning  "
              << "Trying to write to  port number "<<port
	      <<" I only pull data from this port.\n";
    return (RESOURCE_ERROR|ADDRESS_INVALID);
  }
  if(LastTime>td.Time)return (TIMESTAMP_ERROR|TIMESTAMP_TOO_OLD);
  if ((!(port<Ins))||(port<-1)){
    std::cerr << "\nFilter \"" << FilterName <<"\" warning  "
              << "Trying to write to  port number "<<port
	      <<" I only have "<<(Ins)<<" input ports.\n";
    return (RESOURCE_ERROR|ADDRESS_INVALID);
  }

  if (typeCheck(port,&td)){
	std::cerr<<"Filter \"" << FilterName << "\""
                 << " got an invalid type written to it. "<<std::endl; 
	return TYPE_ERROR;
  }

  CureDO(60) { CureCERR(0) << "Written to with "; td.print(); }
  // Try to perform the updates that have been put off so far
  if (port==3) {
    // We got a measurement
    PendingDataList.add(td);
  }
  while (true) {
    // Get the first item in the list of stored data
    if (PendingDataList.get(0,Data[3])) break;   
    if (Data[3].Time>=LastTime){
      Data[3].setEqual(MSet);
      int ind=getSensorIndex();
      int r1=0;
      if (ind>-1)
	{
	  r1=getDead(Data[3].Time,ind);//***********Calls Predict************''
	if (r1)
	  {
	    if (r1==TYPE_ERROR)return r1;
	    else if (!(r1&TIMESTAMP_TOO_OLD))return 0;
	  }
	}
      //Now the Filter should be have predicted this sensor pose 
      if (!r1)
	if (Data[3].Time==LastTime)
	  if (MSet.getNumberOfElements() != 0) 
	    if (update())return 0;//***************HERE IS Update***********      
    }
    PendingDataList.remove(0);
    Data[Ins+3]=(PoseOut);
    setOutputs();
  }
  return 0;
}
int SLAMFilter::getDead(Timestamp &t, int ind)
{ 
  Pose3D p;   
  unsigned short r=m_Cumulator.out(1)->read(p,t);
  if (r) return r;
 
  Data[Ins+2]=p;   
  m_Cumulator.out(0)->read(p,t);
  Data[Ins+1]=p;
  Data[Ins+1].setEqual(DeadInc);
  Data[Ins+2].setEqual(DeadCum);
  if (!(Ports[0].pullData(p,t))){
    if (typeCheck(0,&p)){
      std::cerr<<"Filter \"" << FilterName << "\""
	       << " got an invalid type on pulling extra rotation. "
	       <<std::endl; 
      return TYPE_ERROR;
    }
  } 
  Data[0].setEqual(XtraRot);
  Data[Ins]=Data[0];
  if (ind>-1){
    unsigned short r1=0;
    if (LastSensorTime[ind]!=MSet.Time){
      if (in(4+ind)->IsPull)
	{
	  r1=Ports[4+ind].pullData(p,MSet.Time);
	  if (!r1)
	    Data[4+ind]=(p);
	}
      Data[4+ind].setEqual(p);
      RobotSensorPose[ind].add(XtraRot,p);
      if (XtraRot.Time>p.Time)
	RobotSensorPose[ind].Time=XtraRot.Time;//MSet.Time;
      else RobotSensorPose[ind].Time=p.Time;//MSet.Time;
      predict();//***************HERE IS PREDICT*********** 
      LastTime=MSet.Time;
    }
  }
  return r;
}
   

bool SLAMFilter::canPull(const int port)
{
  if ((port == 3) || !(port<Ins)) {
    return false;
  }
return true;
}

unsigned short SLAMFilter::typeCheck(int port, TimestampedData *inputData)
{

  if (inputData)
    {
      if (((port>3)&&(port<3+NumberOfSensors))||((port>-1)&&(port<3)))
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
	      if (sd->getTPointClassType()==POSE3D_TYPE)
		{
		  sd->setEqual(Data[port]);
		}
	      else{
		std::cerr << "\nFilter \"" << FilterName <<"\" warning  "
			  << "Wrong type of input to  "<<port << std::endl;
		return TYPE_ERROR;
	      }
	    }
	  else{
	    std::cerr << "\nFilter \"" << FilterName <<"\" warning  "
		      << "Wrong type of input to  "<<port << std::endl;
	    return TYPE_ERROR;
	  }
	  if (!Checked[port])return 0;
	  Checked[port]--;
	  if (port==1)
	    {
	      if(Data[1].getTPointSubType()&0x8000)return 0;
	      else {
		std::cerr << "\nFilter \"" << FilterName <<"\" warning  "
			  << "Must write Incremental Pose Data on port "<<port
                          << " and not type " 
                          << std::hex << "0x" << Data[1].getTPointSubType() 
                          << std::dec << std::endl;
	      }
	    }
	  else
	    {
	      if ((Data[port].getTPointSubType()&0x8000)==0)return 0;
	      else {
		std::cerr << "\nFilter \"" << FilterName <<"\" warning  "
			  << "Must not write Incremental Pose Data on port "
			  <<port
                          << " and not " 
                          << std::hex << "0x" << Data[port].getTPointSubType() 
                          << std::dec << std::endl;
	      }
	    }
	  return TYPE_ERROR;
	}
      else if (port==3)
	{
	  if (inputData->getClassType()==MEASUREMENTSET_TYPE)
	    {
	      if (inputData->isPacked())
		{
		  inputData->narrowPackedData()->
		    unpack(*Data[3].getTPointer());
		}
	      else Data[3]=*inputData;   
	      return 0;
	    }
	  else if (inputData->narrowSmartData())
	    {
	      SmartData *sd=inputData->narrowSmartData();
	      if (sd->getTPointClassType()==MEASUREMENTSET_TYPE)
		{
		  sd->setEqual(Data[3]);
		  return 0;
		}
	      else{
		std::cerr << "\nFilter \"" << FilterName <<"\" warning  "
			  << "Wrong type of input to  "<<port << std::endl;
		return TYPE_ERROR;
	      }
	    }
	  else{
	    std::cerr << "\nFilter \"" << FilterName <<"\" warning  "
		      << "Wrong type of input to  "<<port
                      << std::endl;
	    inputData->print();
	    return TYPE_ERROR;
	  }
	  return TYPE_ERROR;
	}
   
      else if ((port<Ins)&&(port>-2)){
	if (inputData->isPacked())
	  {
	    inputData->narrowPackedData()->
	      unpack(*Data[port].getTPointer());
	  }
	      else Data[port]=*inputData;   
      
      }
 }
  return 0;
}



}//Cure
