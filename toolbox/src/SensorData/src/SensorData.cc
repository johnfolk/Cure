//
// = LIBRARY
//
// = FILENAME
//    SensorData.cc
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

#include "SensorData.hh"

#ifndef DEPEND
#endif

namespace Cure {

  SensorData::SensorData(unsigned short type, unsigned short id)
    :SensorType(type),
     SensorID(id)
  {
  }
  
  int getSensorType(const std::string &match,unsigned short &sensortype){
    if (match=="LongBaseLine")
      sensortype=SensorData::SENSORTYPE_LONG_BASELINE;
    else if ((match=="Unknown")||(match=="unknown"))
      sensortype=SensorData::SENSORTYPE_UNKNOWN;
    else if ((match=="Sick")||(match=="sick"))
      sensortype=SensorData::SENSORTYPE_SICK;
    else if ((match=="Camera")||(match=="camera"))
      sensortype=SensorData::SENSORTYPE_CAMERA;
    else if ((match=="Sonar")||(match=="sonar"))
      sensortype=SensorData::SENSORTYPE_SONAR;
    else if ((match=="Contact")||(match=="contact"))
      sensortype=SensorData::SENSORTYPE_CONTACT;
    else if ((match=="GPS")||(match=="gps"))
      sensortype=SensorData::SENSORTYPE_GPS;
    else if ((match=="Inertial")||(match=="inertial"))
      sensortype=SensorData::SENSORTYPE_INERTIAL;
    else if ((match=="RangeBearing")||(match=="rangeBearing")
	     ||(match=="rangebearing"))
      sensortype=SensorData::SENSORTYPE_RANGEBEARING;
    else if ((match=="Compass")||(match=="compass"))
      sensortype=SensorData::SENSORTYPE_COMPASS;
    else if ((match=="Robot")||(match=="Robot"))
      sensortype=SensorData::SENSORTYPE_ROBOT;
    else if ((match=="Odometry")||(match=="Odometry"))
      sensortype=SensorData::SENSORTYPE_ODOMETRY;
    else if ((match=="Position")||(match=="position"))
      sensortype=SensorData::SENSORTYPE_POSITION;
    else if ((match=="Actuation")|(match=="actuation"))
      sensortype=SensorData::SENSORTYPE_ACTUATION;
    else if ((match=="Altitude")|(match=="altitude"))
      sensortype=SensorData::SENSORTYPE_ALTITUDE;
    else if ((match=="Depth")||(match=="depth"))
      sensortype=SensorData::SENSORTYPE_DEPTH;
    else if ((match=="Velocity")||(match=="velocity"))
      sensortype=SensorData::SENSORTYPE_VELOCITY;
    else if (match=="DVL") 
      sensortype=SensorData::SENSORTYPE_DVL;
    else if ((match=="Force")||(match=="force"))
      sensortype=SensorData::SENSORTYPE_FORCE;
    else if ((match=="Control")||(match=="control"))
      sensortype=SensorData::SENSORTYPE_CONTROL;
    else return 1;
    return 0;
  }
 
}
