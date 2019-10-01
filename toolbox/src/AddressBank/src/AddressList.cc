// = RCSID
//    $Id: AddressList.cc ,v 1.1 2004/03/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2004 John Folkesson
//    

#include "AddressList.hh"
using namespace std;
namespace Cure {

int AddressList::add(Address *pa)
{
  if (Next==0)
    {
      Element=pa;
      Next=new AddressList();
      return 1; 
    }
  return Next->add(pa);
}
int AddressList::remove(Address *pa)
{
  if (Next==0) return 1;
  if (Element==pa)
    {
      Element=Next->Element;
      AddressList *l=Next;
      Next=Next->Next;
      l->Next=0;
      delete l;
      return 0;
    }
  return Next->remove(pa);
}
int AddressList::remove(unsigned long n)
{
  if (Next)
    {
      if (Element->Index==n)
	return remove(Element);
      else return Next->remove(n);
    }
  return 1; 
}

}
