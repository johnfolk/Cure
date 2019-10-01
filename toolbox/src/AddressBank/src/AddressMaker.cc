//
// = FILENAME
//    AddressMaker.hh
//
// = FUNCTION
//
// = AUTHOR(S)
//    John folkesson
//
// = COPYRIGHT
//    Copyright (c) 2006 John Folkesson
//
/*----------------------------------------------------------------------*/

#include "AddressMaker.hh"
#include "SensorData.hh"
#include "ThreadMerger.hh"
#include "CureDebug.hh"
#include "SocketAddress.hh"
#include "ServiceAddressProxy.hh"

#include "ConfigFileReader.hh"
#include <sstream>  // istringstream

int Cure::initAddressBank(const std::string &cfgFile){
  ConfigFileReader cfg;
  if (cfg.init(cfgFile)) return -1;
  return initAddressBank(cfg);
}
int Cure::initAddressBank(ConfigFileReader &cfg){
  std::string desarglist="";
  std::string desparams;
  std::list<std::string> strings;
  if (cfg.getParamStrings("THREADSPACES", true, strings, desarglist)){
    CureCERR(20)<<"The ccf files did not have a THREADSPACES section.\n";
    return 1;
  }
  ShortMatrix depths(0,1);
  unsigned short depth;
  for (std::list<std::string>::iterator pi = strings.begin();
       pi != strings.end(); pi++) {       
    std::istringstream strdes(*pi);
    while (strdes>>depth){
      depths.append((short)depth,0);
    }
  }

  std::cerr<<"initAddressBank with "<<depths.Rows<<" thread spaces\n";
  if (depths.Rows==0){
    std::cerr<<"FAIL no depths????\n";
    return 1;
  }
  Cure::AddressBank::theAddressBank(depths.Rows,(unsigned short *)depths.Element);   
  if (Cure::AddressBank::theAddressBank().getNumberOfThreads()!=depths.Rows)
    {
      std::cerr<<"ERRROR NumberOfThreads was already set to "<<
	Cure::AddressBank::theAddressBank().getNumberOfThreads()<<
	".  The Behavior is now unpredictable!!!!";
      return 1;
    }
  return 0;
}

