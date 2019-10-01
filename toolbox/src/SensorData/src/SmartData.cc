// = RCSID
//    $Id: SmartData.cc ,v 1.1 2004/03/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2004 John Folkesson
//    
#include "SmartData.hh"
#include "DataSet.hh"
#include "CureDebug.hh"
#include "PackedTimestampedString.hh"


using namespace Cure;

void Cure::writeString(Cure::TimestampedData & p, 
		       std::vector<std::string> &vec, 
		       int significantFigures)
{
  if (p.getClassType()==DATASET_TYPE)
    {
      std::ostringstream sstr;
      DataSet *ds=p.narrowDataSet();
      unsigned short version=0;
      sstr<<(int)p.getClassType() << " "
	 << (int)p.getID() << " "
	 << p.getSubType() << " "
	 << p.Time.Seconds << " " 
	 << p.Time.Microsec << " "
	 << version << " "
	 << ds->SetSize << "\n";
      vec.push_back(sstr.str());
      for (int i=0; i<ds->SetSize;i++)
	writeString((*ds)(i),vec,significantFigures);
      return;
    }
  if (p.getClassType()==SMARTDATA_TYPE)
    {
      SmartData *sd=p.narrowSmartData();
      return writeString(*sd->getTPointer(),vec,significantFigures);
    }
  std::ostringstream sstr;  
  if (p.getClassType()==STRING_TYPE)
    {     
      sstr <<(int)STRING_TYPE  << " "
	   << (int)p.getID() << " "
	   << p.getSubType() << " "
	   << p.Time.Seconds << " " 
	   << p.Time.Microsec << " "
	   << p.version() <<" ";
      std::string s;
      if (p.isPacked()){
	PackedTimestampedString *ts=p.narrowPackedTimestampedString();
	sstr<<ts->getStringType()<<" ";
	ts->get(s);
      }else {
	TimestampedString *ts=p.narrowTimestampedString();
	sstr<<ts->getStringType()<<" ";
	ts->get(s);
      }
      sstr<<s;
    }else{  
      SmartData tPoint;
      tPoint.setPacked(p);
      PackedData *pd = tPoint.getPackedData();
      sstr <<(int)pd->getClassType() << " "
	   << (int)pd->getID() << " "
	   << pd->getSubType() << " "
       << pd->Time.Seconds << " " 
	   << pd->Time.Microsec << " "
	   << pd->Version << " "
	   << pd->getShortDataSize() << " "
           << pd->getDataSize();
       // Write short data array ShortData
      unsigned short *bd = pd->ShortData;
      for (unsigned long i = 0; i < pd->getShortDataSize(); i++)
	sstr << " " << bd[i];
      // Write double data array Data
      double *d = pd->Data;
       if (significantFigures>18)significantFigures=18;
      if (significantFigures<1)significantFigures=1;
      for (unsigned long i = 0; i < pd->getDataSize(); i++)
	{
	  char ch[32];
	  gcvt(d[i],significantFigures,ch);
	  sstr <<" "<<ch;
	}
     }
  sstr << "\n";
   vec.push_back(sstr.str());
}


