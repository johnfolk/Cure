// = FILENAME
//    HelpFunctions.hh 
//
// = FUNCTION
//
// = AUTHOR(S)
//    Patric Jensfelt
//
// = COPYRIGHT
//    Copyright (c) 2003 Patric Jensfelt
//
/*----------------------------------------------------------------------*/

#ifndef CURE_HELPFUNCTIONS_HH
#define CURE_HELPFUNCTIONS_HH

#ifndef DEPEND
#include <sys/time.h>
#include <cmath>
#include <cstdio>
#include <string>
#endif

#ifndef TWOPI
#define TWOPI              6.28318530717959
#endif

#ifndef HELPFCN_MAX
#define HELPFCN_MAX(a,b) ((a)>(b)?(a):(b))
#endif

#ifndef HELPFCN_MIN
#define HELPFCN_MIN(a,b) ((a)<(b)?(a):(b))
#endif

namespace Cure {

/**
 * This namespace contains various help functions for going between
 * units, calculating angle differences, distances between points and
 * lines, etc
 *
 * @author Patric Jensfelt
 * @see
 */
namespace HelpFunctions {
  //===========================================================================
  //============================ Time stuff ===================================
  //===========================================================================

  /**
   * @return the current time
   */
  double getCurrentTime();

  //===========================================================================
  //======================== Print numbers ====================================
  //===========================================================================

  /**
   * @return a string out of a double
   */
  std::string doubleToString(double v);

  /**
   * @return a string out of an int
   */
  std::string intToString(int v);

  /**
   * @return a string out of an long
   */
  std::string longToString(long v);


  //===========================================================================
  //========================= max,min etc =====================================
  //===========================================================================

  /**
   * @return biggest of two numbers
   */
  double max(double a, double b);

  /**
   * @return biggest of two numbers
   */
  long max(long a, long b);

  /**
   * @return biggest of two numbers
   */
  int max(int a, int b);

  /**
   * @return smallest of two numbers
   */
  double min(double a, double b);

  /**
   * @return smallest of two numbers
   */
  long min(long a, long b);

  /**
   * @return smallest of two numbers
   */
  int min(int a, int b);

  //===========================================================================
  //===========================================================================
  //===========================================================================

  /**
   * Limit the value of a variable to be between min and max
   *
   * @param v variable to apply limits to
   * @param min lower limit for the variable
   * @param max upper limit for the variable
   * @return the limited value
   */
  double limitValue(double v, double min, double max);

  /**
   * Limit the mangitude of a variable to be between -maxMag and +maxMag
   *
   * @param v variable to apply limits to
   * @param maxMag the limit for the magnitude
   * @return the limited value
   */
  double limitValueSymm(double v, double maxMag);

  /**
   * Limit the value of a variable to be between min and max
   *
   * @param v variable to apply limits to
   * @param min lower limit for the variable
   * @param max upper limit for the variable
   * @return the limited value
   */
  double limitAndSetValue(double &v, double min, double max);

  /**
   * Limit the mangitude of a variable to be between -maxMag and +maxMag
   *
   * @param v variable to apply limits to
   * @param maxMag the limit for the magnitude
   * @return the limited value
   */
  double limitAndSetValueSymm(double &v, double maxMag);

  //===========================================================================
  //======================== Angle stuff ======================================
  //===========================================================================

  /** 
   * @return the angle [-pi, pi) 
   */
  double modPI(const double ang);

  /** 
   * @return the angle [0, 2*pi) 
   */
  double mod2PI(const double ang);

  /** 
   * Transforms angle in degs to radians 
   */
  double deg2rad(const double ang);

  /** 
   * Transforms angle in radians to degs 
   */
  double rad2deg(const double ang);

  /**
   * Makes sure that a is [0, 2pi)
   */
  void normalizeAngle(float &a);

  /**
   * @return angle difference between a1 and a2 [-pi, pi)
   */  
  double angleDiffRad(const double a1, const double a2);

//===========================================================================
//================ Conversion between distance units ========================
//===========================================================================

  /** Transforms a distance in inches to mm */
  double inch2mm(const double len);

  /** Transforms a distance in inches to m */
  double inch2m(const double len);

  /** Transforms a distance in tens of inches to mm */
  double tinch2mm(const double len);

  /** Transforms a distance in tens of inches to m */
  double tinch2m(const double len);

  /** Transforms a distance in mm to inches */
  double mm2inch(const double len);

  /** Transforms a distance in mm to tens of inches */
  double mm2tinch(const double len);

