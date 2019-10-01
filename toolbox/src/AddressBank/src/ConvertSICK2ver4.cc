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
#include "FileAddress.hh"
#include "TimestampedData.hh"
using namespace Cure;
int main(int argc, char * argv[])
{
  const char *optstring = "h:o:f:";
  char *args = "[-h help] [-f filename] [-o outfilename]";
  char *filename = "notset";
  char *filename2 = "notset";
  char o = getopt(argc, argv, optstring);
  while (o != -1) {
    switch (o) {
    case 'f':
      filename = optarg;
      break;
    case 'o':
      filename2 = optarg;
      break;
    case 'h':
    case '?':
      std::cerr << "Usage: " << argv[0] << " " << args << std::endl;
      return -1;
      break;
    }
    o = getopt(argc, argv, optstring);
  }
  Cure::FileAddress fa;
  if(strcmp(filename2,"notset")!=0) fa.openWriteFile(filename2);
  else{
    std::cerr << "Need to specify out file name\n";
    std::cerr << "Usage: " << argv[0] << " " << args << std::endl;
    return -1;
  }   
  if (strcmp(filename,"notset") == 0) {
    std::cerr << "Need to specify file name\n";
    std::cerr << "Usage: " << argv[0] << " " << args << std::endl;
    return -1;
  }
  std::fstream fs;
  fs.open(filename, std::ios::in);
  if (!fs.is_open()) {
    std::cerr << "Could not open file \"" << filename << "\"\n";
    std::cerr << "Usage: " << argv[0] << " " << args << std::endl;
    return -1;
  }
  long lineno = 1;
  Cure::PackedSICK scan;
  unsigned long nShorts, nDoubles;
  int Version;
  Timestamp temp;
  
  unsigned short classType, id, subType;
  while (fs >> classType >> id >> subType
	 >> temp.Seconds >> temp.Microsec
	 >> Version >> nShorts >> nDoubles) {
    //===============================================
    if (classType == SICKSCAN_TYPE) {

      if (Version != 3) {
        std::cerr << "Not dealing with version " << Version
                  << " for SICKScan on lineno=" << lineno << "\n";
        return -1;
      }
      if (Version==4)
	{
	  std::cerr << "Already version " << Version
                  << " for SICKScan on lineno=" << lineno << "\n";
	 return -1;
	}
      scan.setSubType(subType);
      scan.Time=temp;
      if (4 != scan.Version) {
        std::cerr << "\nERROR: Version mismatch!!! got "
                  << 4 << " expected " << scan.Version 
		  << std::endl;
         return 1;
        
      }
      if (scan.setShortDataSize(nShorts+1)){
	std::cerr << "\nERROR #Shorts according to file "
                  << std::endl;
        return 1;
      } else {
	unsigned short *bd=scan.ShortData;	
        int dim = scan.getShortDataSize()-1;
	for (int i = 0; i < 3; i++) 
	  if ( !(fs >> bd[i]) ) {
	    return -1;
	  }	  
	 
	bd[3]=0;
	for (int i = 4; i < dim; i++) 
	  if ( !(fs >> bd[i+1]) ) {
	    fs.close();
	    std::cerr << "Failed reading shorts  "
		      << i<<" "<<dim<<" "<<nShorts 
			<< std::endl;
	      return 1;
	  }
      
	dim = scan.setDataSize(nDoubles);
	double *d =scan.Data;
	for (int i = 0; i < dim; i++) {
	  if (!(fs>>d[i])) {
	    fs.close();
	    std::cerr << "Failed reading doubles for type "
		      << classType 
		      << std::endl;
	    return 1;
	  }
	}
	
      }
    }
    std::cerr<<".";
    fa.write(scan);
  }
      std::cerr<<"\nok\n";
  return 0;
} 