unsigned short Cure::readString(Cure::SmartData &tPoint,std::string &str)
{      
  unsigned long nShorts, nDoubles;
  int version;
  Timestamp temp;
  std::istringstream strdes(str);
  unsigned short classType, id, subType;    
  if(strdes >> classType >> id >> subType
     >> temp.Seconds >> temp.Microsec
     >> version >> nShorts)
    {
      if (classType==DATASET_TYPE){
	  DataSet ds;
	  ds.Time=temp;
	  ds.setID(id);
	  ds.setSubType(subType);
	  ds.setSetSize(nShorts);
	  tPoint=ds;
	  return 0;
      }else if (classType==STRING_TYPE){
	if (tPoint.setup(classType, subType, false)) 
	  return (RESOURCE_ERROR);
	tPoint.setTime(temp);  
	TimestampedString *ts=
	  tPoint.getTPointer()->narrowTimestampedString();
	ts->setStringType(nShorts);
	ts->setID(id);
	int start=str.length()-subType;
	start--;
	if ((start<=0)||(subType>16000)){
	  (*ts)="";
	}else{
	  char temp[subType+1];
	  temp[subType]=0;
	  for (int i=0;i<(int)subType;i++)
	    temp[i]=str[i+start];
	  (*ts)=temp;
	}
	tPoint.pack();
	return 0;
      } else if ( strdes  >> nDoubles) {      	  
	  // Create object to put the data in, we know that it should be a
	  // packed data object
	if (tPoint.setup(classType, subType, true)) 
	    return (RESOURCE_ERROR);
	tPoint.setTime(temp);      
	PackedData *pd = tPoint.getPackedData(); 
	pd->setID(id);
	pd->setSubType(subType);

	if (version != pd->Version) {
	  CureCERR(70) << "\nERROR: Version mismatch, got "
		       << version << " expected " << pd->Version 
		       << " from packed data with "
		       << "classType=" << classType
		       << " subType=" << subType
		       << " id=" << id
		       << " Time=" << temp.Seconds << "." << temp.Microsec
		       << " Version=" << version
		       << " nShorts=" << nShorts
		       << " nDoubles=" << nDoubles
		       << std::endl;
	  if (version > pd->Version) {
	    CureCERR(30) <<"My version is too old to read this, "
			 <<"Aborting read string\n";
	    return (RESOURCE_ERROR|ADDRESS_INVALID);
	  }	 
	}
	if (pd->setShortDataSize(nShorts)){
	  CureCERR(30) << "\nERROR #Shorts according to file "
		       << nShorts << " does not match expecation for"
		       << " classType=" << classType 
		       << " subType=" << subType
		       << " which is " << pd->getShortDataSize()
		       << std::endl;
	  return (RESOURCE_ERROR);
	} else {
	  
	  unsigned short dim = pd->getShortDataSize();
	  if (dim<nShorts)return (RESOURCE_ERROR);
	  unsigned short *bd=pd->ShortData;
	  for (unsigned short i = 0; i < nShorts; i++) {
	    if ( !(strdes >> bd[i]) ) {
	      CureCERR(30) << "Failed reading shorts of type "
			   << classType 
			   << std::endl;
	      return (RESOURCE_ERROR);
	    }
	  }
	  if (dim>nShorts)
	    memset(&bd[nShorts],0,(dim-nShorts)*sizeof(unsigned short));
	  if (pd->convertShortData(version)) {
	    CureCERR(30) <<"My version is too old to read this, "
			 <<"Aborting read string\n";
	    return (RESOURCE_ERROR|ADDRESS_INVALID);
	  }	 
	  
	  dim = pd->setDataSize(nDoubles);
	  double *d = pd->Data;
	  
	  for (unsigned short i = 0; i < nDoubles; i++) {
	    std::string ch;
	    if (!(strdes >>ch)){
	      CureCERR(30) << "Failed reading doubles for type "
			   << classType 
			   << std::endl;
	      return (RESOURCE_ERROR);
	    }
	    d[i]=strtod(ch.c_str(),(char**)NULL);
	  }
	  if (pd->convertData(version)) {
	    CureCERR(30) <<"My version is too old to read this, "
			 <<"Aborting read string\n";
	    return (RESOURCE_ERROR);
	  }	 
	  return 0;
	}
      }
    }
  CureCERR(50) << "Parse Data is not good\n";
  return (RESOURCE_ERROR);
}
 
int Cure::SmartData::writeChar(TimestampedData & p, 
			       char **str, int headlength,
			       int significantFigures)
{
  std::vector<std::string> vec;
  std::vector<std::string>::iterator it;
  writeString(p,vec,significantFigures);
  int len=0;
  for (it=vec.begin();it!=vec.end();it++)
    {
      len+=(it->length()+1);
    }
  len+=1;
  char *chr=(char *)malloc(len+headlength);
  memset(chr,0,headlength);
  len=headlength;
  for (it=vec.begin();it!=vec.end();it++)
    {
      int k=it->length();
      it->copy(chr+len,k);
      len+=(k+1);
      chr[len-1]=0;
    }
  chr[len]=13;
  len++;
  *str=chr;
  return len;
}

