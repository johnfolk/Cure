// = RCSID
//    $Id: RangeHough.hh,v 1.11 2007/09/14 09:14:05 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
// = COPYRIGHT
//    Copyright (c) 2004 John Folkesson
//    
#ifndef CURE_RANGEHOUGH_HH
#define CURE_RANGEHOUGH_HH

#include "SICKScan.hh"
#include "MeasurementSet.hh"
#include "Point2DCloud.hh"
#include "Matrix.hh"
#include "Trig.hh"

#include <string.h>
#include <iostream>
#include <string>

namespace Cure {
  /**
   * This does a fast line extraction from a SICKScan using a two stage 
   * range weighted hough transform followed by a 'grow line from seed'.
   * The first rough hough has an 8 degree gamma increment and 32 rho 
   * levels.  The max from that stage is fed to the second stage with
   * one degree and 8*32=256 rho levels.  These points are then formed
   * into continuous segments with no more than 2 m gaps and 'good' linearity.
   * The sick scan must have angles that are multiples of half degrees.
   *
   * An idea of speed: on Pluto (dual pentium II) with 181 scan segments
   * (1 degree), calling setThresholds(1,3,60,2).
   * This found an average of 9.6 lines per scan and took 6.2 msec 
   * per scan (7500 scans in test set).  
   * Setting the Max Range lower or min length higher
   * would speed this up as would finding tracked walls before 
   * looking for new ones.  For example with setThresholds(2,3,30,2), we
   * get 2.5 lines per scan and took 2.9 msec per scan.
   *
   * @author John Folkesson
   *   
   */
class RangeHough 
{
 public:
  /** 
   * The min allowed length*length for a line (units m^2).
   * @default 1
   */
  double LengthSqThreshold;

  /** 
   * The scans> MaxRange will be ignored with higher resolution
   * for the remaining points.
   * @default 60 meters
   */
  double MaxRange;

  /** 
   * The max allowed distance of points from line, remove outliers.
   * @default .09 m
   */
  double RhoThreshold;

  /** 
   * The max allowed variance in distance of points from the line, 
   * final check.
   * @default .0027 m^2
   */
  double VarRhoThreshold;

  /**
   * Half laser beam width used to adjust the points after finding the lines.
   *
   * @default .0.0075,  [rad]
   */
  double HalfWidth;

  /**
   * The threshold used when checking for bias in rho (units m). When calculate
   * the square root of the sum of drho(i)*drho(i-1) divided by
   * (N-1), where drho is the distance from the point to the line. The
   * idea is that points that are on the same side of the line it will
   * give a positive result that we can put a threshold on.
   * 
   * If <0 no check for bias is done
   * @default -1 no check
   */
  double RhoBiasThreshold;

  /**
   * The parameter that is used for testing for an endpoint.
   * The first scan point not on the line must be this far behind
   * the line.  Each successive point must back off an additional 
   * EndPointBehind distance up to four scans points and .5 meters
   * away from endpoint.
   *
   * @default  .05 meters
   */
   double  EndPointBehind;
  
  /**
   * Here you can find the Lines after calling findLines.  The subType
   * will be set to NumberofLines.  This can be written to an address.
   */
  MeasurementSet Lines;


  Trig Trigger;
  Matrix Points;
  /**
   * This is set with setScan(SICKScan *s), (or findLines(SICKScan *s).
   */
  SICKScan *Sick;
 
 /**
   * This will equal Lines.getSubType().
   */
  short NumberOfLines;      

protected:
  Point2DCloud Cloud;
  Matrix Accumulator1;//32x45  8 degree
  Matrix Accumulator2;//8x8    1 degree
  short ScanIndex[361];
  double ScaledScan[361];
  short Rho1[8303];  //361x23
  short Rho2[2888];  //361x8
  int MaxGamma1;
  int MaxRho1;
  double *LinePointsPtr;
  int MaxR2;
  int MaxC2;
  short Acc1Result[361];
  short Acc2Result[361];
  short LineResult[361];
  short Count1;
  short Count2;
  short LineCount;
  short Gamma;
  int NumberOfPoints;
  short IndexIncrement;
  short BottomIndex;

