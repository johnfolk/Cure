//    $Id: DataSlotAddress.cc,v 1.17 2008/05/05 16:55:27 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#include "DataSlotAddress.hh"
#include "MutexHelper.hh"
#include "CureDebug.hh"


using namespace Cure;

int DataSlotAddress::accumulatePose(Cure::Timestamp &beforeTime,
				    Cure::Timestamp &nextTime, Pose3D &inc)
{
  Timestamp t=nextTime;
  int interp=0;
  Pose3D p,pbefore, pnext;
  inc=p;
  if(read(pnext,&interp,t))return 1;
  t=beforeTime; 
  if(read(pbefore,&interp,t))return 1;
  inc.setSubType(pbefore.getSubType());
  interp=1;
  if (read(p,&interp,t))
    {
	inc.minusPlus_(pbefore,pnext);
	inc.Covariance=pnext.Covariance;
	inc.Covariance-=pbefore.Covariance;
	inc.Time=nextTime;
	Matrix lambda;
	inc.Covariance.symmetricEigen(lambda);
	for (int i=0;i<lambda.Rows;i++)
	  if (lambda(i,i)<=0)inc.Covariance+=lambda(i,i)+1E-12;
	  else if (lambda(i,i)<1E-12)inc.Covariance+=1E-12;
	return 0;
    }
  t=p.Time;
  interp=-1;
  if (read(p,&interp,t))return 1;
  if (p.Time>beforeTime){
    if (p.Time>=nextTime){
      inc.minusPlus_(pbefore,pnext);
      inc.Covariance=pnext.Covariance;
      inc.Covariance-=pbefore.Covariance;
      inc.Time=nextTime;
      Matrix lambda;
      inc.Covariance.symmetricEigen(lambda);
      for (int i=0;i<lambda.Rows;i++)
	  if (lambda(i,i)<=0)inc.Covariance+=lambda(i,i)+1E-12;
	  else if (lambda(i,i)<1E-12)inc.Covariance+=1E-12;
      return 0;
    }
    inc.minusPlus_(pbefore,p);
    inc.Covariance=p.Covariance;
    inc.Covariance-=pbefore.Covariance;
    Matrix lambda;
    inc.Covariance.symmetricEigen(lambda);
    for (int i=0;i<lambda.Rows;i++)
      if (lambda(i,i)<=0)inc.Covariance+=lambda(i,i)+1E-12;
      else if (lambda(i,i)<1E-12)inc.Covariance+=1E-12;
  }    
  t=p.Time;
  interp=1;
  Pose3D cum;
  cum.setSubType(inc.getSubType());
  while(!(read(p,&interp,t)))
    if (p.Time<nextTime){
      t=p.Time;    
      cum.add_(inc,p);
      inc=cum;      
    }else if (p.Time==nextTime){
      cum.add_(inc,p);
      inc=cum;
      inc.Time=nextTime;
      return 0;
    }else{
      cum.add_(inc,pnext);
      inc=cum;
      inc.Time=nextTime;
      return 0;
    }
  return 1;
}

DataSlotAddress::DataSlotAddress(const std::string name, 
				 const unsigned short thread):Address(thread)
{
  setName(name);
  slotInit();
}

DataSlotAddress::DataSlotAddress( short depth, 
				  unsigned char classType,
				  unsigned short subType,
				  bool compressed, 
				  bool dotypecheck,
				  const unsigned short thread):
  Address(thread)
{
  Slot.setup(depth, classType, subType, compressed,dotypecheck);
  slotInit();
}

DataSlotAddress::~DataSlotAddress()
{
  disconnectAddress();
}
void
DataSlotAddress::disconnectAddress(){
  
  // Stop the pusher thread if it is running
  if (m_PushThreadRunning) {
    m_PushThreadRunning = false;
    pthread_join(m_Tid, NULL);
    UseLock = false;
    UseThread = false;
  }
  Address::disconnectAddress();
  if (NumPushClientsAlloc) {
    NumPushClientsAlloc = 0;
    delete [] PushClients;
    PushClients = 0;
    NumPushClients = 0;
  }
}

void
DataSlotAddress::slotInit()
{ 
  //AddressBank::theAddressBank().add(this); 
  UseLock = false;
  UseThread = false;

  PushClients = 0;
  NumPushClientsAlloc = 0;
  NumPushClients = 0;

  m_Tid = 0;
  m_PushThreadRunning = false;

  pthread_mutex_init(&m_SignalMutex, NULL);
  pthread_cond_init(&m_SignalCond, NULL);
  m_Descriptor=Slot.Descriptor;
}

