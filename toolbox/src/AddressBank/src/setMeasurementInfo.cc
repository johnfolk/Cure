//
// = LIBRARY
//
// = FILENAME
//    setMeasurementInfo.cc
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
#include "MeasurementSet.hh"

#ifndef DEPEND
#endif

int main(int argc, char **argv)
{
  const char *optstring = "hi:o:T:S:I:";
  char *args = "[-h help] [-i inputfile] [-o outputfile] "
    "[-T measurementType] [-S sensorType] [-I sensorID]";
  char *inputfile = "notset";
  char *outputfile = "notset";
  int measurementType = -1;
  int sensorType = -1;
  int sensorID = -1;
  char o = getopt(argc, argv, optstring);
  while (o != -1) {
    switch (o) {
    case 'T':
      measurementType = atoi(optarg);
      break;
    case 'S':
      sensorType = atoi(optarg);
      break;
    case 'I':
      sensorID = atoi(optarg);
      break;
    case 'i':
      inputfile = optarg;
      break;
    case 'o':
      outputfile = optarg;
      break;
    case 'h':
    case '?':
      std::cerr << "Usage: " << argv[0] << " " << args << std::endl;
      return -1;
      break;
    }
    o = getopt(argc, argv, optstring);
  }

  if (strcmp(inputfile, "notset") == 0) {
    std::cerr << "You need to specify an input file\n";
    std::cerr << "Usage: " << argv[0] << " " << args << std::endl;
    return -1;
  }

  if (strcmp(outputfile, "notset") == 0) {
    std::cerr << "You need to specify an output file\n";
    std::cerr << "Usage: " << argv[0] << " " << args << std::endl;
    return -1;
  }

  Cure::FileAddress input, output;
  input.openReadFile(inputfile);
  output.openWriteFile(outputfile);

  Cure::MeasurementSet ms;
  while (input.read(ms) == 0) {

    for (unsigned int i = 0; i < ms.getNumberOfElements(); i++) {
      if (measurementType >= 0)
        ms.Measurements[i].MeasurementType = measurementType;
      if (sensorType >= 0)
        ms.Measurements[i].SensorType = sensorType;
      if (sensorID >= 0)
        ms.Measurements[i].SensorID = sensorID;
    }

    output.write(ms);
  }
}
