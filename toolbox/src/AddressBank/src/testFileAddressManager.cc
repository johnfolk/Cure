//
// = LIBRARY
//
// = FILENAME
//    testFileAddressManager.cc
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

#include "FileAddressManager.hh"
#include "ConfigFileReader.hh"
#include "AddressBank.hh"

#ifndef DEPEND
#include <signal.h>
#endif

bool run = true;
void sigtrap(int signo) { run = false; }

int main(int argc, char **argv)
{
const char *optstring = "hO:W:H:P:c:";
  char *args = "[-h help] [-O odomfile] [-W wallfile] [-H hline file] "
    "[-P point file] [-c config file]";
  std::string odomfile="-", wallfile="-",hlinefile="-",pointfile="-";  
  std::string cfgFile = "-";
  char o = getopt(argc, argv, optstring);
  while (o != -1) {
    switch (o) {
    case 'c':
      cfgFile = optarg;
      break;
    case 'O':
      odomfile = optarg;
      break;
    case 'W':
      wallfile = optarg;
      break;
    case 'H':
      hlinefile = optarg;
      break;
    case 'P':
      pointfile = optarg;
      break;
    case 'h':
    case '?':
      std::cerr << "Usage: " << argv[0] << " " << args << std::endl;
      return -1;
    }
    o = getopt(argc, argv, optstring);
  }

  Cure::FileAddressManager fam;
  Cure::FileAddress output;

  if (output.openWriteFile("fam_out.dat")) return -1;

  if (cfgFile != "-") {
    Cure::ConfigFileReader cfg;
    if (cfg.init(cfgFile.c_str())!=Cure::ConfigFileReader::NO_ERROR) return -1;
    if (fam.addOdomFile(cfg, &output)) return -1;
    if (fam.addSensorFile(cfg, 3, &output)) return -1;
  } else {
    if (fam.addOdomFile(odomfile, &output,0)) return -1;
    fam.addFile(wallfile, MEASUREMENTSET_TYPE, &output,0);
    fam.addFile(hlinefile, MEASUREMENTSET_TYPE, &output,0);
    fam.addFile(pointfile, MEASUREMENTSET_TYPE, &output,0);
  }

  signal(SIGINT, sigtrap);

  while (run && fam.step() == 0) {
    std::cerr << ".";
  }

  if (!run) std::cerr << "Aborted with Ctrl-c\n";
  std::cerr << "Done\n";
}