  /** Transforms a distance in m to inches */
  double m2inch(const double len);

  /** Transforms a distance in m to tens of inches */
  double m2tinch(const double len);

  /** Transforms a distace in m to mm */ 
  double m2mm(const double len);

  /** Transforms a distace in mm to m */ 
  double mm2m(const double len);

  //===========================================================================
  //======================== Distance stuff ===================================
  //===========================================================================

  /**
   * Returns the distance from (x,y) to the line given by angle phi
   * through point (xL, yL). The distance is returned with sign where
   * positive means that the point is to the right of the line given that
   * you are facing in the direction of the line.
   *
   * Algorithm: scalar product with direction cosine pointing normal to
   * the right of the line, that in direction phi - 90 degs. Note that
   * sin(x-90) = -cos(x) and cos(x-90) = sin(x).
   * 
   * @param x x coord of point [mm]
   * @param y y coord of point [mm]
   * @param xL x coord of point on the line [mm]
   * @param yL y coord of point on the line [mm]
   * @param phi direction of the line [rad] 
   */
  double distPt2Line(double x, double y, 
		      double xL, double yL, double phi);

  /**
   * Returns the distance from (x,y) to the line given by angle phi
   * through point (xL, yL). The special thing about this function is
   * that instead of sending in the angle of the line you send in the
   * direction cosines. The distance is returned with sign where
   * positive means that the point is to the right of the line given that
   * you are facing in the direction of the line.
   *
   * Algorithm: scalar product with direction cosine pointing normal to
   * the right of the line, that in direction phi - 90 degs. Note that
   * sin(x-90) = -cos(x) and cos(x-90) = sin(x).
   * 
   * @param x x coord of point [mm]
   * @param y y coord of point [mm]
   * @param xL x coord of point on the line [mm]
   * @param yL y coord of point on the line [mm]
   * @param cosL cos(phi) where phi is direction of the line.
   * @param sinL sin(phi) where phi is direction of the line.
   */
  double distPt2Line(double x, double y, 
		      double xL, double yL, 
		      double cosL, double sinL);

  /**
   * Returns the distance from P(x,y) to the line given by two points:
   * A(x,y) and B(x,y). The distance is signed, where positive means that
   * the point is to the right of the line facing from A to B.
   *
   * @param xA x coord of point A
   * @param yA y coord of point A
   * @param xB x coord of point B
   * @param yB y coord of point B
   * @param xP x coord of point C
   * @param yP y coord of point C
   */
  double distPt2LinePts(double xA, double yA,
			double xB, double yB,
			double xP, double yP);

  /**
   * @return the squared distance between between point (x1,y1) and
   * (x2,y2)
   */
  double sqrDistPt2Pt(double x1, double y1, double x2, double y2);
  
  /**
   * Function that calculates the squared distance between a point
   * (x,y) and the closest point on a line segment between the end
   * points (xS,yS) and (xE,yE).
   *
   * @param x x-coord for point
   * @param y y-coord for point
   * @param xS x-coord for start point
   * @param yS y-coord for start point
   * @param xE x-coord for end point
   * @param yE y-coord for end point
   * @param t position for closest point on line
   *
   * @return squared shortest distance between a point and a line segment
   */
  double sqrDistPt2Segment(double x, double y,
                           double xS, double yS, double xE, double yE,
                           double *t = NULL);

  /**
   * Function that calculates the distance between a point (x,y) and
   * the closest point on a line segment between the end points
   * (xS,yS) and (xE,yE). This function will use the function
   * sqrDistPt2Segment(...)
   *
   * @param x x-coord for point
   * @param y y-coord for point
   * @param xS x-coord for start point
   * @param yS y-coord for start point
   * @param xE x-coord for end point
   * @param yE y-coord for end point
   * @param t position for closest point on line
   *
   * @return shortest distance between a point and a line segment
   */
  double distPt2Segment(double x, double y,
                        double xS, double yS, double xE, double yE,
                        double *t = NULL);


  /**
   * Function that calculates the shortest distance between two
   * segments. If they intersect the distance is 0. 
   * 
   * @param x1s x-coordinate of start point for segment 1
   * @param y1s y-coordinate of start point for segment 1
   * @param x1e x-coordinate of end point for segment 1
   * @param y1e y-coordinate of end point for segment 1
   * @param x2s x-coordinate of start point for segment 2
   * @param y2s y-coordinate of start point for segment 2
   * @param x2e x-coordinate of end point for segment 2
   * @param y2e y-coordinate of end point for segment 2
   */
  double shortestSegmentDist(double x1s, double y1s, double x1e, double y1e,
                             double x2s, double y2s, double x2e, double y2e);


