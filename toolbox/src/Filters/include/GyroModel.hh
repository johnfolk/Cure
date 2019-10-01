//    $Id: GyroModel.hh,v 1.10 2007/09/14 09:13:50 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_GYROMODEL_HH
#define CURE_GYROMODEL_HH


#include "PoseErrorModel.hh"

namespace Cure{

/**
 * This is an example of a Gyro model.
 * It has  1-in ports and  3-out ports.
 * It works for gyros with pitch and roll measurements adjusted
 * to the absolute vertical (using the gravity vector).
 * It adds the uncertainty in the change to theta.
 * 
 * port 0, Input 0 is the Cumulated Gyro theta phi, psi Poses.
 *
 * port 1, Output 0 is the Incremental change in rotation  
 *  since the last Calc().  
 * The uncertainty in that change is also included.
 *
 *
 * port 2, Output 1 is the last input 0 for Pose (adjusted for bias)
 * but the Covariance is just the dependent cumulated covariance.
 * 
 * port 3 Out 2 is A DataSet with outs 0 and 1 togther
 *
 * @see Cure::PosErrorModel for a disscussion of errors.   
 * 
 * You can write to port -1 (or leave off the port)
 * as the trigger input. The pose that is written has
 * it's timestamp used to read Input 0 and calc.
 *
 * Output 0 
 *  The 'delta pose' has a theta that is comparable to odometry;
 * 
 * 
 * 
 * 
 *
 *
 * @author John Folkesson
 */
class GyroModel: public PoseErrorModel
{
public:
  /**
   * This is the covariance of the angle theta per sec. 
   * The default is GyroErrorRate=1E-6;
   */
  double GyroErrorRate;
  /**
   *  This bias times the time interval will be subtracted.
   *  ThetaChange=(Theta-Prevtheta-ThetaBias*dt)*ThetaScaleBias).
   *  Postitive numbers makes the robot turn more to the right.
   */
  double ThetaBias;
  double ThetaScaleBias;
  /**
   *This is the estimated covariance in the Timestamp.
   */
  double TimeError;
  /**
   *This is the estimated absolute covariance in phi and psi.
   */
  double VerticalError;
protected:
  double PrevTheta;
public:
  /**
   * 
   * 
   */
  GyroModel();
  int config(const std::string &params);
  int configVer1(const std::string &params);
 virtual ~GyroModel(){}
protected:
  virtual unsigned short calc();
  virtual unsigned short typeCheck(int port,TimestampedData *inputData=0);  
  virtual void precalc(int port){}
};

} // namespace Cure

#endif // CURE_ODOMETERMODEL_HH
