//    $Id: AddressBank.hh,v 1.19 2008/05/05 16:55:26 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_ADDRESSBANK_HH
#define CURE_ADDRESSBANK_HH


#include "TimestampedData.hh"
#include "Address.hh"
#include "AddressList.hh"
#include "MutexWrapper.hh"

#include <pthread.h>

namespace Cure{


class AddressBankThread
{
public:
  AddressList *Addresses;
  AddressList Services;
  short Size;
  unsigned long Mask;
  unsigned long NextIndex;
  bool *Permanent;
   MutexWrapper ThreadMutex;
 public:
  /**
   * @param depth the Bank will set up a hash table for the Addresses 
   * of Size=2^(depth), so chose Size for about the number of 
   * Adresses stored at any given time.  It works for any depth
   * but intelligent choices make it more efficient.  
   */
  AddressBankThread(short depth=7);
  ~AddressBankThread();
  /** copy's an exsiting bank but should only be used when this is empty*/
  void operator=(AddressBankThread &a);
  /**
   * This will try to put pa where trying to access pa while other
   * threads are deleting other addresses in this space is will work.
   * Other threads can already read/write/add... with no problems.
   * @param pa the address to protect
   * @return true if success false if there are more permanent address
   *         than Size=2^depth.
   */
  bool makePermanent(Address *pa);
  /**
   * This will resize the hash table.  All permanent addresses
   * will need to try and make permanent again after calling this.
   * Which will change their index which means others holding that index
   * won't find them...  Hey just call this before you start making permanent.
   * @param depth Size=2^(depth)
   */
  void resizeHash(short depth); 
  /**
   * This will resize the hash table to the optimal size.
   * @see resizeHash
   */
  void optimizeHash();
  /**
   * Call at start of process to make multithreading safe if you care.
   *
   * param p true for multithreading false to turn off protection.
   */
  /**
   * This calls write on the Address.
   *
   * @param index  the index in the Bank
   * @param p TimestampdeData to be copied
   * @return 0 if ok, else ADDRESS_INVALID TIMESTAMP_ERROR etc.
   */
  unsigned short write(const unsigned long index,  TimestampedData& p);

  /**
   * This calls read on the Address.
   *
   * @param index  the index in the Bank
   * @param result TimestampdeData to be copied into
   * @param t requested Data will be interpreted based on interpolate 
   * @param interpolate flag for requesting interpolation to t.
   * @return 0 if ok, else ADDRESS_INVALID or TIMESTAMP_ERROR....
   */
   unsigned short read(const unsigned long index, TimestampedData& result, 
		       const Timestamp t=0,
		       const int interpolate=0);
  
  bool isThreadSafe(const unsigned long index,Address * pa);
  /*
   * Add the Address to the Bank and set its index.
   * 
   * 
   * @param pa The Address to add
   * 
   */

  void  add(Address *pa);
  /*
   * Add the Address to the Bank and set its index.
   * 
   * otherwise returnsADDRESS_INVALID.
   * 
   * @param pa The Address to add
   * @return 0 if ok, else POSE_ADDRESS_INVALID.
   */

  void addService(Address *pa);

  /**
   * Removes Address 'pa' and sets its index to 0.
   * @param pa The Address to remove
   */
  void removeService(Address *pa);

  /**
   * gets a named ServiceAddress's index and .
   * @param service The name of the service
   * @param index the index is returned here
   * @return true if found else false
   */
   bool getService(const std::string &service, unsigned long &index);

 
  
  /**
   * Removes Address 'pa' and sets its index to 0.
   * @param pa The Address to remove
   */
  void remove(Address *pa);

  
  /**
   * Permanently subscribe for push data to pa when the Address 
   * of index hase new data.  
   *
   * pa will have Write called on it after writing.
   * These are persistant.  (ie. they remain on the list after being called)
   * They are stoped by disconnect.
   * If they return ADDRESS_INVALID they are normally stoped.
   *
   * @param pa the Address to be pushed to.
   * @param index of the Address doing the pushing.
   * @return 0 if ok or ADDRESS_INVALID.
   */
  unsigned short push(Address * pa, const unsigned long index);  
  /**
   * Have the Address of index will read data from pa whenever 
   * it needs input.
   * This is permanent.
   *
   * pa will have read called on it when input is needed.
   * These are persistant.  (ie. they remain on the list after being called)
   * They are stoped by disconnect.
   * If they return ADDRESS_INVALID they are normally stoped.
   *
   *
   * @param pa the Address to be read from.
   * @param index of the Address that will use the input.
   * @return 0 if ok or ADDRESS_INVALID.
   */
  unsigned short pull(Address * pa, const unsigned long index);  
  
