// = AUTHOR(S)
//    John Folkesson
//    
// = COPYRIGHT
//    Copyright (c) 2004 John Folkesson
//    


#ifndef CURE_LINKEDARRAY_HH
#define CURE_LINKEDARRAY_HH
#include "Matrix.hh"
namespace Cure {

/**
 * This is a Linked list of double array's.  The LinkedArray can either
 * manage the allocateion of the memory to the double array elements
 * or the user can chose to.
 *
 * @author John Folkesson 
 */
class LinkedArray
{  
public:
  /**
   * The size of the double array pointed to by Element.
   */
  int Length;
  /**
   * Next=0 at the end of the chain, (ie. where also Element=0).
   */
  LinkedArray *Next;
  /*Pointer to previous link in chain*/
  LinkedArray *Prev;
  /**The double array itself.*/
  double *Element;
protected:
  /**
   * If the LinkedArray is managing the memory this saves a pointer
   * to the double array.
   */
  double *AllocatedElement;


 public:
  /**
   * This creates an empty LinkedArray (Next=0)
   */
  LinkedArray();
  /**
   * Create a LinkedArray and allocates an array of size len.
   * @param len the length (size) of the double array to allocate
   */
  LinkedArray(const int len);

  /**
   * This allows the caller to create a LinkedArray with 
   * the values of elements set equal to the values in a.
   * It copies the values from a to a double array that is
   * allocated by the LinkedArray object.
   * @param a The double array with values to be assigned to elements
   * @param len the length (size) of the new double array to be allocated.
   */
  LinkedArray(double *a, int len=-1);
  ~LinkedArray();
  
  /**
   * This allows you to change the array Element. 
   * If the Length=len  this method does nothing.
   * It will call delete and new if any change in size is needed.
   * It will not delete a user allocated array, but it will
   * not be pointing to it either (if len!=Length).
   * @param len the length (size) of the double array to allocate
   */
  void reallocate(const int len);

  /**
   * This allows the caller to assign his own double array to Element.
   * This avoids coping the array elements but the caller
   * is then responsible to delete the array at the right time.
   * @param a The double array to be assigned to Element
   * @param len the length (size) of the double array.
   */
  void setArray(double *a, const int len);

  /**
   * This will copy a and all LinkedArray's linked to 
   * a->Next to a chain staring from this LinkedArray..
   * @param a the LinkedArray to be copied.
   */
  void operator = (const LinkedArray& a);
  /**
   * @param d All elements of the array will be set to equal d.
   */
  void operator = (const double d);

  /**
   * This allows the caller to add a LinkedArray to the chain.
   * A new LinkedArray will be added at the end of the chain
   * and the values of elements will be set equal to the values in a.
   * So this copies the values pointer to by a not the pointer itself.
   * @param a The double array with values to be assigned to elements
   * @param len the length (size) of the new double array.
   */
  void add(double *a, int len);
  /**
   * This treats each row of a as a seperate double array of length a.Columns
   * and adds them sequencially at the end of the chain.
   * @param a the matrix to copy the array values from.
   */
  void add(Cure::Matrix & a);
  /**
   * This simple cuts out this Linked array from the chain and connects
   * its Prev to its Next. 
   */
  void cut();
  /**
   * This deletes its Element and Next
   */
  void clear();
  /**
   * Returns the Array elements.
   */
  double& operator() (const int i){
    return( Element[i] );
  }
  /**
   * Returns the Array elements.
   */
  double operator() (const int i) const { 
    return( (*(LinkedArray *)this).operator()(i) );
  }
  
  /**
   * Returns the Array elements of a LinkedArray r links away.
   * @param r the number of links to traverse before grabing Element[c]
   * @param c the array element from the rth array to be returned.
   */
  double& operator() (const int r, const int c){
     if (r>0)
      return (*Next)(r-1,c);
    return( Element[c]);
  }
  /**
   * Returns the Array elements of a LinkedArray r links away.
   * @param r the number of links to traverse before grabing Element[c]
   * @param c the array element from the rth array to be returned.
   */
  double operator() (const int r, const int c) const { 
    return( (*(LinkedArray *)this).operator()(r,c) );
  }
  /** Prints info to the display*/
  void print();
};

} // namespace Cure;


#endif 
