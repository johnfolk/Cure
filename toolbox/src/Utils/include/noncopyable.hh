// This is a slightly modified version of Boosts noncopyable class

//  (C) Copyright Boost.org 1999-2003. Permission to copy, use, modify, sell
//  and distribute this software is granted provided this copyright
//  notice appears in all copies. This software is provided "as is" without
//  express or implied warranty, and with no claim as to its suitability for
//  any purpose.

//  See http://www.boost.org/libs/utility for documentation.

#ifndef CURE__NONCOPYABLE_HH
#define CURE__NONCOPYABLE_HH

namespace Cure {

/**
 * Interface to prevent copy assignment and copy construction.
 *
 * Every class that does not implement its own copy constructor and assignment
 * operator should inherit from this class. This will insure that the class is
 * not used in an undefined way. 
 */
class noncopyable
{
 protected:
    noncopyable() {}
    ~noncopyable() {}
 private:
    noncopyable( const noncopyable& );
    const noncopyable& operator=( const noncopyable& );
};

} // namespace Cure

#endif  // CURE__NONCOPYABLE_HH

