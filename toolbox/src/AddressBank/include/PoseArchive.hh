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

#ifndef Cure_PoseArchive_hh
#define Cure_PoseArchive_hh

#include "Pose3D.hh"
#include "DataSlotAddress.hh"
#include "MutexWrapper.hh"

namespace Cure {

/**
 * This class has a buffer with pose data. It can use this buffer to
 * get poses at time within the archive, possibly using
 * interpolation. The class also supports getting pose data in some
 * other coordinate system with a fixed or very slowly varying
 * transform with respect to the pose in the archive. An example of
 * the latter would be to get world poses from odometry data.
 *
 * @author Patric Jensfelt
 */
class PoseArchive {
public:
  /// Archie with old pose data
  Cure::DataSlotAddress m_Archive;

  /// Mutex that protecs the archive from multiple acces (assuming
  /// that you lock it before you use the archive and unlock it
  /// afterwards).
  Cure::MutexWrapper m_Mutex;
public:

  /**
   * Constructor
   */
  PoseArchive(int length = 100);

  /**
   * Destructor
   */
  ~PoseArchive();

  /**
   * Set the minumum displacement in one x,y,z or one of the euler
   * angles to add a new pose to the archive. This allows us to avoid
   * filling the buffer with the data from the same pose.
   *
   * @param minXYZdiff minimum change required in either x,y,z from the
   * previsouly stored pose to add a new pose [m]
   * @param minAngdiff minimum change required in either of the euler
   * angles from the previsouly stored pose to add a new pose [rad]
   */
  void setMinDiffThresholds(double minXYZdiff, double minAngdiff);

  /**
   * Use this function to add new pose data to the archive.
   *
   * @param p pose
   *
   * @return 0 if pose added, 1 if already added, 2 if not moved enough 
   */
  int addPose(const Cure::Pose3D &p);

  /**
   * Get the last pose added to the archive
   */
  Pose3D getLastAddedPose() { return m_LastAddedPose; }

  /**
   * Use this function to get an interpolation from to the poses in
   * the archive to a certain time.
   * 
   * @param t time to interpolate too.
   * @param p pose to put result in
   * 
   * @return 0 if OK, else error code
   *                  archive empty  (TIMESTAMP_ERROR|NO_DATA_AVAILABLE)
   *                  time in future (TIMESTAMP_ERROR|NO_DATA_AVAILABLE|TIMESTAMP_IN_FUTURE)
   *                  time too old   (TIMESTAMP_ERROR | TIMESTAMP_TOO_OLD)
   *
   * @see getExtrapolation
   */
  int getInterpolation(const Cure::Timestamp &t, Cure::Pose3D &p);

  /**
   * Use this function to get an interpolation from to the poses in
   * the archive to a certain time.
   * 
   * @param t time to interpolate too.
   * @param p pose to put result in
   * 
   * @return 0 if OK, else error code
   *                  archive empty  (TIMESTAMP_ERROR|NO_DATA_AVAILABLE)
   *                  time in future (TIMESTAMP_ERROR|NO_DATA_AVAILABLE|TIMESTAMP_IN_FUTURE)
   *                  time too old   (TIMESTAMP_ERROR | TIMESTAMP_TOO_OLD)
   *
   * @see getExtrapolation
   */
  int getInterpolation(const double t, Cure::Pose3D &p);

  /**
   * Use this function to get estimate of the pose at a certain time
   * and allow for extraplation from the last pose in the archive.
   * 
   * @param t time to extrapolate too.
   * @param p pose to put result in
   * @param maxT max time to extrapolate forward from last pose in archive
   * 
   * @return 0 if OK, else error code
   *                  archive empty  (TIMESTAMP_ERROR|NO_DATA_AVAILABLE)
   *                  time in future (TIMESTAMP_ERROR|NO_DATA_AVAILABLE|TIMESTAMP_IN_FUTURE)
   *                  time too old   (TIMESTAMP_ERROR | TIMESTAMP_TOO_OLD)
   *
   * @see getExtrapolation
   */
  int getExtrapolation(const Cure::Timestamp &t, Cure::Pose3D &p,
                       const double maxT = 0.5);

