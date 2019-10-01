// = RCSID
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2005 John Folkesson
//    


#include "SensorUser.hh"
#include "CureDebug.hh"
#include "DataFilterAddress.hh"
#include "DataSet.hh"
#include "DataSlotAddress.hh"
#include "SensorData.hh"
#include "MeasurementSet.hh"
#include "SmartData.hh"
#include <sstream>  // istringstream
#include "ConfigFileReader.hh"
namespace Cure {

  SensorUser::SensorUser(short numin, short numout,
		       short numberOfSensors,
			 short msetSize,
		       const unsigned short thread)
    :DataFilter(numin, numout, true,thread)
{
  CureCERR(30)<<"SensorUser Constructor:"<<numin<<" "<<numout<<" "<<numberOfSensors<<" "<<msetSize<<"\n";
  m_Initialized=true;
  m_PoseInitService="PoseInitService";
  MeasurementIndex.reallocate(0,6);
  AllwaysPredictSensor=0;
  SensorIndex=0;
  LastSensorTime=0;
  MinPendingTime=0.0;
  m_Sensors=0;
  m_DeadService=0;
  m_UseDeadService=false;
  m_DeadServiceName="DeadService";
  m_UpdateSequentially=true;
  m_DeadFlag=6;
  if (numout<4)  {
    CureCERR(30)<< "\nFilter \"" << FilterName <<"\" warning  "
              << "Needs at least "<<(4)<<" Outputs "
	      <<" I only have "<<(Outs)<<" output ports.\n";  
   CureCERR(30)<<" You will soon get a seg fault\n";
  }
  if (numin<3+numberOfSensors){
    CureCERR(30) << "\nFilter \"" << FilterName <<"\" warning  "
              << "Needs at least "<<(3+numberOfSensors)<<" Ins "
	      <<" I only have "<<(Ins)<<" input ports.\n";  
    numberOfSensors=Ins-3;
  }
  if (numberOfSensors<1){
    CureCERR(30)<<FilterName<<" must have at least 1 sensor\n";
    numberOfSensors=1;
  }
  NumberOfSensors=numberOfSensors;
  AllwaysPredictSensor=new bool[NumberOfSensors];
  SensorIndex=new short[NumberOfSensors];
  RobotSensorPose=new SensorPose[NumberOfSensors];
  LastSensorTime=new Timestamp[NumberOfSensors];
  SensorPoseCovType=new unsigned short[NumberOfSensors];
  m_Sensors=new SensorOffset[NumberOfSensors];
  for (short i=0; i<NumberOfSensors;i++){
    SensorPoseCovType[i]=63;
    Ports[3+i].setDataNeeded(false);
    AllwaysPredictSensor[i]=false;
    SensorIndex[i]=-1;
  }
  MSet.setNumberOfElements(msetSize);
  MSet.Time = 0.0;
  Command c;
  Data=new SmartData[Ins+Outs+1];
  Data++;
  Pose3D p;
  Data[0]=p;
  Data[1]=MSet;;
  Data[2]=c;
  for (short i=0; i<NumberOfSensors;i++)
    Data[3+i]=p;
  Data[Ins]=p;
  Data[Ins+1]=p;
  Data[Ins+2]=p;
  Data[Ins+3]=p;
  LastTime=0.0;
  FilterName = "SensorUser";
  Ports[-1].setInterpolate(false);
  Ports[1].setInterpolate(false);
  Ports[-1].setDataNeeded(false);
  Ports[1].setDataNeeded(false);
  for (short i=Ins;i<Ins+Outs;i++)
    Ports[i].SetDescriptor=true;
  PredictInterpolate=0;
}


SensorUser::~SensorUser()
{    
   if (SensorPoseCovType)delete [] SensorPoseCovType;
  SensorPoseCovType=0;
  if(AllwaysPredictSensor)
    delete []AllwaysPredictSensor;
 AllwaysPredictSensor=0;
  if(SensorIndex)
    delete []SensorIndex;
  SensorIndex=0;
  if(RobotSensorPose)
    delete []RobotSensorPose;
  if(m_Sensors)
    delete []m_Sensors;
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
int SensorUser::configure(ConfigFileReader &cfg,
			  std::string  &filtername)
{
  int r=DataFilter::configure(cfg,filtername);
  if (r)return r;
  FilterName=filtername;
  std::string desarglist="";
  std::string desparams;
  std::list<std::string> strings;
  if (cfg.getParamStrings(FilterName, true, strings, desarglist))
    return 0;
  for (std::list<std::string>::iterator pi = strings.begin();
       pi != strings.end(); pi++) {       
    std::istringstream strdes(*pi);
    std::string cmd;
    if(!(strdes>>cmd))return 1;
    if (((cmd=="Measurement")||
	 (cmd=="measurement"))||(cmd=="MeasurementType")){
      DataDescriptor d;
      bool fail=false;
      if((strdes>>d.Name)){
	std::string tmp;
	if (cfg.getDescritptor(d)){
	  fail=true;
	} else if (strdes>>tmp){
	  int i=MeasurementIndex.Rows;
	  MeasurementIndex.grow(i+1,MeasurementIndex.Columns);
	  MeasurementIndex(i,0)=d.ClassType;
	  MeasurementIndex(i,1)=d.ID;
	  MeasurementIndex(i,2)=d.SensorType;
	  MeasurementIndex(i,3)=d.SensorID;
	  MeasurementIndex(i,4)=getMeasurementClass(tmp);
	  short temp=-1;
	  strdes>>temp;
	  MeasurementIndex(i,5)=temp;
	} else fail=true;
      } else fail=true;
      if (fail)	    
	std::cerr<<FilterName
		 <<"Failed to setup measurement descriptor "<<d.Name<<"\n ";
    }else if (cmd=="PredictInterpolate")
      strdes>>PredictInterpolate;
  }
  return 0;
}

short SensorUser::getMeasurementClass(DataDescriptor &d, unsigned short mt)
{
  mt=(mt>>12);
  short test=mt;
  for (int i=0; i<MeasurementIndex.Rows;i++){
    if (d.ClassType==MeasurementIndex(i,0))
	if (d.SensorType==MeasurementIndex(i,2))
	  if (d.SensorID==MeasurementIndex(i,3))
	    if (((MeasurementIndex(i,1))==0)||(d.ID==MeasurementIndex(i,1)))
	      if ((MeasurementIndex(i,5)<0)||(MeasurementIndex(i,5)==test))
		return MeasurementIndex(i,4);
    
    
  }
  return 0;
}

int SensorUser::config(const std::string &params)
{
  std::istringstream strdes(params);
  std::string cmd;
  if(!(strdes>>cmd))return 1;
  else if ((cmd=="m_DeadService")||(cmd=="m_DeadServiceName")){
    if((strdes>>cmd)) m_DeadServiceName=cmd;
    if (!getService(m_DeadServiceName,m_DeadService)){
      std::cerr<<FilterName<<"Failed to find DeadService\n";
    }
  } else if (cmd=="m_DeadFlag"){
    strdes>>m_DeadFlag;
  } else if (cmd=="m_UseDeadService"){
    strdes>>m_UseDeadService;
  }else if (cmd=="MinPendingTime"){
    double d=0;
    if ((strdes>>d)) {
      MinPendingTime=d;;
    } 
  } else if (cmd=="AllwaysPredict"){
    int b;
    if ((strdes>>cmd>>b)) {
      for (short i=0;i<NumberOfSensors;i++)
	if (m_Sensors[i].m_Name==cmd)AllwaysPredictSensor[i]=b;
    } 
  } else if (cmd=="m_Initialized"){
      bool d=0;
      if ((strdes>>d))m_Initialized=d;
  } else if (cmd=="m_PoseInitService"){
    if ((strdes>>cmd))m_PoseInitService=cmd;
  } else if (cmd=="m_UpdateSequentially"){
    strdes>>m_UpdateSequentially;  
  }
  return DataFilter::config(params);
}
int SensorUser::configSensor(const std::string &params,
			     const std::string &sensordef)
{
  std::istringstream strdes(sensordef);
  std::string cmd;
  if(!(strdes>>cmd))return 1;
  int i=0;
  while (i<NumberOfSensors){
    if (m_Sensors[i].m_Name==cmd){
      break;
    } else if (m_Sensors[i].m_Name=="NoName"){
      m_Sensors[i].m_Name=cmd;
      break;
    }      
    i++;
  } 
  if(i<NumberOfSensors){
    if (strdes>>cmd){
      unsigned short type,id;
      Cure::getSensorType(cmd,type);
      m_Sensors[i].setSensorType(type);
      if (strdes>>id)
	m_Sensors[i].setSensorID(id);
      std::istringstream str2(params);
      if (str2>>cmd>>cmd>>cmd){
	if (cmd=="CovType")
	  str2>>SensorPoseCovType[i];
      }
   }
    //This simpiler object is what all the existing apps use so I
    //had better init it.
    RobotSensorPose[i]=m_Sensors[i].m_RobotSensorPose;
   
  }
 return DataFilter::configSensor(params,sensordef);
}

int SensorUser::configGlobal(const std::string &params,
			      StateData &p)
  {
    std::cerr<<params<<"\n";
    p.print();
    std::istringstream strdes(params);
    std::string cmd, name;
    unsigned short  dim=0;
    unsigned short type=0;
    int r=m_StateID.Rows;
    if(!(strdes>>cmd>>name>>cmd))return 1;
    else   if (cmd=="PoseState"){
      type=63;
      while (strdes>>cmd)
	if (cmd=="CovType")
	  if (strdes>>type)break;
      p.setPose(PoseOut,type);
    }  if (cmd=="ExtraRotation"){
      type=0;
      while (strdes>>cmd)
	if (cmd=="CovType")
	  if (strdes>>type)break;
      p.setPose(XtraRot,type);
   
    }else if (cmd=="Sensor"){
      int i=0;
      while (i<NumberOfSensors){
	if (m_Sensors[i].m_Name=="NoName")  break;
	i++;
      }
      if (i<NumberOfSensors){
	m_Sensors[i].m_Name=name;
	type=63;
	while (strdes>>cmd)
	  if (cmd=="CovType")
	    if (strdes>>type)break;
	m_Sensors[i].setupPose(p,type);
      }
    }else if (cmd=="SensorOffset"){
      if (strdes>>cmd){
	int i=0;
	while (i<NumberOfSensors){
	  if (m_Sensors[i].m_Name==cmd){
	    break;
	  } else if (m_Sensors[i].m_Name=="NoName"){
	    m_Sensors[i].m_Name=cmd;
	    break;
	  }      
	  i++;
	} 
	if(i<NumberOfSensors){
	  type=63;
	  while (strdes>>cmd)
	    if (cmd=="CovType")
	      if (strdes>>type)break;
	  m_Sensors[i].setupPose(p,type,false);
	}
      }
    } 
    std::cerr<<type<<" "<<m_StateID.Rows<<"\n";     
    if (p.StateID.Rows){
      int k=0;
      dim=calcRows(type);
      if (dim>p.StateID.Rows)dim=p.StateID.Rows;
      m_StateID.grow(r+dim,1);
      for (int i=0;i<p.StateID.Rows;i++){
	if (type&1){
	  m_StateID(k+r,0)=p.StateID(i,0);
	  k++;
	}
	type=(type>>1);
      }
    }
    return 0;
  }


void SensorUser::setMinPendingTime(double d)
{
  MinPendingTime=d; 
}

int
SensorUser::getSensorIndex()
{
   
  int st = Descriptor.SensorType;
  int id = Descriptor.SensorID; 
  if (Descriptor.ClassType==DATASET_TYPE){
    return SensorIndex[0]; 
  }
  //  if (Descriptor.ClassType==MEASUREMENTSET_TYPE){
  //  if (MSet.getNumberOfElements() == 0)  return SensorIndex[0]; 
  // }
  for (short i=0;i<NumberOfSensors;i++) {
    if (st==RobotSensorPose[i].m_SensorType &&
        id==RobotSensorPose[i].m_SensorID) {
      return SensorIndex[i]; 
    }
  }
  //Here we have a registered Descriptor that doesn't match any sensor offset.
  //We use Ins as the no offset index.
  if (Descriptor.ClassType==MEASUREMENTSET_TYPE){
    for (int i=0;i<MSet.getNumberOfElements();i++){
      if (getMeasurementClass(Descriptor,MSet(i).MeasurementType))return Ins;
    }
  }
  //  MSet.getDescriptor(Descriptor);
  CureCERR(40)<<FilterName
	      <<" Unknown Sensor Data.  No match to Descriptor.\n"
	      <<"Or getMeasurementClass put <=0 in column 4"
	      <<"Or too few # of sensors in the constructor.";
  CureDO(55){
    Descriptor.print();
    std::cerr<<"In:\n";
    MeasurementIndex.print();
  }
  return -1;
}

unsigned short SensorUser::read(TimestampedData& p,const int port, 
				const Timestamp t,
				const int interpolate)
{
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

unsigned short SensorUser::write(TimestampedData& td,const int port)
{
  if ((port==0)){
    CureCERR(30) << "\nFilter \"" << FilterName <<"\" warning  "
              << "Trying to write to  port number "<<port
	      <<" I only pull data from this port.\n";
    return (RESOURCE_ERROR|ADDRESS_INVALID);
  }
  if(LastTime>td.Time)return (TIMESTAMP_ERROR|TIMESTAMP_TOO_OLD);
  if ((!(port<Ins))||(port<-1)){
    CureCERR(30) << "\nFilter \"" << FilterName <<"\" warning  "
              << "Trying to write to  port number "<<port
	      <<" I only have "<<(Ins)<<" input ports.\n";
    return (RESOURCE_ERROR|ADDRESS_INVALID);
  }

  if (typeCheck(port,&td)){
	CureCERR(50)<<"Filter \"" << FilterName << "\""
                 << " got an invalid type written to it. "<<std::endl; 
	return TYPE_ERROR;
  }
  if (port==2)return processCommand();
  CureDO(60) { CureCERR(0) << "Written to with "; td.print(); }
  // Try to perform the updates that have been put off so far
  if (port==1) {
    if (!m_Initialized){
      unsigned long ind=0;
      if (getService(m_PoseInitService,ind)){
	Pose3D p;
	unsigned short r=readData(ind, p);
	if (!r){ 
	  setPose(p);
	  m_Initialized=true;
	  LastTime=p.Time;
	}else {
	  //We throw out data until we are initialized
	  return 0;
	}
      }else {
	CureCERR(30) << "SensorUser Filter "<<FilterName
		     <<" Can not find "<<m_PoseInitService<<
	  " on "<<Thread<<"\n"; 
	return 0;
	}
    }
    // We got a measurement or sensor data
    DataSet *ds=td.narrowDataSet();
    if (ds){
      for (int i=0;i<ds->SetSize; i++){
	TimestampedData *ts=ds->getTPointer(i);
	if (ts->Time>YoungestTime)YoungestTime=ts->Time;
	PendingDataList.addOrdered(*ts);
      }
    }else{
      if (td.Time>YoungestTime)YoungestTime=td.Time;
      PendingDataList.addOrdered(td);
    }
    while (true) {
      // Get the first item in the list of stored data
      if (PendingDataList.get(0,Data[1])) break;   
      Timestamp upTime=Data[1].Time;
      Timestamp tm=YoungestTime;
      tm-=upTime;
      if (tm<MinPendingTime)break;
      Data[1].getDescriptor(Descriptor);
      if (Descriptor.ClassType==MEASUREMENTSET_TYPE)
	Data[1].setEqual(MSet);
      if (upTime>=LastTime){
	int ind=getSensorIndex();
	int r1=0;
	if (ind>-1)
	  {
	    
	    r1=getDead(upTime,ind);//***********Calls Predict************''
	    if (r1)
	      {
		if (r1==TYPE_ERROR)return r1;
		else if ((r1&NO_DATA_AVAILABLE))return 0;
		else if ((r1&TIMESTAMP_TOO_OLD))
		  {
		    if (m_UpdateSequentially){
		      CureCERR(30)<<"Filter \"" << FilterName << "\""
			       <<"has gotten TIMSTAMP_TOO_OLD when tring to predict"
			       <<"Skiping data,  A buffer may be too small or "
			       <<"the procewss is not keeping up. Errors likely";
		      LastTime=upTime;
		      PendingDataList.remove(0);
		      return r1;
		    }
		    r1=0;
		  }
	      }
	  }
	//Now the Filter should be have predicted this sensor pose 
	if (!r1)
	  if ((!m_UpdateSequentially)||(upTime==LastTime))
	    {
	      if ((Descriptor.ClassType!=MEASUREMENTSET_TYPE)||
		  (MSet.getNumberOfElements() != 0) )
		if (update())return 0;//******HERE IS Update***********      
	    }
	for (short i=0;i<Outs; i++)
	  Data[Ins+i].setDescriptor(Ports[Ins+i].m_Descriptor);
   	setOutputs();
      }
      PendingDataList.remove(0);    
    }
  }
  return 0;
}

  

int SensorUser::getDead(Timestamp &t, int ind)
{ 
  Timestamp tmp=t;
  tmp-=LastTime;
  double dt=tmp.getDouble();
  if (LastTime!=(long)0){
    if (dt<0){
      CureCERR(30)<<"Predict into the past aborted\n";
      return (TIMESTAMP_ERROR|TIMESTAMP_TOO_OLD);
    } 
  }  
  int r=0;
  if (LastTime<t){
    if (m_UseDeadService){
      DataSet ds;
      ds.Time=LastTime;
      r=readService(m_DeadServiceName,m_DeadService,ds,t,m_DeadFlag);
      if (r&ADDRESS_INVALID){
	CureCERR(10)<<FilterName<<m_DeadService<<" Can't find the DeadService\n";
	return (ADDRESS_INVALID|RESOURCE_ERROR);
      }
      else if (r) return r;
      Data[Ins+1]=ds(0);
      Data[Ins+2]=ds(1);
      Data[Ins+1].setEqual(DeadInc);
      Data[Ins+2].setEqual(DeadCum);
    }else {
      getState(PoseDataSet);
      PoseDataSet.setAllTimes(LastTime);
      Ports[0].setSequenced(t,PredictInterpolate);
      r=Ports[0].pullData(PoseDataSet,t);
      if (r)return r;
      if (typeCheck(0,&PoseDataSet)){
	CureCERR(20)<<"Filter \"" << FilterName << "\""
		 << " got an invalid type on pulling DeadReckoning. "
		 <<std::endl; 
	return TYPE_ERROR;
      }
      Data[Ins+3]=PoseDataSet; 
      Data[Ins]=PoseDataSet(0); 
      Data[Ins+1]=PoseDataSet(1);
      Data[Ins+2]=PoseDataSet(2);
      Data[Ins].setEqual(XtraRot); 
      Data[Ins+1].setEqual(DeadInc);
      Data[Ins+2].setEqual(DeadCum);
    }
  }
  for (short i=0;i<NumberOfSensors;i++){
    Pose3D p;
    unsigned short r1=0;
    if (i<Ins){
      if (LastSensorTime[i]<t){
	if (in(3+i)->IsPull)
	  {
	    r1=Ports[3+i].pullData(p,t);
	    if (!r1)
	      Data[3+i]=(p);
	  }
	Data[3+i].setEqual(p);
	RobotSensorPose[i].add(XtraRot,p);
	if (XtraRot.Time>p.Time)
	  RobotSensorPose[i].Time=XtraRot.Time;
	else RobotSensorPose[i].Time=p.Time;
      }
    }
    
  } 
  if (t>LastTime){
    predict();//***************HERE IS PREDICT*********** 
    LastTime=t;
  }
  return r;
}
   

bool SensorUser::canPull(const int port)
{
  if ((port == 1) || !(port<Ins)) {
    return false;
  }
return true;
}

unsigned short SensorUser::typeCheck(int port, TimestampedData *inputData)
{

  if (inputData)
    {
      if (((port>2)&&(port<2+NumberOfSensors)))
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
	  if ((Data[port].getTPointSubType()&0x8000))return 0;
	  else {
	    std::cerr << "\nFilter \"" << FilterName <<"\" warning  "
		      << "Must not write Incremental Pose Data on port "
		      <<port
                          << " and not " 
		      << std::hex << "0x" << Data[port].getTPointSubType() 
		      << std::dec << std::endl;
	  }
	  
	  return TYPE_ERROR;
	}
      else  if (port==0)
	{
	  if (inputData->getClassType()==SMARTDATA_TYPE)
	    {
	      SmartData *sd=inputData->narrowSmartData();
	      if (sd->getTPointClassType()==DATASET_TYPE)
		inputData->narrowSmartData()->setEqual(PoseDataSet);
	      else return TYPE_ERROR;	      
	    }
	  else if (inputData->getClassType()==DATASET_TYPE)
	    if (inputData!=&PoseDataSet)PoseDataSet=*inputData;
	  if (PoseDataSet.SetSize<3){
	    std::cerr << "\nFilter \"" << FilterName <<"\" warning  "
		      <<"Input 0 DataSet Size to small\n";
	    return TYPE_ERROR;	      
	  }
	  for (int i=0; i<3; i++)
	    if (PoseDataSet.getDataClassType(i)!=POSE3D_TYPE){
	      std::cerr << "\nFilter \"" << FilterName <<"\" warning  "
			<< "Wrong type of input to  "<<port << std::endl;
	      return TYPE_ERROR;
	    }
	  if(!(PoseDataSet.getDataSubType(1)&0x8000)){
	    std::cerr << "\nFilter \"" << FilterName <<"\" warning  "
		      << "Must write Incremental Pose Data on port "<<port
		      << " and not type " 
		      << std::hex << "0x" << (PoseDataSet.getDataSubType(1))
		      << std::dec << std::endl;
	    return TYPE_ERROR;
	  }
	  if((PoseDataSet.getDataSubType(2)&0x8000)){
	    std::cerr << "\nFilter \"" << FilterName <<"\" warning  "
		      << "Must not write Incremental Pose Data on port "<<port
		      << " and not type " 
		      << std::hex << "0x" << (PoseDataSet.getDataSubType(2))
		      << std::dec << std::endl;
	    return TYPE_ERROR;
	  }
	  
	  *data(port)=*inputData;   
	  return 0;
	}
      else if (port==2)
	{
	  if (inputData->getClassType()!=COMMAND_TYPE)return TYPE_ERROR;
	  *data(port)=*inputData;   
	  return 0;
	}
      else if (port==1)
	{
	  if ((inputData->getClassType()==MEASUREMENTSET_TYPE)||
	      (inputData->narrowSensorData()))
	    {
	      if (inputData->isPacked())
		{
		  inputData->narrowPackedData()->
		    unpack(*Data[3].getTPointer());
		}
	      else Data[1]=*inputData;   
	      return 0;
	    }
	  else if (inputData->narrowSmartData())
	    {
	      SmartData *sd=inputData->narrowSmartData();
	      if (sd->getTPointClassType()==MEASUREMENTSET_TYPE)
		{
		  sd->setEqual(Data[1]);
		  return 0;
		}
	      sd->setEqual(Data[1]);
	      return 0;
	    }
	  else if (inputData->narrowDataSet()){
	    Data[1]=*inputData;
	    return 0;
	  }
	} else if ((port<Ins)&&(port>-2)){
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
