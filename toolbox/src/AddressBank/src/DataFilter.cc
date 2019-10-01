
// = RCSID
//    $Id: DataFilter.cc ,v 1.1 2004/03/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2004 John Folkesson
//    
#include "DataFilter.hh"
#include "DataFilterAddress.hh"
#include "CureDebug.hh"
#include "ConfigFileReader.hh"
#include <sstream>  // istringstream
using namespace Cure;

DataFilter::DataFilter(short nIn, short nOut,
                       bool addTriggerPort, const unsigned short thread)
{
  Ins=nIn;
  Outs=nOut;
  HasTriggerPort=addTriggerPort;
  ErrorCode=0;
  Thread=thread;
  // if ((AddressBank::theAddressBank().getNumberOfThreads()>1)){
    if (HasTriggerPort) 
      {
	Ports = new DataFilterAddress[Ins + Outs + 1];
	for (short i = 0; i < (Ins + Outs + 1); i++) {
	  Ports[i].setThread(thread);
	}
	Ports += 1; // Hack to make it possible to do Ports[-1]
      } 
    else 
      {
	Ports = new DataFilterAddress[Ins + Outs];
	for (short i = 0; i < (Ins + Outs); i++) {
	  Ports[i].setThread(thread);
	}
      }
    //}

  if (HasTriggerPort) 
      {
	for (short i=-1; i<(Ins+Outs);i++)
	  Ports[i].init(this, i);
      } 
  else 
    {
      for (short i=0; i<(Ins+Outs);i++)
	Ports[i].init(this, i);
    }
  Checked=new short[Ins];
  for (short i=0; i<Ins; i++)
    Checked[i] = 5;
  FilterName = "DataFilter";
}
DataFilter::~DataFilter()
{
  disconnectAddresses();
  if (Ports) {
    if (HasTriggerPort)
      Ports -= 1;   
    delete [] Ports;
    Ports = 0;
  }
  if (Checked) {
    delete [] Checked;
    Checked = 0;
  }
  
}
void DataFilter::disconnectAddresses()
{
  if (Ports) {
    short n=Ins+Outs;
    if (HasTriggerPort){
      n++;
      Ports -= 1;
    }    
    for (short i = 0; i < n; i++) {
      unsigned short t=Ports[i].Thread;
      if (t!=Thread){
	// Need locks on both Thread (have already) and t
	AddressBank::theAddressBank().getThread(t);
	Ports[i].disable();
	Ports[i].Thread=Thread;
	AddressBank::theAddressBank().releaseThread(t);
      }
      else Ports[i].disable();
    }
    if (HasTriggerPort)Ports++;
  }
}