  //===========================================================================
  //======================== Line intersection ================================
  //===========================================================================

  /**
   * Check for intersection between two lines A and B described by
   * two points which the line passes through. 
   * 
   *   lineA: (lAx1,lAy1) -> (lAx2,lAy2)
   *   lineB: (lBx1,lBy1) -> (lBx2,lBy2)
   * 
   * tA and tB (if non-NULL) will be set if an intersection is
   * found. tA=0 corresponds to the first point of line A and tA=1 to
   * its second points. The same applies to tB.
   *
   * The intersection point is thus
   *
   * x = lAx1 + tA * (lAx2 - lAx1)
   * y = lAy1 + tA * (lAy2 - lAy1)
   * 
   * @return true if the lines intersect, else false
   */
  bool linesIntersect(double lAx1, double lAy1, 
                      double lAx2, double lAy2,
                      double lBx1, double lBy1, 
                      double lBx2, double lBy2,
                      double *tA = NULL, double *tB = NULL);

  /**
   * Check for intersection between two line segments A and B
   * described by their start and end points
   * 
   *   lineA: (lAxS,lAyS) -> (lAxE,lAyE)
   *   lineB: (lBxS,lByS) -> (lBxE,lByE)
   * 
   * tA and tB will be set (if non-NULL) if an intersection is
   * found. tA=0 corresponds to the start point of line A and tA=1 to
   * its end points. The same applies to tB. An intersection requires
   * that 0<=tA<=1 and 0<=tB<=1.
   *
   * The intersection point is given by
   *
   * x = lAxS + tA * (lAxE - lAxS)
   * y = lAyS + tA * (lAyE - lAyS)
   *
   * @return true if the segments intersect, else false
   */
  bool segmentsIntersect(double lAxS, double lAyS, 
                         double lAxE, double lAyE,
                         double lBxS, double lByS, 
                         double lBxE, double lByE,
                         double *tA = NULL, double *tB = NULL);
  
  //===========================================================================
  //======================== Transformations ==================================
  //===========================================================================
  
  /** 
   * This function will take a point (x, y) expressed in world
   * coordinates and transform them to a local coordinate system. The
   * origin of the local coordinate system is at position (tX, tY)
   * expressed in the world coordinates and is rotated an angle rot
   * [rad].
   *
   * @param x x-coordinate to transform (will be changed!)
   * @param y y-coordinate to transform (will be changed!)
   * @param tX x-coord of the origin of the local coord. in the world coord
   * @param tY y-coord of the origin of the local coord. in the world coord
   * @param rot angle from world coord x-axis to local coord. x-axis
   */
  void transfPtW2L(double &x, double &y, double tX, double tY, double rot);

  /** 
   * This function will take a point (x, y) expressed in world
   * coordinates and transform them to a local coordinate system. The
   * origin of the local coordinate system is at position (tX, tY)
   * expressed in the world coordinates and is rotated an angle rot
   * [rad].
   *
   * @param x x-coordinate to transform (will be changed!)
   * @param y y-coordinate to transform (will be changed!)
   * @param tX x-coord of the origin of the local coord. in the world coord
   * @param tY y-coord of the origin of the local coord. in the world coord
   * @param cosR cos(rot angle from world coord x-axis to local coord. x-axis)
   * @param sinR sin(rot angle from world coord x-axis to local coord. x-axis)
   */
  void transfPtW2L(double &x, double &y, double tX, double tY, 
                   double cosR, double sinR);

  /** 
   * This function will take a point (x, y) expressed in local
   * coordinates and transform them to a world coordinate system. The
   * origin of the local coordinate system is at position (tX, tY)
   * expressed in the world coordinates and is rotated an angle rot
   * [rad].
   *
   * @param x x-coordinate to transform (will be changed!)
   * @param y y-coordinate to transform (will be changed!)
   * @param tX x-coord of the origin of the local coord. in the world coord
   * @param tY y-coord of the origin of the local coord. in the world coord
   * @param rot angle from world coord x-axis to local coord. x-axis
   */
  void transfPtL2W(double &x, double &y, double tX, double tY, double rot);