  /**
   * This is the Weight threshold for the Hough stages, bigger is faster.
   */  
  double WeightThreshold; 

  /**
   * This is used to set a minimum variance for the line.
   * It represents the estimated variance in the scanpoint's position 
   * perpendicular to the laser beam at 1 meter range.  
   * Sets how much to increase the intrinsic error in the scan point location
   * with average distance square to the line's points (units m^2 per m^2)
   * The statistical variance is compared to the variance derived from 
   * this sensor model and the maximum of the two is used.
   * @default 4E-6
   */
  double VarRange;

  /**
   * This is used to set a minimum variance for the line.
   * Sets the intrinsic error in the scan point location orthogonal to
   * the range dependent error.
   * (units m^2).
   * @default .0001
   */
  double SensorVariance;

  /** 
   * The min allowed number of points for a line, > 2.
   */
  short CountThreshold;  

  /**
   * Max allowed gap between two points along a line not to split the
   * line into two
   */
  double MaxGap;  

  double AdjustedWeightThreshold;
  double DGamma;
  short Acc1Indecies[1440];
  short Acc1Count;
  short Acc2Indecies[64];
  short Acc2Count;
 public:
  RangeHough();
  void printConfiguration();

  /**
   * Function that can be used when configuring this component from a
   * configuration file with a string of parameters
   *
   * @return 0 if ok, else error code
   */
  int config(const std::string &arglist);
		     
  /**
   * This will set the thresholds according to a minimum length
   * and min number of points to a line.  
   * @param minlength This is the min length for a line [m]. (1m default)
   * @param minPointsPerLine This is the least number of scan points for
   * a line (10 default)
   * @param rhoThreshold max allowed distance between points and estimated 
   * line [m].
   * @param varRhoThreshold max allowed variance in the rho parameter [m^2]
   * @param halfWidth variable for compensation of beam width [rad]
   * @param dGamma param used when growing the line. It is used to allow for 
   * larger variations in rho when poinst are further apart without having
   * to increase RhoThreshold [rad]
   * @param maxSegmentGap maximum allow gap between two point along the line
   * not to split it in two [m]
   * @param maxrange The scan ranges larger than 31.5/32 times this are 
   * ignored. [m]
   * @param scanResHalfdegrees The spacing of the scan points in units of 1/2 
   * degree.
   */
  void setThresholds(double minlength, short minPointsPerLine, 
                     double rhoThreshold, double varRhoThreshold,
                     double halfWidth, double dGamma, double maxGap,
                     double maxrange,short scanResHalfDegrees);


  /**
   * Set the parameter that is used for testing for a bias in
   * rho. This can be used in a simple test to determin if the
   * structure that we are fitting a line to is in reality an arc for
   * example.
   * 
   * We threshold sqrt(Sum(drho(i)*drho(i+j))/(N-1))
   *
   * @param rhoBiasThreshold threshold for discarding a line with a
   * rho bias. If <0 no check is done (the default).
   */
  void setBiasThreshold(double rhoBiasTreshold);

