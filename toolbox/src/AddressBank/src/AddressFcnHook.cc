//
// = AUTHOR(S)
//    Patric Jensfelt
//    
//    Copyright (c) 2005 Patric Jensfelt
//    

#include "AddressFcnHook.hh"
#include "AddressBank.hh"

namespace Cure {

AddressFcnHook::AddressFcnHook(int(*fcn)(Cure::TimestampedData &),
			       const unsigned short thread)
  :Address(thread),
   m_FcnPtr(fcn)
{
}


AddressFcnHook::~AddressFcnHook()
{
}

unsigned short 
AddressFcnHook::write(Cure::TimestampedData& p) {
  if (!m_FcnPtr) return ADDRESS_INVALID;
  return m_FcnPtr(p);
}

} // namespace Cure
