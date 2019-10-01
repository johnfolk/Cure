//    $Id: AddressList.hh,v 1.4 2007/09/14 09:13:48 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_ADDRESSLIST_HH
#define CURE_ADDRESSLIST_HH

#include "Address.hh"

namespace Cure{

/**
 * A Addresslist is ta linked list of Addresses.
 *
 * @author John Folkesson 
*/
class AddressList
{
public:
  Address *Element;
  AddressList *Next;
public:
  AddressList(){
    Element=0;
    Next=0;
  }
  ~AddressList(){
    if (Next)delete Next;
  }
  int add(Address *pa);
  int remove(Address *pa);
  int remove(unsigned long n);
  Address * get(unsigned long n){
  if (Next)
    {
      if (Element->Index==n)
	return Element;
      else return Next->get(n);
    }
  return 0; 
}
  bool get(const std::string &str, unsigned long &index){
  if (Next)
    {
      if (Element->m_Descriptor.Name==str){
	index=Element->Index;
	return true;
      }
      else return Next->get(str,index);
    }
  return false; 
}

};
} // namespace Cure

#endif // CURE_ADDRESSLIST_HH

