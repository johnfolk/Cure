//    $Id: ThreadSwitcher.cc,v 1.3 2007/09/16 09:04:42 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2005 John Folkesson
//    

#include "ThreadSwitcher.hh"
#include "CureDebug.hh"

using namespace Cure;

ThreadSwitcher::ThreadSwitcher(unsigned short threads[2])
  :DataFilter(1,1,false,threads[0])
{
  m_PullCalled=false;
  m_PushCalled=false;
  m_PushLocked=true;
  m_PullLocked=true;
  initPorts(threads);
}
ThreadSwitcher::ThreadSwitcher(unsigned short threadin, 
			       unsigned short threadout)
  :DataFilter(1,1,false,threadin)
{
  m_PullCalled=false;
  m_PushCalled=false;
  m_PushLocked=true;
  m_PullLocked=true;
  unsigned short threads[2];
  threads[0]=threadin;
  threads[1]=threadout;
  initPorts(threads);
}


ThreadSwitcher::~ThreadSwitcher()
{
}

bool ThreadSwitcher::setPushLocked()
{
  m_PushCalled=false;  
  if (out(0)->NumPushClients>0){
    bool r=true;
    for (short i=0;i<out(0)->NumPushClients;i++){
      bool r2=AddressBank::theAddressBank().
	isThreadSafe(out(0)->Thread,
		     out(0)->PushClients[i],
		     in(0));
      r=r&r2;
    }
    m_PushLocked=(!r);
  }else m_PushLocked=true;
  return m_PushLocked;
}
bool ThreadSwitcher::setPullLocked()
{
  m_PullCalled=false;  
  if (in(0)->IsPull){
    bool r=AddressBank::theAddressBank().isThreadSafe(in(0)->Thread,
					       in(0)->PullClient,
					       out(0));
    m_PullLocked=(!r);
  }else m_PullLocked=true;
  return m_PullLocked;
}
bool ThreadSwitcher::canPull(const int port)
{
  if (port!=0)return false;
  m_PullCalled=1;
  return true;
}

bool ThreadSwitcher::canPush(const int port)
{
  if (port!=1)return false;
  m_PushCalled=1;
  return true;
}