  /*
   * Removes Address 'pa' from the push/pull list.
   * @param pa the Address to be removed.
   * @param index the index of the Address to disconnect from.
   * @return 0 if ok or ADDRESS_INVALID.
   */
  unsigned short disconnect(Address * pa, const unsigned long index);
  /*
   * Returns status of the Address.
   * @return 0 if ok, else ADDRESS_INVALID. 
   */    
  unsigned short query(const unsigned long index);

public:

 void lockThread()
  {
    ThreadMutex.lock();
  }
 int tryThread()
  {
    return ThreadMutex.mutex_trylock();
  }
  
  void unlockThread()
  {
    ThreadMutex.unlock();
  }
  
};



/**
 * The AddressBank is a
 * keeper of Address pointers.  
 * Address's are stored here which can have 
 * read/write/push/pull/disconnect called on them.
 * The main purpose of AddressBank is to return ADDRESS_INVALID
 * when the call is made to a deleted object. 
 * That then eventually leads to the Address being disconnected. 
 * Thus the objects pointed to can be safely removed and deleted without 
 * having to track down all pointers.
 * Thus, Addresses can be both added and removed.
 *
 *
 * Addresses need to register with the Bank (add) and then remove
 * themselves when deleted. That normally happens without the user or
 * programer being aware of it.
 *
 * The default AddressBank is a static global object gotten by 
 * calling AddressBank::theAddressBank(...).  The first such
 * call can initialize the object to number of threads and their depths.
 * 
 * 
 *
 * For multiple threads using the bank one can to call
 * theAddressBank(numthreads, depths);  at the start of the process.
 * This will set up separate address spaces for each thread.  The addresses
 * themselves should then be created with the proper thread in the constructor.
 * The address will then only read and write automatically inside their own
 * thread space.  That space is protected from the other threads by a mutex.
 * so a thread writing from outside the space could call writeLocked(..)
 * that is a blocking call.  Normally one would start a thread to call 
 * write locked using a thread merger for example.  If one is working in one 
 * thread one need not do anything.
 * 
 *
 *
 * @author John Folkesson
 * @see  Address .
 * @see  DataFilter 
 * @see  DataSlot 
 * @see  FileAddress 
 */
class AddressBank
{
public:
private:
  AddressBankThread *Thread;
protected: 
  unsigned short NumberOfThreads;
public:
  /**
   *
   * @param numthreads the number of address thread spaces to set up.
   * @param depths the Bank will set up  hashs tables for the Addresses 
   * of Size=2^(depth), so chose Size for about the number of 
   * Adresses stored at any given time.  It works for any depth
   * but intelligent choices make it more efficient.  
   * Give an array of length numthreads her with vaules in range 1 to not to
   * big 7 is default.
   */
  AddressBank(unsigned short numthreads=1, unsigned short *depths=0){
    Thread=0;
    NumberOfThreads=0;
    std::cout
      <<"****************************************************************\n"
      <<"*|                                                            |*\n"
      <<"*|              Better Robotics Programming by                |*\n"
      <<"*|                           CURE                             |*\n"
      <<"*|                                                            |*\n"
      <<"================================================================\n"
      <<"****************************************************************\n";
    makeThreads(numthreads,depths);
  }
  ~AddressBank(){
    std::cerr<<"AddressBank deleting:\nIf I hang its because someone is not"
	     <<"releasing a thread to me.\n";
    for (unsigned short  i=0; i<NumberOfThreads;i++)
      getThread(i);
    if (Thread)delete[]Thread;
    std::cerr<<"AddressBank Deleted\n";
    std::cout
      <<"****************************************************************\n"
      <<"*|                                                            |*\n"
      <<"*|                         Good bye                           |*\n"
      <<"*|                           CURE                             |*\n"
      <<"*|                                                            |*\n"
      <<"================================================================\n"
      <<"****************************************************************\n";
  }

