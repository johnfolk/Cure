#include "include/SocketWrapper.hh"

#include <netdb.h>

#include <unistd.h>
#include <iostream>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include "CureDebug.hh"

using namespace Cure;
using namespace Hal;

SocketWrapper::SocketWrapper()
{
  m_WriteMaxLen = -1;
  m_FD = -1;
  m_Blocksize = 16384;

  init();
}
SocketWrapper::~SocketWrapper()
{
  if (m_Open)closeFD();
}
void SocketWrapper::init()
{
  // assume default blocksize
  m_Initialized=false;
  memset(&m_SockAddr, 0, sizeof(m_SockAddr));
  m_SockAddr.sin_family = AF_INET;
  m_SockAddr.sin_port = 3012;
  m_SockAddr.sin_addr.s_addr = htonl (INADDR_ANY);
  //memset(m_SockAddr.sin_zero, 0, 8);
  m_Open = false;
  m_ReadBuf = m_WriteBuf = NULL;
  m_ReadBufLen = m_WriteBufLen = 0; 

}



void SocketWrapper::init(std::string hostname,int port)
{
  if (m_Initialized)return;
  //  std::cerr<<"Socket init host: "<<hostname<<" port: "<<port<<"\n";
  // assume default blocksize
  memset(&m_SockAddr,0,sizeof(m_SockAddr));
  m_SockAddr.sin_port = htons(port);
  m_SockAddr.sin_family = AF_INET;
  theSocketMutex().lock();
  struct hostent *he = gethostbyname(hostname.c_str());
  if (he==NULL){
    theSocketMutex().unlock();
    return;
  }
  m_SockAddr.sin_addr = *((struct in_addr *)he->h_addr);
  theSocketMutex().unlock();
  // memset(m_SockAddr.sin_zero, 0, 8);
  m_Open = false;
  m_ReadBuf = m_WriteBuf = NULL;
  m_ReadBufLen = m_WriteBufLen = 0;
  m_Initialized=true;
}
int SocketWrapper::connectServer(int port)
{
  //std::cerr<<" SocketWraper connectServer: ";
  // create listen socket
  m_FD = socket(AF_INET, SOCK_STREAM, 0);
  if (m_FD==-1) return -1;
  
  int yes = 1;
  if (setsockopt(m_FD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))<0){
    close(m_FD);
    m_FD=-1;
    return -1;
  }
  memset(&m_SockAddr,0,sizeof(m_SockAddr));
  m_SockAddr.sin_port = htons(port);
  m_SockAddr.sin_family = AF_INET;
  if (bind(m_FD, (struct sockaddr *)&m_SockAddr, sizeof(struct sockaddr))<0){
     close(m_FD);
    m_FD=-1;
    return -1;    
  }
  if (listen(m_FD, 3)){
    close(m_FD);
      m_FD=-1;
      return -1;    
    }
  m_Open = true;
  return 0;
}
int SocketWrapper::connectClient(std::string hostname,int port)
{
  if ((m_FD = socket(AF_INET, SOCK_STREAM, 0))<0){
    close(m_FD);
    m_FD=-1;
    return -1;
  }
  if (m_FD==-1) return -1;
  init(hostname,port);
  if(connect(m_FD, (struct sockaddr *)&m_SockAddr, 
	     sizeof(struct sockaddr)) <0) {
    shutdown(m_FD,SHUT_RDWR);
    close(m_FD);
    m_FD=-1;
    CureCERR(100)<<"failure to connect\n"; 
    return -1;
  }
  m_Open = true;
  return 0;
}

SocketWrapper * SocketWrapper::acceptClient()
{
  // create listen socket
  SocketWrapper  *tmp=new SocketWrapper();
  int addrlen = sizeof(tmp->m_SockAddr);
  tmp->m_FD = accept(m_FD, (struct sockaddr *)&(tmp->m_SockAddr), 
		 (socklen_t *)&addrlen);
  if (tmp->m_FD<0){
    delete tmp;
    return NULL;
  }
  tmp->m_Open=true;
  return tmp;
}

int SocketWrapper::closeFD(void)
{
  allQueueFlush();
  if (m_Open){
    m_Open = false;
    if (m_FD>-1)
      close(m_FD); 
    m_FD=-1;
  }
  return 0;
}