using namespace Cure;
using namespace Cure::Hal;
AddressMaker::AddressMaker(){
  m_Cfg=0;
  m_Configfile="";
  m_NumberServices=0;
  m_Services=0;
  m_NumberFiles=0;
  m_Files=0;
  m_Debugs=0;
  m_NumberDebugs=0;
  m_Banks=0;
  m_NumberBanks=0;
  m_Slots=0;
  m_NumberSlots=0;
  m_Filters=0;
  m_NumberFilters=0;
  m_NumberDevices=0;
  m_Devices=0;
}
void AddressMaker::disableAll()
{
  stopDevices();
  for (long i=0;i<m_NumberDevices;i++){
    unsigned short t=m_Devices[i]->Thread;
    AddressBank::theAddressBank().getThread(t);
    m_Devices[i]->disable();
    AddressBank::theAddressBank().releaseThread(t);
  }
  for (long i=0;i<m_NumberFilters;i++){
    unsigned short t=m_Filters[i]->Thread;
    AddressBank::theAddressBank().getThread(t);
    m_Filters[i]->disable();
    AddressBank::theAddressBank().releaseThread(t);
  }
  for (long i=0;i<m_NumberServices;i++){
    unsigned short t=m_Services[i]->Thread;
    AddressBank::theAddressBank().getThread(t);
    m_Services[i]->disable();
    AddressBank::theAddressBank().releaseThread(t);
  }
  for (long i=0;i<m_NumberFiles;i++){
    unsigned short t=m_Files[i]->Thread;
    AddressBank::theAddressBank().getThread(t);
    m_Files[i]->disable();
    AddressBank::theAddressBank().releaseThread(t);
  }
  for (long i=0;i<m_NumberSlots;i++){
    unsigned short t=m_Slots[i]->Thread;
    AddressBank::theAddressBank().getThread(t);
    m_Slots[i]->disable();
    AddressBank::theAddressBank().releaseThread(t);
  }
  for (long i=0;i<m_NumberDebugs;i++){
    unsigned short t=m_Debugs[i]->Thread;
    AddressBank::theAddressBank().getThread(t);
    m_Debugs[i]->disable();
    AddressBank::theAddressBank().releaseThread(t);
  }
  for (long i=0;i<m_NumberBanks;i++){
    unsigned short t=m_Banks[i]->Thread;
    AddressBank::theAddressBank().getThread(t);
    m_Banks[i]->disable();
    AddressBank::theAddressBank().releaseThread(t);
  }
}
void AddressMaker::deleteAll(){
  disableAll();
  while(m_NumberDevices>0){
    m_NumberDevices--;
    unsigned short t=m_Devices[m_NumberDevices]->Thread;
    AddressBank::theAddressBank().getThread(t);
    delete m_Devices[m_NumberDevices];
    AddressBank::theAddressBank().releaseThread(t);
  }
  if (m_Devices)delete[]m_Devices;
  m_NumberDevices=0;
  m_Devices=0;

  while(m_NumberFilters>0){
    m_NumberFilters--;
    unsigned short t=m_Filters[m_NumberFilters]->Thread;
    AddressBank::theAddressBank().getThread(t);
    CureCERR(60)<<"deleting"<<m_Filters[m_NumberFilters]->FilterName;
    delete m_Filters[m_NumberFilters];
    AddressBank::theAddressBank().releaseThread(t);
    CureCERR(60)<<" successful \n";
  }
  if (m_Filters)delete[]m_Filters;
  m_NumberFilters=0;
  m_Filters=0;
  while(m_NumberServices>0){
    m_NumberServices--;
    unsigned short t=m_Services[m_NumberServices]->Thread;
    AddressBank::theAddressBank().getThread(t);
    delete m_Services[m_NumberServices];
    AddressBank::theAddressBank().releaseThread(t);
  }
 if (m_Services)delete[]m_Services;
  m_NumberServices=0;
  m_Services=0;
 
  while(m_NumberFiles>0){
    m_NumberFiles--;
    unsigned short t=m_Files[m_NumberFiles]->Thread;
    AddressBank::theAddressBank().getThread(t);
    delete m_Files[m_NumberFiles];
    AddressBank::theAddressBank().releaseThread(t);
  }
  if (m_Files)delete[]m_Files;
  m_NumberFiles=0;
  m_Files=0;
  
  while(m_NumberSlots>0){
    m_NumberSlots--;
    unsigned short t=m_Slots[m_NumberSlots]->Thread;
    AddressBank::theAddressBank().getThread(t);
    delete m_Slots[m_NumberSlots];
    AddressBank::theAddressBank().releaseThread(t);
  }
  if (m_Slots)delete[]m_Slots;
  m_NumberSlots=0;
  m_Slots=0;

  while(m_NumberDebugs>0){
    m_NumberDebugs--;
    unsigned short t=m_Debugs[m_NumberDebugs]->Thread;
    AddressBank::theAddressBank().getThread(t);
    delete m_Debugs[m_NumberDebugs];
    AddressBank::theAddressBank().releaseThread(t);
  }
  if (m_Debugs)delete[]m_Debugs;
  m_NumberDebugs=0;
  m_Debugs=0;
  
  while(m_NumberBanks>0){
	m_NumberBanks--;
	unsigned short t=m_Banks[m_NumberBanks]->Thread;
	AddressBank::theAddressBank().getThread(t);
	delete m_Banks[m_NumberBanks];
	AddressBank::theAddressBank().releaseThread(t);
  }
  if (m_Banks)delete[]m_Banks;
  m_NumberBanks=0;
  m_Banks=0;
}
AddressMaker::~AddressMaker(){
  if (m_Cfg!=0)delete m_Cfg;
  m_Cfg=0;
  deleteAll();      
}

