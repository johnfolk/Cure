//    $Id: GyroAdjOdometry.hh,v 1.8 2007/09/14 09:13:50 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2005 John Folkesson
//    

#ifndef CURE_GYROADJODOMETRY_HH
#define CURE_GYROADJODOMETRY_HH

#include "PoseFilter.hh"
#include "FuseFilter.hh"

namespace Cure{
  
/**
 * This takes its inputs from an OdometerModel and GyroModel and outputs a 
 * dead-reckoning fused incremental 2D pose, a fused cumulated 2D  pose and
 * a rotation in the earth frame cooresponding to the pitch/roll. 
 * The fused output is based on the odometer xy change and the fusion of the
 * two estimates of the theta change based on there relative covariances.
 * The phi and psi are taken as the values given by the gyros.  The 
 * incremental pose has a covariance in x-y-theta.  This is in the robot 
 * frame. 
 *
 * This class has 4 inputs 3 outputs.
 *
 * output 0 is Rot(k) Rotation from Cum(k) to the robot frame (phi/psi) 
 *             with uncertianty
 * output 1 is Inc(k) it has xyztheta changes in robot frame with uncertianty 
 * output 2 is Cum(k) it has xyztheta from the start position
 *
 * So Cum(k)=Cum(k-1)+Inc(k) 
 *
 * and Robot(k)=Cum(k)+Rot(k) 
 *
 * @see PoseFilter.
 * @see FuseFilter.
 * inputs 0 and 1 should be a xytheta type odometer error model
 * inputs 2 and 3 should look like theta phi psi gyro model with 
 * phi and psi given in the absolute frame (gravity vector is measured)
 * 
 * @author John Folkesson
 */
class GyroAdjOdometry: public FuseFilter
{

protected:
public:

  /**
   * 
   * 
   * 
   */
  GyroAdjOdometry();
  virtual ~GyroAdjOdometry(){}
 
protected:

  virtual unsigned short calc();
  virtual unsigned short typeCheck(int port,TimestampedData *inputData=0);
};


} // namespace Cure

#endif // CURE_FUSEFILTER_HH
