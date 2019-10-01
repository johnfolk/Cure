#include "SocketAddress.hh"
#include "CureDebug.hh"
#include "DataSet.hh"
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <vector>

int writes(int fd, void * cmd, unsigned int sz)
{
  return write(fd,cmd,sz);
}

int  reads(int fd, void * cmd, unsigned int sz)
{
  return read(fd,cmd,sz);
}

using namespace Cure;

using namespace Hal;

SocketAddress::SocketAddress(const std::string &name,
			     const unsigned short thread) 
  :Cure::Hal::DeviceAddress(name, 0,thread)
{
  m_RetryPeriod=5.0;
  m_Verbose=0;
  m_Running = false;
  m_Stopped=true;
  m_NumberClients=0;
  m_Clients=0;
  m_PushLocked=false;
  m_Hostname="127.0.0.1";
  m_Server=true;
  m_Port=3300;
  m_Persistent=true;
  m_DelimitCharacter=13;
  socketpair(AF_LOCAL, SOCK_STREAM, 0, m_SockPair);  
}

SocketAddress::~SocketAddress()
{
  stopDevice();
  for (unsigned int i=0; i<m_NumberClients;i++)
    {
      delete m_Clients[i];
      m_Clients[i]=0;
    }
  if (m_Clients)delete []m_Clients;
  m_Clients=0;
  close(m_SockPair[0]);
  close(m_SockPair[1]); 
}
unsigned short SocketAddress::configDevice(const std::string &param)
{
  std::istringstream strdes(param);
  std::string cmd;
  if(!(strdes>>cmd))return 1;
  if (cmd=="m_Verbose")
    strdes>>m_Verbose;
  if (cmd=="m_Port"){
    strdes>>m_Port;
  }
  if (cmd=="m_Hostname"){
    strdes>>m_Hostname;
  }
  if (cmd=="m_Server"){
    strdes>>m_Server;
  } else if (cmd=="m_Persistent"){
    strdes>>m_Persistent;
  }else if (cmd=="m_DelimitCharacter"){
    strdes>>m_DelimitCharacter;
  }

  
  if (cmd=="m_RetryPeriod"){
    double d=0;
    if (strdes>>d)m_RetryPeriod=d;
  }  if (cmd=="StartNow"){
    startDevice(); 
  }

  if (cmd=="m_PushLocked")
    strdes>>m_PushLocked;
  return 0;
}

unsigned short SocketAddress::startSocket()
{
  m_WriteMutex.lock();
  if (!m_Sock.m_Open){
    if (m_RetryPeriod>0){
      Cure::Timestamp t=m_LastTry;
      t+=m_RetryPeriod;
      if (t<t.getCurrentTime())
	m_LastTry.setToCurrentTime();
      else {
	m_WriteMutex.unlock();
	return RESOURCE_ERROR;
      }
    }
    if (m_Server){
      if (m_Verbose)std::cerr<<"Conecting Server: ";
      if (m_Sock.connectServer(m_Port)){
	m_WriteMutex.unlock(); 
	if (m_Verbose)
	  std::cerr<<m_DeviceName<<" Failed to connect Server Socket\n";
	return RESOURCE_ERROR;
      }
    } else {	
      if (m_Verbose)std::cerr<<"Conecting Client: ";
      if (m_Sock.connectClient(m_Hostname,m_Port)){
	m_WriteMutex.unlock(); 
	if (m_Verbose)
	  std::cerr<<m_DeviceName<<" Failed to connect Client Socket\n";
	return RESOURCE_ERROR;
      }
    }
    if (m_Verbose)std::cerr<<m_DeviceName<<" Success connected Socket\n";
  }
  m_WriteMutex.unlock();
  return 0;
}
unsigned short SocketAddress::startDevice()
{
  if (m_Verbose)std::cerr<<"StartingSocket "<<m_Server<<"\n";
  if (startSocket())
    if (!m_Persistent)return RESOURCE_ERROR;
  m_WriteMutex.lock();
  if (m_Stopped){
    m_Running = true;
    m_WriteMutex.unlock(); 
    if (pthread_create(&m_Thread, NULL, tramp, this)){
      m_WriteMutex.lock();
      m_Running=false;
      m_Sock.closeFD();
      m_WriteMutex.unlock();
      return RESOURCE_ERROR;
    }
  }else m_WriteMutex.unlock(); 
  return 0;
}
unsigned short SocketAddress::stopDevice()
{
  std::cerr<<m_DeviceName<<" stopDevice";
  m_WriteMutex.lock(); 
  if (!m_Stopped)
    if (m_Running){
      m_Running = false; 
      long len=0;
      writes(m_SockPair[0], &len, sizeof(long));
      m_WriteMutex.unlock(); 
      std::cerr<<m_DeviceName<<" joining thread\n";
      pthread_join(m_Thread, NULL);
      std::cerr<<m_DeviceName<<" joined thread\n";
      return 0;
    }
  m_WriteMutex.unlock(); 
  return 0;
}

