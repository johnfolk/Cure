//    $Id: PanTiltModel.hh,v 1.6 2007/09/14 09:13:51 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_PANTILTMODEL_HH
#define CURE_PANTILTMODEL_HH


#include "PoseErrorModel.hh"

namespace Cure{

/**
 * This is an example of a PanTilt model.
 * It has  1-in ports and  3-out ports.
 * It works for gyros with pitch and roll measurements adjusted
 * to the absolute vertical (using the gravity vector).
 * It adds the uncertainty in the change to theta.
 * 
 * port 0, Input 0 is the Cumulated PanTilt theta, psi Poses.
 *
 * port 1, Output 0 is the Incremental change in theta in 
 * the Earth frame since the last Calc().  
 * The uncertainty in that change is also included.
 * The phi and psi are left at the cumulated values 
 *
 * port 2, Output 1 is the last input 0 for Pose
 * but the Covariance is just the dependent cumulated covariance.
 * 
 * Out(2) is a DataSet with out(0) and out(1)
 * 
 * The pan axis is taken as z while the tilt axis is x.
 *
 * @see PoseErrorModel for a disscussion of errors.   
 * 
 * You can write to port -1 (or leave off the port)
 * as the trigger input. The pose that is written has
 * it's timestamp used to read Input 0 and calc.
 *
 *
 * @author John Folkesson
 */
class PanTiltModel: public PoseErrorModel
{
public:
  /**
   *This is the estimated covariance in the Timestamp.
   */
  double TimeError;
protected:
public:
  /**
   *
   *
   */
  PanTiltModel();
  virtual ~PanTiltModel(){}
protected:
  virtual unsigned short calc();
  virtual unsigned short typeCheck(int port,TimestampedData *inputData=0);  
};

} // namespace Cure

#endif // CURE_ODOMETERMODEL_HH