  /**
   * Use this function to get estimate of the pose at a certain time
   * and allow for extraplation from the last pose in the archive.
   * 
   * @param t time to extrapolate too.
   * @param p pose to put result in
   * @param maxT max time to extrapolate forward from last pose in archive
   * 
   * @return 0 if OK, else error code
   *                  archive empty  (TIMESTAMP_ERROR|NO_DATA_AVAILABLE)
   *                  time in future (TIMESTAMP_ERROR|NO_DATA_AVAILABLE|TIMESTAMP_IN_FUTURE)
   *                  time too old   (TIMESTAMP_ERROR | TIMESTAMP_TOO_OLD)
   *
   * @see getExtrapolation
   */
  int getExtrapolation(const double t, Cure::Pose3D &p,
                       const double maxT = 0.5);

  /**
   * Use this function to get a prediction for the pose at a certain
   * time in another coordinate system that can be considered to have
   * a fixed transformation with respect to the poses in the
   * archive. This function will first get the pose at the time of
   * pOld, calculate the difference in archive poses to the last known
   * and then use this to get the new pose in the other coordinate
   * system.
   *
   * @param pOld old pose (input) 
   * @param tNew time of interpolated pose
   * @param pNew new predicted pose (output)
   *
   * @return 0 if OK, else error code
   *                  archive empty  (TIMESTAMP_ERROR|NO_DATA_AVAILABLE)
   *                  time in future (TIMESTAMP_ERROR|NO_DATA_AVAILABLE|TIMESTAMP_IN_FUTURE)
   *                  time too old   (TIMESTAMP_ERROR | TIMESTAMP_TOO_OLD)
   */
  int getPoseInterpolation(const Cure::Pose3D &pOld, 
                           const Cure::Timestamp &tNew,
                           Cure::Pose3D &pNew);

  /**
   * Use this function to get a prediction for the pose at a certain
   * time in another coordinate system that can be considered to have
   * a fixed transformation with respect to the poses in the
   * archive. This function will first get the pose at the time of
   * pOld, calculate the difference in archive poses to the last known
   * and then use this to get the new pose in the other coordinate
   * system.
   *
   * @param pOld old pose (input) 
   * @param tNew time of interpolated pose
   * @param pNew new predicted pose (output)
   *
   * @return 0 if OK, else error code
   *                  archive empty  (TIMESTAMP_ERROR|NO_DATA_AVAILABLE)
   *                  time in future (TIMESTAMP_ERROR|NO_DATA_AVAILABLE|TIMESTAMP_IN_FUTURE)
   *                  time too old   (TIMESTAMP_ERROR | TIMESTAMP_TOO_OLD)
   */
  int getPoseInterpolation(const Cure::Pose3D &pOld, 
                           const double tNew,
                           Cure::Pose3D &pNew);

  /**
   * Use this function to get a prediction for the pose possibly at a
   * future time in another coordinate system that can be considered
   * to have a fixed transformation with respect to the poses in the
   * archive. This function will first get the pose at the time of the
   * pOld, calculate the difference in archive pose to the
   * extrapolation time and then use this to get the new pose in the
   * other coordinate system.
   *
   * @param pOld old pose (input) 
   * @param t time to extrapolate to
   * @param pNew new predicted pose (output)
   * @param maxT max allowed extrapolation from last archive pose
   *
   * @return 0 if OK, else error code
   *                  archive empty  (TIMESTAMP_ERROR|NO_DATA_AVAILABLE)
   *                  time in future (TIMESTAMP_ERROR|NO_DATA_AVAILABLE|TIMESTAMP_IN_FUTURE)
   *                  time too old   (TIMESTAMP_ERROR | TIMESTAMP_TOO_OLD)
   */
  int getPoseExtrapolation(const Cure::Pose3D &pOld, const Cure::Timestamp &t,
                           Cure::Pose3D &pNew,
                           const double maxT = 0.5);