int AddressMaker::open(const std::string &cfgFile){
  if ((m_Cfg)&&(m_Configfile!=cfgFile)){
    delete (m_Cfg);
    m_Cfg=0;
  }
  if (m_Cfg==0){
    m_Configfile=cfgFile; 
    m_Cfg=new ConfigFileReader(); 
    if (m_Cfg->init(m_Configfile)) return -1;
    initAddressBank(*m_Cfg);
  }  
  return 0;
}
int AddressMaker::configure(const std::string &cfgFile){  
  checkDescriptors(cfgFile);
  makeServices(cfgFile);
  makeFiles(cfgFile);
  makeDebugs(cfgFile);
  makeBanks(cfgFile);
  makeFilters(cfgFile);
  makeDevices(cfgFile);
  hookup(cfgFile);
  return 0;
}
int AddressMaker::checkDescriptors(const std::string &cfgFile){
  if (open(cfgFile))return 1;
  std::string desarglist="";
  std::string desparams;
  std::list<std::string> strings;
  if (m_Cfg->getParamStrings("DATADESCRIPTORS", true, strings, desarglist)){
    CureCERR(20)<<"The ccf files did not have a DATADESCRIPTORS section.\n";
    return 1;
  }
  int n=strings.size();
  DataDescriptor des[n];
  int i=0;

  for (std::list<std::string>::iterator pi = strings.begin();
       pi != strings.end(); pi++) {       
    std::istringstream strdes(*pi);
    if ((strdes>>des[i].Name)){
      std::string match;
      int r=des[i].setTo((*pi),match);
      if (r>-1)return r;
      if (!m_Cfg->getSensor(match)){
	std::istringstream strde(match);
	if(strde>>match>>match>>des[i].SensorID){
	  getSensorType(match,des[i].SensorType);
	}
      }else i--;
    }else i--;
    i++;
  }
  int m=n-i;
  n=i;
  int k=0;
  for (i=0;i<(n-1);i++)
    for (int j=i+1;j<n;j++)
      if (des[i]==des[j])
	if (des[i].Name!=des[j].Name)
	  if (des[i].ID!=0){
	    k++;
	    CureDO(50){
	      CureCERR(10)<<"There were two descriptor defines that gave same disecriptor:\n";
	      des[i].print();
	      des[j].print();
	      CureCERR(10)<<"These should be the same data just different names\n";
	    }
	  }
  if (k)
    CureCERR(50)<<"The ccf file had "<<k<<"Indistinguishable Descriptors.\n";
  if (m)
    CureCERR(50)<<"ERROR: The ccf file had "<<m<<"Unreadable Descriptors.\n";
  return (m);
}

short AddressMaker::makeFiles(const std::string &cfgFile)
{
  if (open(cfgFile))return 1;
  if( m_Files)return 1;
  std::string arglist="";
  std::list<std::string> strings;
  std::string filepath=".";
  m_Cfg->getFilePath(filepath);
  unsigned long timeoffset=0;
  if (m_Cfg->getParamStrings("FILEADDRESS", true, strings, arglist)){
    CureCERR(20)<<"The ccf files did not have a FILEADDRESS section.\n";
    return 1;
  }
   m_NumberFiles=strings.size();
    if (m_NumberFiles>0)
     {
       m_Files=new Cure::FileAddress*[m_NumberFiles];
       m_NumberFiles=0;
       for (std::list<std::string>::iterator pi = strings.begin();
	    pi != strings.end(); pi++) { 
	 int newfile=0;
	 std::istringstream sst(*pi);
	 bool dname=false;
	 std::string opentype, filename, desname;
	 if((sst>>opentype)){
	   if (opentype=="Write"){
	     if ((sst>>filename)){
	       if((sst>>desname))dname=true;
	       unsigned short thread=0;
	       sst>>thread;
	       AddressBank::theAddressBank().getThread(thread);
	       m_Files[m_NumberFiles]=new FileAddress(thread); 
	       AddressBank::theAddressBank().releaseThread(thread);
	       m_Files[m_NumberFiles]
		 ->openWriteFile(filepath + "/" + filename);
	       newfile=1;
	     }
	   } else if (opentype=="Read"){
	     if ((sst>>filename)){
	       if((sst>>desname))dname=true;
	       unsigned short thread=0;
	       sst>>thread;
	       AddressBank::theAddressBank().getThread(thread);
	       m_Files[m_NumberFiles]=new FileAddress(thread); 
	       AddressBank::theAddressBank().releaseThread(thread);
	       m_Files[m_NumberFiles]
		 ->openReadFile(filepath + "/" + filename);
	       newfile=1;
	     }
	   }else if ((opentype=="FilePath")|(opentype=="Filepath")|
		     (opentype=="m_FilePath")|(opentype=="filepath")){
	     sst>>filepath;
	   }else if ((opentype=="TimeOffset")|(opentype=="Timeoffset")|
		     (opentype=="m_TimeOffset")|(opentype=="timeoffset")){
	     sst>>timeoffset;
	   }
	   if (newfile){
	     m_Files[m_NumberFiles]->m_TimeOffset.Seconds=timeoffset;
	     if((dname)){
	       m_Files[m_NumberFiles]->m_Descriptor.Name=desname;
	       m_Cfg->getDescritptor(m_Files[m_NumberFiles]->m_Descriptor);
	       int usedes;
	       if((sst>>usedes))
		 m_Files[m_NumberFiles]->m_UseDescriptor=usedes;
	     }  
	     std::string desarglist2="";
	     std::list<std::string> strings2;
	     if (!(m_Cfg->getParamStrings(filename, true, strings2, desarglist2)))
	       for (std::list<std::string>::iterator pi2 = strings2.begin();
		    pi2 != strings2.end(); pi2++) {       
		 m_Files[m_NumberFiles]->configure(*pi2);
	       }
	   }	
	 }
	 m_NumberFiles+=newfile;  
       }
     }
    return m_NumberFiles;
}


