//
// = LIBRARY
//
// = FILENAME
//    testConfigFileReader.cc
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

#include "ConfigFileReader.hh"
#include "CureDebug.hh"

#ifndef DEPEND
#include <iostream>
#include <unistd.h>
#endif

using namespace Cure;

int main(int argc, char **argv)
{
  cure_debug_level = 30;

  const char *optstring = "hc:d:";
  char *args = "[-h help] [-c cfgfile] [-d debug level]";
  char *cfgfile = "-";
  char o = getopt(argc, argv, optstring);
  while (o != -1) {
    switch (o) {
    case 'd':
      cure_debug_level = atoi(optarg);
      break;
    case 'c':
      cfgfile = optarg;
      break;
    case 'h':
    case '?':
      std::cerr << "Usage: " << argv[0] << " " << args << std::endl;
      return -1;
      break;
    }
    o = getopt(argc, argv, optstring);
  }

  if (strcmp(cfgfile, "-") == 0) {
    std::cerr << "missing config file\n";
    std::cerr << "Usage: " << argv[0] << " " << args << std::endl;
    return -1;
  }

  Cure::ConfigFileReader cfg;
  if (cfg.init(cfgfile) != ConfigFileReader::NO_ERROR) return -1;

  std::string name;
  if (cfg.getRobotName(name) != ConfigFileReader::NO_ERROR) {
    CureCERR(30) << "WARNING: Found no robot name\n";
  }
  std::list<int> feats;
  if (cfg.getFeaturesToUse(feats) != ConfigFileReader::NO_ERROR) {
    CureCERR(30) << "WARNING: Did not find what features to use\n";
  }
  std::list<Cure::SensorPose> sensors;
  if (cfg.getSensorPoses(sensors) != ConfigFileReader::NO_ERROR) {
    CureCERR(30) << "WARNING: Did not find sensor poses\n";
  }
  std::string filePath="";
  if (cfg.getFilePath(filePath) != ConfigFileReader::NO_ERROR) {
    CureCERR(30) << "WARNING: Found not filepath\n";
  }
  std::string aFilePath="";
  if (cfg.getAdaBoostHypFilePath(aFilePath) != ConfigFileReader::NO_ERROR) {
    CureCERR(30) << "WARNING: Found not adaboost filepath\n";
  }
  std::map<int, std::string> sfiles, ffiles,afiles;
  if (cfg.getSensorFiles(sfiles) != ConfigFileReader::NO_ERROR) {
    CureCERR(30) << "WARNING: Found no sensor files\n";
  }
  if (cfg.getFeatureFiles(ffiles) != ConfigFileReader::NO_ERROR) {
    CureCERR(30) << "WARNING: Found no feature files\n";
  }
  if (cfg.getAdaBoostHypFiles(afiles) != ConfigFileReader::NO_ERROR) {
    CureCERR(30) << "WARNING: Found no adaboost files\n";
  }
  std::map<int, double> fdelays, sdelays;
  if (cfg.getSensorDelays(sdelays) != ConfigFileReader::NO_ERROR) {
    CureCERR(30) << "WARNING: Found no sensor delays\n";
  }
  if (cfg.getFeatureDelays(fdelays) != ConfigFileReader::NO_ERROR) {
    CureCERR(30) << "WARNING: Found no feature delays\n";
  }
  std::map<int, std::string> hConfig;
  if (cfg.getHelpersConfig(1, hConfig) 
      != ConfigFileReader::NO_ERROR) {
    CureCERR(30) << "WARNING: Found no helper config\n";
  }
  std::map<int, std::string> vfeConfig;
  if (cfg.getVisionExtractorsConfig(vfeConfig) != 
      ConfigFileReader::NO_ERROR) {
    CureCERR(30) << "WARNING: Found no vision extractors config\n";
  }
  std::string rhCfg;
  if (cfg.getRangeHoughConfig(1,rhCfg) != ConfigFileReader::NO_ERROR) {
    CureCERR(30) << "WARNING: Found no range hough config\n";
  }
  int odType;
  std::string odParams;
  if (cfg.getOdomModel(odType, odParams) != ConfigFileReader::NO_ERROR) {
    CureCERR(30) << "WARNING: Found no odommodel\n";
  }
  std::map<int, double> mdists;
  if (cfg.getMahalanobisDists(mdists) != ConfigFileReader::NO_ERROR) {
    CureCERR(30) << "WARNING: Did not find mahal dists\n";
  }
  std::string paramString;  
  std::list<std::string> paramStrings;
  std::string junk;
  if (cfg.getParamString("SENSORCHAINS", true, paramString, junk) != 
      ConfigFileReader::NO_ERROR) {
    CureCERR(30) << "WARNING: Did not find SENSORCHAINS\n";
  }
  if (cfg.getParamStrings("SENSORCHAINS", true, paramStrings, junk) != 
      ConfigFileReader::NO_ERROR) {
    CureCERR(30) << "WARNING: Did not find SENSORCHAINS\n";
  }

  std::cerr << "name: \"" << name << "\"\n";
  std::cerr << "Features: {";
  for (std::list<int>::iterator i = feats.begin(); i != feats.end(); i++) {
    std::cerr << *i << " ";
  }
  std::cerr << "}\n";
  std::cerr << "sensors:\n";
  for (std::list<Cure::SensorPose>::iterator i = sensors.begin(); 
       i != sensors.end(); i++) {
    double nr[9],a[3];
    i->extractAngles(nr, a);
    std::cerr << "        Type=" << i->m_SensorType
              << " ID=" << i->m_SensorID
              << " x=" << i->Position(0)
              << " y=" << i->Position(1)
              << " z=" << i->Position(2)
              << " theta=" << a[0]
              << " phi=" << a[1]
              << " psi=" << a[2]
              << std::endl;
  }
  std::cerr << "odommodel: type=" << odType << " params:\""
            << odParams << "\"\n"; 
  std::cerr << "filePath: \"" << filePath << "\"\n";
  std::cerr << "sensorfiles:\n";
  for (std::map<int, std::string>::iterator i = sfiles.begin();
       i != sfiles.end(); i++) {
    std::cerr << "        " << i->first << ": \""
              << i->second << "\"\n";
  }
  std::cerr << "featurefiles:\n";
  for (std::map<int, std::string>::iterator i = ffiles.begin();
       i != ffiles.end(); i++) {
    std::cerr << "        " << i->first << ": \""
              << i->second << "\"\n";
  }
  std::cerr << "featuredelays:\n";
  for (std::map<int, double>::iterator i = sdelays.begin();
       i != sdelays.end(); i++) {
    std::cerr << "        " << i->first << ": "
              << i->second << "\n";
  }
  std::cerr << "featuredelays:\n";
  for (std::map<int, double>::iterator i = fdelays.begin();
       i != fdelays.end(); i++) {
    std::cerr << "        " << i->first << ": "
              << i->second << "\n";
  }
  std::cerr << "Adaboost filePath: \"" << aFilePath << "\"\n";
  std::cerr << "adaboost files:\n";
  for (std::map<int, std::string>::iterator i = afiles.begin();
       i != afiles.end(); i++) {
    std::cerr << "        " << i->first << ": \""
              << i->second << "\"\n";
  }
  std::cerr << "helpers:\n";
  for (std::map<int, std::string>::iterator i = hConfig.begin();
       i != hConfig.end(); i++) {
    std::cerr << "        " << i->first << ": \""
              << i->second << "\"\n";
  }
  std::cerr << "vision extractors:\n";
  for (std::map<int, std::string>::iterator i = vfeConfig.begin();
       i != vfeConfig.end(); i++) {
    std::cerr << "        " << i->first << ": \""
              << i->second << "\"\n";
  }
  std::cerr << "RangeHough arglist: \"" << rhCfg << "\"\n";
  std::cerr << "mahalanobis:\n";
  for (std::map<int, double>::iterator i = mdists.begin();
       i != mdists.end(); i++) {
    std::cerr << "        " << i->first << ": "
              << i->second << "\n";
  }
  std::cerr << "paramString=\"" << paramString << "\n";
  std::cerr << "paramStrings=\"\n";
  for (std::list<std::string>::iterator si = paramStrings.begin();
       si != paramStrings.end(); si++) {
    std::cerr << "\"" << *si << "\"\n";
  }
}
