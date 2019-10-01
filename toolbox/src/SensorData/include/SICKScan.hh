
//
// = LIBRARY
//
// = FILENAME
//    SICKScan.hh
//
// = AUTHOR(S)
//    Patric Jensfelt
//
// = COPYRIGHT
//    Copyright (c) 2004 Patric Jensfelt
//
/*----------------------------------------------------------------------*/

#ifndef Cure_SICKScan_hh
#define Cure_SICKScan_hh

#include "SensorData.hh"


#ifndef DEPEND
#include <string.h>  // memcpy
#include <unistd.h>  // NULL
#include <iostream>  // ostream
#endif

// Forward declaration(s)
class PackedSICK;
namespace Cure {

/**
 * This class wraps the sensor data from the SICK sensor. It assumes
 * that the scan corresponds to sampling 180 degrees with the same
 * angle step between all scan points.
 * 
 * The SubType for this is 0000 IFNN NNNN NNNN 

 * I = 1  if m_NIntensityLevels !=0;
 * F = 1 if m_NFlags !=0;
 * N is the m_NPts<722;
 * @author Patric Jensfelt 
 * @see
 */
class SICKScan : public SensorData
{  
public:
  enum ScannerType {
    SCANNER_TYPE_PLS = 1,
    SCANNER_TYPE_LMS200,
    SCANNER_TYPE_LMS291,
  };

public:
  
  friend class PackedSICK;
  /**
   * Create empty SICKScan
   */
  SICKScan(unsigned short id = 0);
  
  SICKScan(Timestamp &t,unsigned short id = 0);

  /**
   * Create a copy of a SICKScan
   */
  SICKScan(const SICKScan &src);

  void init();
  /**
   * Destructor
   */
  virtual ~SICKScan();
  /*
   * This is how we (efficiently) get a SICKScan from a TimestampedData.
   */
  SICKScan * narrowSICKScan(){return this;}
  /**
   * Copy operator
   */
  SICKScan& operator=(const SICKScan &src);  
  void operator = (TimestampedData& p)
  {
    SICKScan *ss=p.narrowSICKScan();
    if (ss)operator=(*ss);
    else TimestampedData::operator=(p);
  }
  /**
   * Sets the scan range values equal to the double array.
   * The ranges array  had better be m_NPts long.
   */
  void operator = (double *ranges)
  {
    memcpy(m_Ranges,ranges, m_NPts* sizeof(double));
  }
  /**
   * Returns the elements of the m_Ranges array.
   */
  double& operator()(const unsigned short r) {  
    return( m_Ranges[r] );
  }
  /**
   * Returns the elements of the m_Ranges array.
   */
  double operator()(const unsigned short r) const {
    return( (*(SICKScan *)this).operator()(r) );
  }

  /**
   * @return type of scanner that produced the scan
   */
  unsigned short getScannerType() const { return m_ScannerType; }

  /**
   * @return number of scan points
   */
  int getNPts() const { return m_NPts; }
  
  /**
   * @return the angle difference between two consecutive scan
   * points [rad]
   */
  double getAngleStep() const { return m_AngleStep; }
  
  /**
   * @return angle to first scan point [rad]
   */    
  double getStartAngle() const { return m_StartAngle; }
  
  /**
   * @return the resolution in range which depends on the mode the
   * sensor is in [m].
   */
  double getRangeResolution() const { return m_RangeResolution; }
  
  /**
   * @return maximum reading that the sensor can deliver in teh
   * current setting [m]. Can be used whe settings thresholds etc
   */
  double getMaxRange() const { return m_MaxRange; }
  
  /**
   * @return pointer to range measurements [m]
   */
  double* getRanges() { return m_Ranges; }
  
  /**
   * @return range for scan point i, <0 returned if index is out of
   * bounds
   */
  double getRange(int i) const;
  
  /**
   * @return number of flags in the flag bit mask for each scan point
   */
  bool getNFlags() const { return m_NFlags; }
  
