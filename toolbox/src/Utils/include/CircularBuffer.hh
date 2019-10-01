//
// = FILENAME
//    CircularBuffer.hh
//
// = FUNCTION
//
// = AUTHOR(S)
//    Patric Jensfelt
//
// = COPYRIGHT
//    Copyright (c) 2006 Patric Jensfelt
//
/*----------------------------------------------------------------------*/

#ifndef Cure_CircularBuffer_h
#define Cure_CircularBuffer_h

#include "CureDebug.hh"

#ifndef DEPEND
#include <vector>
#include <string>
#endif

namespace Cure {

/**
 * This class implements a template for a circular buffer that can
 * host any type of items as long as they have a constructor without
 * argument which is needed when allocating the buffer.
 *
 * @author Patric Jensfelt
 */
template <class ITEMFACTORY, class BUFFERITEM>
class CircularBuffer {
public:
  /**
   * Constructor
   *
   * @param maximum number of items in the buffer
   */
  CircularBuffer(const std::string &name = "NONAME");

  /**
   * Destructor
   */
  virtual ~CircularBuffer();

  /**
   * Call this function to reset the buffer which after this cal will
   * be treated as empty. The information in the buffer items is still
   * there though so it is up to the user of the user to clear it if
   * so desired.
   */
  void resetBuffer();

  /**
   * Use this function to change the size of the buffer
   *
   * This function is destructive and will clear all data
   *
   * @param length new length of the buffer
   */
  void resize(int length);

  /**
   * Use this function to step to the next item in the buffer. If the
   * buffer is not full the m_NumItems variable will also be
   * increment. It is up to the user of the buffer to make sure that
   * any old data thta might be in a buffer item that now becomes the
   * current again is remove if need be.
   */
  void stepToNextItem()
  {
    // Step index forward to next item
    m_CurrentIndex++;

    // Make sure that the index is 0<=index<m_MaxNumItems
    m_CurrentIndex = m_CurrentIndex % m_MaxNumItems;

    // Update the variable telling how many items the buffer has if
    // the buffer s not yet full
    if (m_NumItems < m_MaxNumItems) m_NumItems++;
  }

  /**
   * Use this function to query how many items that is in the buffer
   * @return number of items in the buffer
   */
  int getNumItems() const { return m_NumItems; }

  /**
   * Use this function to get the capacity of the buffer, i.e. how
   * many items it can take
   *
   * @return maximum number of items in the buffer
   */
  int getMaxNumItems() const { return m_MaxNumItems; }

  /**
   * This functionn returns true if the buffer is empty
   * @return true if the buffer is empty
   */
  bool isEmpty() const { return (m_NumItems == 0); }

  /**
   * This functionn returns true if the buffer is full
   * @return true if the buffer is full
   */
  bool isFull() const { return (m_NumItems == m_MaxNumItems); }

  /**
   * This function returns the index of the current item in the buffer
   *
   * @return index of the current position in the buffer
   */
  int getCurrentIndex() const { return m_CurrentIndex; }

  /**
   * This functio returns the index of the oldest item in the
   * buffer. If there are not items in the buffer -1 is returned.
   *
   * @return index of oldest buffer item, -1 if no items
   */
  int getOldestIndex() const 
  { 
    if (m_NumItems > 0) {
      if (m_NumItems < m_MaxNumItems) {
        return 0;
      } else {
        return ( (m_CurrentIndex + 1) % m_MaxNumItems );
      }
    } else {
      return -1;
    }
  }

  /**
   * This function returns the index of the next item in the buffer.
   *
   * @return index of next buffer item
   */
  int getNextIndex() const 
  { 
    return ( (m_CurrentIndex + 1) % m_MaxNumItems );
  }

  /**
   * This function helps you calculate the index of a buffer item
   * relative to the current one. Relative index 0 is the current, 1
   * is the previous, 2 is two before the curent and so on.
   * 
   * @param index index relative the current item, (0=current,
   * 1-previous, etc), -1 is returned in case of errors
   *
   * @return index of item at index relative to the current one, -1 in
   * case of error
   */
  int getIndexRelCurrent(int index) const;

  /**
   * This function returns the current item in the circular buffer, a
   * null pointer is returned in case of error
   *
   * @return pointer to current buffer item, 0 in case of error
   */
  BUFFERITEM* getCurrentItem() const;

  /**
   * This function returns the oldest item in the circular buffer, a
   * null pointer is returned in case of error
   *
   * @return pointer to the oldest buffer item, 0 in case of error
   */
  BUFFERITEM* getOldestItem() const;

  /**
   * This function returns the next item in the circular buffer.
   *
   * @return pointer to the next buffer item
   */
  BUFFERITEM* getNextItem() const;

  /**
   * This function returns the item at a certain index in the circular
   * buffer, a null pointer is returned in case of error
   *
   * @param index index of item to get in the buffer
   *
   * @return pointer to item item at certain index, 0 in case of error
   */
  BUFFERITEM* getItem(int index) const;

  /**
   * This function returns the item at a certain index in the circular
   * buffer, a null pointer is returned in case of error
   *
   * @param index index relative the current item, (0=current,
   * 1-previous, etc), 0 is returned in case of error
   *
   * @return pointer to item item at index relative to current, 0 in
   * case of error
   */
  BUFFERITEM* getItemRelCurrent(int index) const;

