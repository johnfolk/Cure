//
// = LIBRARY
//
// = FILENAME
//    RLDisplayHelper.cc
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

#include "RLDisplayHelper.hh"

#include "CureDebug.hh"
#include "TimestampedData.hh"  // Error codes
#include "Matrix.hh"
#include "Pose3D.hh"
#include "SICKScan.hh"

#ifndef DEPEND
#endif

namespace Cure {

RLDisplayHelper::RLDisplayHelper(bool freeProxyOnDelete)
  :m_Rlp(0),
   m_FreeProxyOnDelete(freeProxyOnDelete)
{}

RLDisplayHelper::~RLDisplayHelper()
{
  if (m_Rlp != 0 && m_FreeProxyOnDelete) {
    delete m_Rlp;
    m_Rlp = 0;
  }
}
 
int 
RLDisplayHelper::connect(const std::string &rlHostname, 
                         const std::string &robot)
{
  if (m_Rlp == 0) m_Rlp = new RoboLookProxy;

  if (!m_Rlp->connected()) {
    if (m_Rlp->Connect(rlHostname.c_str(),5555) == -1) {
      CureCERR(20) << "WARNING: Failed to connect to RoboLook server on \""
                    << rlHostname << "\"\n";
      delete m_Rlp;
      m_Rlp = NULL;
      return RESOURCE_ERROR;
    }
  }

  int rid = 0;

  // Make sure that there is no robot with this id
  m_Rlp->deleteRobot(rid);

  // To make sure that we do not exhaust the graphics resources we
  // only update the screen once a second unless we force an update
  m_Rlp->setSceneUpdateInterval(1.0);
  
  m_Rlp->createRobot(RL_ENV_EST, rid, robot.c_str());
  m_Rlp->setShowLaserScan(rid, 0, true);

  clearEstEnv(m_Rlp);

  CureCERR(30) << "Sucessfully connected to RoboLook and created robot "
               << robot << " with rid=" << rid << std::endl;

  return 0;
}

int
RLDisplayHelper::disconnect()
{
  if (m_Rlp == 0) {
    CureCERR(20) << "WARNING: Cannot disconect without a proxy object!\n";
    return RESOURCE_ERROR;
  }

  m_Rlp->Disconnect();
  delete m_Rlp;

  return 0;
}

int
RLDisplayHelper::clearEstEnv(RoboLookProxy *rlp)
{
  if (rlp == 0) return RESOURCE_ERROR;

  rlp->clearLines(RL_ENV_EST);
  rlp->clearDoors(RL_ENV_EST);
  rlp->clearStars(RL_ENV_EST);
  rlp->clearCubes(RL_ENV_EST);
  rlp->clearWalls(RL_ENV_EST);
  rlp->clearMarkers(RL_ENV_EST);
  rlp->clearEllipses(RL_ENV_EST);
  rlp->clearEllipsoids(RL_ENV_EST);
  rlp->clearRectangles(RL_ENV_EST);

  return 0;
}

int 
RLDisplayHelper::setRobotPose(int rid, const Cure::Pose3D &p, 
                              RoboLookProxy *rlp,
                              bool displayUnc, double scale)
{
  if (rlp == 0) return RESOURCE_ERROR;

  // Set the robot pose
  rlp->setRobotPose(rid,
                    p.Position(0), 
                    p.Position(1), 
                    p.Position(2),
                    p.getTheta());

  if (!displayUnc) return 0;

  RL_EllipseItem uncEl;
  calcUncEllipse(p, uncEl, scale);
  rlp->addEllipses(RL_ENV_EST, &uncEl, 1, true);

  return 0;
}

int 
RLDisplayHelper::setRobotPose(const Cure::Pose3D &p, RoboLookProxy *rlp,
                              bool displayUnc, double scale)
{
  if (rlp == 0) return RESOURCE_ERROR;
  return setRobotPose(rlp->lastRid(), p, rlp, displayUnc, scale);
}

int
RLDisplayHelper::calcUncEllipse(const Cure::Pose3D &p, RL_EllipseItem &el,
                                double scale)
{
  // Calculate uncertainty ellipse. We hack it and just take the upper
  // position related part of the matrix
  Cure::Matrix xycov(2);
  xycov(0,0) = p.Covariance(0,0);
  xycov(1,0) = p.Covariance(1,0);
  xycov(0,1) = p.Covariance(0,1);
  xycov(1,1) = p.Covariance(1,1);
  double lambda[2];
  Matrix evec,lam;
  xycov.symmetricEigen(lam,evec);
  lambda[0]=sqrt(lam(0,0));
  lambda[1]=sqrt(lam(1,1));

  el.xC = p.Position(0);
  el.yC = p.Position(1);
  el.z = p.Position(2);
  el.major = scale*lambda[0];
  el.minor = scale*lambda[1];
  el.majorAngle = 180.0/M_PI*atan2(evec(1,0), evec(0,0));
  el.color = 7;

  return 0;
}

int
RLDisplayHelper::setEstEnvPose(double x, double y, double theta,
                               RoboLookProxy *rlp)
{
  if (rlp == 0) return RESOURCE_ERROR;

  rlp->setEnvPose(RL_ENV_EST, x, y, 0, theta);
  return 0;
}

int
RLDisplayHelper::setScan(Cure::SICKScan &s, int rid,
                         RoboLookProxy *rlp)
{
  if (rlp == 0) return RESOURCE_ERROR;

  rlp->setLaserScan(rid, 0, s.getNPts(),
                    s.getStartAngle(), s.getAngleStep(),
                    0, s.getRanges(), 1.0);
  return 0;
}

int
RLDisplayHelper::setScan(Cure::SICKScan &s, RoboLookProxy *rlp)
{
  if (rlp == 0) return RESOURCE_ERROR;
  return setScan(s, rlp->lastRid(), rlp);
}

} // namespace Cure
