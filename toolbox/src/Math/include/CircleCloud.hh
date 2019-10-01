// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_CIRCLECLOUD_HH
#define CURE_CIRCLECLOUD_HH

#include "LinkedArray.hh"
#include "Matrix.hh"
//#include "Transformation3D.hh"
namespace Cure {
/**
 * @author John Folkesson
 */
/**
 * A CircleCloud is a set of points (distance x,y,x²,y²,xy,x³,y³,x²y,xy²) 
 * and some functionality to fit to the points circles in the xy plane.
 * The 'distance' is not a z, but rather it is a measure of the 'distance' 
 * from the other points in terms of coorelation.  So that points with very
 * different distances are uncoorelated.  
 * This distance can be used to forget old points in the cloud while holding
 * on to the last few measurement points.  One is free to define 'distance'
 * as time, or path length of robot, or the norm of the relative covariance 
 * between measurment poses... 
 */
class CircleCloud
{
public:
  /**
   * Each LinkedArray contains a vector:
   * (distance x,y,x²,y²,xy,x³,y³,x²y,xy²) 
   * for a point measurement.
   */
  Cure::LinkedArray Cloud;
  /**
   * The estimated Center position of the circle in the xy plane.
   */
  double Center[2];
  double Radius;
  /**
   * This is and estimate of the covariance matrix for the vector
   * (Center[0], Center[1], Radius). 
   */
  double Cov[9];
  /**
   * This is the statistical variance in the Radius based on the Center.
   */
  double VarRadius;
  /*
   * Intrinsic variance in the point locations. 
   */
  double SensorVar;  
  /**
   * This keeps track of the sums of products needed for the fitting.
   * sum(x,y,x²,y²,xy,x³,y³,x²y,xy²)
   */
  double Sums[9];
  /**
   * Sums/Cloud.Rows
   */
  int CloudCount;
public:
  CircleCloud();

  /*
   * Adds points to the cloud.
   * Takes a set of distance, x,y points in the map frame and starts the 
   * Cloud with them.
   * @param c a N x 3 matrix of absolute (distance,x,y) values. 
   * @param minDistance if !=0 then points with distance<minDistance
   * are first discarded before points from c are added.
   */
  void merge(const Cure::Matrix & c,double minDistance=0);

  void operator = (const CircleCloud & c);
    double& operator() (const int r, const int c){
    return( Cloud(r,c) );
  }
  double operator() (const int r, const int c) const { 
    return( (*(CircleCloud *)this).operator()(r,c) );
  }


  /**
   * Calculates the least square fit to a circcle.
   * Minimizes the square error in the radius squared,
   * d=sum [(x-cx)²+(y-cy)²-R²]².
   * After calling this one can use the vector 
   * (Center[0], Center[1], Radius) and its Covariance matrix Cov.
   */
  int fitCircle();
  /*
   * Use the current Center position and calculate the 
   * mean Radius and VarRadius.
   */
  int fitRadius();
  /**
   * @param minDistance removes all points with distance<minDistance
   * if minDistance=0 then removes none. 
   * @return a pointer to the LinkedArray at the end of the 
   * chain starting at cloud. 
   */
  Cure::LinkedArray * prune(double minDistance=0);
};
}
#endif
