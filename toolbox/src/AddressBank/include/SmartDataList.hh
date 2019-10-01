//
// = AUTHOR(S)
//    John Folkesson, Patric Jensfelt
//    
//    Copyright (c) 2004 John Folkesson, Patric Jensfelt
//    

#ifndef CURE_SMARTDATALIST_HH
#define CURE_SMARTDATALIST_HH

#include "SmartData.hh"

namespace Cure{

/**
 * A SmartDatalist is a linked list of SmartDataes.
 *
 * @author John Folkesson, Patric Jensfelt
*/
class SmartDataList
{
public:
  /** Link to next element in the list */
  SmartDataList *Next;

  /** 
   * The key can for example be the port that the data in the list was
   * written to when used to buffer input data in a Filter for
   * example.
   */
  int Key;

  /** The data for this element */
  SmartData Element;

protected:

  bool Used;

public:

  /**
   * Constructor
   * Creates an empty list 
   */
  SmartDataList();

  /** 
   * Constructor
   * Creates a list with one element
   * @param p is copied into the Element
   * @param key the key
   */
  SmartDataList(TimestampedData & p,int key=0);

  /** 
   * Destructor that deletes all elements in the list linked to this
   * element. 
   *
   * NOTE that you are on your own if you start deleting object in the
   * list. Use the function remove instead!!
   */
  ~SmartDataList();

  /**
   * Query the number of elements. 
   * @return number of items in list (counted from calling item)
   */
  long getLength() const;

  /**
   * Same as add except that it keeps oldest data closest. 
   */
  int addOrdered(TimestampedData &pa, int key=0);
  /**
   * Add element at the end of the list and give this element a certain index
   *
   * @param p copied to the element added to the list.
   * @param key the key to assign to this element
   * @return number of items in list (counted from calling item)
   */
  int add(TimestampedData &p, int key=0);

  /**
   * Get the element at position n from the list where n=0 is the
   * first element in the list. Can also get the index corresponding
   * to this item (not the same as n!). 
   *
   * remove(0) will thus get the first item (FIFO) and remove(list.Count)
   * will remove the last (LIFO)
   *
   * @param n The position in the chain to get starting from 0 for first.
   * @param p The (n+1)th element is copied to this
   * @param key returns with the (n+1)th element's Key 
   * @return 0 if ok else 1
   */
  int get(unsigned long n, SmartData & p,int * key = 0);  

  TimestampedData *getTPointer(unsigned long n);  

  /**
   * Remove item n from the list where n=0 is the first element in the
   * list.
   *
   * remove(0) will thus remove the first item and remove(list.getLength()-1)
   * will remove the last.
   *
   * @param n The position in the chain to remove starting from 0 for first.
   * @return 0 if ok else 1
   */
  int remove(unsigned long n);

  /**
   * Get AND remove and element at position n from the list where n=0
   * is the first element in the list. Can also get the index
   * corresponding to te item in the list
   *
   * remove(0) will thus get the first item (FIFO) and
   * remove(list.getLength()-1)) will remove the last (LIFO)
   * @param n The position in the chain to get starting from 0 for first.
   * @param p The (n+1)th element is copied to this
   * @param key returns with the (n+1)th element's Key 
   * @return 0 if ok else 1
   */
  int remove(unsigned long n,SmartData & p, int *key = 0);

};
} // namespace Cure

#endif 

