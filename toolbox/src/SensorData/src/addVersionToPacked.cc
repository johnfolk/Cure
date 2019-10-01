//
// = LIBRARY
//
// = FILENAME
//    transfPackedData.cc
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

#include "Transformation/TimestampedData.hh"

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
  unsigned short Version = 1;

  while (fs >> Type >> timeSecs >> timeUSecs) {

    int t = ((Type >> 24) & 0x7F);
    //===============================================
    if (t == SICKSCAN_TYPE) {
      std::string data;
      getline(fs, data);
      Version = 1;
      std::cout << Type << " "
                << timeSecs << " "
                << timeUSecs << " "
                << Version
                << data << std::endl;

      //===============================================
    } else if (t == POSE3D_TYPE) {
      std::string data;
      getline(fs, data);
      Version = 1;
      std::cout << Type << " "
                << timeSecs << " "
                << timeUSecs << " "
                << Version
                << data << std::endl;

      //===============================================
    } else if (t == MEASUREMENTSET_TYPE) {
      
      int nm = ((Type)&0xFFFF); // Number of measurements
      Version = 2;

      std::cout << Type << " "
                << timeSecs << " "
                << timeUSecs << " "
                << Version;

      /**
       * The layout of the ShortData is:
       * DatasizeH, DataSizeL,
       * mtype,keyH,keyL,Z.Rows,Box.R/C,W.Rows,W.Columns,V.Rows
       * mtype,keyH,keyL,Z.Rows,Box.R/C,W.Rows,W.Columns,V.Rows
       * and so on for each measurmenet
       */
      unsigned short dsH, dsL;
      if ( !(fs >> dsH >> dsL) ) {
        std::cerr << "Failed to read DataSizeH and DataSizeL on lineno="
                  << lineno << std::endl;
        return -1;
      }
      std::cout << " " << dsH << " " << dsL;
      unsigned short s;
      for (int i = 0; i < nm; i++) {
        for (int j = 0; j < 8; j++) {
          if ( !(fs >> s) ) {
            std::cerr << "Failed to read short " << j 
                      << " for meas " << i << " on lineno="
                      << lineno << std::endl;
            return -1;
          }
          std::cout << " " << s;
          if (j == 0) {
            std::cout << " " << VISION_TYPE 
                      << " " << 1;  // Philips camera
          }
        }
      }
      std::string ddata;
      getline(fs, ddata);
      std::cout << ddata << std::endl;

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
