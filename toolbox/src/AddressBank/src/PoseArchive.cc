//
// = FILENAME
//    PoseArchive.hh
//
// = FUNCTION
//
// = AUTHOR(S)
//    Patric Jensfelt
//
// = COPYRIGHT
//    Copyright (c) 2006 Patric Jensfelt
//
/*----------------------------------------------------------------------*/

#include "PoseArchive.hh"

#include "CureDebug.hh"
#include "HelpFunctions.hh"

namespace Cure {

PoseArchive::PoseArchive(int length)
  :m_Archive(length)
{
  m_MinXYZDiff = 0.02;
  m_MinAngDiff = Cure::HelpFunctions::deg2rad(5);
}

PoseArchive::~PoseArchive()
{}

void
PoseArchive::setMinDiffThresholds(double minXYZDiff, double minAngDiff)
{
  m_MinXYZDiff = minXYZDiff;
  m_MinAngDiff = minAngDiff;
}

int 
PoseArchive::addPose(const Cure::Pose3D &p)
{
  m_LastAddedPose = p;

  m_Mutex.lock();

  // Check if the difference is large enough to add the new pose
  Cure::Pose3D lastP;
  if (m_Archive.read(lastP) == 0) {

    if (p.getTime() == lastP.getTime()) {
      m_Mutex.unlock();
      CureCERR(40) << "Pose with timestamp " << p.getTime() 
                   << " already added\n";
      return 1;
    }

    Cure::Pose3D diffP;
    diffP.minusPlus(lastP, p);

    if (fabs(diffP.getX()) < m_MinXYZDiff &&
        fabs(diffP.getY()) < m_MinXYZDiff &&
        fabs(diffP.getZ()) < m_MinXYZDiff) {
  
      double ang[3];
      diffP.getAngles(ang);

      if (fabs(ang[0]) < m_MinAngDiff && 
          fabs(ang[1]) < m_MinAngDiff && 
          fabs(ang[2]) < m_MinAngDiff) {

        m_Mutex.unlock();
        CureCERR(60) << "Not moved enough,"
                     << " dx=" << diffP.getX()
                     << " dy=" << diffP.getY()
                     << " dz=" << diffP.getZ()
                     << " da0=" << Cure::HelpFunctions::rad2deg(ang[0])
                     << "deg da1=" << Cure::HelpFunctions::rad2deg(ang[1])
                     << "deg da2=" << Cure::HelpFunctions::rad2deg(ang[2])
                     << std::endl;
        return 2;
      }
    }
  }

  Cure::Pose3D tmp(p); // To get around the fact that DatSlotAddress
                       // does not except const arguments
  m_Archive.write(tmp);
  m_Mutex.unlock();
  return 0;
}

int
PoseArchive::getInterpolationOutsideArchive(const Cure::Timestamp &t, 
                                            Cure::Pose3D &p)
{
  Cure::Pose3D p1;
  m_Mutex.lock();
  int err = m_Archive.read(p1);
  m_Mutex.unlock();

  if (err) return err;

  if (t > m_LastAddedPose.getTime()) {
    return (TIMESTAMP_ERROR|NO_DATA_AVAILABLE|TIMESTAMP_IN_FUTURE);
  }

  Cure::Pose3D &p2 = m_LastAddedPose;

  double p1Ang[3], p2Ang[3];
  p1.getAngles(p1Ang);
  p2.getAngles(p2Ang);
  
  // Estimate the difference (the speed for the 6 axes)
  Cure::Pose3D diffP;
  double diffAng[3];
  diffP.setX(p2.getX() - p1.getX());
  diffP.setY(p2.getY() - p1.getY());
  diffP.setZ(p2.getZ() - p1.getZ());
  diffAng[0] = Cure::HelpFunctions::angleDiffRad(p2Ang[0], p1Ang[0]);
  diffAng[1] = Cure::HelpFunctions::angleDiffRad(p2Ang[1], p1Ang[1]);
  diffAng[2] = Cure::HelpFunctions::angleDiffRad(p2Ang[2], p1Ang[2]);
  diffP.setAngles(diffAng);
  
  Cure::Timestamp dt12(p2.getTime() - p1.getTime());
  Cure::Timestamp dt1t(t - p1.getTime());
  double scale = dt1t.getDouble() / dt12.getDouble();

  p.setX(p1.getX() + scale * diffP.getX());
  p.setY(p1.getY() + scale * diffP.getY());
  p.setZ(p1.getZ() + scale * diffP.getZ());
  double pAng[3];
  pAng[0] = p1Ang[0] + scale * diffAng[0];
  pAng[1] = p1Ang[1] + scale * diffAng[1];
  pAng[2] = p1Ang[2] + scale * diffAng[2];
  p.setAngles(pAng);
  p.setTime(t);
  return 0;
}

int  
PoseArchive::getInterpolation(const Cure::Timestamp &t, Cure::Pose3D &p)
{
  m_Mutex.lock();
  int err = m_Archive.read(p, t);
  m_Mutex.unlock();

  if (err & TIMESTAMP_IN_FUTURE) {
    // Check if we have added a pose after this but which is not added
    // to the archive because the robot did not move long enough.
    if (t < m_LastAddedPose.getTime()) {
      return getInterpolationOutsideArchive(t, p);
    }
  }

  return err;
}

int  
PoseArchive::getInterpolation(const double t, Cure::Pose3D &p)
{
  return getInterpolation(Cure::Timestamp(t), p);
}

int
PoseArchive::calcExtrapolation(const Cure::Timestamp &t, 
                               const Cure::Pose3D &p1,
                               const Cure::Pose3D &p2,
                               Cure::Pose3D &p)
{
  double p1Ang[3], p2Ang[3];
  p1.getAngles(p1Ang);
  p2.getAngles(p2Ang);
  
  // Estimate the difference (the speed for the 6 axes)
  Cure::Pose3D diffP;
  double diffAng[3];
  diffP.setX(p2.getX() - p1.getX());
  diffP.setY(p2.getY() - p1.getY());
  diffP.setZ(p2.getZ() - p1.getZ());
  diffAng[0] = Cure::HelpFunctions::angleDiffRad(p2Ang[0], p1Ang[0]);
  diffAng[1] = Cure::HelpFunctions::angleDiffRad(p2Ang[1], p1Ang[1]);
  diffAng[2] = Cure::HelpFunctions::angleDiffRad(p2Ang[2], p1Ang[2]);
  diffP.setAngles(diffAng);

  Cure::Timestamp dt12(p2.getTime() - p1.getTime());
  Cure::Timestamp eTime(t - p2.getTime());
  double scale = eTime.getDouble() / dt12.getDouble();
  
  p.setX(p2.getX() + scale * diffP.getX());
  p.setY(p2.getY() + scale * diffP.getY());
  p.setZ(p2.getZ() + scale * diffP.getZ());
  double pAng[3];
  pAng[0] = p2Ang[0] + scale * diffAng[0];
  pAng[1] = p2Ang[1] + scale * diffAng[1];
  pAng[2] = p2Ang[2] + scale * diffAng[2];
  p.setAngles(pAng);
  p.setTime(t);

  // Approximate the covariance to be the same as the last pose in the
  // archive
  p.setCovType(p2.getCovType());
  p.Covariance = p.Covariance;
  
  return 0;
}

int
PoseArchive::getExtrapolationOutsideArchive(const Cure::Timestamp &t, 
                                            Cure::Pose3D &p,
                                            const double maxT)
{
  if (t > m_LastAddedPose.getTime() + Cure::Timestamp(maxT)) {
    return (TIMESTAMP_ERROR|NO_DATA_AVAILABLE|TIMESTAMP_IN_FUTURE);
  }

  Cure::Pose3D p1;
  m_Mutex.lock();
  int err = m_Archive.read(p1);
  m_Mutex.unlock();

  if (err) return err;

  return calcExtrapolation(t, p1, m_LastAddedPose, p);
}

int  
PoseArchive::getExtrapolation(const Cure::Timestamp &t, Cure::Pose3D &p,
                              const double maxT)
{
  m_Mutex.lock();

  int err = m_Archive.read(p, t);

  // Check if we have to actually perform the extrapolation
  if ((err & TIMESTAMP_IN_FUTURE) && maxT > 0) {

    // Get the last two poses from the archive
    Cure::Pose3D p1, p2;
    err = m_Archive.read(p2);    
    if (err) {
      m_Mutex.unlock();
      CureCERR(20) << "Failed to get p2\n";
      return err;
    }

    if (t > p2.getTime() && t < m_LastAddedPose.getTime()) {
      m_Mutex.unlock();
      return getInterpolationOutsideArchive(t, p);
    }

    Cure::Timestamp eTime(t - p2.getTime());
    if (eTime.getDouble() > maxT) {
      m_Mutex.unlock();
      return getExtrapolationOutsideArchive(t,p,maxT);
    }

    Cure::Timestamp t1(p2.getTime()); t1 -= Cure::Timestamp(0.01);
    err = m_Archive.read(p1, t1);
    if (err) {
      m_Mutex.unlock();
      CureCERR(20) << "Failed to get p1\n";
      return err;
    }

    m_Mutex.unlock();

    return calcExtrapolation(t, p1, p2, p);
  }

  // Pose was within archive and we already have the result
  m_Mutex.unlock();

  return err;
}

int  
PoseArchive::getExtrapolation(const double t, Cure::Pose3D &p,
                              const double maxT)
{
  return getExtrapolation(Cure::Timestamp(t), p, maxT);
}

int  
PoseArchive::getPoseInterpolation(const Cure::Pose3D &pOld, 
                                  const Cure::Timestamp &tNew,
                                  Cure::Pose3D &pNew)
{
  return getPoseExtrapolation(pOld, tNew, pNew, 0);
}

int  
PoseArchive::getPoseInterpolation(const Cure::Pose3D &pOld, 
                                  const double tNew,
                                  Cure::Pose3D &pNew)
{
  return getPoseExtrapolation(pOld, Cure::Timestamp(tNew), pNew, 0);
}

int  
PoseArchive::getPoseExtrapolation(const Cure::Pose3D &pOld, 
                                  const Cure::Timestamp &tNew,
                                  Cure::Pose3D &pNew,
                                  const double maxT)
{
  // We begin by trying to get the archive pose at the time of the
  // first pose
  Cure::Pose3D archOld, archNew;
  int err = getExtrapolation(pOld.getTime(), archOld, maxT);
  if (err) return err;

  // Then we get the archive pose of the new pose
  err = getExtrapolation(tNew, archNew, maxT);
  if (err) return err;

  // Now calculate the difference in archive pose from the time of the
  // old pose to the new one we look for
  Cure::Pose3D diffArch;
  diffArch.minusPlus(archOld, archNew);
  
  // Now add the archive pose difference to the old pose and voila we
  // have the new pose in the same coordinate system as the old
  // (assuming that the transformation been archive pose and the other
  // pose is fixed).
  pNew.add(pOld, diffArch);
  pNew.setTime(tNew);
  return 0;
}

int  
PoseArchive::getPoseExtrapolation(const Cure::Pose3D &pOld, 
                                  const double t,
                                  Cure::Pose3D &pNew,
                                  const double maxT)
{
  return getPoseExtrapolation(pOld, Cure::Timestamp(t), pNew, maxT);
}

int
PoseArchive::getTransformationToArchive(const Cure::Pose3D &p2,
                                          Cure::Pose3D &T)
{
  Cure::Pose3D p1;
  Cure::Pose3D p2const(p2);
  int err = getInterpolation(p2const.getTime(), p1);
  if (err) return err;
  T.minusPlus_(p2const, p1);
  return 0;
}

int
PoseArchive::getTransformationFromArchive(const Cure::Pose3D &p2,
                                          Cure::Pose3D &T)
{
  Cure::Pose3D p1;
  Cure::Pose3D p2const(p2);
  int err = getInterpolation(p2const.getTime(), p1);
  if (err) return err;
  T.minusPlus_(p1, p2const);
  return 0;
}

}; // namespace Cure