  /**
   * You can overload this function if you want to create some other
   * type of buffer item such as a subclass of BUFFERITEM
   *
   * @param frame the frame index for the new frame
   *
   * @return pointer to new buffer item object
   */
  BUFFERITEM* getNewBufferItem(int frame) { return (new BUFFERITEM); }

protected:
  int m_MaxNumItems;
  int m_NumItems;

  int m_CurrentIndex;

  std::string m_BufferName;

  std::vector<BUFFERITEM*> m_Buffer;  
};

template <class ITEMFACTORY, class BUFFERITEM>
CircularBuffer<ITEMFACTORY,BUFFERITEM>::CircularBuffer(const std::string &name)
  :m_MaxNumItems(0),
   m_NumItems(0),
   m_CurrentIndex(-1),
   m_BufferName(name)
{}

template <class ITEMFACTORY, class BUFFERITEM>
CircularBuffer<ITEMFACTORY, BUFFERITEM>::~CircularBuffer()
{
  for (int i = 0; i < m_MaxNumItems; i++) {
    delete m_Buffer[i];
  }
}

template <class ITEMFACTORY, class BUFFERITEM>
void
CircularBuffer<ITEMFACTORY,BUFFERITEM>::resetBuffer()
{
  m_NumItems = 0;
  m_CurrentIndex = -1;
}

template <class ITEMFACTORY, class BUFFERITEM>
void
CircularBuffer<ITEMFACTORY,BUFFERITEM>::resize(int length)
{
  if (length < 0) {
    CureCERR(30) << "CircularBuffer(" << m_BufferName << ") length=" 
                 << length << "<0 not allowed!!! using 1\n";
    length = 1;
  }

  for (int i = 0; i < m_MaxNumItems; i++) {
    delete m_Buffer[i];
  }

  if (length < 0) {
    CureCERR(30) << "CircularBuffer(" << m_BufferName << ") length=" 
                 << length << "<0 not allowed!!! using 1\n";
    length = 1;
  }

  ITEMFACTORY* pFactory = static_cast<ITEMFACTORY*>(this);

  m_NumItems = 0;
  m_CurrentIndex = -1;
  m_MaxNumItems = length;
  m_Buffer.resize(m_MaxNumItems);
  for (int i = 0; i < m_MaxNumItems; i++) {
    m_Buffer[i] = pFactory->getNewBufferItem(i);
  }

  CureCERR(30) << "CircularBuffer(" << m_BufferName << ") resized to size "
               << m_MaxNumItems << " and now empty\n";
}

template <class ITEMFACTORY, class BUFFERITEM>
inline int
CircularBuffer<ITEMFACTORY,BUFFERITEM>::getIndexRelCurrent(int index) const
{
  if (index < 0 || index >= m_NumItems) {
    CureCERR(20) << "index=" << index << " out of bounds (0<=index<"
                 << m_NumItems << ") in CircularBuffer("
                 << m_BufferName << ")\n";
    return -1;
  }

  return (m_CurrentIndex - index + m_MaxNumItems) % m_MaxNumItems;
}

template <class ITEMFACTORY, class BUFFERITEM>
inline BUFFERITEM* 
CircularBuffer<ITEMFACTORY,BUFFERITEM>::getCurrentItem() const
{
  if (m_NumItems == 0) {
    CureCERR(20) << "No items yet in CircularBuffer("
                 << m_BufferName << ")\n";
    return 0;
  }

  return m_Buffer[m_CurrentIndex];
}

template <class ITEMFACTORY, class BUFFERITEM>
inline BUFFERITEM* 
CircularBuffer<ITEMFACTORY,BUFFERITEM>::getOldestItem() const
{
  if (m_NumItems == 0) {
    CureCERR(20) << "No items yet in CircularBuffer("
                 << m_BufferName << ")\n";
    return 0;
  }
  
  return m_Buffer[getOldestIndex()];
}

template <class ITEMFACTORY, class BUFFERITEM>
inline BUFFERITEM* 
CircularBuffer<ITEMFACTORY,BUFFERITEM>::getNextItem() const
{
  return m_Buffer[getNextIndex()];
}

template <class ITEMFACTORY, class BUFFERITEM>
inline BUFFERITEM* 
CircularBuffer<ITEMFACTORY,BUFFERITEM>::getItem(int index) const
{
  if (index < 0 || index >= m_NumItems) {
    CureCERR(20) << "index=" << index << " out of bounds (0<=index<"
                 << m_NumItems << ") in CircularBuffer("
                 << m_BufferName << ")\n";
    return 0;
  }

  return m_Buffer[index];
}

template <class ITEMFACTORY, class BUFFERITEM>
inline BUFFERITEM* 
CircularBuffer<ITEMFACTORY,BUFFERITEM>::getItemRelCurrent(int index) const
{
  if (index < 0 || index >= m_NumItems) {
    CureCERR(20) << "index=" << index << " out of bounds (0<=index<"
                 << m_NumItems << ") in CircularBuffer("
                 << m_BufferName << ")\n";
    return 0;
  }

  return m_Buffer[(m_CurrentIndex - index + m_MaxNumItems) % m_MaxNumItems];
}


}; // namespace Cure

#endif // Cure_CircularBuffer_h //
