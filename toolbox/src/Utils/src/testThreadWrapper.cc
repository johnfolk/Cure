//
// = LIBRARY
//
// = FILENAME
//    testThreadWrapper.cc
//
// = FUNCTION
//
// = AUTHOR(S)
//    Patric Jensfelt
//
// = DESCRIPTION
//    
// = COPYRIGHT
//    Copyright (c) 2004 Patric Jensfelt
//
/*----------------------------------------------------------------------*/

#include "ThreadWrapper.hh"

#ifndef DEPEND
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#endif

/**
 * Helper class to test ThreadWrapper
 *
 * @author Patric Jensfelt
 * @see
 */
class PrintThread : public Cure::ThreadWrapper {
protected:
  static long m_IdCntr;
public:
  PrintThread(long usecsToSleep) 
    :Cure::ThreadWrapper("PrintThread"),
     m_SleepTime(usecsToSleep),
     m_Id(m_IdCntr++)
  {}

protected:
  void eventLoop()
  {
    while(!stopEventloop_) {
      std::cerr << "Thread " << m_Id << " with pid " 
                << getpid() << " and tid "
                << pthread_self() << std::endl;
      usleep(m_SleepTime);
    }
  }
private:
  long m_SleepTime;
  int m_Id;
};

long PrintThread::m_IdCntr = 0;

bool run = true;
void sigtrap(int signo) { run = false; }

int main(int argc, char **argv)
{
  PrintThread t1(1000000);
  PrintThread t2(200000);

  t1.start();
  t2.start();

  while (run) usleep(100000);

  t1.stop();
  t2.stop();

  std::cerr << "Done!\n";
}