void DataFilter::initPorts(unsigned short *threads, 
			   short triggerthread)
{
  if (triggerthread<0)triggerthread=Thread;
  if (!threads){
    std::cerr<<"Call to DataFilter::InitPorts with threads==NULL???\n";
    return;
  }
  if (Ports) {
    short n=Ins+Outs;
    for (short i = 0; i < n; i++) {
      unsigned short t=Ports[i].Thread;
      if (Ports[i].Index==0)t=Thread;
      unsigned short t2=threads[i];
      if (t!=Thread)
	AddressBank::theAddressBank().getThread(t);
      if ((t2!=Thread)&&(t2!=t))
	AddressBank::theAddressBank().getThread(t2);
      Ports[i].setThread(t2);
      if (t!=Thread)
	AddressBank::theAddressBank().releaseThread(t);
      if ((t2!=Thread)&&(t2!=t))
	AddressBank::theAddressBank().releaseThread(t2);
    }
    if (HasTriggerPort){
      unsigned short t=Ports[-1].Thread;
      if (Ports[-1].Index==0)t=Thread;
      unsigned short t2=triggerthread;
      if (t!=Thread)
	AddressBank::theAddressBank().getThread(t);
      if ((t2!=Thread)&&(t2!=t))
	AddressBank::theAddressBank().getThread(t2);
      Ports[-1].setThread(t2);
      if (t!=Thread)
	AddressBank::theAddressBank().releaseThread(t);
      if ((t2!=Thread)&&(t2!=t))
	AddressBank::theAddressBank().releaseThread(t2);
    
    }
  }



}
int DataFilter::configure(ConfigFileReader &cfg,
			  std::string  &filtername)
{
  FilterName=filtername;
  std::string desarglist="";
  std::string desparams;
  std::list<std::string> strings;
  if (cfg.getParamStrings(FilterName, true, strings, desarglist)){
     std::cerr<<"Found now configurationsection for "<<FilterName<<"\n";
    return 0;
  }
  for (std::list<std::string>::iterator pi = strings.begin();
       pi != strings.end(); pi++) {       
    std::istringstream strdes(*pi);
    std::string cmd;
    if(!(strdes>>cmd))return 1;
    if (((cmd=="InputData")||(cmd=="Inputdata"))||(cmd=="inputdata")){
      for (short i=0; i<Ins;i++){
	if((strdes>>Ports[i].m_Descriptor.Name)){
	  if (cfg.getDescritptor(Ports[i].m_Descriptor)){
	    CureCERR(80)<<FilterName<<"Failed to find input descriptor "<<
	      i<<"\n ";
	  }
	  if (data(i)) data(i)->setDescriptor(Ports[i].m_Descriptor);
	} 
      }
    } else  if (((cmd=="OutputData")||
		 (cmd=="Outputdata"))
		||(cmd=="outputdata")){
      
      for (short i=0; i<Outs;i++){
	if((strdes>>Ports[i+Ins].m_Descriptor.Name)){
	  if (cfg.getDescritptor(Ports[i+Ins].m_Descriptor)){
	    CureCERR(80)<<FilterName<<"Failed to find output descriptor "<<
	      i<<"\n ";
	  }
	  if (data(i+Ins))data(i+Ins)->setDescriptor(Ports[i+Ins].m_Descriptor);
	}
      }
    } else  if (((cmd=="Global")||
		 (cmd=="global"))
		||(cmd=="GLOBAL")){
     if((strdes>>cmd)){
	int key=0;
	if (!cfg.getGlobal(cmd,key)){
	  StateData p;
	  if (!(p=cmd)){
	    p=(*pi);
	    p.setStateID(key);
	    configGlobal(*pi,p);
	  }
	}
      }
    }  else  if (((cmd=="Sensor")||
		    (cmd=="sensor"))
		 ||(cmd=="SENSOR")){
      if((strdes>>cmd)){
	if (!cfg.getSensor(cmd))
	  configSensor(*pi,cmd);
      }
    } else  if (cmd=="Print"){
      printConfig();
      if((strdes>>cmd))
	if (cmd=="pause"){
	  std::cerr<<"Waiting for enter";
	  getchar();
	}
    }else config(*pi);
  } 
  return 0;
}  

int DataFilter::getDescriptor(short index, DataDescriptor &des){
  if ((index<(Ins+Outs))&&(index>-1)){
    des=Ports[index].m_Descriptor;
    return 0;
  }
  return 1;
}
int DataFilter::matchInputDescriptor(DataDescriptor &des){
  if ((des.Name=="NONE")||(des.Name==""))return -1;
  for (short i=0; i<(Ins); i++)
    if (des==Ports[i].m_Descriptor)
      if (des.Name==Ports[i].m_Descriptor.Name)return i;
  return -1;
}

DataFilterAddress*
DataFilter::in(short num)
{
  if (num < 0 || num >= Ins) {
    CureCERR(20) << "WARNING: Input port " << num << " out of bounds for "
                 << FilterName << " which has " << Ins << " input ports\n";
    return 0;
  }

  return &Ports[num];
}

DataFilterAddress*
DataFilter::out(short num)
{
  if (num < 0 || num >= Outs) {
    CureCERR(20) << "WARNING: Output port " << num << " out of bounds for "
                 << FilterName << " which has " << Outs << " output ports\n";
    return 0;
  }

  return &Ports[Ins+num];
}

DataFilterAddress* 
DataFilter::trigger()
{
  if (!HasTriggerPort) {
    CureCERR(20) << "WARNING: " << FilterName 
                 << " does not have a trigger port\n";
    return 0;
  }

  return &Ports[-1];
}

unsigned short DataFilter::write( TimestampedData& inputData,const int port)
{
  ErrorCode=getInputs(inputData,port);
  if (ErrorCode&((TYPE_ERROR|ADDRESS_INVALID)|RESOURCE_ERROR))
    return RESOURCE_ERROR;
  if (ErrorCode&NO_INTERPOLATE)return NO_INTERPOLATE;
  precalc(port);
  unsigned short r1=calc();
  if (r1)return (r1);
  setOutputs();
  return 0;
}
unsigned short DataFilter::setOutputs()
{
  for (short i=Ins; i<(Ins+Outs);i++)
    Ports[i].pushData(*data(i));
  return 0;
}