unsigned short SocketAddress::runEventLoop()
{
  int lengthofchar=-1;
  m_WriteMutex.lock(); 
  m_Stopped=false;
  m_WriteMutex.unlock(); 
  CureCERR(30)<<"SocketAddress::runEventLoop: "
	      <<m_DeviceName<<" is running\n"<<
    "       Host: "<<m_Hostname<<" Port: "<<m_Port<<"\n";
  
  while(m_Running) {
    int timeout=-1;
    if (m_Persistent)
      if (startSocket())timeout=1000;
    struct pollfd *fds;
    int numfd= 1;
    if (timeout==-1)numfd=(2 + m_NumberClients);
    fds = (struct pollfd *)calloc(1, numfd*sizeof(struct pollfd));
    fds[0].fd = m_SockPair[1];
    fds[0].events = POLLIN;
    if (timeout==-1){
      fds[1].fd = m_Sock.m_FD;
      fds[1].events = (POLLIN|POLLHUP);//|POLLNVAL|POLLERR);
      if (!m_Server) if(m_Sock.m_WriteBufLen) fds[1].events |= POLLOUT;
      for(unsigned short i = 0; i < m_NumberClients; i++) {
	fds[2+i].fd = m_Clients[i]->m_FD;
	fds[2+i].events = POLLIN|POLLHUP;//|POLLNVAL|POLLERR;
	if(m_Clients[i]->m_WriteBufLen) fds[2+i].events |= POLLOUT;
      }
    }
    int ret;	
    ret = poll(fds, numfd, timeout);
 
    if ((ret == -1)||(ret==0)) {
      free(fds);
      continue;
    }
    if(fds[0].revents & POLLIN) {
      int ret=sizeof(long);
      if (lengthofchar==-1) //we need to read length
	ret=reads(this->m_SockPair[1], &lengthofchar, sizeof(long));
      if (ret!=sizeof(long)){
	std::cerr<<"ERROR in SocketAddress reading socket pair.\n"<<
	  "Should always get 4 bytes.\n";
	lengthofchar=-1; //I don't know. Try again?
      }
      if (lengthofchar>0){//there is a pointer coming
	void *ptr=0;
	ret=reads(this->m_SockPair[1], &ptr, sizeof(void *));
	if (ret==sizeof(void*)){//got a pointer
	  char *sp=(char *)ptr;
	  if (sp){ //just checking
	    if (timeout==-1){
	      if (m_Server){
		for(short i=0; i<(int)m_NumberClients; i++) 
		  m_Clients[i]->appendWriteQueue(sp,lengthofchar);
	      }else {
		m_Sock.appendWriteQueue(sp,lengthofchar);
	      }
	    }
	    lengthofchar=-1; //to read next lenth
	    free(sp);
	  }else lengthofchar=-1;
	}//if no more in buffer we just loop 
      } else lengthofchar=-1; //fail or wakeup to stop
    }
    if (timeout==-1){
      for(unsigned short i=0; i<m_NumberClients; i++) {
	if(fds[2+i].revents & (POLLERR | POLLHUP | POLLNVAL)) {	
	  //Client died
	  delete m_Clients[i];
	  m_Clients[i]=0;
	}else{
	  if(fds[2+i].revents & POLLIN) {
	    m_Clients[i]->directRead();
	    if(!m_Clients[i]->m_Open) {
	      delete m_Clients[i];
	      m_Clients[i]=0;
	      continue;
	    }
	  }
	}
	if (m_Clients[i]){
	  if(fds[2+i].revents & POLLOUT) {
	    m_Clients[i]->directWrite();
	    if(!m_Clients[i]->m_Open) {
	      delete m_Clients[i];
	      m_Clients[i]=0;
	    }
	  }
	}
      }
      for(unsigned short i=0; i<m_NumberClients; i++) {
	if (!m_Clients[i]){
	  for (unsigned short j=i+1; j<m_NumberClients; j++) {
	    m_Clients[j-1]=m_Clients[j];
	  }
	  m_NumberClients--;
	  m_Clients[m_NumberClients]=0;
	}
      }
      
      if (!m_Server){
	bool died=false;
	if(fds[1].revents & (POLLERR | POLLHUP | POLLNVAL)) {	
	  //Server died
	  died=true;
	  m_Sock.closeFD();
	}else{
	  if(fds[1].revents & POLLIN) 
	    m_Sock.directRead();
	  if(m_Sock.m_Open) 
	    if(fds[1].revents & POLLOUT) 
	      m_Sock.directWrite();
	  
	}
	if(!m_Sock.m_Open) died=true;
	if ((died)&&(!m_Persistent)){
	  m_WriteMutex.lock(); 
	  m_Running=false;
	  m_Stopped=true;
	  m_WriteMutex.unlock(); 
	  return 0;
	}
      }else {
	if(fds[1].revents & POLLIN){
	  // new client
	  SocketWrapper **temp=m_Clients;
	  m_Clients=new SocketWrapper * [m_NumberClients+1];
	  memcpy(m_Clients,temp,sizeof(SocketWrapper*)*m_NumberClients);
	  m_Clients[m_NumberClients]=m_Sock.acceptClient();
	  if (m_Clients[m_NumberClients]!=0)
	    m_NumberClients++;
	}
      }
    }
    free(fds);
      
    // now look for command/response stuff
    if (timeout==-1){
      if (m_Server){
	for(unsigned short i=0; i<m_NumberClients; i++) {
	  char *str;
	  int pos=m_Clients[i]->readLine(&str, m_DelimitCharacter);
	  while(pos >0) {
	    if (!parse(str,pos)){
	      m_TPoint.unpack();
	      if (m_PushLocked)
		pushDataLocked(*m_TPoint.getTPointer());
	      else
		pushData(*m_TPoint.getTPointer());
	    }
	    free(str);
	    pos=m_Clients[i]->readLine(&str,m_DelimitCharacter);
	  }
	}

      }else{
	char *str;
	int pos=m_Sock.readLine(&str,m_DelimitCharacter);
	while(pos >0) {
	  if (!parse(str,pos)){
	  m_TPoint.unpack();
	  if (m_PushLocked)
	    pushDataLocked(*m_TPoint.getTPointer());
	  else
	    pushData(*m_TPoint.getTPointer());
	  }
	  free(str);
	  pos=m_Sock.readLine(&str,m_DelimitCharacter);
	}
      }
    }
  }
  
  m_WriteMutex.lock(); 
  m_Stopped=true;
  m_WriteMutex.unlock(); 
  return 0;
}