 /** 
   * This resizes the Hash on thread 0.
   * Permanent addresses  
   * will need to try and make permanent again after calling this.
   * Which will change their index which means others holding that
   * index won't find them...  Hey just call this before you start
   * making permanent.
   *
   * @param depth the Bank will set up a hash table for the Addresses 
   * of Size=2^(depth), so chose Size for about the number of 
   * Adresses stored at any given time.  It works for any depth
   * but intelligent choices make it more efficient.  
   * Give  a vaule in range 1 to not to
   * big, 7 is default.
   */
 void resizeHash(short depth){
    Thread[0].resizeHash(depth);
  }
  /** 
   *
   * All permanent addresses
   * will need to try and make permanent again after calling this.
   * Which will change their index which means others holding that index
   * won't find them...  Hey just call this before you start making permanent.
   * @param depth the Bank will set up a hash table for the Addresses 
   * of Size=2^(depth), so chose Size for about the number of 
   * Adresses stored at any given time.  It works for any depth
   * but intelligent choices make it more efficient.  
   * Give  a vaule in range 1 to not to
   * big, 7 is default.
   */
  void resizeHash(unsigned short thread,short depth){
    if (thread>=NumberOfThreads)thread=0;
    Thread[thread].resizeHash(depth);
  }
  /**
   *  All permanent addresses
   * will need to try and make permanent again after calling this.
   * Which will change their index which means others holding that index
   * won't find them...  Hey just call this before you start making permanent.
   * @param depth the Bank will set up a hash table for the Addresses 
   * of Size=2^(depth), so chose Size for about the number of 
   * Adresses stored at any given time.  It works for any depth
   * but intelligent choices make it more efficient.  
   * Give  a vaule in range 1 to not to
   * big, 7 is default.
   */
  void resizeHashLocked(unsigned short thread,short depth){
    if (thread>=NumberOfThreads)thread=0;
    getThread(thread);
    Thread[thread].resizeHash(depth);
    releaseThread(thread);
  }
  /**
   *  All permanent addresses
   * will need to try and make permanent again after calling this.
   * Which will change their index which means others holding that index
   * won't find them...  Hey just call this before you start making permanent.
   * This will resize the hash table to the optimal size.
   */
  void optimizeHash(){Thread[0].optimizeHash();}
  void optimizeHash(unsigned short thread){Thread[thread].optimizeHash();}
  
  /**
   * This locks an Address thread space
   * @param thread the thread space index to lock
   */
  void getThread(const unsigned short thread){
    if (NumberOfThreads>thread)
      Thread[thread].lockThread();
    else std::cerr<<"ERROR AddressBank::getThread Thread "<<thread
		  <<" requested in a Bank of size "<<NumberOfThreads<<"\n";
  }
  /**
   * This trys locks an Address thread space returns imeadiatly.
   * one must check if reurn is 0 then the releaseThread 
   * must be called eventually otherwise relasethread should not be called.
   * @param thread the thread space index to lock
   * @return error code of phread_mutex_try_lock; 0 if sucessfully locked 
   */
  int  tryThread(const unsigned short thread){
    return Thread[thread].tryThread();
  }
  /**
   * This unlocks an Address thread space
   * @param thread the thread space index to unlock
   */
  void releaseThread(const unsigned short thread){
    Thread[thread].unlockThread();
  }
  /**
   * Gets the NumberOfThreads.
   * @return NumberOfThreads;
   */
  unsigned short getNumberOfThreads(){return NumberOfThreads;}
  void makeThreads(unsigned short numthreads=1, unsigned short *depths=0){
    if (Thread)return;
    if (numthreads<1)numthreads=1;
    Thread=new AddressBankThread[numthreads];
    if (depths){
      for (unsigned short  i=0; i<numthreads;i++){
	Thread[i].resizeHash(depths[i]);
      }
    }    
    NumberOfThreads=numthreads;
  }

  unsigned short write(const unsigned long index,  TimestampedData& p){
    return Thread[0].write(index,p);}
  unsigned short write(const unsigned short thread,
		       const unsigned long index,  
		       TimestampedData& p) {
    return Thread[thread].write(index,p);  }
  unsigned short writeLocked(const unsigned short thread,
			       const unsigned long index,  
			       TimestampedData& p) {
    getThread(thread);
    unsigned short r=Thread[thread].write(index,p);  
    releaseThread(thread);
    return r;
  }
  /**
   * This calls read on the AddressBankThread.
   *
   */
   unsigned short read(const unsigned long index, 
		       TimestampedData& result, 
		       const Timestamp t=0,
		       const int interpolate=0){
     return Thread[0].read(index,result,t,interpolate);
   }
  unsigned short  read(const unsigned short thread, 
		       const unsigned long index, 
		       TimestampedData& result, 
		       const Timestamp t=0,
		       const int interpolate=0){
    return Thread[thread].read(index,result,t,interpolate);
  }
  unsigned short  readLocked(const unsigned short thread, 
		       const unsigned long index, TimestampedData& result, 
		       const Timestamp t=0,
		       const int interpolate=0){
    getThread(thread);
    unsigned short r=Thread[thread].read(index,result,t,interpolate);
    releaseThread(thread);
    return r;
  }
  /**
   *
   * @return true if  Address(Thread,index) permits read/write from
   *  other threads.
   */
  bool isThreadSafe(const unsigned short thread, 
			       const unsigned long index,Address * pa);
  
