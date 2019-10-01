//    $Id: PoseFilter.hh,v 1.8 2008/05/13 20:29:00 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_POSEFILTER_HH
#define CURE_POSEFILTER_HH
#include "DataFilter.hh"
#include "DataFilterAddress.hh"
#include "Pose3D.hh"


#include <iostream>
namespace Cure{



/**
 * PoseFilter is a processor of pose data. 
 * It has an array of Pose3D's (Poses) associated with its Ports.
 * 
 * Calling Push on these Ports will cause a reference to an element 
 * in this array to be writen (Not A Copy).
 * One should therefore read/copy the returned values but not change the
 * object pushed, (ie. don't use it as a temp Pose varible). 
 * And copy it before calling any 'external' read/writes
 * This policy is more efficient but requires trust.
 * This is easy to override by changing the Data(i) method
 * to copy the member to a workPose array each time its
 * called.
 *
 * @author John Folkesson
 */
  class PoseFilter: public Cure::DataFilter
{
public:
  Pose3D *Poses;
public:
  /**
   * 
   * 
   */
  PoseFilter( short nIn, short nOut,unsigned short thread=0);
  virtual ~PoseFilter();



protected:
  TimestampedData * data(int port) {
    if (port < 0 || port >= (Ins+Outs)) {
      std::cerr << "WARNING: PoseFilter::data port " << port 
                << " out of bounds must be >0 and <"
                << Ins+Outs << std::endl;
    }
    return Poses+port;
  }
  /**
   * A conditional set of the value of an input ports data object.
   * This will set Poses[port]=inputData if InputData is the right type.
   * The default right type just checks if it can be made into a Pose3D.
   * If inputData==0 this does nothing and returns 0.
   * @param port the port for the data.
   * @param inputData the data to check type of.
   * @return 0 if ok else TYPE_ERROR.
   */
  virtual unsigned short typeCheck(int port,TimestampedData *inputData=0);
  };
  
} // namespace Cure

#endif // CURE_POSEFILTER_HH