short AddressMaker::makeDebugs(const std::string &cfgFile)
{
  if (open(cfgFile))return 1;
  if( m_Debugs)return 1;
  std::string arglist="";
  std::list<std::string> strings;
  
  if (m_Cfg->getParamStrings("DEBUGADDRESS", true, strings, arglist)){
    CureCERR(20)<<"The ccf files did not have a DEBUGADDRESS section.\n";
    return 1;
  }
  m_NumberDebugs=strings.size();
    if (m_NumberDebugs>0)
     {

       m_Debugs=new Cure::DebugAddress*[m_NumberDebugs];
       m_NumberDebugs=0; 
       for (std::list<std::string>::iterator pi = strings.begin();
	    pi != strings.end(); pi++) { 
	 std::istringstream sst(*pi);
	 std::string  desname;
	 
	 std::string cmd;
	 if((sst>>cmd>>desname))
	   if (cmd=="Create"){
	     unsigned short thread=0;
	     sst>>thread;
	     AddressBank::theAddressBank().getThread(thread);
	     m_Debugs[m_NumberDebugs]=new DebugAddress(thread); 	
	     m_Debugs[m_NumberDebugs]->configure(*m_Cfg,desname);
	     AddressBank::theAddressBank().releaseThread(thread);
	     m_NumberDebugs++;  
	   }
      }
     }
    return m_NumberDebugs;
}

short AddressMaker::makeBanks(const std::string &cfgFile)
{
   if (open(cfgFile))return 1;
  if(m_Banks)return 1;
  std::string arglist="";
  std::list<std::string> strings;
  
  if (m_Cfg->getParamStrings("FILTERBANKADDRESS", true, strings, arglist)){
     CureCERR(20)<<"The ccf files did not have a FILTERBANKADDRESS section.\n";
    return 1;
  }
   m_NumberBanks=strings.size();
    if (m_NumberBanks>0)
     {
       m_Banks=new Cure::FilterBankAddress*[m_NumberBanks];
       m_NumberBanks=0;
       for (std::list<std::string>::iterator pi = strings.begin();
	    pi != strings.end(); pi++) { 
	 std::istringstream sst(*pi);
	 std::string   desname;
	 unsigned short thread=0;
	 m_Banks[m_NumberBanks]=0; 	
	 std::string cmd;
	 if((sst>>cmd>>desname))
	   if (cmd=="Create"){
	     sst>>thread;

	     AddressBank::theAddressBank().getThread(thread);
	     unsigned short thread2=thread;
	     m_Banks[m_NumberBanks]=new FilterBankAddress(thread2);
	     m_Banks[m_NumberBanks]->setName(desname);
	     std::string arglist2="";
	     std::list<std::string> strings2;
	     if (!m_Cfg->getParamStrings(desname, true, strings2, arglist2)){
	       for (std::list<std::string>::iterator pi2 = strings2.begin();
		    pi2 != strings2.end(); pi2++) { 
		 std::istringstream sst2(*pi2);
		 std::string cmd;
		 short depth=1;
		 DataDescriptor d;
		 if (sst2>>cmd>>depth>>d.Name)
		   if (cmd=="addPort"){
		     m_Cfg->getDescritptor(d);
		     unsigned short subtyp=0;
		     int compressed=0;
		     int dotychk=0;
		     int verbose=0;
		     sst2>>subtyp>>compressed>>dotychk>>verbose;
		     DataPort *dp=m_Banks[m_NumberBanks]
		       ->addPort(depth,d,
				 subtyp,compressed,
				 dotychk);
		     if (dp)dp->Verbose=verbose;
		     
		   }
	       }
	     }
	     AddressBank::theAddressBank().releaseThread(thread); 	
	     m_NumberBanks++;  
	   }
       }
     }
    return m_NumberBanks;
}


