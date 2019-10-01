#include <iostream>
#include <stdlib.h> 
#include <fstream>
#include <string>
#include <unistd.h>
#include "CameraHelper.hh"

using namespace std;
using namespace Cure;

int main(int argc, char * argv[])
{
  const char *optstring = "hi:o:";
  char *args = "[-h help] [-i inputfile] [-o outputfile] ";
  char *inputfile = "-";
  char *outputfile = "camCalResult.txt";
  char o = getopt(argc, argv, optstring);
  while (o != -1) {
    switch (o) {
    case 'i':
      inputfile = optarg;
      break;
    case 'o':
      outputfile = optarg;
      break;
    case 'h':
      cerr<<"input file: "<<endl<<endl<< 
	"Principle_point_x    Principle_point_y   Focal_length"<<endl<< 
	"Priori_camera_offset_x   Priori_camera_offset_y  Priori_camera_offset_z"<<endl<< 
	"Priori_camera_offset_theta  Priori_camera_offset_phi  Priori_camera_offset_psi"<<endl;
      cerr<<"Number_of_robot_poses Number_of_image_points"<<endl;
      cerr<<"Robot_pose_0_x Robot_pose_0_y Robot_pose_0_theta (int)pixel_x1 (int)pixel_y1...(int)pixel_xN (int)pixel_yN"<<endl;
      cerr<<"Robot_pose_1_x Robot_pose_1_y Robot_pose_1_theta (int)pixel_x1 (int)pixel_y1...(int)pixel_xN (int)pixel_yN"<<endl;
      cerr<<"Robot_pose_2_x Robot_pose_2_y Robot_pose_2_theta (int)pixel_x1 (int)pixel_y1...(int)pixel_xN (int)pixel_yN"<<endl;
    case '?':
      std::cerr << "Usage: " << argv[0] << " " << args << std::endl;
      return -1;
      break;
    }
    o = getopt(argc, argv, optstring);
  }

  if (strcmp(inputfile, "-") == 0) {
    std::cerr << "You must give input file with calib data\n";
    std::cerr << "Usage: " << argv[0] << " " << args << std::endl;
    return -1;
  }

  ifstream rs(inputfile);
  ofstream ros(outputfile);
  double imageCenter[2];
  double focalLength;
  Cure::Transformation3D  prioriCamOffset;
  rs >>imageCenter[0]>>imageCenter[1]>>focalLength;
  double x[6];
  rs >>x[0]>>x[1]>>x[2]>>x[3]>>x[4]>>x[5];
  prioriCamOffset=x;
  CameraHelper camH(imageCenter,focalLength, 
		    prioriCamOffset);
  int numberPoses;
  rs>>numberPoses;
  int numberMatches;
  rs>>numberMatches;
  camH.setNumberOfPoints(numberPoses,numberMatches);
  Cure::Transformation3D robotpose;
  int pixel[2*numberMatches]; 
  while (rs >>x[0])
    {
      rs >>x[1]>>x[2];
      robotpose.setXYTheta(x);
      for (int i=0; i<numberMatches; i++)
	rs >>pixel[2*i]>>pixel[2*i+1];
      camH.recordPixels(robotpose,pixel);
    }
  Cure::Transformation3D result;   
  camH.getCameraOffset(result);   
  result.getCoordinates(x);
  cerr<<"RESULT iter 0:"<<endl;
  result.print();
  ros<<x[0]<<" "<<x[1]<<" "<<x[2]<<" ";
  ros<<x[3]<<" "<<x[4]<<" "<<x[5]<<endl;
 
  camH.setPrioriCameraOffset(result);
  camH.getCameraOffset(result);     
  result.getCoordinates(x);
  cerr<<"RESULT iter 1:"<<endl;
  result.print();
  for (int i=0; i<100; i++)
    {
      camH.setPrioriCameraOffset(result);
      camH.getCameraOffset(result);
    }   
  ros<<x[0]<<" "<<x[1]<<" "<<x[2]<<" ";
  ros<<x[3]<<" "<<x[4]<<" "<<x[5]<<endl;
  camH.setPrioriCameraOffset(result);
  camH.getCameraOffset(result);     
  result.getCoordinates(x);
  cerr<<"RESULT iter 102:"<<endl;
  result.print();
  ros<<x[0]<<" "<<x[1]<<" "<<x[2]<<" ";
  ros<<x[3]<<" "<<x[4]<<" "<<x[5]<<endl;
 
  camH.setPrioriCameraOffset(result);
  camH.getCameraOffset(result);     
  result.getCoordinates(x);
  cerr<<"RESULT iter 103:"<<endl;
  result.print();
  ros<<x[0]<<" "<<x[1]<<" "<<x[2]<<" ";
  ros<<x[3]<<" "<<x[4]<<" "<<x[5]<<endl;
  ros.close();

  std::cerr << std::endl << std::endl;
  std::cout << "Resulting camera offset (paste into config file):\n";
  std::cout << x[0] << " " << x[1] << " " << x[2] << " "
            << x[3] << " " << x[4] << " " << x[5] << std::endl;
  
  return 0;
  
}








