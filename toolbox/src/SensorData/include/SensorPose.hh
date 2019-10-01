
//
// = LIBRARY
//
// = FILENAME
//    SensorPose.hh
//
// = AUTHOR(S)
//    Patric Jensfelt
//
// = COPYRIGHT
//    Copyright (c) 2005 Patric Jensfelt
//
/*----------------------------------------------------------------------*/

#ifndef Cure_SensorPose_hh
#define Cure_SensorPose_hh

#include "SensorData.hh"
#include "Pose3D.hh"

#ifndef DEPEND
#include <iostream>  // ostream
#endif

// Forward declaration(s)
namespace Cure {

/**
 * This class wraps the the pose for a certain sensor. This means that
 * it stores not only the pose but also the type of sensor and its id.
 *
 * @author Patric Jensfelt 
 * @see Pose3D
 */
class SensorPose : public Pose3D
{  
public:

  /**
   * Type of sensor @see TimestampedData.hh for a list of defined types
   */
  unsigned short m_SensorType;

  /**
   * ID of this particular sensor, each sensor in an array could need
   * its own id.
   */
  unsigned short m_SensorID;

public:
  friend class PackedSensorPose;

  /**
   * Constructor
   */
  SensorPose();
  
  /**
   * Constructor
   */
  SensorPose(unsigned short type, unsigned short id = 0);
  
  /**
   * Constructor
   */
  SensorPose(unsigned short type, const Pose3D &p, unsigned short id = 0);
  
  /**
   * Copy constructor
   */
  SensorPose(const SensorPose &src);
  void setSensorType( unsigned short tp){m_SensorType=tp;}
  unsigned short  getSensorType()const {return m_SensorType;}
  void setSensorID( unsigned short tp){m_SensorID=tp;}
  unsigned short  getSensorID()const {return m_SensorID;}
  

  /**
   * Destructor
   */
  virtual ~SensorPose();

  /*
   * This is how we (efficiently) get a SensorPose from a TimestampedData.
   */
  SensorPose* narrowSensorPose() { return this; }

  /**
   * Copy operator
   */
  SensorPose& operator=(const SensorPose &src)
  {
    Pose3D::operator=(src);
    m_SensorType = src.m_SensorType;
    m_SensorID = src.m_SensorID;
    return (*this);
  }

  /**
   * Copy operator
   */
  void operator = (TimestampedData& p)
  {
    SensorPose *sp = p.narrowSensorPose();
    if (sp)operator=(*sp);
    else TimestampedData::operator=(p);
  }

  /**
   * Copy operator that just copies the pose part
   */
  void operator = (const Pose3D &pose)
  {
    Pose3D::operator=(pose);
  }

  virtual void print();

protected:
  void initSP();
};

} // namespace Cure

std::ostream& operator << (std::ostream& os, const Cure::SensorPose &sp);

#endif // SICKScan_hh
