//    $Id: FilterAddressMaker.hh,v 1.4 2007/09/14 09:13:50 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef Cure_FilterAddressMaker_hh
#define Cure_FilterAddressMaker_hh

#include "AddressMaker.hh"


namespace Cure{

class FilterAddressMaker:public AddressMaker{

public:
  Cure::DataFilter * makeFilterSubClass(const std::string classname,
					const std::string constargs);
  
};



}
#endif
