//
// = LIBRARY
//
// = FILENAME
//    SensorData.hh
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

#ifndef SensorData_hh
#define SensorData_hh

#ifndef DEPEND
#endif

#include "TimestampedData.hh"

// Forward declaration(s)

namespace Cure {

/**
 * Base class for sensor data which is a special type of TimestampedData
 * 
 * @author Patric Jensfelt 
 * @see
 */
class SensorData :public TimestampedData{
public:
  /**
   * Defines the different sensor types
   */

  enum SensorTypes {
    SENSORTYPE_UNKNOWN = 0,
    SENSORTYPE_SICK,
    SENSORTYPE_CAMERA,
    SENSORTYPE_SONAR,
    SENSORTYPE_CONTACT,
    SENSORTYPE_GPS,
    SENSORTYPE_INERTIAL,
    SENSORTYPE_RANGEBEARING,
    SENSORTYPE_COMPASS,
    SENSORTYPE_ROBOT,
    SENSORTYPE_ODOMETRY,
    SENSORTYPE_POSITION,
    SENSORTYPE_ACTUATION,
    SENSORTYPE_LONG_BASELINE,
    SENSORTYPE_ALTITUDE,
    SENSORTYPE_DEPTH,
    SENSORTYPE_VELOCITY,
    SENSORTYPE_DVL,
    SENSORTYPE_FORCE,
    SENSORTYPE_CONTROL,
  };

  /**
   * Type of sensor used to produce this sensor data
   */
  unsigned short SensorType;
  /** Each sensor in an array might have its own id*/
  unsigned short SensorID;
  /**
   * This will assign the relavent parts of a DataDescriptor to
   * this object.
   * @param d the descriptor to assign to this objects.
   * @return 0 if ok 1 if wrong class. 
  */
  virtual  int setDescriptor(const Cure::DataDescriptor & d){
    if(getClassType()!=d.ClassType)return 1;
    if (SensorType!=d.SensorType)return 1;
    setID(d.ID);
    SensorID=d.SensorID;
    return 0;
   }
  virtual void setSensorID(unsigned short id) {SensorID=id;}
  virtual void setSensorType(unsigned short type) {SensorType=type;}

  /**Gets the Descriptor*/
  virtual  void getDescriptor(Cure::DataDescriptor & d) const {
    d.ClassType=getClassType();
    d.ID=getID();
    d.SensorType=SensorType;
    d.SensorID=SensorID;
    d.Name="";
  }
  virtual unsigned short getSensorType() const{return SensorType;}
  virtual unsigned short getSensorID() const{return SensorID;}
  SensorData *narrowSensorData(){return this;}
protected:

  /**
   * Constructor that makes this an abstract class
   * @param type the type
   * @param id the id
   */
  SensorData(unsigned short type=0, unsigned short id=0);
  
private:

}; // class SensorData
  int getSensorType(const std::string &match,unsigned short &sensortype);

}; // namespace Cure

#endif // SensorData_hh
