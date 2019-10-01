//    $Id: SimpleOdoModel.hh,v 1.8 2007/09/14 09:13:51 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_SIMPLEODOMODEL_HH
#define CURE_SIMPLEODOMODEL_HH

#include "PoseErrorModel.hh"

namespace Cure{

/**
 * This is an example of an odometer model.
 * It models the drive system incremental errors.
 *
 *  The error is modeled as a covariance in distance s traveled and
 *  in theta the angle theta turned:
 * 
 * Covariance in s = Max (1E-10, s*CovPerMeter);
 * Covariance in theta = CovPerRad*theta + s*AngleDriftFactor + 5E-12. 
 * where r= s/theta for theta>.001 else r=100
 *
 * The correlation coefiecent between s and theta =(1/((1/r)+(4*r)));
 *  
 * A small error othogonal to these errors is added to make the 
 * 3x3 cov matrix full rank. 
 *
 * @author John Folkesson
 */
class SimpleOdoModel: public PoseErrorModel
{
public:
  /**
   * 
   * 
   */
  SimpleOdoModel();

  /**
   * The uncertainty in the distance [(m*m)/m]
   * @default 1E-4
   */
  double CovDistPerMeter;

  /**
   * The uncertainty in the angle [(rad*rad)/rad]
   * @default 1E-4 
  */
  double CovAngPerRad;

  /**
   * The uncertainty in angle per traveled distance [(rad*rad)/m] 
   * @default 2E-7
   */
  double CovAngPerMeter;


  /**
   * The uncertainty in the movement perpendicular to the direction of travel
   *  [(m*m)/m].
   * This is used to eliminate some effects of the linearization.
   * @default 1E-5
   */
  double CovNormalPerMeter;

  /**
   * The uncertainty in the position due to pure rotation [(m*m)/rad].
   * @default 1E-8;
   */
  double CovXYPerRad;   
  /** 
   * The drift in angle per traveled meter [rad/m]
   * 0 is no systematic angle drift when moving forward
   */
  double BiasAngPerMeter;

  /**
   * The drift in angle per rotated radian [rad/rad]
   * 0 is no systematic angle drift from turning
   */
  double BiasAngPerRad;

  /**
   * Scale factor that compansated for scale errors in distance. 
   * 0 corresponds to no scale error in distance traveled
   */
  double BiasDistPerMeter;

  /**
   * Configure the odometric model based on a set of string of parameters
   */
  virtual int config(const std::string &params);


protected:
  virtual unsigned short calc();
  virtual unsigned short typeCheck(int port,TimestampedData * inputData=0);  

protected:
  /** 
   * We have to remember the last pose we wrote on the input so that
   * we can calculate the difference since last
   */
  Pose3D m_LastPose;

  int configVer1(const std::string &params);
};
  
} // namespace Cure

#endif
