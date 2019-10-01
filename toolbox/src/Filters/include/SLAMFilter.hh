//
// = LIBRARY
//
// = FILENAME
//    SLAMFilter.hh
//
// = FUNCTION
//
// = AUTHOR(S)
//    John Folkesson
//
// = DESCRIPTION
//    
// = COPYRIGHT
//    Copyright (c) 2005 John Folkesson
//
/*----------------------------------------------------------------------*/

#ifndef SlamFilter_HH
#define SlamFilter_HH
#include "PoseCumulator.hh"
#include "SmartDataList.hh"
namespace Cure {


/**
 *                 DEPRECIATED Use SensorUser
 *
 * This class encapsulated the filters needed to build a map from
 * dead reckoning and measurment data. 
 *
 * Inputs:

 *      0 The extra rotation of the robot, (Output 0 of a Fuser), 
 *        The data should be in a DataSlot and this port set to pull.
 *        If not set to pull or if the data is not updated.
 *        the latest data will be used (ie do nothing if you don't
 *        want any extra rotation after dead reckoning).
 *      1 Must be set to pull from a DataSlot that has the incremental
 *        dead reckoning with uncertainty. (output 1 from a fuser)
 *      2 Must be set to pull from a DataSlot that has the cumulated
 *        dead reckoning with uncertainty. (output 2 from a fuser)
 *      3 sequenced MeasurementSet.  these trigger the calculation.
 * 4 to (3+NumberOfSensors) set to pull the sensor offsets from a 
 *        Slot.  Alternatively, if one has static offsets one can
 *        simply write the offset one time to this port and that value 
 *        will be used. 
 * >NumberOfSensors+3 Defined in subclass.
 *      
 * Outputs:
 *       0 is the last value of input 0 for 'Extra Robot Rotation'.    
 *       1 is the Incremental change in the deadreckoning frame at 
 *         the time of the last update.  
 *         Its SubType will be (1### #### #### ####).  
 * 
 *       2 is the value of input 2 for cumulated dead reckoning Pose at  
 *          the time of the last update.  
 *
 *       3 The last updated SLAM pose of the robot (ie the corrected Output 2).
 *      >3 Defined in subclass.
 *
 *
 * 
 * @author John Folkesson 
 * @see
 */
class SLAMFilter : public DataFilter {
  
public:
  /**
   * Setting the covariance type of the SENSORPOSE in the cfg file will
   * Allow the offset from the robot to the sensor to be included in
   * the EKF state.
   *
   * @param numberOfSensors The number of separate sensors each will get 
   *        its own offset pose.
   * @param measurmentSetSize the maximum number of measurments to
   *        that will be written to the filter.   
   * 
   * 
   * 
   */
  SLAMFilter(short numin=5, short numout=4,
	     short numberOfSensors=1,
	     short measurementsetSize=100);
  ~SLAMFilter();

  /** 
   * @return The Pose of the robot. NOTE that this might
   * not correspond to the current physical position of the robot.
   */
  const Pose3D& getPose();

protected:
  /**
   * Write data toe the filter 
   *
   */
  virtual unsigned short write( Cure::TimestampedData& p,const int port=-1);
  virtual unsigned short read(Cure::TimestampedData& result,const int port, 
			      const Cure::Timestamp t=0,
			      const int interpolate=0);
 
  /**
   * This take  SensorPose ... MSet
   *  m_Kalman->incrementalPredict(PoseOut, DeadInc);     
   *  PoseOut.Time = DeadInc.Time;
   *
   */
  virtual int predict(){  return 0;}
  /**
   *  This uses MSet, SensorPose, and PoseOut to do the update.
   *
   * Some outline:
   * ind=getSensorIndex()
   * m_Kalman->offsetPredict(SensorPose,ind); 
   * ...
   * Match MSet to map
   * ...
   * m_Kalman->updateSensor(fd->m_Matches, MSet.getNumberOfElements(), 
   *			       PoseOut, map2info, fd->m_Mahalanobis,ind);
   *
   */
  virtual int update(){    return 0;}

  virtual bool canPull(const int port);

  /**
   * Find the index of the sensor for the current measurement.
   * @return the sensor index.
   */
  virtual int  getSensorIndex();

  TimestampedData * data(int port) {
    if (port<Ins)
      return &Data[port];
    return Data[port].getTPointer();
 }
  /**
   * This is what copies the data from inputData to data(port)
   * 
   */
  virtual unsigned short typeCheck(int port,TimestampedData * inputData=0);

  /**
   * This does a predict cycle getting the dead reckoning first.
   * After calling this the robot to sensor Pose is in SensorPose
   * and the DeadInc and DeadCum are set.
   * predict is called.
   * The predict should set Poseout.
   *
   *
   * @param t the time to cumulate dead reckoning to.
   * @ind the Senor index to get the offset for.
   *
   */
  int getDead(Timestamp &t, int ind);

protected:

  SmartDataList  PendingDataList;
  Timestamp LastTime;
  Timestamp *LastSensorTime;

  int *SensorIndex;
  SmartData * Data;
  PoseCumulator m_Cumulator;
public:

  /**
   * If true the filter will allways reset the robot to sensor pose and its 
   * Covariance.  Even when no new information on it came in.
   * This might be ok if you expect for instance vibrations to be the 
   * main source of uncertainty.
   */
  bool *AllwaysPredictSensor;
  MeasurementSet MSet; 
  Pose3D DeadInc;      
  Pose3D XtraRot;      
  Pose3D DeadCum;      
  Pose3D PoseOut;
  //holds the last inputed robot to sensorposes
  SensorPose *RobotSensorPose;
  /**
   * Max 5 set in constructor
   */      
  short NumberOfSensors;

}; 

inline const Pose3D&
SLAMFilter::getPose() { return PoseOut; }

} // namespace Cure

#endif // SLAMFilter_hh