  /**
   * @return flag bitmask for scan point i
   */
  unsigned char getFlags(int i) const;
  
  /**
   * @return number of intensity levels for each scan point
   */
  bool getNIntensityLevels() const { return m_NIntensityLevels; }
  
  /**
   * @return intensity level for scan point i
   */
  unsigned char getIntensity(int i) const;

  /**
   * Set the number of points in a scan.
   */
  void setNumberOfPoints(unsigned short num);
  
  /**
   * Set the type of scanner that produced this scan
   */
  void setScannerType(unsigned short t) { m_ScannerType = t; }

  /**
   * @param r maximum reading [m]. Can be used whe settings thresholds etc
   */
  void setMaxRange(double r) { m_MaxRange=r; }

  /**
   * @param r the step in rads
   */
  void setAngleStep(double r) { m_AngleStep=r; }
  /**
   * @param r the angle for ranges[0] in rads
   */
  void setStartAngle(double r) { m_StartAngle=r; }
  
  /**
   * Import scan in raw format
   */
  bool import (Timestamp timeStamp,
	      int nPts,
	      double *ranges,
	      double angleStep,
	      double startAngle=0,
	      double maxRange=82,
	      double rangeResolution=.05,
	      int nFlags=0,
	      int nIntenistyLevels=0,
	      unsigned char *flags = NULL,
	      unsigned char *intensities = NULL);
  
  /**
   * This functions takes an array of raw measurements from a SICK
   * sensor and translates it into ranges, intensities, etc. The
   * arrays that are to be filled in are assumed to be big enough
   */
  bool interpretMeas(//Inputs
		     bool PLS,
		     int nPts, 
		     const unsigned short *meas,
		     short partial,
		     short scanPart,
		     short scanVariant,
		     short measMode,
		     short measUnit,
		     short outUnit,
		     // Outputs
		     double &angleStep,
		     double &startAngle,
		     double &maxRange,
		     double &rangeResolution,
		     double *ranges,
		     int &intensityLevels,
		     int &nFlags,
		     unsigned char *intensities = NULL,
		     unsigned char *flags = NULL);
  
  /**
   * Debugging functions
   */
  bool createRandomMeasurement(double minRange, double maxRange);

  unsigned short checkClassType(unsigned char t);
  void setSubType(unsigned short t);
  
protected:
  /**
   * Set the SubType according to nPts ect.
   * 
   * The SubType for this is 0000 IFNN NNNN NNNN 
   * I = 1  if m_NIntensityLevels !=0;
   * F = 1 if m_NFlags !=0;
   * N is the m_NPts<722;
   * The lower 16 bits can be used to specify different standard types of 
   * 
   * I expect to add to this as I understand more.
   */
  void calcSubType();
  
private:
  unsigned short m_ScannerType;

  int m_NPts;  
  double m_Ranges[721];
  /** Rads per step (ie. between m_Ranges[i] and m_Rangers[i-1] */
  double m_AngleStep;
  /** Angle of Rangers[0] in rads */
  double m_StartAngle;
  /** Typically in meters */
  double m_MaxRange;
  /** Additional info on precision of ranges in m*/
  double m_RangeResolution;
  
  int m_NFlags;
  unsigned char m_Flags[721];
  
  int m_NIntensityLevels;
  unsigned char m_Intensities[721];

};

inline double SICKScan::getRange(int i) const
{
  if (0 <= i && i < m_NPts) return m_Ranges[i];
  else return -1;
}

inline unsigned char SICKScan::getFlags(int i) const
{
  if (0 <= i && i < m_NPts) return m_Flags[i];
  else return 0;
}

inline unsigned char SICKScan::getIntensity(int i) const
{
  if (0 <= i && i < m_NPts) return m_Intensities[i];
  else return 0;
}

} // namespace Cure

std::ostream& operator << (std::ostream& os, const Cure::SICKScan &scan);

#endif // SICKScan_hh
