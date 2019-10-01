//
// = LIBRARY
//
// = FILENAME
//    ThreadWrapper.cc
//
// = FUNCTION
//
// = AUTHOR(S)
//    Patric Jensfelt
//
// = DESCRIPTION
//    
// = BEGIN<COMMENT>
// = END<COMMENT>
//
// = SEE ALSO
//
// = EXAMPLE
// = BEGIN<CODE>
// = END<CODE>
//
// = COPYRIGHT
//    Copyright (c) 2003 Patric Jensfelt
//
/*----------------------------------------------------------------------*/

#include <ThreadWrapper.hh>

#include "HelpFunctions.hh"

#ifndef DEPEND
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#endif

namespace Cure {

ThreadWrapper::ThreadWrapper(const std::string &name)
  :name_(name)
{
  run_ = false;
  pid_ = 0;
}

ThreadWrapper::~ThreadWrapper()
{
  //std::cerr << "Destructing " << name_ << std::endl;
}

bool
ThreadWrapper::setup() 
{
  return true;
}

bool
ThreadWrapper::start () {
  run_ = true;
  stopEventloop_ = false;
  return (pthread_create(&tid_, NULL, create, this) == 0);
}

bool
ThreadWrapper::stop(double timeout) {
  if (isRunning()) {

    stopEventloop_ = true;

    if(timeout < 0) { //Blocking join
      if (pthread_join(tid_, NULL)) {
	return false;
      }
      return true;
    }

    if (timeout > 0) {
      // Try to wait for the thread to exit on its own

      // Wait until eventLoop has stopped (run_==false) or timeout
      // seconds has expired.
      double timeoutTime = HelpFunctions::getCurrentTime() + timeout;
      while (HelpFunctions::getCurrentTime() < timeoutTime && isRunning()) {
        usleep(10000);
      }
      if (isRunning()) {
        //DebugCERR(30) << "ThreadWrapper::stop() Cancel timed out for "
        //            << name_ << " (timeout=" << timeout << ")\n";
      }
    } else {
      //DebugCERR(30) << "ThreadWrapper::stop() Canceling " 
      //            << name_ << " immediately (timeout=" << timeout << ")\n";
    }

    if (run_) {
      //DebugCERR(30) << "ThreadWrapper::stop() Canceling " << name_ << "\n";
      if (pthread_cancel(tid_)) {
        //DebugCERR(20) << "ThreadWrapper::stop() Failed to cancel thread " 
        //            << name_ << std::endl;
      }
      run_ = false;
    }

    //DebugCERR(30) << "ThreadWrapper::stop() Joining " << name_ << std::endl;
    if (pthread_join(tid_, NULL)) {
      //DebugCERR(20) << "ThreadWrapper::stop() Failed to join thread " 
      //          << name_ << std::endl;
    }
    
    //DebugCERR(30) << "Done with stop in " << name_ << std::endl;
  } else {
    //DebugCERR(30) << "stop() " << name_ << " not running\n";
  }

  return true;
}

void* 
ThreadWrapper::create(void *ptr) {
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

  reinterpret_cast<ThreadWrapper*>(ptr)->pid_ = getpid();
  reinterpret_cast<ThreadWrapper*>(ptr)->setup();
  reinterpret_cast<ThreadWrapper*>(ptr)->eventLoop();

  return NULL;
}

}