unsigned short  Cure::SmartData::readChar(char *chr,int len)
{
  std::vector<std::string> vec;  
  while (len>1){  //separate line into string tokens
    std::vector<std::string>::iterator it;
    std::string s;
    vec.push_back(s);
    it=vec.end()-1;
    (*it)=chr;
    len-=(it->length());
    chr+=it->length();
    len--;  //skip blank
    chr++;
  }
  std::vector<std::string>::iterator it;
  len=0;
  SmartData *sd=this;
  it=vec.begin();
  if (it==vec.end())return RESOURCE_ERROR;
  unsigned short ret=readString(*sd,*it);
  if (ret)return ret;
  if (sd->getTPointer()->getClassType()==DATASET_TYPE){
    DataSet *ds=sd->getTPointer()->narrowDataSet();
    for (int i=0;i<ds->SetSize;i++){
      it++;
      if (it==vec.end())return RESOURCE_ERROR;
      sd=&(*ds)(i);
      ret=readString(*sd,*it);
      if (ret)return ret;
      if (sd->getTPointer()->getClassType()==DATASET_TYPE){
	DataSet *ds2=sd->getTPointer()->narrowDataSet();
	for (int j=0;j<ds2->SetSize;j++){
	  it++;
	  sd=&(*ds2)(j);
	  if (it==vec.end())return RESOURCE_ERROR;
	  ret=readString(*sd,*it);
	  if (ret)return ret;
	  if (sd->getTPointer()->getClassType()==DATASET_TYPE){
	    DataSet *ds3=sd->getTPointer()->narrowDataSet();
	    for (int jj=0;jj<ds3->SetSize;jj++){
	      it++;
	      sd=&(*ds3)(jj);
	      if (it==vec.end())return RESOURCE_ERROR;
	      ret=readString(*sd,*it);
	      if (ret)return ret;
	      if (sd->getTPointer()->getClassType()==DATASET_TYPE){
		DataSet *ds4=sd->getTPointer()->narrowDataSet();
		for (int jjj=0;jjj<ds4->SetSize;jjj++){
		  it++;
		  sd=&(*ds4)(jjj);
		  if (it==vec.end())return RESOURCE_ERROR;
		  ret=readString(*sd,*it);
		  if (ret)return ret;
		  if (sd->getTPointer()->getClassType()==DATASET_TYPE){
		    CureCERR(30)<<"ERROR Too many nested DataSets to read strings\n";
		    return RESOURCE_ERROR;
		  }
		}
	      }
	    }
	  }
	}
      }
    }
  }
  return 0;
}


