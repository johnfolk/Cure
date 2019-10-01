// = RCSID
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2005 John Folkesson
//    


#include "DataDescriptor.hh"

#ifndef DEPEND

#include <sstream>  // istringstream
#include <ctype.h>  // isdigit

#endif

using namespace Cure;

DataDescriptor::DataDescriptor(std::string n)
{
  Name=n;
  ClassType=0;
  ID=0;
  SensorType=0;
  SensorID=0;
}
  
void DataDescriptor::operator = (const std::string &line){
  std::string tmp;
  std::istringstream str(line);
  if ( !(str >> tmp) ) {
    return;
  } else if (tmp[0] == '#') {
    return;
  } else if (isdigit(tmp[0])) {
    return;
    } else {
      std::string cname;
      if (!(str>>cname))return;
      if (cname=="Pose3D")
	ClassType=POSE3D_TYPE;	  
      else if (cname=="TimestampedData")
	ClassType=TIMESTAMP_TYPE;	  
      else if (cname=="SICKScan")
	ClassType=SICKSCAN_TYPE; 
      else if (cname=="Odometry")
	ClassType=ODOMETRY_TYPE;
      else if (cname=="RangeData")
	ClassType=RANGE_TYPE;
      else if (cname=="GenericData")
	ClassType=GENERIC_TYPE;
      else if (cname=="SonarData")
	ClassType=SONAR_TYPE;
      else if (cname=="MeasurementSet")
	ClassType=MEASUREMENTSET_TYPE;
      else if (cname=="VisionData")
	ClassType=VISION_TYPE;
      else if (cname=="SmartData")
	ClassType=SMARTDATA_TYPE;
      else if (cname=="SensorPose")
	ClassType=SENSORPOSE_TYPE;
      else if (cname=="Command")
	ClassType=COMMAND_TYPE;
      else if (cname=="ContactData")
	ClassType=CONTACT_TYPE;
      else if (cname=="GridData")
	ClassType=GRID_TYPE;
      else if (cname=="DataSet")
	ClassType=DATASET_TYPE;
      else if (cname=="StateData")
	ClassType=STATE_TYPE;
      else if (cname=="String")
	ClassType=STRING_TYPE;
      else {
	ClassType=0xFF;
	std::cerr << "DataDescriptor ClassType not configurable " 
		  <<tmp << "\n";
      }
      if (!(str>>ID))return;
      if (!(str>>SensorType))return;
      if (!(str>>SensorID))return;
      return;
    }
}

int  DataDescriptor::setTo(const std::string &line, std::string &sensor){
  std::string tmp;
  std::istringstream str(line);
  if ( !(str >> tmp) ) {
    return 1;
  } else if (tmp[0] == '#') {
    return 1;
  } else if (isdigit(tmp[0])) {
    return 1;
    } else {
      bool hassens=false;
      std::string cname;
      if (!(str>>cname))return 1;
      if (cname=="Pose3D")
	ClassType=POSE3D_TYPE;	  
      else if (cname=="TimestampedData")
	ClassType=TIMESTAMP_TYPE;	  
      else if (cname=="SICKScan"){
	ClassType=SICKSCAN_TYPE; 
	hassens=true;
      }
      else if (cname=="Odometry")
	ClassType=ODOMETRY_TYPE;
      else if (cname=="RangeData")
	ClassType=RANGE_TYPE;
      else if (cname=="GenericData")
	ClassType=GENERIC_TYPE;
      else if (cname=="SonarData"){
	ClassType=SONAR_TYPE;
	hassens=true;
      }
      else if (cname=="MeasurementSet"){
	ClassType=MEASUREMENTSET_TYPE;
	hassens=true;
      }
      else if (cname=="VisionData"){
	ClassType=VISION_TYPE;
	hassens=true;
      }
      else if (cname=="SmartData")
	ClassType=SMARTDATA_TYPE;
      else if (cname=="SensorPose"){
	ClassType=SENSORPOSE_TYPE;
	hassens=true;
      }
      else if (cname=="Command")
	ClassType=COMMAND_TYPE;
      else if (cname=="ContactData")
	ClassType=CONTACT_TYPE;
      else if (cname=="GridData")
	ClassType=GRID_TYPE;
      else if (cname=="DataSet")
	ClassType=DATASET_TYPE;
      else if (cname=="StateData")
	ClassType=STATE_TYPE;
      else {
	ClassType=0xFF;
	std::cerr << "DataDescriptor ClassType not configurable " 
		  <<tmp << "\n";
	return 1;
      }
      if (!(str>>ID))return 1;
      if (hassens){
	if (!(str>>sensor))return -1;
	if (isdigit(sensor[0]))SensorType=atoi(&sensor[0]);      
	else return -1;
	if (!(str>>SensorID))return 1;
      }     
      return 0;
    }
}