short AddressMaker::makeSlots(const std::string &cfgFile)
{
  if (open(cfgFile))return 1;
  if( m_Slots)return 1;
  std::string arglist="";
  std::list<std::string> strings;
  
  if (m_Cfg->getParamStrings("DATASLOTADDRESS", true, strings, arglist)){
    CureCERR(20)<<"The ccf files did not have a DATASLOTADDRESS section.\n";
    return 1;
  }
  m_NumberSlots=strings.size();
  if (m_NumberSlots>0)
    {
       m_Slots=new Cure::DataSlotAddress*[m_NumberSlots];
       m_NumberSlots=0;
       for (std::list<std::string>::iterator pi = strings.begin();
	    pi != strings.end(); pi++) { 
	 std::istringstream sst(*pi);
	 std::string  cmd, desname;
	 short depth=1;
	 unsigned short subtyp=0;
	 int compressed=0;
	 int dotypchk=0;
	 m_Slots[m_NumberSlots]=0;
	 DataDescriptor d;
	 if((sst>>cmd>>depth>>d.Name))
	   if (cmd=="Create"){
	     unsigned short thread=0;
	     sst>>thread;
	     unsigned short thread2=thread;
	     m_Cfg->getDescritptor(d);
	     AddressBank::theAddressBank().getThread(thread);
	     m_Slots[m_NumberSlots]=new DataSlotAddress(depth,d.ClassType,
							subtyp,compressed,
							dotypchk, thread2);
	     AddressBank::theAddressBank().releaseThread(thread); 	
	     int num=0;
	     m_Slots[m_NumberSlots]->setName(d.Name);
	     if((sst>>num)){
	       m_Slots[m_NumberSlots]->setUseLock(num);
	       if((sst>>num))
		 m_Slots[m_NumberSlots]->setUseThread(num);
	     }
	     m_NumberSlots++;  
	   }
       }
    }

  return m_NumberSlots;
}     

ThreadSwitcher * AddressMaker::addSwitcher(unsigned short threads[2]){
  AddressBank::theAddressBank().getThread(threads[0]);
  ThreadSwitcher *ts=new ThreadSwitcher(threads);
  AddressBank::theAddressBank().releaseThread(threads[0]);
  Cure::DataFilter** m=m_Filters;
  m_Filters=new Cure::DataFilter*[m_NumberFilters+1];
  memcpy(m_Filters,m,m_NumberFilters*sizeof (Cure::DataFilter *));
  m_Filters[m_NumberFilters]=ts;
  m_NumberFilters++;
  delete [] m;
  return ts;
}
short AddressMaker::makeFilters(const std::string &cfgFile)
{
  if (open(cfgFile))return 1;
  if( m_Filters)return 1;
  std::string arglist="";
  std::list<std::string> strings;
  
  if (m_Cfg->getParamStrings("DATAFILTER", true, strings, arglist)){
    CureCERR(20)<<"The ccf files did not have a DATAFILTER section.\n";
    return 1;
  }
  m_NumberFilters=strings.size();
  if (m_NumberFilters>0)
    {
      m_Filters=new Cure::DataFilter*[m_NumberFilters];
      m_NumberFilters=0;
      for (std::list<std::string>::iterator pi = strings.begin();
	   pi != strings.end(); pi++) { 
	std::istringstream sst(*pi);
	m_Filters[m_NumberFilters]=0;
	
	std::string cmd,filtername;
	if((sst>>cmd>>filtername)){
	  if (cmd=="Create"){
	    unsigned short thread=0;
	    sst>>thread;
	    unsigned short thread2=thread;
	    std::string classname;
	    std::string configline;
	    m_Cfg->getConstructorInfo(filtername,classname,configline);	    
	    CureCERR(40)<<"Try to Make a Filter "<<configline<<" ";
	    AddressBank::theAddressBank().getThread(thread); 	
	    m_Filters[m_NumberFilters]=makeFilterSubClass(classname,
							  configline, thread2);
	    if (m_Filters[m_NumberFilters])
	      {
		m_Filters[m_NumberFilters]->configure(*m_Cfg,
						      filtername);
		m_NumberFilters++;  
	      }else std::cerr<<" Failed "<<filtername<<"\n";
	    AddressBank::theAddressBank().releaseThread(thread); 	
	  }
	}
      }
    }
  return m_NumberFilters;
}


