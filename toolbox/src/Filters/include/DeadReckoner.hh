//    $Id: DeadReckoner.hh,v 1.12 2007/09/16 09:04:42 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef Cure_DeadRecckoner_hh
#define Cure_DeadRecckoner_hh

#include "PoseErrorModel.hh"
#include "DataSlotAddress.hh"
#include "FuseFilter.hh"
#include "PoseCumulator.hh"
namespace Cure{


  
/**
 * The DeadReckoner is used to give a common interface for robot
 * deadreckoning in for all users of robot pose information.  Thus the
 * probblem of interpolation and cumulation to a timestamp, the fusing
 * of inertial and odometry into one estimat and estimating the
 * uncertianty is taken care of.  This buffers one or two pose
 * estimate inputs, adds uncertainty at a specified even time period
 * (or when triggered) and if needed fusing the two estimates.  The
 * output is stored in slots for use by multiple users.  Various output ports
 * are provided to give different sorts of packageing of the pose information.
 * One can get individul Poses or whole sets of related Poses interpolated to 
 * cover specified intervals of time.  
 *
 * This  class takes the inputs for two Models  and outputs a 
 * dead-reckoning fused incremental pose and a cumulated dead-reckoning pose. 
 *
 * It will not pull any input data.  It waits for data to be written. 
 * 
 *
 * This class has 2 inputs 5 outputs.
 *
 * Input 0, port 0: Cumulated pose (odometry).
 * Input 1, port 1: Cumulated pose (inertial).
 *

 * Output 0, port 2: Rotation is an additional pose to bring you 
 *                   from the cumulative deadreckoning to the robot frame.
 *                   Typically this the the pitch and roll component.
 *
 *                   Robot(t) = Cumulated(t) + Robot_Rotation(t).
 *
 * Output 1, port 3: Incremental deadreckoning with uncertainty,
 *                   This is the incremental pose in the robot frame at
 *                   the begining of the interval.  It is not the
 *                   change in x y z...!
 *
 *                   To get a dead reckoning estimate, 
 *
 *                   Cumulated(t) = Cumulated(t-1) + Incremental(t).
 *
 * Output 2, port 4: Cumulated deadreckoning with uncertainty,
 * 
 * Output 3 
 *            Gives all three as a DataSet object interpolated to a
 *            time interval.  Read this with a DataSet containing 3
 *            Pose3D's.  The Time interval will be from timestamp of
 *            the read return object to the time of t. ie(read(p,t)
 *            give from p.Time to t) The Pose3D's in the dataset(0..2)
 *            are as for outs(0..2)
 *
 * Output 4 
 *           Gives the transformation from DeadReckoning to Odometry frames
 *           So To find a pose in the  Dead Frame dead(t) cooresponding to a 
 *           given pose in the odometry Frame odo(t).
 *
 *           dead(t)=out4Pose + odo(t) 
 *  
 *           also if you have a goal in the dead frame and want to transform
 *           it to the odometry frame to pass it to a low level controller do:
 *
 *           odogoal.minusPlus_(out4Pose,deadgoal);
 *
 *
 * Output 5 gives the Corrected Odometry at the latest time available.
 *          It gives the deadrecking out(2) + out(0) +increamental
 *          change in the odometry between the time of out(2) and 
 *          the current time.  So the pushed out is at the same time
 *          as the calc and the incremental odometry is 0.  On 
 *         read one can get the best continuous estimate of where the 
 *         robot is as of the latest odometry (read with t=0).  
 *          
 * 
 * SUMMARY OF USE  
 *
 * This filter expects data to be written to it.  It will then
 * periodically do its calculation and produce outputs.  A write may
 * not cause any output to be pushed if the time interval since the
 * last calc was less that the period.  The data written is saved and
 * used to interpolate when all data is available at the required
 * time.  Then output is produced.  Instead of waiting a set period of
 * time one can force the calculation by a write to the trigger input.
 *
 * One can use either one or two inputs.  These are fed to two error
 * models which add estimates of the error covariance matrix in the
 * incremental changes.  The 2 models are called modelA and modelB.
 * If modelB is not set the output of modelA goes direct to outputs.
 *
 * The period can be used to provide evenly spaced estimates.  If one
 * uses the trigger one can force an estimate before (or after) the
 * period expires.  So for example, if we want to sync to a SICK at 5 hz
 * we can set period to 0.5 sec. and trigger on the scans.  This will
 * garrentee that deadreckoning will not stop if the scans do but the
 * interval will be quite large.
 *
 * Generally one would have this object pulled from a variety of Pose users.
 * It can then provide the needed data interpolated from the buffered output 
 * of the periodic calculations.  
 *
 * @author John Folkesson
 */
class DeadReckoner: public DataFilter
{
public:	      
  /** Used for Out(3)*/
  DataSet OutSet;
  /**These are The Data in and out.*/
  Pose3D Poses[8];
  /** used to do internal calculations for out(3)*/
  PoseCumulator Cumulator;
  /** This could be a odometry model for example*/
  PoseErrorModel * m_ModelA;
  /** 
   * This could be a Gyro model for example
   * If left unset the only the A input will be used.
   */
  PoseErrorModel * m_ModelB;
 
