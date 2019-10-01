//
// = LIBRARY
//
// = FILENAME
//    SensorUser.hh
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

#ifndef SENSORUSER_HH
#define SENSORUSER_HH


#include "DataFilter.hh"
#include "SensorOffset.hh"
#include "SmartDataList.hh"
#include "DataSet.hh"
namespace Cure {


/**
 * This class is intended to be a base class that can provide
 * estimates of the sensor pose to be used with the some sensor data.
 * The pose estimate is automatically interpolated to the timestamp of
 * the sensor data and the update is called when the sensor pose and
 * sensor data are both available.  Predict can be used to make any adjustments
 * needed in the subclass to bring its state up to the time of the sensor data.
 * 
 *
 * Inputs:
 *      0 Must be set to pull from an address that has the dead reckoning
 *        of the robot pose.  It expects this to be in the form of a DataSet 
 *        as Cure::DeadReckoner.out(3) 
 *        
 *        So the DataSet can contain three different Pose3D objects: 
 *       (0.0) The extra rotation of the robot, (Output 0 of a Fuser), 
 *       (0.1) the incremental dead reckoning with uncertainty. (output
 *             1 from a fuser)
 *       (0.2) the cumulated dead reckoning with uncertainty. (output 2
 *             from a fuser)
 *       (0.3...) (optional) could be anything you want, for instance 
 *             the acceleration measurement with uncertainty GenericData. 
 *             
 *          
 *      1 sequenced Data.  These trigger the calculation. They can be 
 *        MeasurementSets, SensorData or DataSets of SenorData that you
 *        want to update the model with.
 *      2 Command input do what you like with the commands.
 *    
 * 3 to (3+NumberOfSensors) set to pull the sensor offsets from a 
 *        Slot.  Alternatively, if one has static offsets one can
 *        simply write the offset one time to this port and that value 
 *        will be used. 
 * >NumberOfSensors+3 Defined in subclass.
 *      
 * Outputs:
 *       0 is the last value of input 0.0 for 'Extra Robot Rotation'.    
 *       1 is the Incremental change in the deadreckoning frame at 
 *         the time of the last update.  
 *         Its SubType will be (1### #### #### ####).  
 * 
 *       2 is the value of input 0.2 for cumulated dead reckoning Pose at  
 *          the time of the last update.  
 *
 *       3 Simply the values pulled from 0 last in a DataSet.
 *       >3  Defined in subclass
 *
 *
 * SUMMARY OF USE
 *
 *  The data is typically pushed to input(1) in time order.  It is
 *  then put in a queue of pending sensor data. Then the queue is
 *  serviced.  The sensorID is compared to the configured sensors for
 *  the inputs >2 and the SensorPose is calculated.  If for some reason
 *  the sensor pose is not yet available the calculation is postponed.
 *  Otherwise predict and then update are called.  These are
 *  implemented by the subclass.  The predict will use the new pose
 *  estimates DeadInc ect. to bring the internal state of the filter
 *  up to the new location.  Then update incorporates the new sensor
 *  data into the filter state.
 * 
 * @author John Folkesson 
 * @see GridMaker for and example.
 */
class SensorUser : public DataFilter {
  
public:
  /**
   * Setting the covariance type of the SENSORPOSE in the cfg file will
   * Allow the offset from the robot to the sensor to be included in
   * the robot state.
   *
   * @param numberOfSensors The number of separate sensors each will get 
   *        its own offset pose.
   * @param measurmentSetSize the maximum number of measurements to
   *        that will be written to the filter.   
   * 
   */
  SensorUser(short numin=4, short numout=4,
	     short numberOfSensors=1,
	     short measurementsetSize=100,
	     const unsigned short thread=0);
  ~SensorUser();

  /**
   * This will return a Pose.
   *@return some Pose that is defined in subclass
   */
  const Pose3D& getPose();
  void setMinPendingTime(double d);
  virtual int configure(ConfigFileReader &cfg, std::string  &filtername);
  virtual int config(const std::string &params);
  virtual int configGlobal(const std::string &params,
			   StateData &p);
  virtual int configSensor(const std::string &params,
			   const std::string &sensordef);
  /** 
   * Here the subclass binds a short to the ccf string that will later be 
   * matched to input data coming in to in(1);
   */
  virtual short getMeasurementClass(std::string &){return 0;}
  /**
   * This is the memory of what was configuured for this measurement data
   */
  virtual short getMeasurementClass(DataDescriptor &d, 
				    unsigned short measurementType);

protected:
  /** 
   * This has to be implemented to use a PredictFilter 
   * It works as is for a DeadReckoner.  The PredictFilter
   * expects you to put the relavent parts of the state in a dp(4).
   * @seePredictFilter
   */
  virtual void getState(Cure::DataSet &dp){
    dp.setSetSize(3);
    dp.setup(0, POSE3D_TYPE);
    dp.setup(1, POSE3D_TYPE);
    dp.setup(2, POSE3D_TYPE);
  } 