DataFilter * AddressMaker::makeFilterSubClass(const std::string classname,
					      const std::string constargs, 
					      unsigned short thread)
  {
    std::istringstream strdes(constargs);
    std::string cmd;
    //skip the create classname;
    strdes>>cmd>>cmd;
    if (classname=="ThreadMerger") { 
	short numins=1;
	if (strdes>>numins){
	  short numports=2*numins;
	  unsigned short threads[numports];
	  for (short i=0;i<numports;i++){
	    threads[i]=i;
	    strdes>>threads[i];
	  }
	  return new Cure::ThreadMerger(numins,thread,threads);    
	}else if (classname=="ThreadSwitcher") { 
	  unsigned short threads[2];
	  for (short i=0;i<2;i++){
	      threads[i]=i;
	      strdes>>threads[i];
	  }
	  return new Cure::ThreadSwitcher(threads); 
	}  
    }
    
    return 0;
 
}
short AddressMaker::makeServices(const std::string &cfgFile)
{
  if (open(cfgFile))return 1;
  if( m_Services)return 1;
  std::string arglist="";
  std::list<std::string> strings;
  
  if (m_Cfg->getParamStrings("SERVICEADDRESS", true, strings, arglist)){
    CureCERR(20)<<"The ccf files did not have a SERVICEADDRESS section.\n";
    return 1;
  }
   m_NumberServices=strings.size();
    if (m_NumberServices>0)
     {
       m_Services=new Cure::ServiceAddress*[m_NumberServices];
       m_NumberServices=0;
       for (std::list<std::string>::iterator pi = strings.begin();
	    pi != strings.end(); pi++) { 
	 std::istringstream sst(*pi);
	 std::string devicename, cmd;
	 if(!(sst>>cmd>>devicename))return 1;
	 if (cmd=="Create"){
	   unsigned short thread=0;
	   sst>>thread;
	   unsigned short thread2=thread;
	   std::string classname;
	   std::string configline;
	   m_Cfg->getConstructorInfo(devicename,classname,configline);
	   AddressBank::theAddressBank().getThread(thread);	
	   m_Services[m_NumberServices]=makeServiceSubClass(classname,
							    configline,
							    thread2);
	   if (m_Services[m_NumberServices])
	     {
	      
	       m_Services[m_NumberServices]
		 ->configureService(*m_Cfg, devicename);
	       m_NumberServices++;  
	     }
	   AddressBank::theAddressBank().releaseThread(thread);
	 }
       }
     }
    return m_NumberServices;
}