  /** 
   * This function will take a point (x, y) expressed in local
   * coordinates and transform them to a world coordinate system. The
   * origin of the local coordinate system is at position (tX, tY)
   * expressed in the world coordinates and is rotated an angle rot
   * [rad].
   *
   * @param x x-coordinate to transform (will be changed!)
   * @param y y-coordinate to transform (will be changed!)
   * @param tX x-coord of the origin of the local coord. in the world coord
   * @param tY y-coord of the origin of the local coord. in the world coord
   * @param cosR cos(rot angle from world coord x-axis to local coord. x-axis)
   * @param sinR sin(rot angle from world coord x-axis to local coord. x-axis)
   */
  void transfPtL2W(double &x, double &y, double tX, double tY,
                   double cosR, double sinR);

};

//===========================================================================
//===========================================================================
//===========================================================================

//===========================================================================
inline double 
HelpFunctions::getCurrentTime()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (tv.tv_sec + 1.0e-6*tv.tv_usec);
}

//===========================================================================
//===========================================================================
//===========================================================================

//===========================================================================
inline std::string
HelpFunctions::doubleToString(double v)
{
  char buf[64];
  std::sprintf(buf, "%f", v);
  return buf;
}

//===========================================================================
inline std::string
HelpFunctions::intToString(int v)
{
  char buf[64];
  std::sprintf(buf, "%d", v);
  return buf;
}

//===========================================================================
inline std::string
HelpFunctions::longToString(long v)
{
  char buf[64];
  std::sprintf(buf, "%ld", v);
  return buf;
}

//===========================================================================
//===========================================================================
//===========================================================================
inline double
HelpFunctions::max(double a, double b)
{
  return HELPFCN_MAX(a,b);
}

inline long
HelpFunctions::max(long a, long b)
{
 return HELPFCN_MAX(a,b);
}

inline int
HelpFunctions::max(int a, int b)
{
  return HELPFCN_MAX(a,b);
}

inline double
HelpFunctions::min(double a, double b)
{
  return HELPFCN_MIN(a,b);
}

inline long
HelpFunctions::min(long a, long b)
{
  return HELPFCN_MIN(a,b);
}

inline int
HelpFunctions::min(int a, int b)
{
  return HELPFCN_MIN(a,b);
}

//===========================================================================
//===========================================================================
//===========================================================================

inline double 
HelpFunctions::limitValue(double v, double min, double max)
{
  if (v > max) v = max;
  else if (v < min) v  = min;
  return v;
}

//===========================================================================

inline double 
HelpFunctions::limitValueSymm(double v, double maxMag)
{
  if (v > maxMag) v = maxMag;
  else if (v < -maxMag) v  = -maxMag;
  return v;
}

//===========================================================================

inline double 
HelpFunctions::limitAndSetValue(double &v, double min, double max)
{
  if (v > max) v = max;
  else if (v < min) v  = min;
  return v;
}

//===========================================================================

inline double 
HelpFunctions::limitAndSetValueSymm(double &v, double maxMag)
{
  if (v > maxMag) v = maxMag;
  else if (v < -maxMag) v  = -maxMag;
  return v;
}

//===========================================================================
//===========================================================================
//===========================================================================


//===========================================================================
inline double 
HelpFunctions::modPI(const double ang)
{ 
  double tmp = ang;
  
  // Make sure that it is larger than or equal to -pi 
  while (tmp < -M_PI)
    tmp += TWOPI;
  
  // Make sure that it is less than pi
  while (tmp > M_PI)
    tmp -= TWOPI;
  
  return tmp;
}

//===========================================================================
inline double 
HelpFunctions::mod2PI(const double ang)
{ 
  double tmp = ang;
  
  // Make sure that it is larger than or equal to 0
  while (tmp < 0)
    tmp += TWOPI;
  
  // Make sure that it is less than 2*pi
  while (tmp > TWOPI)
    tmp -= TWOPI;
  
  return tmp;
}

//===========================================================================
inline double 
HelpFunctions::deg2rad(const double ang) 
{ 
  return M_PI / 180.0 * ang; 
}

//===========================================================================
inline double 
HelpFunctions::rad2deg(const double ang)
{ 
  return 180.0 / M_PI * ang; 
}


//===========================================================================
inline void 
HelpFunctions::normalizeAngle(float &a)
{
  while (a < 0) {
    a += TWOPI;
  }
  
  while (a >= TWOPI) {
    a -= TWOPI;
  }
}
  
//===========================================================================
inline double 
HelpFunctions::angleDiffRad(const double a1, const double a2)
{
  double tmp = a1 - a2;
  
  while (tmp < -M_PI)
    tmp += TWOPI;
  
  while (tmp >= M_PI)
    tmp -= TWOPI;
  
  return tmp;
}

