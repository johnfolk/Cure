//    $Id: Point2DCloud.hh,v 1.8 2007/09/14 09:14:01 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_POINT2DCLOUD_HH
#define CURE_POINT2DCLOUD_HH
#include "Matrix.hh"
namespace Cure {

/**
 * @author John Folkesson
 */
/**
 * A Point2DCloud is a set of (distance,x,y) points and some
 * functionality.  The 'distance' is not a z, but rather it is a
 * measure of the 'distance' from the other points in terms of the
 * correlation of the frames that the points are given in..  So that
 * points with very different distances are in frames that are very
 * uncertian relative to one another.  That is the robot moved some
 * disatnace and accumulated odometry errors making the points less
 * comparable.  This distance can be used to forget old points in the
 * cloud while holding on to the last few measurement points.  One is
 * free to define 'distance' as time, or path length of robot, or the
 * norm of the relative covariance between measurement poses...
 */
class Point2DCloud
{
public:
  Matrix Cloud;
  /**
  * Approx normal angle to the line with the x axis to within +/-PI/2.
  * This is the approximate viewing angle of the points.  It purpose
  * is to resolve the 180 degree ambiguity in the angle of the fitted
  * line to the points.
  */
  double Orientation; 
  /**
   *  This is the distance from the origin to the line along a normal
   *  to the line.
   */
  double Rho;
  /*
   * The angle the normal makes with the x-axis.  The normal is directed
   * from the origin to the infinite extension of the line segment
   * fitted to the points.  It is perpendicular to the line.
   */
  double Gamma;
  /** The center of the point cloud.*/
  double Center[2];
  /**Not implemented*/
  //  double Radius;
  /**
   *  This is the variance of the line position normal to the line.
   *  It is calculated as the sum of the SensorVar and the variance of the 
   *  data. 
   */
  double SigmaRho; 
  /**
   *  This is the variance of the line angle in radians.
   */
  double SigmaGamma;
  /**
   * A unit vector perpendicular to fitted line.  The normal is
   * directed from the origin to the infinite extension of the line
   * segment fitted to the points.  It is perpendicular to the line.
   */
  double Normal[2];
  /**
   *  This is the variance in the point locations due to the sensor
   *  uncertainty.
   *
   * @default .0001 m^2
   */
  double SensorVar;  
  /*
   * Flag 1 if the points are in the frame of the line.
   * @see Point2DCloud::lineFrame();
   */
  int LineFramed;
public:
  /**The consturctor*/
  Point2DCloud();
  /**
   * This initializes the cloud to c and assumes that the  orientation 
   * (viewing angle), is:
   *     Orientation=atan2((c(c..Rows-1,2)-c(0,2)),
   *	                   (c(0,1)-c(c.Rows-1,1))).
   *  This cooresponds to taking the sick scan points in order when building c.
   *  @param c the rows of c are the (distance,x,y) of the point cloud, the 
   *          order matters as the first and last rows will be used to 
   *          set the orientation.
   */
  void operator =(const Matrix & c);
  /**
   * Copy the cloud.
   * @param c a cloud to copy
   */
  void operator = (const Point2DCloud & c);
  /**
   * Get a particular ref to a value from the cloud.
   * @param r the row index (index to the point)
   * @param c the column (index to distance, x, y)
   *
   */
  double& operator() (const int r, const int c){
    return( Cloud(r,c) );
  }
  /**
   * Get a particular value from the cloud.
   * @param r the row index (index to the point)
   * @param c the column (index to distance, x, y)
   */
  double operator() (const int r, const int c) const { 
    return( (*(Point2DCloud *)this).operator()(r,c) );
  }
  /**
   * Rotate the Cloud to a frame with the x axis along the line, (from 
   * start to end for a scanner), and y in the normal direction, 
   * (perpendicular pointing at the wall from viewing position.) 
   * So x now gives a distance along the line (starting abitrarily) and y
   * gives the pependicular distance fromthe point to the fitted line.
   * After calling this the old points are overwritten and gone.
   */
  void lineFrame();
  /**
   * Return the index of first row with value in 1 column > x.
   * if there are none such returns the index of the last row.
   * If in the lineFrame and lineOrder have been called this can be used 
   * to find a point at a given 'distance' along the line. 
   */
  int findRow(double x);
  /**
   * This will take a point along the line corresponding to the
   * projection on a point from the clould on the line and extent it
   * as far as possible subject to the requirement that the cloud
   * points do not have a too large gap along the line.  
   * One must call lineFrame and lineOrder before this for it to work.
   * 
   * @param startrow The Row of the Cloud to start extending from.
   * @param tighness The maximum gap in the points projections on the line.
   * @param sense direction to extend if -1 goes from end to start direction
   *          else if 1 goes from start to end direction.
   * @return the row of the last point in the extended line
   */
  int extend(int startrow,double tightness,int sense);
  /**
   * Arrange the Cloud in accending order in the 'x' value.  Normally
   * used after lineFrame is called to put the cloud in order of
   * projected distance along the line.
   */
  void lineOrder();