void 
DataSlotAddress::setUseLock(bool useit) {
  if (UseThread && !useit) {
    std::cerr << "Slot " << slotName() << " needs lock if you use thread\n";
    return;
  }

  UseLock = useit;
}

unsigned short
DataSlotAddress::setUseThread(bool useit) {
  unsigned short r = 0;

  setUseLock(true);

  if (useit) {
    if (UseThread) {
      std::cerr << "DataSlotAddress::setUseThread already using thread\n";
    } else {
      lockSlot();
      m_PushThreadRunning = true;
      if (pthread_create(&m_Tid, NULL, createPusherThread, this) != 0) {
        std::cerr << "\nERRROR: Failed to start pusher thread!!!!\n\n";
        r |= THREAD_ERROR;
      }
      unlockSlot();
    }
  } else {
    if (UseThread) {
      lockSlot();
      m_PushThreadRunning = false;
      unlockSlot();
      std::cerr << "Joining DataSlotAddress::pusher\n";
      pthread_join(m_Tid,NULL);
      std::cerr << "Joined DataSlotAddress::pusher\n";
    } else {
      std::cerr << "DataSlotAddress::setUseThread already not using thread\n";
    }
  }
  UseThread = useit;

  return r;
}

unsigned short DataSlotAddress::write(TimestampedData & p)
{
  lockSlot();
  unsigned short r = Slot.write(p);
  unlockSlot();


  if (UseThread) {    
    lockQueue();
    m_Queue.add(p);
    unlockQueue();
    
    // Signal to the pusher thread that it is time to read new data
    // and push to clients
    pthread_mutex_lock(&m_SignalMutex);
    pthread_cond_signal(&m_SignalCond);
    pthread_mutex_unlock(&m_SignalMutex);
  } else {
    pushData(&p);
  } 
  return r;
}

void* 
DataSlotAddress::createPusherThread(void *ptr)
{
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

  reinterpret_cast<DataSlotAddress*>(ptr)->pusher();

  return NULL;
}

void DataSlotAddress::pusher()
{
  // Time of last read data from the slot
  Timestamp lastReadTime(0.0);

  // Helps read through the slot if there are more than one entry for
  // a certain timestamp
  int index = 0;

  // Object to put result of the read into
  SmartData sd;

  bool noDataAvailableYet = true;

  while (m_PushThreadRunning) {

    // Index for the last read
    int prevIndex = index;

    // true if we should take a nap and wait for more data
    bool waitForNewData = true;

    // If there was no data last time it is quite likely that there is
    // still no data and thus we oinly chekc for that first instead of
    // trying to read
    if (noDataAvailableYet) {
      // Check we we have got data at all first
      lockQueue();
      noDataAvailableYet = (m_Queue.getLength() == 0);
      unlockQueue();
    }

    if (!noDataAvailableYet) {
      // The read function does its own locking!
      unsigned short r;
      lockQueue();
      r=m_Queue.get(0,sd);
      r = (r|m_Queue.remove(0));
      unlockQueue();
      if (r){
        // Failed to read more data, probably no more data to read
        // we should just wait to be woken up
        waitForNewData = true;
        noDataAvailableYet = ((r & NO_DATA_AVAILABLE) > 0);
      } else {
        if (index != prevIndex ||
            lastReadTime < sd.getTime() ||
            index == 0) {
          if (index == 0) index = 1;
          lastReadTime = sd.getTime();
	  
          // Push the data to the clients
	  pushDataLocked(sd.getTPointer());
          
          // We read new data and there might be more so we should not
          // wait to be signaled until we read the next one
          waitForNewData = false;
	  
        } else {
          std::cerr << "DataSlotAddress::pusher Logic error\n";
          waitForNewData = true;
        }
      }
    }

    if (waitForNewData) {
      // There is no more data to read and we should wait until we are
      // signaled to read again or we get a timeout and check anyway
      // just to be sure
      Cure::Timestamp t(Cure::Timestamp::getCurrentTime());
      t += 200000;  // Sleep 200.000us at most waiting for new data
      struct timespec ts;
      ts.tv_sec = t.Seconds;
      ts.tv_nsec = 1000 * t.Microsec;

      // This the standard construction when using condition
      // variables. We always use a condition variable together with a
      // mutex. We lock the mutex and call the wait funciton. When we
      // do this we automatically unlock the mutex and when signaled
      // the mutex is locked again. We then have to unlock it.
      CureDO(60) {
        std::cerr << "Going to sleep @ "
                  << Cure::Timestamp::getCurrentTime() << std::endl;
      }
      pthread_mutex_lock(&m_SignalMutex);
      pthread_cond_timedwait(&m_SignalCond, &m_SignalMutex, &ts);
      pthread_mutex_unlock(&m_SignalMutex);

      CureDO(60) {
        std::cerr << "Woken up  @ "
                  << Cure::Timestamp::getCurrentTime() << std::endl;
      }

      // When we got here we either got signaled through the condition
      // variable, through a normal signal or we timed out. In all
      // these cases we should go to the top of the loop and check
      // first if we should continue in the loop at all and if so if
      // we can read more data.
    }
  }
}

