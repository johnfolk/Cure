
//
// = LIBRARY
//
// = FILENAME
//    SensorOffset.hh
//
// = AUTHOR(S)
//    John folkesson
//
// = COPYRIGHT
//    Copyright (c) 2006 john Folkesson
//
/*----------------------------------------------------------------------*/

#ifndef Cure_SensorOffset_hh
#define Cure_SensorOffset_hh

#include "SensorPose.hh"
#include <sstream>  // istringstream
#ifndef DEPEND
#include <iostream>  // ostream
#endif

// Forward declaration(s)
namespace Cure {

/**
 * This class wraps the the pose for a certain sensor. This means that
 * it stores not only the pose but also the type of sensor and its id.
 * And its name
 *
 * @author John Folkesson
 * @see Pose3D
 */
class SensorOffset 
{  
public:

  /**
   * This is the transfromation from the Robot Frame to the Sensor Frame.
   * It is the sum of the sensor chain poses.
   */
  SensorPose m_RobotSensorPose;
  Matrix m_Jacobian;
  std::string m_Name;
  Pose3D *m_Offset;
  Pose3D *m_PartialSum;
protected:
  int m_NumberOfOffsets;
  
  
public:


  /**
   * Constructor
   */
  SensorOffset();
  
  
  SensorOffset(const SensorOffset &src);

  /**
   * Destructor
   */
  virtual ~SensorOffset();
  
  //  int configSensorOffset(const std::string &param, 
  //		 const std::string &sensordef);

  void setNumberOfOffsets(unsigned short num);
  unsigned short getNumberOfOffsets(){return m_NumberOfOffsets;}
  /**
   * Copy operator
   */
  SensorOffset& operator=(const SensorOffset &src);

  unsigned short getSensorType(){return m_RobotSensorPose.m_SensorType;}
  unsigned short getSensorID(){return m_RobotSensorPose.m_SensorID;}
  
  /** 
   * First call this and then get the result in m_Jacobian and
   * m_RobotSensorPose.  This calculates the chain of offsets and pust
   * the jacobian of all variable pose coordinates in m_Jacobian.  The
   * first column is the fisrt column of m_Offset[0]'s Covaraince
   * coordinate.
   */
  void calcRobotSensorPose();
  void setSensorType(unsigned short type){m_RobotSensorPose.setSensorType(type);}
  void setSensorID(unsigned short type){m_RobotSensorPose.setSensorID(type);}
  //int setupPose(std::istringstream &strdes,unsigned short type=0,
  //	bool notoffset=true, int offset=-1);
  int setupPose(StateData &p,unsigned short type,
		bool notoffset=true, int offset=-1);
protected:

};



} // namespace Cure

std::ostream& operator << (std::ostream& os, const Cure::SensorOffset &sp);

#endif // SICKScan_hh
