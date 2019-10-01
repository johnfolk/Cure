// = AUTHOR(S)
//    John Folkesson
//    
//    March 11, 2004
//
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CAMERAHELPER_H
#define CAMERAHELPER_H

#include "Transformation3D.hh"
/**
 * This helper can take the pixel's cooresponding to the same point in 
 * N images taken from N different robot poses and calculate the offset
 * from the robot pose to the camera.
 * One must provide the camera parameters focallength and 'principle points'
 * (imageCenter).  Also a approximate offset is needed.  The translation in
 * the approximate offset is assumed correct but the rotation will be 
 * recalculated.
 *
 * One can call this repeatedly using different points and then average the 
 * results.
 * 
 * So for one point one must call these:
 *
 * setCameraCalibration(imageCenter,focalLength);
 * setPrioriCameraOffset(prioriCamOffset);
 * setNumberOfPoints(numPoses,nMatches);
 * recordPixel(robotpose_1, pixels_1);
 * recordPixel(robotpose_2, pixels_2);
 * recordPixel(robotpose_3, pixels_3);
 * ...
 * getCameraOffset(result);
 *
 * Typical image data has (u,v) as integer pixels in a frame with (0,0)
 * in the upper left corner and u increasing as you move right
 * and v as you move down.
 *
 * My pix coordinates are defined as:
 * pix_x = (u - Center_u)
 * pix_y = (v - Center_v)
 * So that 
 *     |    -pix_x   |
 * V = |  focal_length | is the 3D vector towards the point in the Camera 
 *     |    pix_y   | Frame. The camera axis is along Y direction. 
 * Where focal_length is in pixels
 *
 */
class CameraHelper
{
public:
  Cure::Transformation3D *T, PrioriCameraOffset, PostCameraOffset;
  double ImageCenter[2];
  double FocalLength;
  Cure::Matrix Pix;
protected:
  int NumberMatches;
  int NumberPoses;
  int NextPose;
public:
  CameraHelper(){
    NextPose=-1;
    T=0;
    setNumberOfPoints(3,1);
  }
  CameraHelper(double imageCenter[2], double focalLength, 
	       Cure::Transformation3D & prioriCamOffset){
    NextPose=-1;
    setCameraCalibration(imageCenter,focalLength);
    setPrioriCameraOffset(prioriCamOffset);
    T=0;
    setNumberOfPoints(3,1);
  }
  ~CameraHelper(){
    if (T) delete [] T;
    T=0;
  }
  /*
   * @imageCenter the pixel that the optical axis passes thru.
   * @param focalLength in pixels
   */
  void setCameraCalibration(double imageCenter[2],double focalLength){
    ImageCenter[0]=imageCenter[0];
    ImageCenter[1]=imageCenter[1];
    FocalLength=focalLength;
  }
  /**
   * @param prioriCamOffset The approximate offset from the robot 
   *                        to the cammera the translation must be correct.
   */
  void  setPrioriCameraOffset(Cure::Transformation3D &prioriCamOffset){
    PrioriCameraOffset=prioriCamOffset;    
  }
  void setNumberOfPoints(int numPoses,int nMatches);
  int getNumberMatches(){
    return NumberMatches;
  }
  int getNumberPoses(){
    return NumberPoses;
  }
  /**
   * Use this to set both the pixel and the robot pose in one go.
   * After calling this 3 times one can getCameraOffset.
   * @param robotpose the pose of the frame you want the relative cammera 
   *                  offset from at the time of the pixels
   * @param pixels This is exactly the pixels of the image points given by 
   *               examining the raw image as (ux,uy,...
  */
  void  recordPixels(Cure::Transformation3D &robotpose, int *pixels); 
  /**
   * This calculates a new 'Post' camear offset after calling recordPixel
   * 3 times.  Untill then it returns the last calculated offset.
   * After calling this one can start inputing another pixel.
   *
   * @param result the new camera offset is returned here
  */
  void getCameraOffset(Cure::Transformation3D &result);
  /**
   * Call if you want to start over entering  pixels from the first Pose.
   */
  void reset(){NextPose=-1;}
protected:
  void calcPostOffset();

};

#endif
