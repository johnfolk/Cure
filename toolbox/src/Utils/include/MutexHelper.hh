//
// = LIBRARY
//
// = FILENAME
//    MutexHelper.hh
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

#ifndef CureMutexHelper_hh
#define CureMutexHelper_hh

#ifndef DEPEND
#include <pthread.h>
#endif

namespace Cure {

// Forward declaration(s)
class MutexWrapper;

/**
 * This class helps you to unlock the mutex by unlocking the mutex in
 * the destructor. It is typically helpful in functions where you lock
 * the mutex in on eplace and then have many different return
 * statements where there would otherwise have to be calls to unlock
 * to avoid causing deadlocks.
 *
 * NOTE that this class is intended as a helper class within a single
 * thread, therefore multiple calls to lock will not result in a
 * deadlock.
 *
 * int foo(int i) {
 *   MutexHelper helper(&barMutex);  // Will lock the barMutex for you
 *
 *   // no matter what return statement is used the helper object will
 *   // be detsroyed and with it the mutex will be unlocked
 *
 *   switch (bar) {
 *   case 0:
 *     return 1;
 *   case 1:
 *     return 2:
 *   }
 *  
 *  return 0;
 * }
 * 
 * @author Patric Jensfelt 
 * @see
 */
class MutexHelper {
public:
  /** 
   * Creates empty helper that does not do anything
   */
  MutexHelper();

  /**
   * Creates a helper class that first locks the mutex and then makes
   * sure that it is unlocked when this helper object is destroyed
   */
  MutexHelper(pthread_mutex_t *mutex);

  /**
   * Creates a helper class that first locks the mutex and then makes
   * sure that it is unlocked when this helper object is destroyed
   */
  MutexHelper(MutexWrapper &mutex);

  ~MutexHelper();

  /** 
   * Function to use in combination with default constructor to tell
   * what mutex to use. Will begin by locking the mutex
   */
  void setMutex(pthread_mutex_t *mutex);

  /** 
   * Function to use in combination with default constructor to tell
   * what mutex to use. Will begin by locking the mutex
   */
  void setMutex(MutexWrapper &mutex);

  /**
   * Remove pointer to the mutex and make the helper not do anything,
   * if there was a mutex registered already this mutex will be
   * unlocked first
   */
  void removeMutex();

  /**
   * Will lock the mutex if this class does not already know that ist
   * has been unlocked. 
   */
  void lock();

  /**
   * Unlocks the mutex
   */
  void unlock();

  /** 
   * Tris to lock the mutet
   *
   * @return true if mutex was successfully locked
   */
  bool tryLock();

protected:

  /** 
   * @return true if we have a reference to anything
   */
  bool doAnything() const;
private:

  /** Not 0 when constructor with MuetxWrapper is used */
  MutexWrapper *m_Wrapper;

  /** Not 0 when constructor with pthread_mutex_t is used */
  pthread_mutex_t *m_Mutex;

  /** True when the mutex has been locked by this helper class */
  bool m_Locked;

}; // class MutexHelper

inline bool
MutexHelper::doAnything() const {
  return ( (m_Mutex != 0) || (m_Wrapper != 0) );
}

} // namespace Cure

#endif // CureMutexHelper_hh
