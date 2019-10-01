//
// = LIBRARY
//
// = FILENAME
//    RLDisplayHelper.hh
//
// = FUNCTION
//
// = AUTHOR(S)
//    Patric Jensfelt
//
// = DESCRIPTION
//    
// = COPYRIGHT
//    Copyright (c) 2004 Patric Jensfelt
//
/*----------------------------------------------------------------------*/

#ifndef RLDisplayHelper_hh
#define RLDisplayHelper_hh

#include "RoboLookProxy.h"

#ifndef DEPEND
#include <string>
#endif


namespace Cure { 

// Forward declaration(s)
class Pose3D; 
class SICKScan;

/**
 * Helps you connect and config RoboLookProxy. You typically do the
 * same things during config every time so this class just collects
 * all those things in one place.
 * 
 * @author Patric Jensfelt 
 * @see
 */
class RLDisplayHelper {
public:
  RoboLookProxy *m_Rlp;

public:

  /**
   * Constructor in the case you want to use this class to help
   * with the connection phase.
   *
   * @param freeProxyOnDelete set to false if you want to be
   * responsible for deleting the proxy when this object is deleted,
   * else true
   */
  RLDisplayHelper(bool freeProxyOnDelete = true);

  /**
   * Destructor
   */
  ~RLDisplayHelper();

  /**
   * Helps you connect to the RoboLook server and clear all the
   * objects in the estimated environment, etc.
   *
   * @param rlHostname the name of the host running RoboLook
   * @param robot the name of the robot to create an icon for
   *
   * @return 0 is OK
   */
  int connect(const std::string &rlHostname, const std::string &robot);

  /**
   * Disconnects from RoboLook.
   *
   * @return 0 is OK
   */
  int disconnect();

  /**
   * Function to call if you want to clear the estimated environment
   * from all objects. 
   *
   * This function will call the static version with m_Rlp as argument
   *
   * @return 0 is OK
   */
  int clearEstEnv();

  /**
   * Set the pose of the robot.
   *
   * This function just calls the static version with m_Rlp as argument
   *
   * @param p the pose of the robot
   * @param displayUnc if true an uncertainty ellipse will be shown
   * @param scale can be used to scale the uncertainty ellipse to make
   * it easier to see
   *
   * @return 0 is OK
   */
  int setRobotPose(const Cure::Pose3D &p, bool displayUnc = false, 
                   double scale = 1.0);

  /**
   * Set the pose of the estimated pose of the environment with
   * respect to the true so that you can make them overlap
   *
   * This function just calls the static version with m_Rlp as argument
   *
   * @param x the x-coordinate of the pose of the estimated environment
   * @param y the y-coordinate of the pose of the estimated environment
   * @param theta the orientation of the of the estimated environment
   *
   * @return 0 is OK
   */
  int setEstEnvPose(double x, double y, double theta);

  /** 
   * Send a new laser scan to a certain robot
   *
   * This function just calls the static version with m_Rlp as argument
   *
   * @param s the scan to send to the robot
   * @param rid the id of the robot to send the scan to
   *
   * @return 0 is OK
   */
  int setScan(Cure::SICKScan &s, int rid);

  /**
   * Same as above but uses lastRid() for rid. That is the robot id
   * for the last message effecting a robot sent to RoboLook
   *
   * @param s the scan to send to the robot
   *
   * @return 0 is OK
   */
  int setScan(Cure::SICKScan &s);

  //======= STATIC HELPER FUNCTION =========================================

  /**
   * Clear all objects from the estimated environment. This function
   * is made static so that it can be used without instantiating
   *
   * @param rlp pointer to a RoboLookProxy to use for the communication
   *
   * @return 0 is OK
   */
  static int clearEstEnv(RoboLookProxy *rlp);

  /**
   * Set the robot pose given a Pose3D object
   * 
   * @param rid id of the robot to set pose of
   * @param p pose of the robot
   * @param rlp pointer to a RoboLookProxy to use for the communication
   * @param displayUnc if true an uncertainty ellipse will be shown
   * @param scale can be used to scale the uncertainty ellipse to make
   * it easier to see
   *
   * @return 0 is OK
   */
  static int setRobotPose(int rid, const Cure::Pose3D &p, RoboLookProxy *rlp,
                          bool displayUnc = false, double scale = 1.0);

  /**
   * Set the robot pose given a Pose3D object. This function will use
   * the function lastRid from RoboLookProxy to set the pose of the
   * last robot for which a command was sent. Will thus only work if
   * you work with one robot from a RoboLook proxy object.
   *
   * @param p pose of the robot
   * @param rlp pointer to a RoboLookProxy to use for the communication
   * @param displayUnc if true an uncertainty ellipse will be shown
   * @param scale can be used to scale the uncertainty ellipse to make
   * it easier to see
   *
   * @return 0 is OK
   */
  static int setRobotPose(const Cure::Pose3D &p, RoboLookProxy *rlp,
                          bool displayUnc = false, double scale = 1.0);

  /**
   * Takes a Pose3D object and calculates the corresponding
   * uncertainty ellipse from the covariance matrix using only the
   * xy-part, i.e the upper 2x2 submatrix.
   *
   * @param p robot psoe to get uncertainty and thus ellipse from
   * @param el the ellipse object to put the result in
   * @param scale can be used to scale the uncertainty ellipse to make
   * it easier to see
   * 
   * @return 0 is OK
   */
  static int calcUncEllipse(const Cure::Pose3D &p, RL_EllipseItem &el,
                            double scale = 1.0);

  /**
   * Send a new laser scan to a certain robot
   * 
   * @param s the laser scan to display
   * @param rid robot id to send scan to
   * @param rlp pointer to RoboLookProxy to use for the communication
   *
   * @return 0 is OK
   */
  static int setScan(Cure::SICKScan &s, int rid, RoboLookProxy *rlp);

  /**
   * Send a new laser scan to a robot specified by lastRid
   *
   * @param s the laser scan to display
   * @param rlp pointer to RoboLookProxy to use for the communication
   *
   * @return 0 is OK
   */
  static int setScan(Cure::SICKScan &s, RoboLookProxy *rlp);

  /**
   * Set the pose of the estimated pose of the environment with
   * respect to the true so that you can mae them overlap
   *
   * @param x the x-coordinate of the pose of the estimated environment
   * @param y the y-coordinate of the pose of the estimated environment
   * @param theta the orientation of the of the estimated environment
   * @param rlp pointer to RoboLookProxy to use for the communication
   * 
   * @return 0 is OK
   */
  static int setEstEnvPose(double x, double y, double theta, 
                           RoboLookProxy *rlp);

protected:
  bool m_FreeProxyOnDelete;
private:

}; // class RLDisplayHelper

inline int 
RLDisplayHelper::clearEstEnv() { return RLDisplayHelper::clearEstEnv(m_Rlp); }

inline int
RLDisplayHelper::setRobotPose(const Cure::Pose3D &p, 
                              bool displayUnc, 
                              double scale)
{ return RLDisplayHelper::setRobotPose(p, m_Rlp, displayUnc, scale); }

inline int
RLDisplayHelper::setEstEnvPose(double x, double y, double theta)
{ return RLDisplayHelper::setEstEnvPose(x, y, theta, m_Rlp); }

inline int
RLDisplayHelper::setScan(Cure::SICKScan &s, int rid)
{ return RLDisplayHelper::setScan(s, rid, m_Rlp); }

inline int
RLDisplayHelper::setScan(Cure::SICKScan &s)
{ return RLDisplayHelper::setScan(s, m_Rlp); }

} // namespace Cure

#endif // RLDisplayHelper_hh