//===========================================================================
//===========================================================================
//===========================================================================

//===========================================================================
inline double 
HelpFunctions::inch2mm(const double len)
{ return 25.4 * len; }

//===========================================================================
inline double 
HelpFunctions::inch2m(const double len) 
{ return 0.0254 * len; }

//===========================================================================
inline double 
HelpFunctions::tinch2mm(const double len)
{ return 2.54 * len; }

//===========================================================================
inline double 
HelpFunctions::tinch2m(const double len)
{ return 0.00254 * len; }

//===========================================================================
inline double 
HelpFunctions::mm2inch(const double len)
{ return len / 25.4; }

//===========================================================================
inline double 
HelpFunctions::mm2tinch(const double len)
{ return len / 2.54; }

//===========================================================================
inline double 
HelpFunctions::m2inch(const double len)
{ return len / 0.0254; }

//===========================================================================
inline double 
HelpFunctions::m2tinch(const double len)
{ return len / 0.00254; }

//===========================================================================
inline double 
HelpFunctions::m2mm(const double len)
{ return len * 1000.0; }

//===========================================================================
inline double 
HelpFunctions::mm2m(const double len)
{ return len * 0.001; }


//===========================================================================
//===========================================================================
//===========================================================================

//===========================================================================
inline double 
HelpFunctions::distPt2Line(double x, double y, 
			   double xL, double yL, double phi)
{ 
  return std::sin(phi) * (x - xL) - std::cos(phi) * (y - yL); 
}

//===========================================================================
inline double 
HelpFunctions::distPt2Line(double x, double y, 
			   double xL, double yL, 
			   double cosL, double sinL)
{ 
  return sinL * (x - xL) - cosL * (y - yL); 
}

//===========================================================================
inline double
HelpFunctions::distPt2LinePts(double xA, double yA,
			      double xB, double yB,
			      double xP, double yP)
{ 
  return ((xP-xA)*(yB-yA)-(yP-yA)*(xB-xA))/hypot(yB-yA,xB-xA); 
}

//===========================================================================
inline double 
HelpFunctions::sqrDistPt2Pt(double x1, double y1, double x2, double y2)
{
  double dx = x2 - x1;
  double dy = y2 - y1;
  return (dx * dx + dy * dy);
}

//===========================================================================
inline double 
HelpFunctions::sqrDistPt2Segment(double x, double y,
                                 double xS, double yS, double xE, double yE,
                                 double *t)
{
  // The vector from the start point of the line to the point
  double vx = x - xS;
  double vy = y - yS;

  // Direction vector for the line
  double dx = xE - xS;
  double dy = yE - yS;

  // The dot product between the vector above and the line
  double proj = vx * dx + vy * dy;

  // Take care of the different cases and normalize proj so that it is in
  // [0,1] where 0 is the start point and 1 is the end point
  if (proj < 0) {
    // The projection of the point ends up before the start point,
    // i.e. outside the segment. This means that the start point is
    // the closest point
    proj = 0;
  } else {
    double sqrLineLen = dx * dx + dy * dy;
    if (proj >= sqrLineLen) {
      // The projection of the point ends up after the end point,
      // i.e. outside the segment. This means that the end point is
      // the closest point. We have to change the vector to the point
      // ot be from the end point instead.
      proj = 1;
      vx = x - xE;
      vy = y - yE;
    } else {
      // Projection is inside the segment, normalize t to be in [0,1]
      proj /= sqrLineLen;
      vx -= proj * dx;
      vy -= proj * dy;
    }
  }

  if (t != NULL) {
    *t = proj;
  }

  return (vx * vx + vy * vy);
}

