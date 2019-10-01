//
// = LIBRARY
//
// = FILENAME
//    DebugFunctions.hh
//
// = FUNCTION
//
// = AUTHOR(S)
//    Patric Jensfelt
//
// = EXAMPLE
// = BEGIN<CODE>
//    DebugCERR(30) << "This will be printed if cure_debug_level is "
//                  << ">= 30\n";
//    DebugDO(30) { 
//      std::cerr << "This will be printed if"
//                << " cure_debug_level is >= 30\n";
//    }
//
// = END<CODE>
//
// = COPYRIGHT
//    Copyright (c) 2003 Patric Jensfelt
//
/*----------------------------------------------------------------------*/

#ifndef CureDebug_hh
#define CureDebug_hh

#ifndef DEPEND
#include <iostream>
#include <stdio.h>
#include <sys/time.h>
#include <string>
#endif

extern int cure_debug_level;

namespace Cure {
  std::string getCureCERRTimeString();
}

#ifdef TURN_OFF_DEBUG
#define CureCERR(l) if (0) std::cerr 
#else
#define CureCERR(l) if (l <= cure_debug_level) std::cerr << Cure::getCureCERRTimeString() << " " << __FILE__ << ":" << __LINE__ << ": "
#endif

#ifdef TURN_OFF_DEBUG
#define CureDO(l) if (0) 
#else
#define CureDO(l) if (l <= cure_debug_level) 
#endif

inline std::string Cure::getCureCERRTimeString()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  char buf[24];
  sprintf(buf, "%03d.%03d", 
          int(tv.tv_sec - 1e3*(long(1.0e-3 * tv.tv_sec))),
          int(tv.tv_usec / 1000));
  std::string tmp = buf;
  return tmp;
}

#endif // CureDebug_hh

