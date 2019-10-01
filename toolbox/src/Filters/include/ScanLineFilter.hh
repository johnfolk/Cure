//
// = LIBRARY
//
// = FILENAME
//    ScanLineFilter.hh
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

#ifndef ScanLineFilter_hh
#define ScanLineFilter_hh

#include "DataFilter.hh"
#include "RangeHough.hh"
#include "DataFilterAddress.hh"
#ifndef DEPEND
#endif


namespace Cure {

// Forward declaration(s)

/**
 * Filter that produces lines using the Hough transform from SICKScans.
 * 
 * The filter has two points:
 * Port 0: in(0): scan input
 * Port 1: out(0): measurement set with lines
 * 
 * @author Patric Jensfelt 
 * @see
 */
class ScanLineFilter : public Cure::DataFilter {
public:
  // Port 0 - in
  Cure::SICKScan m_Scan;

  bool PassDuplicateTimeStamps;  
public:
  /**
   * Constructor
   */
  ScanLineFilter();

  /**
   * @return pointer to the data in a certain port
   */
  Cure::TimestampedData* data(int port);

  /**
   * Configure the line filter
   * @param arglist string with configuration data
   */
  int config(const std::string &arglist);

  /**
   * Print the configuration of the filter
   */
  void printConfiguration();


  /**
   * Configura the filter. Most fo the parameters will be passed on to
   * the Range Hough motor.
   */
  void config( double minlength,
               short minPointsPerLine,
               double rhoThreshold,
               double varRhoThreshold,
               double halfWidth,
               double dGamma,
               double maxGap,
               double maxrange,
               short scanResHalfDegrees,
               double rhoBiasTreshold=3E-3);

  /**
   * Set the parameter that is used for testing for an endpoint.
   * 
   * @param behind The first scan point not on the line must be this
   * far behind the line.
   * 
   */
  void setBehind(double behind){
    m_Hough.setEndpointThreshold(behind);
  }

  /**
   *  This sets the variance in the point locations due to the sensor
   *  uncertainty.
   *  The default value is .0001 m^2 independent of distance.  
   *  The variance can change with the average square range to the 
   * line's points by seting the second parameter to other than the default 0.
   * 
   *  If lines with few points have too small a variance increase the 
   *  sensorVariance.  If very far away lines seem to have too small a
   * variance increase variance per square meter.
   * 
   * These amounts are divided by the number of points on the line so
   * for lines with many points the effect of these is small.  Then the 
   * uncertianty is estimated based on the variation in the actual data
   * for the line.
   *
   *  @param sensorVariance the variance of the sensor points in square meters.
   *  @param variancePerSquareRange sets the amount to add to the sensor 
   *         variance proportional to the line points averge square range.
   */
  void setSensorVariance(double sensorVariance, double varianceperSquareRange){
    m_Hough.setSensorVariance(sensorVariance,varianceperSquareRange);
  }
  
  /**
   * This increases (or decreases for factor<1) the quality threshold for 
   * the lines.
   * @param factor one is no change, >1 makes it harder to find lines,
   *               <1 makies it easier. 
   */  
  void increaseLineQuality(double factor){
    m_Hough.increaseLineQuality(factor);
  }
  
  /**
   * This decreases (or increases for factor<1) the amount of clutter,
   * (unwanted small lines). 
   * @param factor one is no change, >1 makes it harder to find lines,
   *               <1 makies it easier. 
   */  
  void removeClutter(double factor){
    m_Hough.removeClutter(factor);
  }
  
  /**
   * This increases (or decreases for factor<1) the parameters
   * for the sensor error model which set the minimum covariance for the 
   * lines.  
   * @param factor one is no change, >1 makes the lines error estimate larger,
   *               <1 makies it smaller. 
   */  
  void increaseMinCovariance(double factor){
    m_Hough.increaseMinCovariance(factor);
  }

  virtual unsigned short setOutputs();
protected:

  /**
   * Performs the actual calculation. This function is called
   * automatically when you write something to the filter.
   */
  unsigned short calc();
  
  Cure::RangeHough m_Hough;
  bool PushOut;
  Cure::Timestamp LastTime;
private:
}; // class ScanLineFilter
  
} // namespace Cure

#endif // ScanLineFilter_hh