  /**
   * Write data to the filter 
   *
   */
  virtual unsigned short write( Cure::TimestampedData& p,const int port=-1);
  virtual unsigned short read(Cure::TimestampedData& result,const int port, 
			      const Cure::Timestamp t=0,
			      const int interpolate=0);
  /**
   * This is called when a new Command is written to in(2)
   * @return the value is returned to the caller of write on in(2)  
   */
  virtual unsigned short processCommand(){return 0;} 
  /**
   * This can bring the subclass state up to the time of 
   * the sensor data.  This is callsed by getDead().
   *
   */
  virtual int predict(){  return 0;}
  /**
   * This can do the main work of the subclass using the sensor data and
   * sensor pose to do wome calculation.
   *  This uses the input sensor data and the predict poses to do the update.
   *
   * So you have the time of SensorPose and PoseOut correct and the
   * input data in either MSet (if it was a MeasurementSet,
   * or just Data[1]
   * otherwise.
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
  virtual int update(){return 0;}
  /**
   * This can be overridden to have input address to reject pull calls.
   * The default is to not allow Pull on in(1) and the outputs. 
   */
  virtual bool canPull(const int port);

  virtual TimestampedData * data(int port) {
    if (port<Ins)
      return &Data[port];
    return Data[port].getTPointer();
 }
  /**
   * Find the index of the sensor for the current Data(1).
   * @return the sensor index.
   */
  virtual int  getSensorIndex();

  /**
   * This is what copies the data from inputData to data(port)
   * 
   */
  virtual unsigned short typeCheck(int port,TimestampedData * inputData=0);

  /**
   * This does a predict cycle getting the dead reckoning first.
   * After calling this the robot to sensor Pose is in RobotSensorPose;
   * and the DeadInc, DeadCum and XtraRot are set.
   * Then predict is called.
   * The predict should set Poseout.
   *
   *
   * @param t the time to calc dead reckoning to.
   * @ind the Senor index to get the offset for.
   *
   */
  virtual int getDead(Timestamp &t, int ind);

  virtual void setPose(Pose3D &){}
protected:
  ShortMatrix m_StateID;
  Timestamp YoungestTime;
  SmartDataList  PendingDataList;
  Timestamp LastTime;
  Timestamp *LastSensorTime;
  short *SensorIndex;
  unsigned short *SensorPoseCovType;
  SmartData * Data;
  /** Descriptor for the current Input data. */
  DataDescriptor Descriptor;
  ShortMatrix MeasurementIndex;
public:
  /** If true the measurments that come after MinPendingTime will be discarded*/
  bool m_UpdateSequentially;  
  /**
   * If true the filter will allways reset the robot to sensor pose and its 
   * Covariance.  Even when no new information on it came in.
   * This might be ok if you expect for instance vibrations to be the 
   * main source of uncertainty.
   */
  bool *AllwaysPredictSensor;
  /**
   * If Measurements sets are input they will be put here before
   *  calling update.
   */
  MeasurementSet MSet; 
  /**
   * Input DataSet of poses, input 0, ends up here.
   */
  DataSet PoseDataSet;
  /**
   * Contains the dead-reckoning estimate of the change in robot pose
   * between two predict calls.  Includes the uncertianty
   */ 
 Pose3D DeadInc;      
  /**
   * Contains the dead-reckoning estimate of  robot pose
   * at the current time,
   */ 
  Pose3D DeadCum;      
  /**
   * Contains the estimate of any additional tranformation of the
   * robot pose between the DeadCum estimate and the RobotSensorPose.
   * This is the same for all sensors
   */ 
  Pose3D XtraRot;      
  /**
   * For Pose providers to use as output 
   */
  Pose3D PoseOut;
  /**
   * Holds the last inputed robot to sensorposes,  The ith sensor is at
   * sensor=DeadCum+XtraRot+RobotSensorPose[i].
   */  
  SensorPose *RobotSensorPose;

  /* 
   * Normally true unless the config has set to false.
   * If false the filter will look for a m_PoseInitService to initialize before 
   * processing any data.
   */
  bool m_Initialized;
  std::string m_PoseInitService;
  SensorOffset *m_Sensors;
  /**
   * The number of sensors being used
   */      
  short  NumberOfSensors;
  /** The index to the deadService */
  unsigned long    m_DeadService;
  /** 
   * If m_UseDeadService the DeadService will be used in place of
   * prediction/Deadreconing
   */
  bool m_UseDeadService;
  /** Configuring the m_DeadServiceName will set m_UseDeadService to true*/
  std::string   m_DeadServiceName;
  /** This is used as the interpolate flag when reading the dead service*/
  unsigned short m_DeadFlag;
  /** 
   * This sets the length in time for the pending list.  The default
   * is 0, measurements are processed as soon as there is
   * deadreckoning.  By setting this >0 one can cause the prediction
   * phase to be delayed untill all measurements are in.  Once the
   * prediction is done one can not use the measurments from earlier
   * times, (at least for a Kalman filter.)  Set it to the longest
   * delay beween different measurements.
   */
  Timestamp MinPendingTime;
  /** This is 0 by default and is used when pulling data from Ports[0]*/
  int PredictInterpolate;
protected:
  virtual short getMeasurementClass(unsigned short mtype)
  {return getMeasurementClass(Descriptor,mtype);}
}; 

inline const Pose3D&
SensorUser::getPose() { return PoseOut; }
} // namespace Cure

#endif // SensorUser_hh
