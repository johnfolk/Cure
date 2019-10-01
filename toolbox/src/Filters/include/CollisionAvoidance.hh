//    $Id: CollisionAvoidance.hh,v 1.10 2007/09/14 09:13:50 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_COLLISIONAVOIDANCE_HH
#define CURE_COLLISIONAVOIDANCE_HH


#include "DataFilter.hh"
#include "CommandExecuter.hh"
#include "GridData.hh"
#include "MotionCommand.hh"

namespace Cure{
/**
 * This object shall take a simple motion command and adjust to speed
 * to a safe value based on how close to obstacles the projected path comes
 * out of incoming sensor data.  It is meant to be used for short distances
 * about 1 meter or less and regular triggers at something like 50 msec.  
 *
 * One should have an even simplier object after this that is not
 * using anything but raw data form odometry, sonar, sick and bumpers.
 * That object should do nothing unless eminent danger is detected.
 *  
 * Command: Service=CURE_AVOIDANCE_SERVICE
 * 
 * Inputs:
 *      0 Must be set to Pull from Out(5) of a DeadReckoner
 *        This must be the frame of the Free Space Grid sent to (0) 
 *      
 *      1 Must be set to pull from a Grid that will be avoided.
 * 
 * Outputs:
 *       None
 *
 *
 *  This object uses a trigger. 
 * 
 *
 * SUMMARY OF USE 
 *
 * One can send this Commands that have Data=(v,w,t),
 * CmdFlags(0,0)=Command::SPEED, These then are interpreted as linear
 * velocity  v, anglar velocity w (rads/sec), and timeout t.  A path
 * with those values and that length of time will be followed by the
 * robot subject to the free space.  The free space is checked as far
 * as the robot can reasonable be expected to travel before the next
 * trigger of this object.  If there are close obstacles the velocity
 * is reduced.  If the path is blocked the robot is stoped and a
 * message sent to ReplyAddress if any:
 * 
 * Reply.CmdFlags(0,0)=Cure::MotionCommand::COMMAND_OK.
 * Reply.CmdFlags(0,1)=Cure::MotionCommand::GOAL_UNREACHABLE.  Also
 *
 * Queries to Status will show Status.CmdFlags(1,3)=1
 *
 *  When the command is completed successfully the robot is stoped and a
 * message sent to ReplyAddress if any:
 *
 * Reply.CmdFlags(0,0)=Cure::MotionCommand::COMMAND_OK.
 * 
 * Reply.CmdFlags(0,1)=Cure::MotionCommand::GOAL_REACHED.  
 *
 *  Queries to Status will show Status.CmdFlags(1,2)=1
 * 
 * If speed command is accepted and ReplyAddress is not zero a reply 
 * will be sent just after the first execution:
 *
 * Reply.CmdFlags(0,0)=Cure::MotionCommand::COMMAND_OK.
 *
 * Reply.CmdFlags(0,1)= one of COMMAND_OK,GOAL_REACHED or GOAL_UNREACHABLE 
 *  
 * Queries to status will show the Status.CmdFlags(0,3)=CmdID 
 *   
 * Piority behaviour is inherited from CommandExecuter.
 *
 * CmdFlags(0,0)=Command::STOP, will be passed on immeadiately and 
 * the status.CmdFlags(1,1) set to 1.
 * Reply will be sent as:
 *
 *  Reply.CmdFlags(0,0)=Cure::MotionCommand::COMMAND_OK.
 *
 *  Reply.CmdFlags(0,1)=Cure::MotionCommand::COMMAND_OK.
 *
 * CmdFlags(0,0)=Command::RELEASE will stop the robot and lower the priority
 * Reply will be sent as:
 *
 *  Reply.CmdFlags(0,0)=Cure::MotionCommand::COMMAND_OK.
 *
 *  Reply.CmdFlags(0,1)=Cure::MotionCommand::COMMAND_OK.
 *
 *
 *  The Status will be:
 *
 * As a @see Cure::CommandExecuter with the addition:
 * CmdFlags(1,0)=AvoidanceOn, CmdFlags(1,1)=Stoped, CmdFlags(1,2)=GoalReached,
 * CmdFlags(1,3)=GoalUnreachable.
 *
 * GoalUnreachable will be 1 if this object detected the obstacle
 * and 2 if it came from down stream. 
 *
 * The CmdData is (Current linear velocity sent to output, angular v,
 * timeout remaining).
 *
 * @author John Folkesson
 */
  class CollisionAvoidance: public DataFilter, public  CommandExecuter
{
public:
  enum Paramenters {
      DRIVE_DELAY = 0,
      SAFE_PERIOD,
      INNER_SAFETY,
      SAFETY,
      SAFETY_VELOCITY,
      MEAN_PERIOD,
      MIN_PERIOD,
      MAX_PERIOD,
      PERIOD_VAR,
  };

  Cure::Matrix Velocity;
  Cure::Pose3D CurrentPose;
  Cure::GridData CurrentGrid;
  double MinVelocity[2];
 