  /**
   * Use this function to get a prediction for the pose possibly at a
   * future time in another coordinate system that can be considered
   * to have a fixed transformation with respect to the poses in the
   * archive. This function will first get the pose at the time of the
   * pOld, calculate the difference in archive pose to the
   * extrapolation time and then use this to get the new pose in the
   * other coordinate system.
   *
   * @param pOld old pose (input) 
   * @param t time to extrapolate to
   * @param pNew new predicted pose (output)
   * @param maxT max allowed extrapolation from last archive pose
   *
   * @return 0 if OK, else error code
   *                  archive empty  (TIMESTAMP_ERROR|NO_DATA_AVAILABLE)
   *                  time in future (TIMESTAMP_ERROR|NO_DATA_AVAILABLE|TIMESTAMP_IN_FUTURE)
   *                  time too old   (TIMESTAMP_ERROR | TIMESTAMP_TOO_OLD)
   */
  int getPoseExtrapolation(const Cure::Pose3D &pOld, const double t,
                           Cure::Pose3D &pNew,
                           const double maxT = 0.5);

  
  /**
   * Use this function to find the transformation from a given pose to
   * the poses in the archive. The pose in the archive to compare
   * against is specified by the timestamp of the input pose.
   *
   * @param p2 input pose from which you want a transformation to
   * the poses in the pose archive.
   * @param T transformation to pose archive pose from the pose you provided
   *
   * @return 0 if OK, else error code
   *                  archive empty  (TIMESTAMP_ERROR|NO_DATA_AVAILABLE)
   *                  time in future (TIMESTAMP_ERROR|NO_DATA_AVAILABLE|TIMESTAMP_IN_FUTURE)
   *                  time too old   (TIMESTAMP_ERROR | TIMESTAMP_TOO_OLD)
   */
  int getTransformationToArchive(const Cure::Pose3D &p2, Cure::Pose3D &T);

  /**
   * Use this function to find the transformation between the poses in
   * the archive and a given pose. The pose in the archive to compare
   * against is specified by the timestamp of the input pose.
   *
   * @param p2 input pose to which you want a transformation from
   * the poses in the pose archive.
   * @param T transformation from pose archive pose to the pose you provided
   *
   * @return 0 if OK, else error code
   *                  archive empty  (TIMESTAMP_ERROR|NO_DATA_AVAILABLE)
   *                  time in future (TIMESTAMP_ERROR|NO_DATA_AVAILABLE|TIMESTAMP_IN_FUTURE)
   *                  time too old   (TIMESTAMP_ERROR | TIMESTAMP_TOO_OLD)
   */
  int getTransformationFromArchive(const Cure::Pose3D &p2, Cure::Pose3D &T);

protected:
  int getInterpolationOutsideArchive(const Cure::Timestamp &t, 
                                     Cure::Pose3D &p);
  
  int getExtrapolationOutsideArchive(const Cure::Timestamp &t, 
                                     Cure::Pose3D &p,
                                     const double maxT);

  int calcExtrapolation(const Cure::Timestamp &t, 
                        const Cure::Pose3D &p1,
                        const Cure::Pose3D &p2,
                        Cure::Pose3D &p);
protected:
  /// minimum change required in either x,y,z from the
  ///previsouly stored pose to add a new pose [m]
  double m_MinXYZDiff;

  /// minAngdiff minimum change required in either of the euler
  /// angles from the previsouly stored pose to add a new pose [rad]
  double m_MinAngDiff;

  /// Last pose added to the PoseArchive but that might not be in the
  /// m_Archive which can be configured to store only poses when the
  /// robot has moved "enough"
  Cure::Pose3D m_LastAddedPose;
}; // class PoseArchive

}; // namespace Cure

#endif // Cure_PoseArchive_hh