//===========================================================================
inline double 
HelpFunctions::shortestSegmentDist(double x1s, double y1s, 
                                   double x1e, double y1e,
                                   double x2s, double y2s, 
                                   double x2e, double y2e)
{
  // Begin by checking for an intersecion because an intersection will
  // mean 0 distance and we can stop.
  if (segmentsIntersect(x1s, y1s, x1e, y1e,
                        x2s, y2s, x2e, y2e)) {
    return 0;
  }
      
  // Calculate squared projected distances between end point of one
  // segment and the other segment
  double sqrd1sl2 = sqrDistPt2Segment(x1s, y1s, 
                                      x2s, y2s, x2e, y2e);
  double sqrd1el2 = sqrDistPt2Segment(x1e, y1e, 
                                      x2s, y2s, x2e, y2e);
  double sqrd2sl1 = sqrDistPt2Segment(x2s, y2s, 
                                      x1s, y1s, x1e, y1e);
  double sqrd2el1 = sqrDistPt2Segment(x2e, y2e, 
                                      x1s, y1s, x1e, y1e);

  // Get the smallest distance
  double minSqrD = sqrd1sl2;
  if (sqrd1el2 < minSqrD) minSqrD = sqrd1el2;
  if (sqrd2sl1 < minSqrD) minSqrD = sqrd2sl1;
  if (sqrd2el1 < minSqrD) minSqrD = sqrd2el1;

  return  sqrt(minSqrD);
}


//===========================================================================
inline double 
HelpFunctions::distPt2Segment(double x, double y,
                      double xS, double yS, double xE, double yE,
                      double *t)
{
  return sqrt(sqrDistPt2Segment(x, y, xS, yS, xE, yE, t));
}

//===========================================================================
//===========================================================================
//===========================================================================

//===========================================================================
inline bool 
HelpFunctions::linesIntersect(double lAx1, double lAy1, 
                              double lAx2, double lAy2,
                              double lBx1, double lBy1, 
                              double lBx2, double lBy2,
                              double *tA, double *tB)
{
  // Base on algorithm found on
  // http://geometryalgorithms.com/Archive/algorithm_0104/algorithm_0104B.htm

  double ux = lAx2 - lAx1;
  double uy = lAy2 - lAy1;
  double vx = lBx2 - lBx1;
  double vy = lBy2 - lBy1;

  double D = ux * vy - uy * vx;

  // First we check if the lines are close to parallell
  if (fabs(D) < 1e-8) {
    return false;
  }

  if (tA != NULL || tB != NULL) {
    double wx = lAx1 - lBx1;
    double wy = lAy1 - lBy1;
    
    if (tA != NULL) 
      *tA = (vx * wy - vy * wx) / D;
    if (tB != NULL) 
      *tB = (ux * wy - uy * wx) / D;
  }

  return true;
}

//===========================================================================

inline bool 
HelpFunctions::segmentsIntersect(double lAxS, double lAyS, 
                                 double lAxE, double lAyE,
                                 double lBxS, double lByS, 
                                 double lBxE, double lByE,
                                 double *tA, double *tB)
{
  double tAA, tBB;
  if (linesIntersect(lAxS, lAyS, lAxE, lAyE,
                     lBxS, lByS, lBxE, lByE, &tAA, &tBB)) {
    if (0 <= tAA && tAA <= 1 && 0 <= tBB && tBB <= 1) {
      if (tA != NULL) *tA = tAA;
      if (tB != NULL) *tB = tBB;
      return true;
    }
  }
                     
  return false;
}

//===========================================================================
//===========================================================================
//===========================================================================

//===========================================================================
inline void 
HelpFunctions::transfPtW2L(double &x, double &y, 
			     double tX, double tY, double rot)
{
  double cosa = std::cos(rot);
  double sina = std::sin(rot);
  double tmpX = x;
  double tmpY = y;
  x =  (tmpX - tX) * cosa + (tmpY - tY) * sina;
  y = -(tmpX - tX) * sina + (tmpY - tY) * cosa;
}

inline void 
HelpFunctions::transfPtW2L(double &x, double &y, 
                           double tX, double tY,
                           double cosR, double sinR)
{
  double tmpX = x;
  double tmpY = y;
  x =  (tmpX - tX) * cosR + (tmpY - tY) * sinR;
  y = -(tmpX - tX) * sinR + (tmpY - tY) * cosR;
}

//===========================================================================
inline void 
HelpFunctions::transfPtL2W(double &x, double &y, 
			     double tX, double tY, double rot)
{
  double cosa = std::cos(rot);
  double sina = std::sin(rot);
  double tmpX = x;
  double tmpY = y;
  x = tmpX * cosa - tmpY * sina + tX;
  y = tmpX * sina + tmpY * cosa + tY;
}
inline void 
HelpFunctions::transfPtL2W(double &x, double &y, 
                           double tX, double tY, 
                           double cosR, double sinR)
{
  double tmpX = x;
  double tmpY = y;
  x = tmpX * cosR - tmpY * sinR + tX;
  y = tmpX * sinR + tmpY * cosR + tY;
}

} // namespace Cure

#endif // CURE_HELPFUNCTIONS_HH

