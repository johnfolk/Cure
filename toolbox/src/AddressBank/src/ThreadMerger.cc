//    $Id: ThreadMerger.cc,v 1.17 2009/03/29 21:18:07 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2005 John Folkesson
//    

#include "ThreadMerger.hh"
#include "MutexHelper.hh"
#include "CureDebug.hh"

using namespace Cure;

ThreadMerger::ThreadMerger(short ins, const unsigned short thread, 
			   unsigned short *threads)
  :DataFilter(ins,ins,false,thread)
{
  MergerInit();
  initPorts(threads);
}


ThreadMerger::~ThreadMerger()
{
  ThreadMerger::disconnectAddresses();
}
void ThreadMerger::disconnectAddresses()
{
  // Stop the pusher thread if it is running
  lockMerger();
  if (m_PushThreadRunning) {
    m_PushThreadRunning = false;
    unlockMerger();
    pthread_mutex_lock(&m_SignalMutex);
    pthread_cond_signal(&m_SignalCond);
    pthread_mutex_unlock(&m_SignalMutex);
    std::cerr << "ThreadMerger::~ThreadMerger joining thread\n";
    pthread_join(m_Tid, NULL);
    std::cerr << "ThreadMerger::~ThreadMerger joined thread\n";
  }else unlockMerger();
  DataFilter::disconnectAddresses();
}
int ThreadMerger::config(const std::string &params)
{
    std::istringstream strdes(params);
    std::string cmd;
    if(!(strdes>>cmd))return 1;
    
    else if (cmd=="m_AllowPause"){
      bool b=false;
      if (strdes>>b)
	m_AllowPause=b;
    }else if (cmd=="m_PushLocked"){
      strdes>>m_PushLocked;
    }
    return 0;

}
void
ThreadMerger::MergerInit()
{ 
  m_PushLocked=true;
  m_QueueLength=0;
  m_Tid = 0;
  m_PushThreadRunning = false;
  m_AllowPause=false;
  pthread_mutex_init(&m_SignalMutex, NULL);
  pthread_cond_init(&m_SignalCond, NULL);
  
  unsigned short r = 0;  
  lockMerger();
  m_PushThreadRunning = true;
  if (pthread_create(&m_Tid, NULL, createPusherThread, this) != 0) {
    std::cerr << "\nERRROR: Failed to start pusher thread!!!!\n\n";
    r |= THREAD_ERROR;  
  }
  unlockMerger();
  
}

unsigned short ThreadMerger::write(Cure::TimestampedData& p,const int port)
{
  unsigned long r=0;
  lockMerger();
  m_QueueLength++;
  r=m_QueueLength;
  DataList.add(p,port);
  unlockMerger();
  // Signal to the pusher thread that it is time to read new data
  // and push to clients
  pthread_mutex_lock(&m_SignalMutex);
  pthread_cond_signal(&m_SignalCond);
  pthread_mutex_unlock(&m_SignalMutex);
  // Tell the write thread how long the queue is (It may adapt 
  // its data rate).
  m_QueueData.Time=p.Time;
  m_QueueData.setSubType(r&0xFFFF);
  m_QueueData.setID(p.getID());
  Ports[port].pushData(m_QueueData);  
  return 0;
}

void* 
ThreadMerger::createPusherThread(void *ptr)
{
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
  reinterpret_cast<ThreadMerger*>(ptr)->pusher();
  return NULL;
}

void ThreadMerger::pusher()
{
  

  // Time of last read data from the slot


  Timestamp lastReadTime(0.0);

  // Helps read through the slot if there are more than one entry for
  // a certain timestamp
  int port = 0;

  // Object to put result of the read into
  SmartData sd;
  int noDataAvailableYet=1;
  lockMerger();
  while (m_PushThreadRunning) {
    noDataAvailableYet=DataList.remove(0, sd,&port);  
    if (noDataAvailableYet==0)m_QueueLength--;
    unlockMerger();
    bool b=pause();
    // true if we should take a nap and wait for more data
    bool waitForNewData = true;


    // Check we we have got data at all first

   

    if (noDataAvailableYet!=0) {
      // Failed to read more data, probably no more data to read
      // we should just wait to be woken up
      waitForNewData = true;
    } else {
      //std::cerr<<"THREAD MEREGER Pushing "<<port<<"\n";;
      //     lockMerger();
      // m_QueueLength--;
      //unlockMerger();
      if (m_PushLocked)
	  pushDataLocked(sd.getTPointer(),port+Ins);
	else pushData(sd.getTPointer(),port+Ins);
	//std::cerr<<"THREAD MErger done pusshing "<<port<<"\n";
	// We read new data and there might be more so we should not
	// wait to be signaled until we read the next one
	waitForNewData = false;
      }
    if (b)resume();
    if (waitForNewData) {
      // There is no more data to read and we should wait until we are
      // signaled to read again or we get a timeout and check anyway
      // just to be sure
      Cure::Timestamp t(Cure::Timestamp::getCurrentTime());
      struct timespec ts;
      ts.tv_sec = t.Seconds;
      ts.tv_nsec = 1000 * (t.Microsec+200000);

      // This the standard construction when using condition
      // variables. We always use a condition variable together with a
      // mutex. We lock the mutex and call the wait funciton. When we
      // do this we automatically unlock the mutex and when signaled
      // the mutex is locked again. We then have to unlock it.
      CureDO(60) {
	t.print();
        std::cerr << " "<<FilterName<<"Going to sleep @ "
                  << Cure::Timestamp::getCurrentTime() << std::endl;
	
      }
      pthread_mutex_lock(&m_SignalMutex);
      pthread_cond_timedwait(&m_SignalCond, &m_SignalMutex, &ts);
      pthread_mutex_unlock(&m_SignalMutex);

      CureDO(60) {
        std::cerr << FilterName<<"Woken up  @ "
                  << Cure::Timestamp::getCurrentTime() << std::endl;
      }

      // When we got here we either got signaled through the condition
      // variable, through a normal signal or we timed out. In all
      // these cases we should go to the top of the loop and check
      // first if we should continue in the loop at all and if so if
      // we can read more data.
    }
    lockMerger();
  }
  unlockMerger();
}
