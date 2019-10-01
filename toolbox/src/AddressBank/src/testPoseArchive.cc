//
// = FILENAME
//    testPoseArchive.cc
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

int main()
{
  Cure::PoseArchive archive;

  Cure::Pose3D odom;

  odom.setX(1);
  odom.setY(1);
  odom.setTheta(0);
  odom.setTime(1);
  if (archive.addPose(odom)) std::cerr << "Not added pose " << odom << "\n";

  odom.setX(2);
  odom.setY(-2);
  odom.setTheta(M_PI_2);
  odom.setTime(2);
  if (archive.addPose(odom)) std::cerr << "Not added pose " << odom << "\n";

  odom.setX(4);
  odom.setY(-6);
  odom.setTheta(M_PI);
  odom.setTime(3);
  if (archive.addPose(odom)) std::cerr << "Not added pose " << odom << "\n";

  odom.setX(4.001);
  odom.setY(-6.001);
  odom.setTheta(M_PI);
  odom.setTime(4);
  if (archive.addPose(odom)) std::cerr << "Not added pose " << odom << "\n";

  std::cout << "=============================================\n";

  int err = archive.getExtrapolation(4.5, odom);
  if (err) std::cerr << "Got err=" << err << std::endl;
  
  std::cout << odom.getTime() << std::endl;
  std::cout << odom << std::endl;

  std::cout << "=============================================\n";

  Cure::Pose3D worldPose;
  worldPose.setX(6.5);
  worldPose.setY(2.5);
  worldPose.setTheta(M_PI_2);
  worldPose.setTime(1);

  err = archive.getPoseExtrapolation(worldPose, 4.5, worldPose);
  if (err) std::cerr << "Got err=" << err << std::endl;
  
  std::cout << worldPose.getTime() << std::endl;
  std::cout << worldPose << std::endl;

  
}