  /*
   * Add the Address to the Bank and set its index.
   * 
   * 
   * @param pa The Address to add
   * @param index A suggestion for the index to assign pa.
   * @return 0 if ok, elseADDRESS_INVALID.
   */
  void add(Address *pa){
    add(pa->Thread,pa);
  }
  /*
   * Add the Address to the Bank and set its index.
   * 
   * @see AddressBankThread::makePermanent(Address *pa)
   * @param pa The Address to make permanent
    * @return true if ok, else false.
   */
  bool makePermanent(Address *pa){
    if (pa->Thread>=NumberOfThreads){
      std::cerr<<"ERROR Addressbanks::makePermanent Thread above number of Threads\n";
      pa->Thread=0;
      pa->Index=0;
    }
    return Thread[pa->Thread].makePermanent(pa);
  }
  void  add(const unsigned short thread, 
		      Address *pa){
    remove(pa);
    pa->Thread=thread;
    if (thread>=NumberOfThreads){
      pa->Thread=0;
      std::cerr<<"ERROR Addressbanks::add Thread above number of Threads\n";
    }
    Thread[pa->Thread].add(pa);
  }
  void  addLocked(const unsigned short thread, 
		      Address *pa){
    remove(pa);
    pa->Thread=thread;
    if (thread>=NumberOfThreads){
      pa->Thread=0;
      std::cerr<<"ERROR Addressbanks::add Thread above number of Threads\n";
    }
    getThread(pa->Thread);
    Thread[pa->Thread].add(pa);
    releaseThread(pa->Thread);
  }

  void addService( Address *pa){
    Thread[pa->Thread].addService(pa);
  }


  void removeService(Address *pa){
    Thread[pa->Thread].removeService(pa);
  }
  void removeServiceLocked(Address *pa){
    //if (pa->Index==0)return;
    getThread(pa->Thread);
    Thread[pa->Thread].removeService(pa);
    releaseThread(pa->Thread);
  }

  
  
  bool getService(const std::string &service, unsigned long &index){
    return Thread[0].getService(service,index);
  }
  bool getService(unsigned short thread,
		  const std::string &service, unsigned long &index){
    return Thread[thread].getService(service,index);
  }
  bool getServiceLocked(unsigned short thread,
			const std::string &service, 
			unsigned long &index){
    getThread(thread);
    bool r=Thread[thread].getService(service,index);
    releaseThread(thread);
    return r;
  }
  
  /**
   * Removes Address 'pa' and sets its index to 0.
   * @param pa The Address to remove
   */
  void remove(Address *pa){
    Thread[pa->Thread].remove(pa);
  }

  unsigned short push(Address * pa, const unsigned long index){
    return Thread[pa->Thread].push(pa,index);
  }
  

  unsigned short pull(Address * pa, const unsigned long index){
    return Thread[pa->Thread].pull(pa,index);
  }
  
  unsigned short disconnect(Address * pa, const unsigned long index){
    return Thread[pa->Thread].disconnect(pa,index);
  }

  /*
   * Returns status of the Address.
   * @return 0 if ok, else ADDRESS_INVALID. 
   */    
  unsigned short query(const unsigned long index){
    return Thread[0].query(index);
  }
  unsigned short query(const unsigned short thread,
		       const unsigned long index){
    return Thread[thread].query(index);
  }
  unsigned short queryLocked(const unsigned short thread,
		       const unsigned long index){
    getThread(thread);
    unsigned short r=Thread[thread].query(index);
    releaseThread(thread);
    return r;
  }
  /**
   * This is the default Address bank for all users.  It is set up at
   * size=128.  One can call TheAddressBankThread.optimizeHash() after setup
   * is done to change this to the best value.  This is not much of an
   * issue unless youn have lots of addresses or severe memory
   * restrictions.
   *
   * One can forget about this alltogether untill one start
   * multithreading of AddressBankThread users.  Then one will need one bank
   * per thread.  For that one must create another AddressBankThread and
   * call setBank on the user objects in all but one of the threads.  One
   * thread can still use this default bank.
   *
   * This stange way of doing it assure that as long as no one calls
   * this static member the object will actually be made.
   * The first time this is called the one and only s_TheAddressBankThread
   * object is made.
   *
   * @param depth the first call will set up a hash tables for the Addresses 
   * of Size=2^(depth), so chose depth for about the number of 
   * Adresses stored at any given time.  It works for any depth
   * but intelligent choices make it more efficient.  
   *
   *  @param numthreads this is the limit on the number of thread s you can 
   *   have separate address spaces for.
   *  @param this is an array of length numthreads that contains the depths
   *         of each bank.  Threads with only 1-3 addresses can have depth 0
   *         larger spaces might want better hashing.
   * 
   */
  static AddressBank& theAddressBank(unsigned short numthreads=1,
				     unsigned short *depths=0){
    static AddressBank s_TheAddressBank(numthreads,depths); 
    return s_TheAddressBank;
  }
private:
};
} // namespace Cure

#endif // CURE_ADDRESSBANK_HH
