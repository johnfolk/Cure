#ifndef SOCKETADDRSS_HH
#define SOCKETADDRSS_HH


#include <pthread.h>
#include "SocketWrapper.hh"
#include "DeviceAddress.hh"
#include "SmartData.hh"
#include "MutexWrapper.hh"


namespace Cure{

namespace Hal{

    /**
     * This class is an easy way to send data between two processes.
     * One can specify the Port and Hostname and whether it is a
     * server or client in a .ccf file.  Then create the Socket
     * address, configure it, and start the device.  It then can be
     * writen to from or have its data pushed to any address in the
     * cure system.  You of course should watch the threads.  So use a
     * ThreadMerger or a DataSlot with a separate thread to push to
     * unless this push is your only thread.  Writing to the
     * SocketAddress is thread safe.
     *
     * Here is all you need to do:
     * 
     * Server side:
     *
     * Cure::Hal::SocketAddress sock("socket");
     *
     * sock.m_Hostname="localhost";
     *
     * sock.m_Port=3245;
     *
     * sock.m_Server=true;
     *
     * sock.startDevice();
     *
     *
     * Client Side:
     *
     * Cure::Hal::SocketAddress sock2("socket2");
     *
     * sock2.m_Hostname="127.0.0.1"; 
     *
     * sock2.m_Port=3245;
     *
     * sock2.m_Server=false; 
     *
     * sock2.startDevice();
     *
     * Then to send data:
     *
     * Cure::Pose3D p;
     *
     * sock.write(p);
     * 
     * To get data in slot for example:
     *
     * Cure::DataSlotAddress slot;
     *
     * slot.setUseLock(true);
     *
     * sock2.push(slot);
     * 
     * Then to read the latest data from another thread:
     * 
     * slot.read(p);
     * 
     * Thats it.
     *
     * The data coming in to the socket is pushed out into cure by the
     * same thread that is reading the socket so if the cure side is
     * going to do some long calculation with this data some mechanism
     * (ie a ThreadMerger) needs to be inserted to allow the socket
     * thread to quickly return to its main job of watching the
     * socket.  Thus the socket can push to a ThreadMerger and the
     * ThreadMerger will queue up the data to be written to cure by a
     * separate thread.
     */
  class SocketAddress : public DeviceAddress
  {

  protected: 
    SocketWrapper **m_Clients;
    SocketWrapper m_Sock;
    unsigned short m_NumberClients;
    int m_SockPair[2];
    pthread_t m_Thread;
    bool m_Running;
    bool m_Stopped;
    
    SmartData m_TPoint;
    MutexWrapper m_WriteMutex;
    Cure::Timestamp m_LastTry;
  public:
    /**
     * If the address space is multithread (ie other threads might
     * interfere with a push call) then set this to true.  The default is 
     * false as the SocketAddress normally writes to a ThreadMerger which is
     * safe already and no lock is needed.
     */
    bool m_PushLocked;
    /**
     *  if <= 0 the SocketAddress will not retry to open connection if
     *  closed (unless m_Persistent is set).  if >0 a write will cause
     *  a attempt at reconnection to server after this period. 
     */
    Cure::Timestamp m_RetryPeriod;
    /**
     * If true the socket will try to reconnect on failure to connect
     * and on loss of connection (hang-up) even if no data is written
     * to the send queue.  If false the (re)connection will only be
     * attempted when there is data to send (ie on write to the
     * Cure::Address).  The persistent reconnect attempts will be at
     * least m_RetryPeriod between.  The minimum time between
     * persistent retrys is 1 second.
     */
    bool m_Persistent;

    /** 
     * This is '/r'=13 by default and is used to mark the ends of
     * messages.  The message protocol is simply a start character of
     * 't' or 'r' for Cure messages and and end character given here.
     * non-cure messages can use other start characters but they must 
     * use a common m"_DelimitCharacter.
     */  
    char m_DelimitCharacter;

    int m_Port;
    /** if !=0 messages are printed m_Verbose times, if <0 forever */
    int m_Verbose;
    std::string m_Hostname;
    /** Set to true to have this become a Server that accept clients*/
    bool m_Server;
    SocketAddress(const std::string &name,
		  const unsigned short thread=0);
    ~SocketAddress();
    unsigned short configDevice(const std::string &param);
    static void *tramp(void *a) { 
      ((SocketAddress *)a)->runEventLoop(); 
      return NULL; 
    }
    /**
     * The main loop of the thread.
     **/
    unsigned short runEventLoop();
    /**
     * Calling this connects the main thread and the socket depending on
     * m_Server as either a Server with a number of clients or a
     * Client that will try to connect to the server.
     **/
    virtual unsigned short startDevice();
    /**
     * Stops the event loop.
     **/
    virtual unsigned short stopDevice();
    /**
     * This will parse the string (with head='t') into m_TPoint.  All
     * read data passes through here and if the header is not
     * understood ('t' of 'r' so far) aternateParse is called and -1
     * returned so a subclass could handle other types of messages.
     * 
     * This method recieves the messages one 'line' at a time.  Cure
     * messages are ASCII with '\r' at the end of each message, this
     * '\r' is the defalut line termination character for
     * SocketWrapper::readLine.  If totally non-cure messages are
     * being sent that have a single message delimiter character as the
     * message delimiter then just set m_DelimitCharacter to that
     * character and overide this method to do what you want (copy and
     * change it in the subclass).  
     * 
     * 
     *
     * @return 0 if ok else
     * (RESOURCE_ERROR|ADDRESS_INVALID); or -1 if header is not 't'
     */
    virtual int  parse(char *chr, int len);
    /**
     * This is where a subclass can override to have other messages
     * read over the socket.  Just write a header byte other than 't' 
     * of 'r' which are being used for timestampedData parse.
     */

    virtual int  alternateParse(char *, int){return -1;}
    /**
     * This will queue the data to write over the socket.
     * This adds the 't' byte as a header and a '\r'=13 byte
     * as the message delimiter.
     * @param p the data to send over the Socket.
     * @return 0 if ok else RESOURCE_ERROR;
     */
    virtual unsigned short write(TimestampedData & p);
    /**
     * Puts a string of bytes in queue to be written to
     * Client's write queues, or (Server Queue.)
     * A subclass could use this to send an
     * arbitrary string over the socket.
     * @param the first byte to right. shoud be allocated
     *        with malloc as it will be free when eventually 
     *        appended to write queues 
     * @param len number of bytes to write
     * @return 0 if OK else RESOURCE_ERROR
     */
    virtual unsigned short write(void *pp, long len); 
  protected:
    /**
     * This makes the TCP connection.
     */
    virtual unsigned short startSocket();
  };
}
}
#endif /* __CFDCtl_h__ */