ServiceAddress * 
AddressMaker::makeServiceSubClass(const std::string classname,
				  std::string constargs,
				  unsigned short thread)
{
  std::istringstream strdes(constargs);
  std::string cmd;
  strdes>>cmd>>cmd;
  if (classname=="ServiceAddress")
    return new Cure::ServiceAddress(cmd,thread); 
  if (classname=="ServiceAddressProxy"){
    unsigned short servicethread=thread;
    strdes>>servicethread;
    return new Cure::ServiceAddressProxy(cmd,thread,servicethread);
  }
  return 0; 
}

 int AddressMaker::makeDevices(const std::string &cfgFile)
{
  if (open(cfgFile))return 1;
  if( m_Devices)return 1;
  
  std::string arglist="";
  std::list<std::string> strings;
  
  if (m_Cfg->getParamStrings("DEVICEADDRESS", true, strings, arglist)){
    CureCERR(20)<<"The ccf files did not have a DEVICEADDRESS section.\n";
    return 1;
  }
  m_NumberDevices=strings.size();
    if (m_NumberDevices>0)
     {
       m_Devices=new Cure::Hal::DeviceAddress*[m_NumberDevices];
       m_NumberDevices=0;
       for (std::list<std::string>::iterator pi = strings.begin();
	    pi != strings.end(); pi++) { 
	 std::istringstream sst(*pi);
	 std::string devicename, cmd;
	 if(!(sst>>cmd>>devicename))return 1;
	 if (cmd=="Create"){
	   std::string classname;
	   std::string configline;
	   unsigned short thread=0;
	   sst>>thread;
	   unsigned short thread2=thread;
	   m_Cfg->getConstructorInfo(devicename,classname,configline);
	   AddressBank::theAddressBank().getThread(thread);
	   m_Devices[m_NumberDevices]=makeDeviceSubClass(classname,
						       configline,thread2);
	   if (m_Devices[m_NumberDevices])
	     {
	       m_Devices[m_NumberDevices]->configure(m_Configfile,
						     devicename);
	       m_NumberDevices++;  
	     }
	   AddressBank::theAddressBank().releaseThread(thread);
	 }
       }
     }
    return m_NumberDevices;
}
short AddressMaker::startDevices(){
  short ret=0;
  for (short i=0;i<m_NumberDevices;i++){
    CureCERR(40)<<"Starting device "<<m_Devices[i]->m_DeviceName<<".";
    unsigned short r= m_Devices[i]->startDevice();
    if (r){CureCERR(40)<<"ERROR:  failed to start. "<<r<<"\n";}
    else {
      CureCERR(40)<<" Started.\n";
      ret++;
    }
  }
  return ret;
}
short AddressMaker::stopDevices(){
  short ret=0;
  for (short i=m_NumberDevices;i>0;i--){
    CureCERR(50)<<"Stoping device "<<m_Devices[i-1]->m_DeviceName<<".";
   unsigned short r= m_Devices[i-1]->stopDevice();
   if (r){CureCERR(10)<<"ERROR:  failed to stop. "<<r<<"\n";}
    else {
      CureCERR(60)<<" Stopped.\n";
      ret++;
    }
  }
  return ret;
}
DeviceAddress * 
AddressMaker::makeDeviceSubClass(const std::string classname,
				 std::string constargs,
				 unsigned short thread)
{ 
  // An example of what you must do in the subclass
  if (classname=="SocketAddress"){
    std::istringstream strarg(constargs);
    std::string name, cname; 
    std::string cmd;
    if ((strarg>>cmd>>cname>>name))
      return new SocketAddress(name,thread);
  }
  return 0;

}
Address * AddressMaker::operator()(const std::string dataname)
{
  for (short i=0;i<m_NumberServices;i++)
    if ((*m_Services[i])==dataname)return m_Services[i];
  for (short i=0;i<m_NumberDevices;i++)
    if ((*m_Devices[i])==dataname)return m_Devices[i];
  for (short i=0;i<m_NumberBanks;i++)
      if ((*m_Banks[i])==dataname)return m_Banks[i];
  for (short i=0;i<m_NumberSlots;i++)
      if ((*m_Slots[i])==dataname)return m_Slots[i];
  for (short i=0;i<m_NumberFiles;i++){
    if ((*m_Files[i])==dataname)return m_Files[i];
  } 
for (short i=0;i<m_NumberDebugs;i++)
      if ((*m_Debugs[i])==dataname)return m_Debugs[i];
 return 0;
}
Address * AddressMaker::operator()(const std::string objname,
				   const std::string dataname)
{
  if (objname=="")return(*this)(dataname);
  for (short i=0;i<m_NumberDevices;i++)
    if ((*m_Devices[i])==objname){
      Cure::Address *a=(*m_Devices[i])(dataname);
      if (a)return a;
    }  
  Address *a=0;
  for (short i=0;i<m_NumberBanks;i++)
    if ((*m_Banks[i])==objname){
      a=(*m_Banks[i])(dataname);
	if (a)return a;
    }  
  for (short i=0;i<m_NumberFilters;i++)
    if ((*m_Filters[i])==objname){
      a=(*m_Filters[i])(dataname);
      if (a)return a;
    }  
  return 0;
}

