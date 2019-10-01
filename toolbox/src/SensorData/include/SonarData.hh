// = AUTHOR(S)
//    John Folkesson
//
// = COPYRIGHT
//    Copyright (c) 2005 John Folkesson
//
/*----------------------------------------------------------------------*/

#ifndef Cure_SonarData_hh
#define Cure_SonarData_hh

#include "SensorData.hh"
#include "TimestampedData.hh"

#ifndef DEPEND
#include <string.h>  // memcpy
#include <unistd.h>  // NULL
#include <iostream>  // ostream
#endif

// Forward declaration(s)
class PackedSonar;
namespace Cure {

/**
 * This class wraps the sensor data from a Sonar sensor. 
 * It implements a beam out to a certain range Range.
 * The beam angle is BeamWidth.  MaxRange and RangeResolution 
 * are also stored here 
 *
 * The SubType for this is 0
 *
 * @author John Folkesson 
 */
class SonarData : public SensorData
{  
public:

public:
  
  friend class PackedSonar;
  /**
   * Create empty SonarData
   * @param id the id of this sonar
   */
  SonarData(unsigned short id = 0);
  /**
   * Create empty SonarData
   * @param t the time
   * @param id the id of this sonar
   */
  SonarData(Timestamp &t,unsigned short id = 0);

  /**
   * Create a copy of a SonarData
   * @param s The object to copy
   */
  SonarData(const SonarData &s);
  /**called by consturctors*/
  void init();
  /**
   * Destructor
   */
  virtual ~SonarData();
  /*
   * This is how we (efficiently) get a SonarData from a TimestampedData.
   */
  SonarData * narrowSonarData(){return this;}
  /**
   * Copy operator
   * @param src The object to copy
   */
  SonarData& operator=(const SonarData &src);  
  /**
   * Copy operator
   * @param p The object to copy
   */
  void operator = (TimestampedData& p)
  {
    SonarData *ss=p.narrowSonarData();
    if (ss)operator=(*ss);
    else TimestampedData::operator=(p);
  }
  /**
   * Sets the scan range value equal to the double.
   * @param range the range of the sonar reading
   */
  void operator = (double range)
  {
    Range=range;
  }
  /**
   * get the range value.
   * @return the Range.
   */
  double getRange()const {  
    return( Range );
  }
  
  /**
   * Get the width of the beam
   * @return the angle width of the sonar beam.
   * [rad]
   */
  double getBeamWidth() const { return BeamWidth; }
  
  /**
   * Gets the resolution (accuracy) of the sonar range
   * @return the resolution in range which depends on the mode the
   * sensor is in [m].
   */
  double getRangeResolution() const { return RangeResolution; }
  
  /**
   * Gets the furthest the sonar can see.
   * @return maximum reading that the sensor can deliver in teh
   * current setting [m]. Can be used whe settings thresholds etc
   */
  double getMaxRange() const { return MaxRange; }
  
  /**
   * Set the range value.
   * @param r  the Range.
   */
  void setRange(double r) {  
    Range=r ;
  }

  /**
   * Set the maximum range for this sonar.
   * @param r maximum reading [m]. Can be used whe settings thresholds etc
   */
  void setMaxRange(double r) { MaxRange=r; }
   /**
   * set the width of the beam
   * @param b the angle width of the sonar beam.
   * [rad]
   */
  void setBeamWidth(double b)  {  BeamWidth=b; }
  /**
   * Sets the resolution (accuracy) of the sonar range
   * @param r the resolution in range which depends on the mode the
   * sensor is in [m].
   * @param r the new reange measurement.
   */
  void setRangeResolution(double r) { RangeResolution=r; }
  
  /**Sets up the object subtype*/ 
  void setSubType(unsigned short t);
  void print(){
    TimestampedData::print();
    std::cerr<<"Sonar SensorID="<<SensorID<<" "<<"Range="<<Range<<"\n";
  }
  
public:
  double Range;
  double BeamWidth;
  double MaxRange;
  double RangeResolution;
  friend std::ostream& operator <<(std::ostream& os, const Cure::SonarData &scan);
protected:
  
private:

};
std::ostream& operator << (std::ostream& os, const Cure::SonarData &scan);

}



#endif // SonarData_hh