unsigned short
DataSlotAddress::pushData(TimestampedData *tp)
{
  unsigned short r = 0;
  if (UseLock) ClientListMutex.lock();
  short n = NumPushClients;
  unsigned long clients[n>0?n:1]; // making sure always got alloc at least 1
  memcpy(clients, PushClients, n * sizeof(unsigned long));
  if (UseLock) ClientListMutex.unlock();
  if (n > 0) {
    for (short i = 0; i < n; i++) {
      if (writeTo(clients[i], *tp) == ADDRESS_INVALID) {
	std::cerr << "WARNING: DataSlotAddress::pusher for "
		  << slotName() << ", failed to write to address with index "
		  << clients[i] << ", disconnecting!!!\n";
	disconnect(clients[i]);
      }
    }
  }
  return r;
}
unsigned short
DataSlotAddress::pushDataLocked(TimestampedData *tp)
{
  unsigned short r = 0;
  if (UseLock) ClientListMutex.lock();
  short n = NumPushClients;
  unsigned long clients[n>0?n:1]; // making sure always got alloc at least 1
  memcpy(clients, PushClients, n * sizeof(unsigned long));
  if (UseLock) ClientListMutex.unlock();
  if (n > 0) {
    for (short i = 0; i < n; i++) {
      if (writeToLocked(clients[i], *tp) == ADDRESS_INVALID) {
	std::cerr << "WARNING: DataSlotAddress::pusher for "
		  << slotName() << ", failed to write to address with index "
		  << clients[i] << ", disconnecting!!!\n";
	disconnect(clients[i]);
      }
    }
  }
  return r;
}

void DataSlotAddress::disconnect(const unsigned long index)
{
  // This object helps to make sure that the mutex is unlocked before
  // leaving this function
  MutexHelper guard;
  if (UseLock) guard.setMutex(ClientListMutex);

  for (short i = 0; i < NumPushClients; i++) {
    if (index == PushClients[i]) {
      for(short j = i + 1; j< NumPushClients; j++) {
	PushClients[j-1] = PushClients[j];
      }
      NumPushClients--;
      return;
    }
  }
  std::cerr << "DataSlotAddress::disconnect, did not find index "
            << index << " for " << slotName() << std::endl;
  return;
}

unsigned short DataSlotAddress::push(Address * pa)
{
  if (pa == 0) {
    std::cerr << "WARNING: DataSlotAddress::push cannot push to Address "
              << "with pointer 0\n";
    return 1;
  }

  // This object helps to make sure that the mutex is unlocked before
  // leaving this function
  MutexHelper guard;
  if (UseLock) guard.setMutex(ClientListMutex);


  CureCERR(40) << "DataSlotAddress \"" << slotName() << "\" pushing to client "
               << "with adress index " << pa->Index 
               << std::endl;

  if (NumPushClients == NumPushClientsAlloc) {
    // Need to allocate more space for more push connections

    unsigned long *temp = new unsigned long[NumPushClientsAlloc + 1];
    for (short i = 0; i < NumPushClientsAlloc; i++) temp[i] = PushClients[i];
    temp[NumPushClientsAlloc] = pa->Index;
    if (NumPushClientsAlloc) delete [] PushClients;
    PushClients = temp;
    NumPushClientsAlloc++;
    NumPushClients++;

    return 0;
  }

  // If we got here there is already allocated space for more connections
  PushClients[NumPushClients] = pa->Index;
  NumPushClients++;

  return 0;
}