unsigned short AddressMaker::push(const std::string fromobjname,
				  const std::string fdataname,
				  const std::string toobjname,
				  const std::string tdataname)
{
  std::cerr<<fromobjname<<" "<<fdataname<<" to "
	   <<toobjname<<" "<<tdataname<<"\n";
  Address *a=(*this)(fromobjname,fdataname);
  if (a){
    Address *b=(*this)(toobjname,tdataname);
    if (b){
      if (a->Thread!=b->Thread){
	CureCERR(100)<<"\n\nYou can't just push from  thread "<<a->Thread
		     <<" to thread "
		     <<b->Thread<<" THREAD ERRROR \n\n";
	std::cerr<<"Making a ThreadSwitcher from "
		 <<a->Thread<<" to "<<b->Thread<<"\n";
	unsigned short threads[2];	
	threads[0]=a->Thread;
	threads[1]=b->Thread;
	ThreadSwitcher *ts=addSwitcher(threads);
	if (ts){
	  AddressBank::theAddressBank().getThread(b->Thread);
	  ts->out(0)->push(b);
	  AddressBank::theAddressBank().releaseThread(b->Thread);
	  b=ts->in(0);
	}
	else {
	  std::cerr<<"Failed to Push, thread violation\n";
	  return (ADDRESS_INVALID);
	}
      }
      CureCERR(80)<<"call push";
      AddressBank::theAddressBank().getThread(a->Thread);
      unsigned short ret=a->push(b);
      AddressBank::theAddressBank().releaseThread(a->Thread);
      return ret;
    }
  }
  if (!a){CureCERR(10)<<"\n"<<fromobjname<<" "<<fdataname<<" ";}
  else{ CureCERR(10)<<"\n"<<toobjname<<" "<<tdataname<<" "; }
  std::cerr<<"Not Found??????????????????????????????\n";
  return ADDRESS_INVALID;
}
unsigned short AddressMaker::pull(const std::string fromobjname,
				  const std::string fdataname,
				  const std::string toobjname,
				  const std::string tdataname)
{
  std::cerr<<toobjname<<" "<<tdataname<<" from "
	   <<fromobjname<<" "<<fdataname<<"\n";
  Address *a=(*this)(fromobjname,fdataname);
  if (a){
    Address *b=(*this)(toobjname,tdataname);
    if (b){
      if (a->Thread!=b->Thread){
	CureCERR(100)<<"\n\nYou can't just pull from  thread "<<a->Thread
		     <<" to thread "
		     <<b->Thread<<" THREAD ERRROR \n\n";
	std::cerr<<"Making a ThreadSwitcher from "
		 <<a->Thread<<" to "<<b->Thread<<"\n";
	unsigned short threads[2];	
	threads[0]=a->Thread;
	threads[1]=b->Thread;
	ThreadSwitcher *ts=addSwitcher(threads);
	if (ts){
	  AddressBank::theAddressBank().getThread(a->Thread);
	  ts->in(0)->pull(a);
	  AddressBank::theAddressBank().releaseThread(a->Thread);
	  a=ts->out(0);
	}else {
	  std::cerr<<"Failed to Pull, thread violation\n";
	  return (ADDRESS_INVALID);
	}
      }
      AddressBank::theAddressBank().getThread(a->Thread);
      unsigned short ret =b->pull(a);
      AddressBank::theAddressBank().releaseThread(a->Thread);
      return ret;
    }
  }
  std::cerr<<"Not Found???????????????????????\n";
  return ADDRESS_INVALID;
}
int AddressMaker::hookup(const std::string &cfgFile)
{
  if (open(cfgFile))return 1;
  std::string arglist="";
  std::list<std::string> strings;
  if (m_Cfg->getParamStrings("HOOKUP", true, strings, arglist)){
    CureCERR(20)<<"The ccf files did not have a HOOKUP section.\n";
    return 1;
  }
  for (std::list<std::string>::iterator pi = strings.begin();
       pi != strings.end(); pi++) { 
    std::istringstream sst(*pi);
    std::string str[6];
    int i=0;
    while(sst>>str[i]){
      if ((((str[i]=="to")||(str[i]=="TO"))||
	   ((str[i]=="from")||(str[i]=="FROM")))||
	  ((str[i]=="To")||(str[i]=="From")))
	{
	  if (i==2){
	    str[3]=str[2];
	    str[2]=str[1];
	    str[1]="";
	    i++;
	  }else if (i!=3){
	    CureCERR(40)<<"AddressMaker ERROR in "<<
	      str[0]<<" hookup \n"<<(*pi)<<"\n";
	    return 1;
	  }
	}
      i++;
      if (i==6)break;
    }
    if (i==5){
      str[5]=str[4];
      str[4]="";
    }
    if (((str[0]=="Pull")||(str[0]=="pull"))||(str[0]=="PULL")){
      if (pull(str[4],str[5],str[1],str[2])){
	CureCERR(40)<<"AddressMaker ERROR in "
		    <<str[0]<<" hookup \n"<<(*pi)<<"\n";
      }
    }else if (((str[0]=="Push")||(str[0]=="push"))||(str[0]=="PUSH")){
      if (push(str[1],str[2],str[4],str[5])){
	CureCERR(40)<<"AddressMaker ERROR in "
		    <<str[0]<<" hookup \n"<<(*pi)<<"\n";
      }
    } else {
      CureCERR(40)<<"AddressMaker ERROR in "<<
	str[0]<<" hookup \n"<<(*pi)<<"\n";
      return 1; 
    }
  }
  return 0;
}