  /**
   * Calculates the least square fit to a line. If in the line frame 
   * it adjusts the cloud to the new Normal.
   * After calling this on can read the Gamma, Rho , SigmaGamma, Center.
   * Before calling this those do not contain meaningful data.
   */
  int fitLine();
  /**
   * Adds a cloud to this either in the line frame or the map frame.
   * It also discards any old points, distance<minDistance.  
   * If in the line frame it orders the points.
   * @param c2 The points to be added 
   * @param minDistance points with distance less than this will be
   * discarded.
   */
  void merge(Point2DCloud & c2,double minDistance);
  /**
   * This changes previously added points at the same distance.  So if
   * you get a better set of points at that distance (ie you have
   * improved the pose estimate for the sensor at that distance), then
   * you can erase the old bad data and putn in the improved estimated
   * point locations.  
   * @param c2 The replacement points.
   */
  void replace(Point2DCloud & c2);
  /**
   * Get the effective measurement values of Gamma, Rho and
   * Covariance relative to some sensor frame.
   * Call fitLine() before calling this or you will get the
   * old values.  
   * @param gammaRho [relative Gamma, realtive Rho] returned here.
   * @param R the Covarance(r,c)<->R[r*2+c] returned here.
   * @param sensorXYTheta the sensor pose used to put Gamma and Rho in
   * the sensor frame.
   */
  void getLineValues(double gammaRho[2],double R[4], 
		    double sensorXYTheta[3]);
  /**
   * This will call (LineFrame if needed and) LineOrder and then
   * get the start and end points of the line.  The points are the
   * projections of the cloud points onto the fitted line.  Should
   * call fitLine before this for it to work.
   * @param x a 4 x 1 matrix with (xstart,ystart,xend,yend)^T returned in it.
   */
  void getEndpoints(Matrix & x);
  /**
   * This can be called repeatedly untill it returns 0 to get all
   * the continuous line segments in the  cloud.  Each call 
   * is made with start set to the return value of the previous call,
   * starting with 0.
   *
   * @param c1 This returns with the those points from this cloud
   * found to meet the requirements for a Line.  @param start The row
   * to start the search for a line.  @param tightness the maximum
   * open distance (with no points) along the line.  @param minCount
   * The minimum number of points in the returned cloud.  @param
   * minLength The minimum length of a line.  @param maxVariance The
   * maximum variance of the distance of the points from the line.
   * @return 0 if no line found, else the start for next search
   * (>=start+minCount).
   */
  int getLine(Point2DCloud &c1, int start, double tightness,
	      int minCount,double minLength, double maxVariance);
  /**
   * Remove a number of points from cloud starting at row start.
   * So after calling getLine you might want to remove the points in c1
   * form this cloud by calling 'remove(start,c1.Cloud.Rows);' 
   * @param start start row to remove points from.
   * @param number number of points to remove.
   */
  void remove(int start, int number);
};

} // namespace Cure

#endif // CURE_VECTOR3D_HH