unsigned short DataFilter::getInputs(TimestampedData & inputData,int port)
{
  unsigned short r=0;
  if (!(port<Ins)){
    CureCERR(20) << "\nDataFilter \"" << FilterName <<"\" warning  "
              << "Trying to get input port number "<<port
	      <<" I only have "<<(Ins)<<" input ports.";
    return (ADDRESS_INVALID|RESOURCE_ERROR);
  }
  if (port>-1)
    if (typeCheck(port,&inputData)){
      CureCERR(20)<<"DataFilter \"" << FilterName << "\""
	       << " got an invalid type written to it. "<<
	"Port # "<<(port)<<std::endl; 
      inputData.print();
      return (TYPE_ERROR|RESOURCE_ERROR);
    }       
  for (short i=0; i<Ins;i++)
    if (i!=port)
      {
	unsigned short r1=Ports[i].pullData(*data(i),inputData.Time);
	r=(r1|r);
	if (r1&(RESOURCE_ERROR)){
	  CureCERR(20)<<"DataFilter \"" << FilterName << "\""
		   << " got RESOURCE_ERROR on reading a pull. "
		   <<"Port # "<<(i )<<std::endl;
	  if (Ports[i].DataNeeded)return r;
	}
	else{
	  if (r1&(NO_INTERPOLATE)){		
	    CureCERR(20)<<"DataFilter \"" << FilterName << "\""
		     << " Needs to be able to interpolate the time "
		     <<"on reading a pull from "
		     <<"Port # "<<(i )<<std::endl;
	    return (r);
	  }
	}
	r1=(r1|typeCheck(i));
	if (r1&(TYPE_ERROR)){
	  CureCERR(20)<<"DataFilter \"" << FilterName << "\""
		   << " got a type error on reading a pull. "<<
	    "Port # "<<(i )<<" Disconnecting "<<r1<<std::endl;
	  Ports[i].IsPull=false;
	  r=(r1|r);
	}
      }
  return r;   
}
/**
 * p.Time=t;
 * getInputs(p,-1)
 * calc()
 * p=data(port);
 */

unsigned short DataFilter::read(TimestampedData& p,const int port, 
				const Timestamp t,
				const int interpolate)
{
  if (!(port<Ins+Outs)){
    CureCERR(20) << "\nDataFilter \"" << FilterName <<"\" warning  "
              << "Trying to read port number "<<port
	      <<" I only have "<<(Ins+Outs)<<"  ports.";
    return (RESOURCE_ERROR|ADDRESS_INVALID);
  }
  if (port<Ins){
    CureCERR(20) << "\nDataFilter \"" << FilterName <<"\" warning  "
              << "Trying to read and input port number "<<port
	      <<" I have "<<(Ins)<<" input ports.";
    return (RESOURCE_ERROR|ADDRESS_INVALID);
  }

  if (time(port)==t)
    {
      setEqual(port,p);
      return 0;
    }
  p.setTime(t);
  ErrorCode=getInputs(p,-1);
  if (ErrorCode){
    if (ErrorCode&(TYPE_ERROR|RESOURCE_ERROR))
      return RESOURCE_ERROR;
    if ((!(t==0))&&(ErrorCode&NO_INTERPOLATE))return ErrorCode;
  } 
  unsigned short r1=calc();
  setEqual(port,p);
  if (r1)return (r1);
  return ErrorCode;
}


DataFilterAddress * DataFilter::operator()(const std::string dataname){
  for (short j=0;j<(Ins+Outs);j++)
    if(Ports[j].m_Descriptor.Name==dataname)return &Ports[j];
  return 0;
}

unsigned short DataFilter::classCheck(int port,TimestampedData * inputData)
{
  if (inputData)
    {
      DataDescriptor d;
      inputData->getDescriptor(d);
      if (Ports[port].sameClass(d)){
	return DataFilter::typeCheck(port,inputData);
      }
      CureCERR(20) << "\nDataFilter \"" << FilterName <<"\" warning  "
		 << "Wrong type of input to  "<<port<<":\n";
      return TYPE_ERROR;
    }      
  return 0;
}
unsigned short DataFilter::matchWild(int port,TimestampedData * inputData)
{
  if (inputData)
    {
      DataDescriptor d;
      inputData->getDescriptor(d);
      if (Ports[port].matchWild(d)){
	return DataFilter::typeCheck(port,inputData);
      }
      CureCERR(20) << "\nDataFilter \"" << FilterName <<"\" warning  "
		 << "Wrong type of input to  "<<port<<":\n";
      return TYPE_ERROR;
    }      
  return 0;
}


  void DataFilter::printConfig(){
    std::cerr<<FilterName<<" Ins "<<Ins<<" Outs "<<Outs<<"\n";
    for (short i=0;i<(Ins+Outs);i++)
      Ports[i].m_Descriptor.print();
  }

