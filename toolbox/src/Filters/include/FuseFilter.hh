//    $Id: FuseFilter.hh,v 1.7 2007/09/14 09:13:50 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_FUSEFILTER_HH
#define CURE_FUSEFILTER_HH

#include "PoseFilter.hh"

namespace Cure{
  /**
 * This Base class takes its inputs from two Models  and outputs a 
 * dead-reckoning fused incremental pose and a cumulated dead-reckoning pose. 
 * This class has 4 inputs 3 outputs.
 *
 * Input 0, port 0: Incremental pose with uncertainty 
 *                  (output 0, port 1 from a PoseErrorModel).
 * Input 1, port 1: Cumulated pose with uncertainty 
 *                  (output 0, port 2 from an PoseErrorModel).
 * Input 2, port 2: Incremental pose with uncertainty 
 *                  (output 0, port 1 from a PoseErrorModel).
 * Input 3, port 3: Cumulated pose with uncertainty 
 *                  (output 0, port 2 from an PoseErrorModel).
 *

 * Output 0, port 4: Rotation is an additional pose to bring you 
 *                   from the cumulative deadreckoning to the robot frame.
 *                   Typically this the the pitch and roll component.
 *
 * Robot(t) = Cumulated(t) + Robot_Rotation(t).
 *
 * Output 1, port 5: Incremental deadreckoning with uncertainty,
 * This is the incremental pose in the robot frame at the begining of the 
 * interval.  It is not the change in x y z...!
 * get a dead reckoning estimate, 
 *
 * Cumulated(t) = Cumulated(t-1) + Incremental(t).
 *
 * Output 2, port 6: Cumulated deadreckoning with uncertainty,
 * 
 *
 * SUMMARY OF USE  
 *
 *  This expects to be able to pull data from at least three of its
 *  inputs.  The fourth can also be set to pull and a trigger used to
 *  start the calculation.  Alternatively one of the inputs can be
 *  written to to start the calculation, in which case, that input need
 *  not be set to pull data.  The calc function needs to be added by a subclass
 * that will actual create a fused estimate of the two inputs.
 *
 *  The output is given a three poses the cumulated pose is the
 *  position of the robot.  It is this pose that the increments are
 *  cumulated into.  The Incremental pose is the changes to the
 *  cumulative pose.  The errors here are what cause dead reckoning
 *  to have unbounded cumulative errors.  The Robot_Rotation is an
 *  extra piece to the robot pose at a given time that is not
 *  cumulated into the subsequent estimates.  So the inertial sensor
 *  may give us the pitch and roll by using the earth's gravitation as
 *  an absoulte reference or this might be some extra pan of the robot
 *  outer body with respect to its chassi that the wheels are attached
 *  to. This rotaion will have errors but they do not accumulate as the
 *  incremental errors do.
 *
 * @author John Folkesson
 */
class FuseFilter: public PoseFilter
{
protected:
public:	      
  /**
   * The constructor.
   */
   FuseFilter();
  virtual ~FuseFilter(){}
protected:
  /**
   * This is just a shell one needs to subclass this and write the
   * calc and set the Pose Types in the constructor...
   */
  virtual unsigned short calc();
  virtual unsigned short typeCheck(int port,TimestampedData *inputData=0);  
};


} // namespace Cure

#endif // CURE_FUSEFILTER_HH
