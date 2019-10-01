//    $Id: Line3D.hh,v 1.4 2007/09/14 09:13:51 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_LINE3D_HH
#define CURE_LINE3D_HH

#include "Point3D.hh"
#include "Line2D.hh"

namespace Cure{

/**
 * Wrapper class for a line in 3D defined by a start and end point
 * 
 * @author John Folkesson
 */
class Line3D 
{
 public:
  Point3D StartPoint;
  Point3D EndPoint;

 public:
  Line3D(){}
  Line3D(Line3D& l){*this=l;}
  Line3D(Point3D& start, Point3D& end);
  virtual ~Line3D(){}

  virtual void operator = (const Line3D& l);
  virtual void operator = (const Line2D& l);
  double getLength()const{return StartPoint.getDistance(EndPoint);}

  void transform(Transformation3D& trans, const Line3D& p);
  void invTransform(Transformation3D&  trans, const Line3D& p);
  int getPixels(double startpixel[2],double endpixel[2],
		double focalLength)const;
  int getPixels(double startpixel[2],double endpixel[2],
		double focalLength,Matrix & jacobian)const;
  
      /**
   * This returns the jacobian of the endpoints in the 
   * Line3D's frame with respect to of the sensor transformation 
   * and the map frame (x,y,z).
   * It is assumed type includes xy at least. 
   *
   *
   * @param jacobian the answer is put here, a 6 x (8..12) Matrix.
   * @param dR  just do trans.getJac(dR), before calling this. 
   * @param type bits 0..5 set the transformation coordinates to use,
   * type = 63 will use all six x,y,z,theta,phi,psi,
   * type = 1  will only use x and so on, however the type is assumed 
   * to include xy (type=3) as a minimum. 
   */
  void getJacobian(Matrix& jacobian, Matrix dR[5],int type=11);
};

} // namespace Cure

#endif // CURE_LINE3D_HH
