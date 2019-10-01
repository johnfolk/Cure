//
// = LIBRARY
//
// = FILENAME
//    testDataSlotAddress.cc
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

#include "FileAddress.hh"
#include "DataSlotAddress.hh"

#ifndef DEPEND
#include <signal.h>
#endif

int run = 1;
void sigtrap(int signo) { run = 0; }

int main(int argc, char **argv)
{
  if (argc < 2) {
    std::cerr << "Need to supply input file name\n";
    std::cerr << "Usage: ./testDataSlotAddress ../Filters/testodo.dat\n";
    return -1;
  }
  Cure::FileAddress in;
  if (in.openReadFile(argv[1])) {
    std::cerr << "Could not open file " << argv[1] << std::endl;
    return -1;
  }
  
  //Cure::DataSlotAddress slot(&bank, 100, TIMESTAMP_TYPE);
  Cure::DataSlotAddress slot( 100);
  //slot.setUseLock(true);
  slot.setUseThread(true);

  Cure::FileAddress out;
  out.openWriteFile("output.dat");
  slot.push(&out);

  signal(SIGINT, sigtrap);
  //Cure::Pose3D podo;
  Cure::SmartData sd;

  std::cerr << "\nPress ctrl-c to stop program before file runs out\n\n";

  int items = 0;
  while (run && in.read(sd) == 0) {
    std::cerr << ".";
    //slot.write(podo);
    slot.write(*sd.getTPointer());

    items++;
    usleep(500000);
  }

  std::cerr << "\nWrote " << items << " items\n";
  std::cerr << std::endl
            << "The first number below is the number of lines in the output "
            << "file.\n"
            << "Unless it is equal to " << items << " or at least damn close "
            << "something is wrong!\n";
  system("wc output.dat");
  out.closeWriteFile();
  std::cerr << "Done\n";
}
