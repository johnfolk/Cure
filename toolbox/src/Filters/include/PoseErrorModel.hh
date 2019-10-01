//    $Id: PoseErrorModel.hh,v 1.9 2007/09/14 09:13:51 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_POSEERRORMODEL_HH
#define CURE_POSEERRORMODEL_HH


#include "DataFilter.hh"
#include "DataSet.hh"
namespace Cure{

/**
 * This is a base class of an error model.
 * It has  1-in ports and  3-out ports.
 * 
 * port 0, Input 0 is the Cumulated Poses from some sensor (odometer, 
 * inertial, compass...). Its SubType must be (0### #### #### ####).
 *
 * port 1, Output 0 is the Incremental change in the cumulated frame at 
 * the pose of the last Calc(). Its SubType must be (1### #### #### ####).  
 * The independent incremental uncertainty in that change is also included.
 * 
 *
 * port 2, Output 1 is the last value of input 0 for cumulated Pose.  
 * Its SubType must be (0### #### #### ####). The Velocity is also 
 * calculated. 
 * 
 * 
 * port 3, Output 2 is a DataSet, p, that p(0) is Incremental change
 *         and p(1) is the cumulated Pose. 
 *        
 *
 * You can write to port -1 (or leave off the port)
 * as the trigger input. The pose that is written has
 * it's timestamp used to read Input 0 and calc.
 *
 * Pose Errors general info:
 * 
 * There are 2 pieces to the pose error.  One is do to 
 * the independent errors in the change in the pose (due to for example 
 * wheel slipage.  These errors accumulate.  The other type of error 
 * is due to quantization and timestamp errors.  This second error 
 * does not accumulate like the first one.  Instead the previous dependent
 * covariance should be subtracted.  Then the new incremental covariance
 * added.  And finally the new dependent covariance added.   
 *
 * @author John Folkesson
 */
class PoseErrorModel: public DataFilter
{
public:
protected:
  Pose3D Poses[3];
  DataSet OutSet;
public:
  PoseErrorModel();
  virtual ~PoseErrorModel(){}
  virtual int config(const std::string &params){return 0;}
protected:

  /**
   * We overload this function just to be able to handle the
   * initialization of the PoseErrorModel class.
   *
   * When we write the first pose to the model this pose must result
   * in a inc pose of 0 on output port 0 and not the difference
   * between 0 and the first pose we write. That is if we write pose
   * (3,4,0) as the first pose the increment from last (there is no
   * last) should be (0,0,0) and not (3,4,0).
   */
  virtual void precalc(int port);
  
  /**
   *
   * What calc does is to use Poses[0] (was read from port 0 above)
   * and Poses[2] (has the last calc() cumulated xytheta, port 2)
   * to calculate Poses[1], port 1 and finally PoseValues[2] port 2.
  */
  virtual unsigned short calc();
  virtual unsigned short typeCheck(int port, TimestampedData * inputData=0);  

  TimestampedData * data(int port) {
    if (port < 0 || port >= (Ins+Outs)) {
      std::cerr << "WARNING: PoseFilter::data port " << port 
                << " out of bounds must be >0 and <"
                << Ins+Outs << std::endl;
    }
    if (port<3)
      return Poses+port;
    return &OutSet;
  }

};

} // namespace Cure

#endif // CURE_ODOMETERMODEL_HH
