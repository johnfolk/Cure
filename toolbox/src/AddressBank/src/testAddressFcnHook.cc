//
// = LIBRARY
//
// = FILENAME
//    offlineSLAM.cc
//
// = FUNCTION
//
// = AUTHOR(S)
//    Patric Jensfelt
//
// = DESCRIPTION
//    
// = COPYRIGHT
//    Copyright (c) 2005 Patric Jensfelt
//
/*----------------------------------------------------------------------*/

#include "CureDebug.hh"

#include "FileAddressManager.hh" 
#include "AddressFcnHook.hh"
#include "ConfigFileReader.hh"

#ifndef DEPEND
#include <signal.h>
#endif

//===========================================================================
//===========================================================================
//===========================================================================

bool run = true;
void sigtrap(int signo) { run = false; }

//===========================================================================
//===========================================================================
//===========================================================================

int processOdom(Cure::TimestampedData &tp)
{
  std::cerr << "Got odom\n";
  return 0;
}

int processScan(Cure::TimestampedData &tp)
{
  std::cerr << "Got scan\n";
  return 0;
}

//===========================================================================
//===========================================================================
//===========================================================================

int main(int argc, char * argv[])
{
  cure_debug_level = 30;

  const char *optstring = "hc:d:e:";
  char *args = "[-h help] [-c config gile] [-d debug level] [-e execMode]";
  char *configfile = "-";  
  int execMode = 0;
  char o = getopt(argc, argv, optstring);
  while (o != -1) {
    switch (o) {
    case 'e':
      execMode = atoi(optarg);
      break;
    case 'd':
      cure_debug_level = atoi(optarg);
      break;
    case 'c':
      configfile = optarg;
      break;
    case 'h':
    case '?':
      std::cerr << "Usage: " << argv[0] << " " << args << std::endl;
      return -1;
    }
    o = getopt(argc, argv, optstring);
  }

  if (strcmp(configfile, "-") == 0) {
    std::cerr << "You must specify a config gile\n";
    std::cerr << "Usage: " << argv[0] << " " << args << std::endl;
    return -1;
  }

  Cure::ConfigFileReader cfg;
  if (cfg.init(configfile)) return -1;

  if (execMode < 0 || execMode > 1) {
    std::cerr << "Only supports execModes 0-(AFAP), 1-step, not "
              << execMode << "\n";
    execMode = 0;
    std::cerr << "Will use execMode=" << execMode << std::endl;
  }


  Cure::AddressFcnHook odomHook(&processOdom);
  Cure::AddressFcnHook scanHook(&processScan);

  Cure::FileAddressManager fam;
  if (fam.addOdomFile(cfg, &odomHook)) return -1;
  if (fam.addSensorFile(cfg, 3, &scanHook)) return -1;

  signal(SIGINT, sigtrap);

  while (run && fam.step() == 0) {
    std::cerr << ".";
    if (execMode == 1) getchar();
  }

  if (!run) {
    std::cout << "Aborted with ctrl-c @ odom time " 
              << fam.m_OdomTime << "\n";    
  }
}
