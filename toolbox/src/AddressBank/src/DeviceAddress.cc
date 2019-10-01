//
// = FILENAME
//    DeviceAddress.hh
//
// = FUNCTION
//
// = AUTHOR(S)
//    Patric Jensfelt
//
// = COPYRIGHT
//    Copyright (c) 2005 Patric Jensfelt
//
/*----------------------------------------------------------------------*/

#include "DeviceAddress.hh"

#include "CureDebug.hh"
#include "AddressBank.hh"
#include "ConfigFileReader.hh"
#include <sstream>  // istringstream


using namespace Cure::Hal;

DeviceAddress::DeviceAddress(const std::string &name, 
			     unsigned short numofDescriptors,
			     const unsigned short thread)
  :Address(thread),
   m_DeviceName(name)
{
  m_CannedDataFileName="";
  m_OutputDescriptors=0;
  m_NumberDescriptors=numofDescriptors;
  if (m_NumberDescriptors>0)
    m_OutputDescriptors=new Cure::DataDescriptor[numofDescriptors];
  m_PushClients = 0;
  m_NumPushClientsAlloc = 0;
  m_NumPushClients = 0;
  CommandExecuter::Service = 0; // not set
}

  DeviceAddress::~DeviceAddress()
{
  if (m_OutputDescriptors)delete[]m_OutputDescriptors;
  m_NumberDescriptors=0;
  m_OutputDescriptors=0;
  disconnectAddress();
}
void DeviceAddress::disconnectAddress(){
  Address::disconnectAddress();
  if ((m_NumPushClientsAlloc)&&(m_PushClients)) delete [] m_PushClients;
  m_PushClients=0;
  m_NumPushClientsAlloc=0;
  m_NumPushClients = 0;
}

int DeviceAddress::configure(const std::string &cfgFile, 
			     const std::string  &devicename)
{
  m_DeviceName=devicename;
  ConfigFileReader cfg;
  if (cfg.init(cfgFile.c_str())){
    return 1;  
  }   
  std::string desarglist="";
  std::string desparams;
  std::list<std::string> strings;
  if (cfg.getParamStrings(m_DeviceName, true, strings, desarglist))return 1;
  for (std::list<std::string>::iterator pi = strings.begin();
       pi != strings.end(); pi++) {       
    std::istringstream strdes(*pi);
    std::string cmd;
    if((strdes>>cmd)){
      if (((cmd=="OutputData")||
	   (cmd=="Outputdata"))
	  ||(cmd=="outputdata")){
	for (short i=0; i<m_NumberDescriptors;i++){
	  if((strdes>>m_OutputDescriptors[i].Name))
	    cfg.getDescritptor(m_OutputDescriptors[i]);
	  else break;
	}
      }else if ((cmd=="m_CannedDataFileName")
		||(cmd=="CannedDataFilename")
		||(cmd=="m_CannedDataFilename")
		||(cmd=="canneddatafilename")){
	std::string name;
	if ((strdes>>name))
	  m_CannedDataFileName=name;
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
      } 
      configDevice(*pi);
    } 
  }
  return 0;
}
 DeviceAddress * DeviceAddress::operator()(const std::string dataname)
{
  for (short i=0;i<m_NumberDescriptors;i++)
    if (m_OutputDescriptors[i].Name==dataname)return(this);
  return 0;
}
void DeviceAddress::addToBank(Cure::FilterBankAddress &fb, long buffersize)
{
  for (int i=0;i<m_NumberDescriptors;i++)
    fb.addPort(buffersize,m_OutputDescriptors[i]);
}
unsigned short 
DeviceAddress::pushData(TimestampedData &tp)
{  
  for (short i = 0; i < m_NumPushClients; i++) {
    if (writeTo(m_PushClients[i], tp) == 
        ADDRESS_INVALID) {
      CureCERR(20) << "WARNING: "
                   << m_DeviceName 
                   << " failed to push data, disconnecting\n";
      disconnect(m_PushClients[i]);
      i--;
    }
  }
  return 0;
}
unsigned short 
DeviceAddress::pushDataLocked(TimestampedData &tp)
{  
  for (short i = 0; i < m_NumPushClients; i++) {
    if (writeToLocked(m_PushClients[i], tp) == 
        ADDRESS_INVALID) {
      CureCERR(20) << "WARNING: "
                   << m_DeviceName 
                   << " failed to push data, disconnecting\n";
      disconnect(m_PushClients[i]);
      i--;
    }
  }
  return 0;
}

void 
DeviceAddress::disconnect(const unsigned long index)
{
  short i = 0; 
  while(i < m_NumPushClients) {
    if (index == m_PushClients[i]) {
      for(short j = i + 1; j< m_NumPushClients; j++) {
	m_PushClients[j-1] = m_PushClients[j];
      }
      m_NumPushClients--;
    } else {
      i++;
    }
  }
}

unsigned short 
DeviceAddress::push(Address * pa)
{
  if (m_NumPushClients == m_NumPushClientsAlloc) {
    // Need to allocate more space for more push connections
    unsigned long *temp = new unsigned long[m_NumPushClientsAlloc + 1];
    for (short i = 0; i < m_NumPushClientsAlloc; i++) temp[i] = m_PushClients[i];
    temp[m_NumPushClientsAlloc] = pa->Index;
    if (m_NumPushClientsAlloc) delete [] m_PushClients;
    m_PushClients = temp;
    m_NumPushClientsAlloc++;
    m_NumPushClients++;
    return 0;
  }
  // If we got here there is already allocated space for more connections
  m_PushClients[m_NumPushClients] = pa->Index;
  m_NumPushClients++;
  return 0;
}

unsigned short 
DeviceAddress::write(TimestampedData & p)
{
  CureCERR(20) << m_DeviceName << ": write() not implemented\n";
  return ADDRESS_INVALID;
}
int DeviceAddress::openCannedDataFile(std::string filename){
  m_CannedDataFileName=filename;
  m_CannedDataFile.open(filename.c_str(),std::ios::in);
  if (m_CannedDataFile.good()) {
    return 0;      
  }
  return 1;
}
void DeviceAddress::closeCannedDataFile()
{
  m_CannedDataFile.close();
}

int DeviceAddress::readCannedDataFile(Cure::Timestamp &tstop){  
  char cbuf[10000];
  while  (tstop>m_LastDataTime){   
    if (m_CannedDataFile.good())m_CannedDataFile.getline(cbuf,sizeof(cbuf));
    else return 1;
    std::string sLine(cbuf);
    parse(sLine);
  } 
  if (tstop==0){   
    if (m_CannedDataFile.good())m_CannedDataFile.getline(cbuf,sizeof(cbuf));
    else return 1;
    std::string sLine(cbuf);
    parse(sLine);
  } 
  tstop=m_LastDataTime;
  return 0;
}


