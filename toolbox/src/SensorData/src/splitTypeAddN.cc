//
// = LIBRARY
//
// = FILENAME
//    reformatPDFiles.cc
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

#ifndef DEPEND
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string>
#endif

#include "PackedSICK.hh"
#include "PackedPose.hh"
#include "TimestampedData.hh"

int main(int argc, char * argv[])
{
  const char *optstring = "hf:";
  char *args = "[-h help] [-f filename]";
  char *filename = "notset";
  char o = getopt(argc, argv, optstring);
  while (o != -1) {
    switch (o) {
    case 'f':
      filename = optarg;
      break;
    case 'h':
    case '?':
      std::cerr << "Usage: " << argv[0] << " " << args << std::endl;
      return -1;
      break;
    }
    o = getopt(argc, argv, optstring);
  }

  if (strcmp(filename,"notset") == 0) {
    std::cerr << "Need to specify file name\n";
    std::cerr << "Usage: " << argv[0] << " " << args << std::endl;
    return -1;
  }

  std::fstream fs;
  fs.open(filename, std::ios::in);
  if (fs <= 0) {
    std::cerr << "Could not open file \"" << filename << "\"\n";
    std::cerr << "Usage: " << argv[0] << " " << args << std::endl;
    return -1;
  }

  long lineno = 1;
  unsigned Type;
  unsigned long timeSecs, timeUSecs;
  unsigned short Version;

  Cure::PackedSICK scan;
  Cure::PackedPose pose;

  while (fs >> Type >> timeSecs >> timeUSecs >> Version) {

    int t = ((Type >> 24) & 0x7F);
    //===============================================
    if (t == SICKSCAN_TYPE) {

      if (Version != 1) {
        std::cerr << "Not dealing with version " << Version
                  << " for SICKScan on lineno=" << lineno << "\n";
        return -1;
      }
      
      unsigned short us1, us2;
      if ( !(fs >> us1 >> us2) ) {
        std::cerr << "Failed to parse short 1 and 2 "
                  << " for SICKScan on lineno=" << lineno << "\n";
        return -1;
      }

      std::string data;
      getline(fs, data);
      Version = 3;
      scan.setSubType(Type&0x0000FFFF);
      std::cout << ((Type&0x7F000000)>>24) << " "
                << ((Type&0x00FF0000)>>16) << " "
                << ((Type&0x0000FFFF)) << " "
                << timeSecs << " "
                << timeUSecs << " "
                << Version << " "
                << scan.getShortDataSize() << " "
                << scan.getDataSize()
                << us1 << " "
                << us2 << " "
                << Cure::SICKScan::SCANNER_TYPE_LMS200 // No extra white space,
                                                       // it is in data
                << data << std::endl;

      //===============================================
    } else if (t == POSE3D_TYPE) {

      if (Version != 1) {
        std::cerr << "Not dealing with version " << Version
                  << " for Pose3D on lineno=" << lineno << "\n";
        return -1;
      }
       
      std::string data;
      getline(fs, data);
      Version = 2;
      pose.setSubType(Type&0x0000FFFF);
      std::cout << ((Type&0x7F000000)>>24) << " "
                << ((Type&0x00FF0000)>>16) << " "
                << ((Type&0x0000FFFF)) << " "
                << timeSecs << " "
                << timeUSecs << " "
                << Version << " "
                << pose.getShortDataSize() << " "
                << pose.getDataSize() // No extra white space, it is in data
                << data << std::endl;

      //===============================================
    } else if (t == MEASUREMENTSET_TYPE) {
      
      int nm = ((Type)&0xFFFF); // Number of measurements

      if (Version != 2) {
        std::cerr << "Not dealing with version " << Version
                  << " for MeasurementSet on lineno=" << lineno << "\n";
        return -1;        
      }

      // We only need to remove the two initial short values that code
      // the number of doubles
      short dsizeH, dsizeL;
      if ( !(fs >> dsizeH >> dsizeL) ) {
        std::cerr << "Failed to parse DataSizeH and DataSizeL shorts "
                  << " for MeasurementSet on lineno=" << lineno << "\n";
        return -1;
      }
      
      unsigned long nDoubles = ((dsizeH << 16) | dsizeL);
      unsigned long nShorts = nm * 10;

      std::string data; // Read the rest of the values
      getline(fs, data);
      Version = 3;

      std::cout << ((Type&0x7F000000)>>24) << " "
                << ((Type&0x00FF0000)>>16) << " "
                << ((Type&0x0000FFFF)) << " "
                << timeSecs << " "
                << timeUSecs << " "
                << Version << " "
                << nShorts << " "
                << nDoubles  // No extra white space, it is in data
                << data << std::endl;

      //===============================================
    } else {
      std::cerr << "Unknonwn type " << t << std::endl;
      std::string junk;
      getline(fs, junk);
    }

    lineno++;
  }

  std::cerr << "Stopped at line no " << lineno << std::endl;
  std::cerr << "Last time " << timeSecs << " " << timeUSecs << std::endl;
  return 0;
} 