  Cure::Timestamp LastTime;
  Cure::Timestamp MinPeriod;
  /** The mean time between triggering this object*/
  double MeanPeriod;
  /**Estimated Variance in Period around MeanPeriod.  This is adaptively learned*/
  double  PeriodVar;
  /**
   * This is the longest possible delay between triggering this
   * object.  It sets the security margin for stopping in time. So if
   * you drive straight at and obstacle then the velocity will
   * decrease exponentially based on this and MeanPeriod.
   *
   *exp(-MeanPeriod/DecayTime)=
   * (1-(MeanPeriod/(MaxPeriod+DriveDelay+sqrt(PeriodVar))))
   * 
   * is an approximation of the exponential decay.
   */
  double  MaxPeriod;
  /**This is the min for the MaxPeriod.  Learning the time will never sink MaxPeriod below this value*/
  double SafePeriod;
 
  /**The outer radius from robot that obstacles will slow the robot down.*/
  double Safety;
  /**The inner radius from robot that obstacles will stop the robot*/
  double InnerSafety;
  /** This is the Velocity when objects are at exactly Safety distance*/
  double SafetyVelocity;
  /**This is the max delay between sending commands to the drive and having anythin happen.*/
  double DriveDelay;
  Cure::Matrix Robot;
  /**radius of the arc robot is travelling on <0 if clockwise*/
  double Radius;
  /**If true the ronbot is following a circular arc else a straight line*/
  unsigned short OnArc;
  /**Has the robot been stoped*/
  bool Stoped;
  /**If stoped is it due to obsacles in the path*/
  unsigned short  GoalUnreachable;
  /** Or has  TimeOut expired*/
  unsigned short  GoalReached;
  /**Is the Avoidance on.  If 0 there is no checking for obstacles at all*/
  unsigned short  AvoidOn;

  
 protected:
  double OldVelocity[2];
  double MaxChange[2];
  int Mode;
  unsigned short AvoidHeadings[360];
  unsigned short GoalHeading;
  unsigned short MaxAvoidHeading;
  unsigned short AvoidHeadingDecrement;
  /**This is the current amount of time that the current command will be in effect.*/
  double TimeOut;
  /** Used to measure the change to TimeOut */
  Timestamp LastSendTime;

  /** This is used to trace the path into the future*/
  double Dx[3];
  double Increment;
public:
  /**
   * 
   * 
   */
  CollisionAvoidance();
  virtual ~CollisionAvoidance();
  CommandAddress *command();
protected:
  int setAvoidVelocity(double th);
  void setAvoidHeadings(double x[2]);
  void decrementAvoidHeadings();
  void resetAvoidHeadings();
  /**
   * Adaptive learning of the distribution of the period of time
   * between calcs.
   */
  void learnTime();
  /**
   * This tries to extrapolate the CurrentPose to the CurrentTime based on the 
   * values in Velocity.
   * @return 1 if the Time of the Current Pose is too far from the CurrentTime
   *           else 0.
   */
  //  int adjustCurrentPose();
  /**
   * This is called when new Speed commands come in to set up the
   * parameters of the path (arc or line) and TimeOut.
   * 
   */
  void setPath();
  /**
   * This updates the TimeOut and makes sure that the next trigger
   * will come before it travels a distance equal to the
   * CurrentCommand's velocity times the TimeOut.  So for small
   * TimeOuts<(MaxPeriod+...) it must go slower than the command velocity..
   * It then calls trace to check safety of the path.
   * @return 1 if Goal reached (it sets up the Reply to be sent).
   *         else trace()
   */
  int setVelocity();
  /**
   * This will lower the Velocity to a safe value for travel until
   * MaxTime+DriveDelay+Sqrt(PeriodVar).  It traces out the path and 
   * checks the grid for lack of known free space.  It slows down if
   * obstascles are near.
   * @return 1 if Goal can't be reached (sets up Reply to send).
   *           else 0.
   */
  int trace();
  /**
   *
   */
  virtual unsigned short comSpeed();
  virtual unsigned short comStop();
  virtual unsigned short comSet();
  virtual unsigned short processReply();
  virtual void stop();
  virtual void speed();
  virtual unsigned short comRelease();
  /**
   * This tells how much to lower the velocity for a given nearby obstacle.
   * @param d the distance to the closest obstacle.
   * @return the ratio to multiply the Velocity by.
   */
  virtual double maxVelRatio(double d);
  virtual unsigned short calc();
  virtual void setStatus();
  /**
   * @param d the distance to the closest obstacle.
   * @param maxt the longest time interval to check .
   * @param maxt the distance traveled so far..
   * @return true if Velocity has changed; 
   */  
  virtual bool limitVelocity(double d,double maxt, double travel);
  virtual Cure::TimestampedData * data(int port);

  /**
   * This will be called after data is written or read to the input port to
   * check the type data written to the port.
   *
   * If inputData is the wrong type for this port this function will
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
  void removeBehind();
};

} // namespace Cure

#endif 