int SocketWrapper::directRead(void)
{
  if (!m_Open)return -1;
  int amtRd;
  char *buf = (char *)malloc(m_Blocksize);
  amtRd = read(m_FD, buf, m_Blocksize);
  if (amtRd == -1){
    if ((errno == EWOULDBLOCK || errno == EAGAIN)) {
      errno = 0;
      free(buf);
      return 0;
    } else {
      errno = 0;
      free(buf);
      return -1;
    }
  } else if (amtRd == 0) {
    free(buf);
    closeFD();
    return 0;
  } else {
    m_ReadBuf = (char *)realloc((void *)m_ReadBuf, m_ReadBufLen + amtRd);
    memcpy(m_ReadBuf + m_ReadBufLen, buf, amtRd);
    m_ReadBufLen += amtRd;
    free(buf);
    return amtRd;
  }
}


int SocketWrapper::directWrite(void)
{
  if (!m_Open)return -1;
  int amtWr = 0;
  int mtw = m_WriteMaxLen == -1 ? m_WriteBufLen : 
    m_WriteMaxLen < m_WriteBufLen ? m_WriteMaxLen :
    m_WriteBufLen;
  if (mtw<1)return 0;
  amtWr = write(m_FD, m_WriteBuf, mtw);
  if (amtWr>0){
    memmove((void *)m_WriteBuf, (void *)(m_WriteBuf + amtWr), 
	    m_WriteBufLen - amtWr);
    m_WriteBuf = (char *)realloc((void *)m_WriteBuf, m_WriteBufLen - amtWr);
    m_WriteBufLen -= amtWr;
    if (m_WriteBufLen == 0)
      m_WriteBuf = NULL;  
    
  } else if(amtWr == -1){
    if (errno == EPIPE) {
      closeFD();
    } else if ((errno == EWOULDBLOCK || errno == EAGAIN)) {
      errno = 0;
      amtWr=0;
    } 
  }
  return amtWr;
}

char * SocketWrapper::readBytes(int nb)
{
  if (nb > m_ReadBufLen || nb < 1)
    return NULL;
  char *ptr = (char *)malloc(nb + 1);
  memcpy(ptr, m_ReadBuf, nb);
  memmove(m_ReadBuf, m_ReadBuf + nb, m_ReadBufLen - nb);
  m_ReadBufLen -= nb;
  if(m_ReadBufLen == 0) {
    free(m_ReadBuf);
    m_ReadBuf = NULL;
  } else {
    m_ReadBuf = (char *)realloc((void *)m_ReadBuf, m_ReadBufLen);
  }
  ptr[nb] = 0;
  return ptr;
}

int SocketWrapper::readLine(char **data, const char c)
{
  if (1 > m_ReadBufLen )
    return 0; 
  int nb=0;
  for (int i=0;i<m_ReadBufLen;i++)
    {
      if (m_ReadBuf[i]==c)
	{
	  nb=i+1;
	  break;
	}
    }
  if (nb <1)
    return 0;
  if (nb==1){
    memmove(m_ReadBuf, m_ReadBuf + nb, m_ReadBufLen - nb);
    m_ReadBufLen -= nb;
    return readLine(data,c);
  }
  *data=(char *)malloc(nb+1);
  memcpy(*data,m_ReadBuf,nb);
  //stick a null to make sure it terminates as a string for cerr<<(*data)
  (*data)[nb]=(unsigned char)0; 
  //  data.assign(m_ReadBuf,nb);
  memmove(m_ReadBuf, m_ReadBuf + nb, m_ReadBufLen - nb);
  m_ReadBufLen -= nb;
  if(m_ReadBufLen == 0) {
    free(m_ReadBuf);
    m_ReadBuf = NULL;
  } else {
    m_ReadBuf = (char *)realloc((void *)m_ReadBuf, m_ReadBufLen);
  }
  return nb;
}
int SocketWrapper::appendWriteQueue(const char *ptr, int len)
{
  if (ptr == NULL || len == 0)
    return -1;
  if (len == -1)
    len = strlen(ptr);
  m_WriteBuf = (char *)realloc((void *)m_WriteBuf, m_WriteBufLen + len);
  memcpy(m_WriteBuf + m_WriteBufLen, ptr, len);
  m_WriteBufLen += len;
  return 0;
}

int SocketWrapper::writeQueueFlush(void)
{
  if (m_WriteBufLen > 0) {
    m_WriteBufLen = 0;
    free(m_WriteBuf);
    m_WriteBuf = NULL;
  }
  return 0;
}

int SocketWrapper::readQueueFlush(void)
{
  if (m_ReadBufLen > 0) {
    m_ReadBufLen = 0;
    free(m_ReadBuf);
    m_ReadBuf = NULL;
  }
  return 0;
}

int SocketWrapper::allQueueFlush(void)
{
  return readQueueFlush() + writeQueueFlush();
}

