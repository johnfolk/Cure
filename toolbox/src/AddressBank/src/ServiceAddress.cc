
// = RCSID
//    $Id: ServiceAddress.cc ,v 1.1 2004/03/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2004 John Folkesson
//    
#include "ServiceAddress.hh"
#include "CureDebug.hh"
#include <sstream>  // istringstream
using namespace Cure;

void ServiceAddress::disconnectAddress()
{
  AddressBank::theAddressBank().removeService(this);
  Address::disconnectAddress();
  if ((NumPushClientsAlloc)&&(PushClients)) delete [] PushClients;
  PushClients=0;
  NumPushClientsAlloc=0;
  NumPushClients = 0;
  PullClient=0;  
  IsPull=false;
}
int ServiceAddress::configureService(ConfigFileReader &cfg,
				     std::string  &name)
{
  m_Descriptor.Name=name;
  cfg.getDescritptor(m_Descriptor);
  std::string desarglist="";
  std::string desparams;
  std::list<std::string> strings;
  if (cfg.getParamStrings(m_Descriptor.Name, true, strings, desarglist))return 1;
  for (std::list<std::string>::iterator pi = strings.begin();
       pi != strings.end(); pi++) {       
    std::istringstream strdes(*pi);
    std::string cmd;
    if(!(strdes>>cmd))return 1;
    if (((cmd=="Global")||
		 (cmd=="global"))
		||(cmd=="GLOBAL")){
      if((strdes>>cmd)){
	int key=0;
	if (!cfg.getGlobal(cmd,key)){
	  StateData p;
	  if (!(p=cmd)){
	    p=(*pi);
	    p.setStateID(key);
	    configServiceGlobal(*pi,p);
	  }
	}
      }
    }  else  if (((cmd=="Sensor")||
		  (cmd=="sensor"))
		 ||(cmd=="SENSOR")){
      if((strdes>>cmd)){
	if (!cfg.getSensor(cmd))
	  configServiceSensor(*pi,cmd);
      }
    } else  if (cmd=="Name"){
      strdes>>m_Descriptor.Name;
    } else  if (cmd=="Print"){
      printServiceConfig();
      if((strdes>>cmd))
	if (cmd=="pause"){
	  std::cerr<<"Waiting for enter";
	  getchar();
	}
    }
    configService(*pi);
  }
  return 0;
}  



void ServiceAddress::printServiceConfig(){
  m_Descriptor.print();
  }

