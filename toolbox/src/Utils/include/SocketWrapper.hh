#ifndef SOCKETWRAPPER_HH
#define SOCKETWRAPPER_HH


#include <string>
#include <arpa/inet.h>
#include "MutexWrapper.hh"
namespace Cure{
namespace Hal{

  /**
   * This class wraps a socket and makes it easier to use.
   * @see Cure::Hal::SocketAddress for an example of use.
   *
   */
  class SocketWrapper{
  public:
    struct sockaddr_in m_SockAddr;
    int m_FD;
    bool m_Open;
    char *m_ReadBuf;
    int m_ReadBufLen;
    char *m_WriteBuf;
    int m_WriteBufLen;
    int m_WriteMaxLen;
    int m_Blocksize;
  protected:
    bool m_Initialized;
  public:
    SocketWrapper();
    virtual ~SocketWrapper();
  protected:
    /**
     * This just clears the members.
     */
    void init();
    /**
     * This is called by connectClient to find the host address and 
     * set up the sockaddr_in structure.  It does nothing after its 
     * first success.
     *
     * @param hostname can be a name or IP address
     * @param port the port to use.
     */
    void init(std::string hostname,int port);
  public: 
    /*
     * @return 0 if ok else -1;
     */
    int connectServer(int port);
    /*
     * @return 0 if ok else -1;
     */
    int connectClient(std::string hostname,int port);
    /*
     * @return new client if ok else NULL;
     */
    SocketWrapper * acceptClient();
    virtual int closeFD();
    /**
     * @return -1 on error else number of bytes read
     */
    int directRead(void);

    /**
     * @return -1 on error else number of bytes writen
     */
    int directWrite(void);
    void setPortMaxWriteLen(int len){m_WriteMaxLen = len;}
    char * readBytes(int nb);
    int readLine(char **data,const char c=13);
    int appendWriteQueue(const char *ptr, int len = -1);
    int writeQueueFlush(void);
    int readQueueFlush(void);
    int allQueueFlush(void);
  /**
   * This strange thing is just because of a warning note on gethostbyname 
   * that indicates it's return struct could be overwritten by another
   * call to gethostbyname in anothe r thread.  So with this mutex 
   * at least that wont happen by another SocketWrapper.
   */
    static MutexWrapper& theSocketMutex(){
    static MutexWrapper s_TheSocketMutex;
    return s_TheSocketMutex;
  }

  };

}
}
#endif /* __CFDCtl_h__ */
