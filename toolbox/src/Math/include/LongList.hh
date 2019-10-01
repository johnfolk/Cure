// = AUTHOR(S)
//    John Folkesson
//    
//    June 26, 2007
//
//    Copyright (c) 2007 John Folkesson
//    
#ifndef CURE_LONGLIST_H
#define CURE_LONGLIST_H


namespace Cure{

  /**
   * List of longs that can be used for example to hold indices
   *
   * @author Patric Jensfelt
   * @see
   */
  class LongList
  {
  public:
    long Element; 
    LongList *Next;
  public:
    LongList(){
      Element=0;
      Next=0;
    }

    ~LongList(){
      if (Next!=0) delete Next;
      Next=0;
    }
    /**
     * Removes all the stored elements.
     */
    void clean(){
      if (Next!=0) delete Next;
      Next=0;
    }
    /**
     * Removes all the stored elements.
     */
    void clear(){
      if (Next!=0) delete Next;
      Next=0;
    }
    /**
     * Add a long to the end of the list.
     */
    void add(long l){
      if (Next==0)
	{
	  Next=new LongList();
	  Element=l;
	  return; 
	}
      Next->add(l);
      return;
    }
    /**
     * @returns the number of elements stored in the list.
     */
    unsigned long count(){
      if (Next==0)return 0; 
      return (1+Next->count());
    }
    /**
     * Adds and element if it is not already on the list.
     * @param l the long to add.
     * return true if l was not already on list; 
    */
    bool addUnique(long l){
      if (Next==0)
	{
	  Next=new LongList();
	  Element=l;
	  return true; 
	}
      if (Element==l)return false;
      return Next->addUnique(l);
    }
    /**
     * Romove the kth element of the list.
     * @param k the postition of the element to remove.
     */
    void remove(long k){
      if(!Next)return;
      if (k==0)
	{
	  Element=Next->Element;
	  if (Next->Next)
	    {
	      Next->remove(0);
	      return;
	    }
	  delete Next;
	  Next=0;
	  return;
	}
      Next->remove(k-1);
      return;
    }
    /*
     * This finds the first instance of long on the list and removes it.
     * @param k The actual long value to find and remove.
     */
    void removeLong(long k){
      if(!Next)return;
      if (k==Element)
	{
	  Element=Next->Element;
	  if (Next->Next)
	    {
	      Next->remove(0);
	      return;
	    }
	  delete Next;
	  Next=0;
	  return;
	}
      Next->removeLong(k);
      return;
    }
    /*
     * This finds the first instance of long on the list.
     * @param k The actual long value to find.
     * @return true if found
     *
     */
    bool find(long k){
      if(!Next)return false;
      if (k==Element)
	return true;
      return Next->find(k);
    }
    /**
     * Make this list hold the same set of longs as another.
     * @param l the list to copy.
     */
    void operator = (const LongList & l){
      clean();
      if (l.Next)
	{
	  Element=l.Element;
	  Next=new LongList();
	  (*Next)=(*l.Next);
	}
    }

  }; // class LongList

  /**
   * List holding pairs of longs
   *
   * @author Patric Jensfelt
   * @see
   */
  class LongPairList
  {
  public:
    unsigned long UnsignedElement; 
    long Element; 
    LongPairList *Next;
  public:
    LongPairList(){
      UnsignedElement=0;
      Element=0;
      Next=0;
    }

    ~LongPairList(){
      if (Next!=0) delete Next;
      Next=0;
    }
    void clean(){
      if (Next!=0) delete Next;
      Next=0;
    }
    void clear(){
      if (Next!=0) delete Next;
      Next=0;
    }
    void add(unsigned long u,long l){
      if (Next==0)
	{
	  Next=new LongPairList();
	  Element=l;
	  UnsignedElement=u;
	  return; 
	}
      Next->add(u,l);
      return;
    }
    long find(unsigned long u){
      if(!Next)return -1;
      if (u==UnsignedElement)return Element;    
      return Next->find(u);
    }
    void replace(unsigned long u,long l){
      if (Next==0)
	{
	  Next=new LongPairList();
	  Element=l;
	  UnsignedElement=u;
	  return; 
	}
      if (UnsignedElement==u)
	{
	  Element=l;
	  return;
	}      
      Next->replace(u,l);
    }
    void addUnique(unsigned long u,long l){
      if (Next==0)
	{
	  Next=new LongPairList();
	  Element=l;
	  UnsignedElement=u;
	  return; 
	}
      if (Element==l)return;
      if (UnsignedElement==u)return;
      Next->addUnique(u,l);
    }
    void remove(long k){
      if(!Next)return;
      if (k==0)
	{
	  Element=Next->Element;
	  UnsignedElement=Next->UnsignedElement;
	  if (Next->Next)
	    {
	      Next->remove(0);
	      return;
	    }
	  delete Next;
	  Next=0;
	  return;
	}
      Next->remove(k-1);
      return;
    }
    void operator = (const LongPairList & l){
      clean();
      if (l.Next)
	{
	  Element=l.Element;
	  UnsignedElement=l.UnsignedElement;
	  Next=new LongPairList();
	  (*Next)=(*l.Next);
	}
    }

  }; // class LongPairList

} // namespace Cure

#endif
