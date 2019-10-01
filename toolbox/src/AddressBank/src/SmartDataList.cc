//
// = AUTHOR(S)
//    John Folkesson, Patric Jensfelt
//    
//    Copyright (c) 2004 John Folkesson, Patric Jensfelt
//    

#include "SmartDataList.hh"
using namespace std;
namespace Cure {

SmartDataList::SmartDataList()
{
  Next=0;
  Key=0;
  Used = false;
}

SmartDataList::SmartDataList(TimestampedData &pa, int key)
{
  Next=0;
  Element = pa;
  Key = key;
  Used = true;
}

SmartDataList::~SmartDataList() 
{
  // "Recursively" call the destructor for all items in the list
  if (Next) {
    delete Next;
  }
}

long SmartDataList::getLength() const
{
  const SmartDataList *p = this;
  long n = 1;
  while (p->Next != 0) {
    p = p->Next;
    n++;
  }
  return n;
}

int SmartDataList::add(TimestampedData &pa, int key)
{
  if (!Used) {
    // There are no element attahced yet so we can simply put the data
    // into this elmenet
    Key=key;
    Element=pa;
    Used = true;
    return 1;
  }

  // Find the end of the list to add the new item.
  SmartDataList *p = this;
  int n = 1;
  while (p->Next != 0) {
    p = p->Next;
    n++;
  }

  // Add a new list item to the list
  p->Next = new SmartDataList(pa, key);

  return (n+1);
}
int SmartDataList::addOrdered(TimestampedData &pa, int key)
{
  if (!Used) {
    // There are no element attahced yet so we can simply put the data
    // into this elmenet
    Key=key;
    Element=pa;
    Used = true;
    return 1;
  }
  if (Element.Time>pa.Time){
    SmartDataList *sdl=Next;
    Next=new SmartDataList(*(Element.getTPointer()), Key);
    Next->Next=sdl;
    Element=pa;
    Key=key;
    return (int)getLength();
  }
  if (Next)return 1+Next->addOrdered(pa,key);
  else Next=new SmartDataList(pa, key);
  return 2;
}

TimestampedData *SmartDataList::getTPointer(unsigned long n)
{
  if (n == 0){
    if (Used) {
      // We want the first element in the list
      return Element.getTPointer(); 
    } else {
      // List is empty now
      return 0;
    }
  }
  if (Next) {
    // Call same function again but with one less level down in the list
    return Next->getTPointer(n-1);
  } 
  return 0;

}
int SmartDataList::get(unsigned long n, SmartData &pa,int * key)
{
  if (n == 0){
    if (Used) {
      // We want the first element in the list
      if (key) *key = Key;
      pa = Element; 
      return 0;
    } else {
      // List is empty now
      return 1;
    }
  }

  if (Next) {
    // Call same function again but with one less level down in the list
    return Next->get(n-1,pa,key);
  } else {
    //std::cerr << "SmartDataList::get Reached end of list, without finding\n";
    return 1;
  }

  std::cerr << "SmartDataList::get  Should not get here!\n";
  return 1; 
}

int SmartDataList::remove(unsigned long n)
{
  if (n==0) {
    // Want to remove this item
    if (Next) {
      // There is at least one item attached to this item. We handle
      // this by simply moving the data from that item up to this
      // item and delete the next item instead. This way the user
      // can still use his pointer if it was pointing to the element
      // to remove.
      Element = Next->Element;
      Key = Next->Key;

      SmartDataList *tmp = Next;

      // Redirect the next pointer to the item after the one to delete
      Next = Next->Next;

      // To stop all items from being deleted we set the Next pointer
      // of the item to delete to 0
      tmp->Next = 0;

      // Delete item
      delete tmp;
    } else {
      Used = false;
    }
  } else {
    if (Next->remove(n-1)) {
      // We reach the end of the list which should not happen!!!
      std::cerr << "SmartDataList::remove(n) Logic error 1!!\n";
      return 1;
    }
  }
  
  return 0;
}

int SmartDataList::remove(unsigned long n, SmartData &pa, int *key){
  // First we get the data we want
  if (get(n, pa, key)) return 1;

  // Then we remove it from the list
  return remove(n);
}


} // namespace Cure