  /** 
   * This could be a GyroAdjOdometry for example 
   *  The PoseErrorModels will feed this and it will feed the output. 
   */
  FuseFilter * m_Fuser;
  /** If out(0) is to be used this is true */ 
  bool m_UseRotate;
  /** If In(1) is to be used this is true */ 
  bool m_UseBInput;

  /** Slot with odometry data */
  DataSlotAddress m_SlotInA;

  /** Slot with for example gyro data */
  DataSlotAddress m_SlotInB;

  /** Slot for DeadReckoning */
  DataSlotAddress m_RobotRotate;

  /** Slot for Deadreckoning */
  DataSlotAddress m_DeadIncremental;

  /** Slot for DeadReckoning */
  DataSlotAddress m_DeadCumulated;
  /**
   * If one input gets this far in front ofthe other, time for the
   * next calculation (NextTime) will be set to the last input time
   * This is to prevent the odd situation of one input not coming in
   * for a while causing no output forever.
   */
  Timestamp m_BufferTime;

  /** The next output time. */
  Timestamp m_NextTime;

  /** The Filter will output data at this period if no trigger comes first */
  Timestamp m_Period;

protected:

  Timestamp m_ATime,m_BTime, m_LastTime;
  /**
   * This helps get things going when first data starts being written.
   * Normally 0.
   */
  unsigned short m_Start;  
public:	      
  /**
   * @param inputbuffersize size of the input buffers, the output will be 
   * twice the size
   * 
   * 
   */
  DeadReckoner(long inputbuffersize = 50);
  /*
   * @param period
   * 
   * 
   */
  DeadReckoner(PoseErrorModel *modelA, Timestamp & period, 
	       PoseErrorModel *modelB=0, FuseFilter *fuser=0, 
	       bool userotate=false,
	       long inputBuffer=50,
	       long outputbuffer=100);
  virtual ~DeadReckoner(){}
  
  /**
   * This will set the ErrorModel any subclass of PoseErrorModel can be given.
   * @param m the model.
   * @param which is 0 for ModelA else modelB is set.
   */  
  void setErrorModel(PoseErrorModel *m, int which);
  
  /**
   * This will set the filter  to use the fuser.
   * one normally will then want to use both inputs.
   * @param userotate This tells if the fuser will be seting out(0). 
   * @param m any FuseFilter to use,
   */
  void setFuser(FuseFilter *m, bool userotate);

  /**
   * this filter will never pull data.  It stores the pushed data for
   * eventual use.
   *
   * @param port the port number    
   * @return false. 
   */
  virtual bool canPull(const int port){
    return false;
  }

  
  /**
   * This will simple read on the DataSlotAddress associated with the 
   * output port.  No calc is forced.
   *
   * For port=5 result should be a DataSet of Size > 2
   * In that case the increments will be interpolated and 
   * Summed up between result.Time and t.  That is result.Time
   * Should be the Time you last read the dead reckoning and 
   * t the time you want now.  Then the incremental pose 
   * (result(1)) and the XtraRot (result(0)) and the
   * Cumulative in (result(2)) 
   *
   * @param result Data to be copied into
   * @param port must be an output port number
   * @param t requested time will be interpreted based on interpolate 
   * @param interpolate flag for requesting interpolation to t.
   * @return 0 if ok, else RESOURCE_ERROR TIMESTAMP_ERROR,
   *                       NO_INTERPOLATE TYPE_ERROR.
  */    
  virtual unsigned short read(Cure::TimestampedData& result,const int port, 
			      const Cure::Timestamp t=0,
			      const int interpolate=0);

protected:

  /**
   * This checks the timestamps on the input slots and writes to the models
   * if its time.
   */
  virtual unsigned short calc();

  /**
   * Calling this just writes the inputData to the slot, no pull done..
   * @param port the port number. 
   * @return 0 if ok, else ADDRESS_INVALID or RESOURCE_ERROR;
   */
  virtual unsigned short getInputs(TimestampedData &inputData,int port=-1);
  /**
   * Calling this writes all the outputs.
   * Normally the default behaviour is what you want but one  can override 
   * this. 
   * @return 0 if ok, else ADDRESS_INVALID, TIMESTAMP_ERROR
   */
  virtual unsigned short setOutputs();

  /**
   * This will be called after data is written or read to the input port to
   * check the type data written to the port.
   *
   * This is overridden to make sure inputData is copied to correctly to 
   * the ports data object and gets as much of the data as possible.
   *
   * Some filters will not check anything.
   * 
   * If inputData is the wrong type for this port this function should return
   * TYPE_ERROR.
   *
   * If inputData==0 then data(port) is checked.
    * 
   * @param port must be an input port number or -1.  
   * @param inputData This is the data writen to the port but not yet 
   * copied to data(port). 
   * @return  0 if ok else TYPE_ERROR  
   */
  virtual unsigned short typeCheck(int port,TimestampedData *inputData=0);

  /**get the data*/  
  TimestampedData * data(int port) {
    if (port < 0 || port >= (Ins+Outs)) {
      std::cerr << "WARNING: PoseFilter::data port " << port 
                << " out of bounds must be >0 and <"
                << Ins+Outs << std::endl;
    }
    if ((port>5)||(port<5))
      return Poses+port; 
    return &OutSet;
  }
protected:
  bool PushOut;
};


} // namespace Cure

#endif // CURE_FUSEFILTER_HH
