//
// = LIBRARY
//
// = FILENAME
//    MutexWrapper.hh
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
//    Copyright (c) 2004 Patric Jensfelt
//
/*----------------------------------------------------------------------*/

#ifndef MutexWrapper_hh
#define MutexWrapper_hh

#ifndef DEPEND
#include <pthread.h>
#endif


namespace Cure {
/**
 * This is a helper class for a mutex. 
 * It simply gives C++ instance of the p_thread_mutex 
 *
 * @author Patric Jensfelt and Daniel Aarno and John Folkesson
 *
 */
class MutexWrapper {
public:
  /**
   * Constructor that creates a new mutex
   *
   */

  MutexWrapper(){
    pthread_mutex_init(&m_Mutex, NULL);
  }

  ~MutexWrapper(){
    pthread_mutex_destroy(&m_Mutex);
  }

  /**
   * Unlock the mutex 
   */
  void unlock(){
    pthread_mutex_unlock(&m_Mutex);
  }

  /**
   * Lock the mutex (blocking) 
   */
  void  lock(){
    pthread_mutex_lock(&m_Mutex);
  }

  /**
   * Tries to acquire the lock (non-blocking)
   *
   * @return true if the lock was acquired, false otherwise.
   */
  bool tryLock(){
    if(pthread_mutex_trylock(&m_Mutex) != 0)
      return false;   
    return true;
  }
  /**
   * Tries to acquire the lock (non-blocking)
   *
   * @return error code of phread_mutex_try_lock 
   */
  int mutex_trylock(){
    return pthread_mutex_trylock(&m_Mutex);
  }
  


public:
  pthread_mutex_t m_Mutex;
}; // class MutexWrapper

};  // namespcace Cure

#endif // MutexWrapper_hh
