//
// = LIBRARY
//
// = FILENAME
//    MutexHelper.cc
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

#include "MutexHelper.hh"
#include "MutexWrapper.hh"

#ifndef DEPEND
#endif

namespace Cure {

MutexHelper::MutexHelper()
  :m_Wrapper(0),
   m_Mutex(0),
   m_Locked(false)
{}

MutexHelper::MutexHelper(MutexWrapper &wrapper)
  :m_Wrapper(&wrapper),
   m_Mutex(0),
   m_Locked(false)
{
  lock();
}

MutexHelper::MutexHelper(pthread_mutex_t *mutex)
  :m_Wrapper(0),
   m_Mutex(mutex),
   m_Locked(false)
{
  lock();
}

MutexHelper::~MutexHelper()
{
  unlock();
}

void
MutexHelper::setMutex(pthread_mutex_t *mutex)
{
  if (doAnything()) {
    unlock();
    m_Wrapper = 0;
  }

  m_Mutex = mutex;
}

void
MutexHelper::setMutex(MutexWrapper &mutex)
{
  if (doAnything()) {
    unlock();
    m_Mutex = 0;
  }

  m_Wrapper = &mutex;
}

void
MutexHelper::removeMutex()
{
  if (doAnything()) {
    unlock();
    m_Mutex = 0;
    m_Wrapper = 0;
  }
}

void 
MutexHelper::lock()
{
  if (!doAnything()) return;

  if (m_Locked) return;

  if (m_Wrapper) m_Wrapper->lock();
  else if (m_Mutex) pthread_mutex_lock(m_Mutex);

  m_Locked = true;
}

void 
MutexHelper::unlock()
{
  if (!doAnything()) return;

  if (!m_Locked) return;

  if (m_Wrapper) m_Wrapper->unlock();
  else if (m_Mutex) pthread_mutex_unlock(m_Mutex);

  m_Locked = false;
}

bool 
MutexHelper::tryLock()
{
  if (m_Locked) return true;

  if (m_Wrapper) m_Locked = m_Wrapper->tryLock();
  else if (m_Mutex) m_Locked = (pthread_mutex_unlock(m_Mutex) == 0);

  return m_Locked;
}

} // namespace Cure
