//
// = LIBRARY
//
// = FILENAME
//    Odometry.hh
//
// = AUTHOR(S)
//    Patric Jensfelt
//
// = COPYRIGHT
//    Copyright (c) 2004 Patric Jensfelt
//
/*----------------------------------------------------------------------*/

#ifndef CURE_ODOMETRY_HH
#define CURE_ODOMETRY_HH

#include "TimestampedData.hh"
#include "Transformation2D.hh"

#ifndef DEPEND
#endif

// Forward declaration(s)
  
namespace Cure {

/**
 * Class for odometric information
 * 
 * @author Patric Jensfelt 
 * @see
 */
class Odometry : public TimestampedData {

  /**
   * The pose part of the odometry
   */
  Transformation2D m_Pose;

  /**
   * Raw encoder data. The way to interpret this depends on the
   * specific robot. The convention for a differential drive robot
   * is that the first element is the left and the second the right
   * wheel encoder.
   */
  long m_Encoder[3];

public:

  /**
   * Constructor
   */
  Odometry();
  
  /**
   * Constructor
   *
   * @param t timestamp
   * @param x x-coordinate
   * @param y y-coordinate
   * @param theta orientation 
   * @param v tranlation velocity
   * @param w steer velocity
   */
  Odometry(const Timestamp &t,
	   double x, double y, double theta,
	   double v, double w);

  /**
   * Constructor
   *
   * @param t timestamp
   * @param p the 2Dpose i.e. x,y and orientation
   */
  Odometry(const Timestamp &t,
	   const Transformation2D &p);
  virtual void init();
  
  /**
   * Destructor
   */
  virtual ~Odometry();

  Odometry * narrowOdometry(){return this;}  
  Odometry& operator=(const Odometry &od);  
  void operator = (TimestampedData& p)
  {
    Odometry *od=p.narrowOdometry();
    if (od)operator=(*od);
    else TimestampedData::operator=(p);
  }

  /**
   * Set the translation velocity
   *
   * @param v translation velocity
   */
  void setVelTranslation(double v) { m_VelTranslation = v; }
  
  /**
   * Set the rotation velocity
   *
   * @param w rotation velocity
   */
  void setVelRotation(double w) { m_VelRotation = w; }
  
  /**
   * @return translation velocity
   */
  double getVelTranslation() const { return m_VelTranslation; }
  
  /**
   * @return rotation velocity
   */
  double getVelRotation() const { return m_VelRotation; }
  
  /**
   * @return left encoder value
   */
  long getEncLeft() const { return m_Encoder[0]; }
  
  /**
   * @return right encoder value
   */
  long getEncRight() const { return m_Encoder[1]; }
  
protected: 
  
private:
  double m_VelTranslation;
  double m_VelRotation;
  
}; // class Odometry
  
} // namespace Cure

#endif // CURE_ODOMETRY_HH
