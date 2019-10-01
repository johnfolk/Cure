//
// = AUTHOR(S)
//    Patric Jensfelt
//    
//    Copyright (c) 2005 Patric Jensfelt
//    

#ifndef CURE_ADDRESSFCNHOOK_HH
#define CURE_ADDRESSFCNHOOK_HH

#include "Address.hh"

namespace Cure {


/**
 * Helper class that allows you to connect an address to a
 * function. This is very usefull for debugging and testing and for
 * being able to program outside of the address-framework when so
 * desired. The limitation now is that "normal" functions or static
 * class functions can be used.
 *
 * @author Patric Jensfelt
 * @see
 */
class AddressFcnHook : public Cure::Address {
public:
  /** 
   * Constructor that takes a function pointer as argument. The
   * function should be on the following form
   *
   * int fcn(Cure::TimestampedData &d);
   *
   * @param fcn pointer to function to call when data is written to
   * this address
   */
  AddressFcnHook( int(*fcn)(Cure::TimestampedData &),
		  const unsigned short thread=0);
  /** Destructor */
  ~AddressFcnHook();

  /** 
   * Called when someone writes to this address. If you call this
   * funcion yourself the function you have registered with the
   * constrcutor should be called.
   *
   * @return 0 is ok, else error code (@see TimestampedData)
   */
  unsigned short write(Cure::TimestampedData& p);

protected:

  /// Pointer to the function to call upon write
  int(*m_FcnPtr)(Cure::TimestampedData &);

}; // class AddressFcnHook

} // namespace Cure

#endif // CURE_ADDRESSFCNHOOK_HH