unsigned short 
Cure::makeTimestampedData(TimestampedData **tpointerpointer,
			  unsigned char classType, 
			  unsigned short subType,
			  bool packed)
{
  TimestampedData *tPointer=*tpointerpointer;
  if (tPointer != 0) {
    if ((tPointer->getClassType() == classType)&&
	(tPointer->isPacked() == packed)){
      tPointer->setSubType(subType);
      return 0;
    }   
    delete tPointer;
    tPointer=0;
    *tpointerpointer=tPointer;
  }
  if (!packed) {
    if (classType == POSE3D_TYPE)
      tPointer=((TimestampedData *)(new Pose3D()));
    else if (classType == SICKSCAN_TYPE)
      tPointer=new SICKScan();
    else if (classType == DATASET_TYPE)
      tPointer=new DataSet();
    else if (classType == SMARTDATA_TYPE)
      tPointer=new SmartData();
    else if (classType == COMMAND_TYPE)
      tPointer=new Command();
    else if (classType == SONAR_TYPE)
      tPointer=new SonarData();
    else if (classType == (MEASUREMENTSET_TYPE))
      tPointer=new MeasurementSet();
    else if (classType == GENERIC_TYPE)
      tPointer=new GenericData();
    else if (classType == GRID_TYPE)
      tPointer=new GridData();
    else if (classType == STATE_TYPE)
      tPointer=new StateData();
    else if (classType == ODOMETRY_TYPE)
      tPointer=new Odometry();
    else if (classType == TIMESTAMP_TYPE)
      tPointer=new TimestampedData();
    else if (classType == SENSORPOSE_TYPE)
      tPointer=new SensorPose();
    else if (classType == STRING_TYPE)
      tPointer=new TimestampedString();
  } else {
    if (classType == POSE3D_TYPE)
      tPointer=new PackedPose();    
    else if (classType == SICKSCAN_TYPE)
	tPointer=new PackedSICK();
    else if (classType == COMMAND_TYPE)
      tPointer=new PackedCommand();
    else if (classType == SONAR_TYPE)
      tPointer=new PackedSonar();
    else if (classType == MEASUREMENTSET_TYPE)
      tPointer=new PackedMeasurementSet();
    else if (classType == GENERIC_TYPE)
      tPointer=new PackedGeneric();
    else if (classType == GRID_TYPE)
      tPointer=new PackedGrid();
    else if (classType == STATE_TYPE)
      tPointer=new PackedState();
    else if (classType == SENSORPOSE_TYPE)
      tPointer=new PackedSensorPose();
    else if (classType == TIMESTAMP_TYPE)
      tPointer=new PackedData();
    else if (classType == STRING_TYPE)
      tPointer=new PackedTimestampedString();
  }  
  if (tPointer == 0) {
    CureCERR(30)<< "Cannot setup SmartData with classType="
	      << (int)classType << std::endl;
    //We must always end up pointing at something.
    makeTimestampedData(tpointerpointer,TIMESTAMP_TYPE, subType,packed);
    return 1;
    }
  tPointer->setSubType(subType);
  *tpointerpointer=tPointer;
  return 0;
}


void SmartData::interpolate_(TimestampedData& a, TimestampedData& b, 
			     Timestamp t)
{
  SmartData *spa=a.narrowSmartData();
  SmartData *spb=b.narrowSmartData();
  if ((spa)&&(spb))return interpolate_(*spa,*spb,t);
  int packit=0;
  if (TPointer->getClassType()!=a.getClassType())*this=a;
  if (TPointer->isPacked())
    {
      packit=1;
      unpack();
    }
  bool test=true;
  if (spa)test=false;
  else if (spb)test=false;
  else if (a.isPacked())test=false;
  else   if (b.isPacked())test=false;
  if (test){
    TPointer->interpolate_(a,b,t);
    Time=TPointer->Time;
    if (packit)pack();
    return;
  }
  SmartData sa(a);
  sa.unpack();
  TimestampedData *pa=sa.TPointer;
  SmartData sb(b);
  sb.unpack();
  TimestampedData *pb=sb.TPointer;
  TPointer->interpolate_(*pa,*pb,t);
  Time=TPointer->Time;
  if (packit)pack();
}
void SmartData::interpolate_(SmartData& a, SmartData& b, 
			     Timestamp t)
{
  if (TPointer->getClassType()!=a.TPointer->getClassType())*this=a;
  bool packit=false;
  if (TPointer->isPacked())
    {
      packit=true;
      unpack();
    }
  bool packa=false;
  TimestampedData *pa=a.TPointer;
  if (pa->isPacked())
    {
      packa=true;
      a.unpack();
      pa=a.TPointer;
    }
  bool packb=false;
  TimestampedData *pb=b.TPointer;
  if (pb->isPacked())
    {
      packb=true;
      b.unpack();
      pb=b.TPointer;
    }
  TPointer->interpolate_(*pa,*pb,t);
  Time=TPointer->Time;
  if (packit)pack();
  if (packa)a.pack();
  if (packb)b.pack();
}