unsigned short SocketAddress::write(TimestampedData & p)
{ 
  bool stopp=m_Stopped;
  if (stopp){
    m_WriteMutex.lock(); 
    stopp=m_Stopped;
    m_WriteMutex.unlock();
  }
  if (stopp){
    if (m_RetryPeriod>0){
      Cure::Timestamp t=m_LastTry;
      t+=m_RetryPeriod;
      if (t<t.getCurrentTime()){
	if (startDevice())return RESOURCE_ERROR;
      }else return RESOURCE_ERROR;
    }else return RESOURCE_ERROR;
  }
  char *str;
  int len=Cure::SmartData::writeChar(p, &str,1);  
  str[0]='t'; //TimestampedData Header
  str[len-1]=m_DelimitCharacter;
  if (m_Verbose!=0){ 
    std::cerr<<"Write "<<m_Server<<" "<<m_DeviceName<<" "<<len
	     <<" "<<str;
    m_Verbose--;
  }  
  return write(str, len);
}
unsigned short SocketAddress::write(void *pp, long len)
{
  if (!m_Running)return RESOURCE_ERROR;
  if (len>0){
    m_WriteMutex.lock();
    int r=writes(m_SockPair[0], &len, sizeof(long));
    writes(m_SockPair[0], &pp, sizeof(void *));
    m_WriteMutex.unlock();
    if (r==-1)return RESOURCE_ERROR;
  }
  return 0;
}


int  SocketAddress::parse(char *chr, int len)
{
  if (m_Verbose!=0){
    std::cerr<<len<<" Read  "<<m_Server<<" "<<m_DeviceName<<" "<<std::string(chr);
    m_Verbose--;
  }
  if (chr[0]=='t')
    return m_TPoint.readChar(chr+1,len-1);
  //Read not yet implemented
  if (chr[0]=='r')return RESOURCE_ERROR;
  
  alternateParse(chr,len);
  return -1;
}
