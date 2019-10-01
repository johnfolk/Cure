//
// = LIBRARY
//
// = FILENAME
//    ThreadWrapper.hh
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

#ifndef ThreadWrapper_hh
#define ThreadWrapper_hh

#ifndef DEPEND
#include <string>
#endif

namespace Cure {

/**
 * The term thread is short for thread of execution. It is a way of describing 
 * processes running in parallel within the same application.
 *
 * Threads are a powerful tool, but can also be tricky. They are
 * typically used to split a task into a number of subtasks that can
 * run nearly independently, with a main thread or primary thread of
 * execution waiting on their completion, then acting on the aggregate
 * of their results.
 *
 * The catch is the word nearly. It is often necessary to share
 * resources (data values, functions, TCP streams, etc.) between
 * different threads. Doing this without one thread interfering with
 * another can be tricky. Semaphores/Mutexes allow threads to synchronize
 * their use of shared resources.
 *
 * @see MutexLocker
 * @author Patric Jensfelt
 */
class ThreadWrapper {
public:
  
  ///Constructor
  ThreadWrapper(const std::string &name);

  /**
   * Destructor
   */
  virtual ~ThreadWrapper();

  /**
   * Start the thread.
   */
  virtual bool start();

  /**
   * Stop the thread (i.e. stop eventLoop) and join it with the
   * calling thread. You can specify a time that you allow for
   * eventLoop to stop on its own until it is canceled with brute
   * force (timeout = 0 gives immediate cancel). You need to set
   * run_=false in eventLoop to tell that it has finished.
   *
   * @param timeout time [s] to allow the thread to try to cancel in a
   * nice way until the thread is brutaly canceled. The run_ parameter
   * is checked to see if the thread has stopped. A negative value of
   * timeout is equal to a timeout of infinity 
   *
   * @see stopEventloop_
   */
  virtual bool stop(double timeout = 0);

  ///Blocking version of stop. This blocks the <b>calling</b> thread
  ///until the two threads join.
  bool join() { return stop(-1); }

  /**
   * @return true if the eventLoop() is running. NOTE that this only
   * works if eventLoop sets run_=false upon exiting.
   */
  bool isRunning() const { return run_; }

  /**
   * The name of the thread
   */
  const std::string& name() const { return name_; }

  /**
   * The process id of the thread
   */
  const pid_t pid() const { return pid_; }

  /**
   * The tread id of the thread that is started with start
   */
  const pthread_t tid() const { return tid_; }

protected:

  /**
   * Sets up things before starting the eventLoop
   */
  virtual bool setup();

  /**
   * The function where the thread lives. It typically contains a loop
   * like. If stopEventloop_ is set to true you should leave the loop and
   * to tell that it has stoppped you should set run_ to false. This
   * is necessary if you want to be able to do a "soft" stop, allow
   * the eventLoop to finish on its own. 
   *
   * while (!stopEventloop_) {
   *   // Do something!!!
   * }
   *
   * run_ = false;
   */
  virtual void eventLoop() = 0;

protected:
  /**
   * Function passed on to pthread_create with a pointer to the
   * object. This function will than call eventLoop().
   */
  static void* create(void *ptr);

protected:
  bool run_;

  /**
   * This variable is set to true if eventLoop should be stopped. The
   * idea is that this variable is monitored and the loop is stopped
   * whenever stopEventloop_ is set to true.
   */
  bool stopEventloop_;

  pid_t pid_;
  pthread_t tid_;
  std::string name_;
};

}; // namespace Cure

#endif // ThreadWrapper_hh
