
// = RCSID
//    $Id: ServiceAddressProxy.cc ,v 1.1 2007/10/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2007 John Folkesson
//    
#include "ServiceAddressProxy.hh"
#include "CureDebug.hh"
#include <sstream>  // istringstream
using namespace Cure;

int ServiceAddressProxy::configService(const std::string  &str)
{
  std::istringstream strdes(str);
  std::string cmd;
  if(!(strdes>>cmd))return 1;
  if ((cmd=="ServiceName")||(cmd=="m_ServiceName")){
    strdes>>m_ServiceName;
    m_Service=0;
  }else  if ((cmd=="Print")||(cmd=="print"))
    printServiceConfig();
  else  return ServiceAddress::configService(str);
  return 0;
}  



void ServiceAddressProxy::printServiceConfig(){
  std::cerr<<"ServiceAddressProxy Thread "<<Thread<<"\n";
  m_Descriptor.print();
  std::cerr<<"True: Service "<<m_ServiceName<<" Thread "<<m_ServiceThread<<"\n";
}