  /**
   * Set the parameter that is used for testing for an endpoint.
   * The first scan point not on the line must be this far behind
   * the line.
   * 
   *
   * @param endpointBehind 
   * 
   */
  void setEndpointThreshold(double endpointBehind){
    EndPointBehind=endpointBehind;
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
  void setSensorVariance(double sensorVariance, 
			 double variancePerSquareRange=-1) {
    if (variancePerSquareRange>=0)VarRange=variancePerSquareRange;
    SensorVariance=sensorVariance;  
  }

  /**
   * This increases (or decreases for factor<1) the quality threshold for 
   * the lines.
   * @param factor one is no change, >1 makes it harder to find lines,
   *               <1 makies it easier. 
   */  
  void increaseLineQuality(double factor);

  /**
   * This decreases (or increases for factor<1) the amount of clutter,
   * (unwanted small lines). 
   * @param factor one is no change, >1 makes it harder to find lines,
   *               <1 makies it easier. 
   */  
  void removeClutter(double factor);

  /**
   * This increases (or decreases for factor<1) the parameters
   * for the sensor error model which set the minimum covariance for the 
   * lines.  
   * @param factor one is no change, >1 makes the lines error estimate larger,
   *               <1 makies it smaller. 
   */  
  void increaseMinCovariance(double factor);


  /**
   * Sets the Sick scan and resets everything.
   * Set a scan only once, (or just call findlines once with a scan)
   * as it resets all saved info each time you call it.
   * 
   * @param s  The sick scan must have angles that are multiples 
   * of half degrees.
   */
  void setScan(SICKScan *s);

  /**
   * Finds all the lines in the scan that meet the threshold requirements.
   * The result is found in this->LineKeys, a short array that hoslds the keys
   * identifing the seperate lines found in the scan, so that LineKey[i]==key
   * means that scan point (*s).(i) is part of the line identified by 'key'.
   * The key of -1 indicates that this point is not part of any line.
   *
   * Give the scan in findlines only once, (or call setScan first once)
   * as it resets all saved info each time you call it.
   * 
   * @param s  The sick scan must have angles that are multiples 
   * of half degrees, the default used the SICKScan previously set.
   * @return The number of lines found.
   */
  int findLines(SICKScan *s=0);

  /**
   * Finds all the lines in the scan between startindex and 
   * endindex that have a gamma and rho near the given gamma and rho.
   * This adds these lines to LineKeys and returns the key of the
   * line closest to gamma and rho where the distance in gamma
   * is divided by dgamma, then squared and added to the similar rho number.
   * If this sum of square errors is <1 the match is considered good enough,
   * otherwise the line will be kept but the key will not be returned. 
   *
   * Calling this can speed up findLines() by removing these known lines
   * from the data.  You need to call setScan(s) before this but only 
   * once per scan.
   *
   * @param startindex the first index into Sick to limit the search to. 
   * @param endindex the last index into Sick to limit the search to. 
   * @param gamma The angle in rads of the normal to the line measured
   * from the x axis. 
   * @param rho the estimated perpendicular distance to the line.
   * @param dgamma the max error of the gamma value.
   * @param drho themax error of the rho value.
   * @return The key of the best match, -1 if no match found.
   */
  short findLine(short startindex, short endindex, double gamma, 
		 double rho, double dgamma, double drho);

  /**
   * This adjusts the xy of the scan for finite beam width, (HalfWidth) based on the
   * lines found in the scan.  Each row of the returned Matrix cooresponds 
   * to a scan range point.
   * @param linePoints An array of NumberofPoints x 2 of adjusted xy's 
   */
  void adjustsPoints(Matrix &linePoints);
 protected:

  int configVer1(const std::string &arglist);

  /**
   * This adjusts the Line's points to be the intersection of each scan arc with
   * the fit line, iterates twice.
   */
  void adjustsLinePoints();

  void setMaxRange(double maxrange);
  void reset1();
  void reset2();

  /**
   * Takes the points from Sick and uses them to fill 
   * accumulator1.
   */
  void fillAcc1();
  /**
   * Finds the max of Acculator1.
   * @return 1 if Acc1Result max is below Threshold1
   *        -1 if the max had too few points,try again.
   *         0 if success, Acc1Result/Count1 contain the info to fillAcc2. 
   */
  int findMax1();
  /**
   * Takes the points from max of Accumlator1 and uses them to fill 
   * accumulator2.
   */
  void fillAcc2();
  /**
   * Finds the max of Acculator 2.
   * @return 1 if Acc2Result max is below Threshold2
   *        -1 if the max had too few points,try again.
   *         0 if success, Acc2Result/Count2 contain the info to try fit line. 
   */
  int findMax2();
  /**
   * @return 1 if Acc2Result contains too few points
   *        -1 if failed to grow a satisfactory line from Acc2Result[0], 
   *        try again.
   *        -2 successfully grew line but it failed bias test, discard points
   *         0 if success, LineResult/Cloud contain the Line. 
   */
  int fitLine();
  short getKey();
  /**
   * Takes the points from the found line and removes them from the 
   * accumulators
   */
  void emptyAccumulators(short key);
};
}
#endif
