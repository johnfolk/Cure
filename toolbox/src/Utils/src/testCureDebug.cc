//
// = LIBRARY
//
// = FILENAME
//    testCureDebug.cc
//
// = FUNCTION
//
// = AUTHOR(S)
//    Patric Jensfelt
//
// = DESCRIPTION
//    
// = COPYRIGHT
//    Copyright (c) 2004 Patric Jensfelt
//
/*----------------------------------------------------------------------*/

#include "CureDebug.hh"

#ifndef DEPEND
#include <iostream>
#endif

int main()
{
  std::cout << "Please input desired debug level (typically 0-60): ";
  std::cin >> cure_debug_level;

  for (int i = 0; i <= 60; i+=5) {
    CureCERR(i) << "I print this on debug level " << i << std::endl;
    CureDO(i) { std::cerr << "I do this on debug level " << i << std::endl; }
  }
}
