//    $Id: Line2D.hh,v 1.3 2007/09/14 09:13:51 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_LINE2D_HH
#define CURE_LINE2D_HH

#include "Point2D.hh"

namespace Cure{

/**
 * Wrapper class for a line in 2D defined by a start and end point
 * 
 * @author John Folkesson
 */
class Line2D 
{
 public:
  Point2D StartPoint;
  Point2D EndPoint;
 public:
  Line2D(){}
  Line2D(const Line2D& l){*this=l;}
  Line2D(double start[2],double end[2]);
  Line2D(Point2D& start,Point2D& end);
  virtual ~Line2D(){}

  virtual void operator = (const Line2D& l);
  //returns 1 if no intersection;
  //returns 0 if intersection is between endPoints;
  //returns -1 if intersection is beyond endPoints;
  int getIntersection(Point2D& res, const Line2D& l)const;
  double getDistance(const Point2D& p)const;
  double getLength()const{return StartPoint.getDistance(EndPoint);} 
  /**
   * Get the line parameters in z.
   *
   * gamma is the angle to the x axis.
   * rho is the perpendicular distance from the origin.
   * thetaStart is the polar angle of the start point.
   * thetaEnd is the polar angle of the end point.
   * 
   * @param type=0 (default) only returns z=(gamma, rho), 
   * type=1  returns z = (gamma, rho, thetaStart),
   * type=2  returns z = (gamma, rho, thetaEnd),
   * type=3  returns z = (gamma, rho, thetaStart, thetaEnd).
   * @param z = (gamma,rho, thetaStart, thetaEnd)
   */
  void getPolarValues(double *z, int type=0)const;


  /**
   * Notice that trans is a full 3D Transformation so
   * trans->Transform2D  does an embedded 2D rotation.
   * (see the Transformation class).
   */
  void transform2D(Transformation3D&  trans,const Line2D& p);

  void invTransform2D(Transformation3D&  trans,const Line2D& p); 
  /**
   * This returns the jacobian of the line parameters in the 
   * Line2D's own frame.
   *
   * @param type bits 6 and 7 set the measurment type, 
   * type=0 (default) only returns gamma and rho jacobian,
   * type=64  returns j = (gamma, rho, thetaStart),
   * type=128  returns j = (gamma, rho, thetaEnd),
   * type=192  returns j = (gamma, rho, thetaStart, thetaEnd),
   * @param jacobian the 2(3,4) x 4  jacobian matrix
   */
 void getJacobian(Matrix& jacobian,int type=0)const;
  /**
   * This returns the jacobian of the endpoints in the 
   * Line2D's frame with respect to of the sensor transformation 
   * and the map frame endpoints (x,y).
   * jacobian={dXs/dXt,dXs/dXps    0   } 
   *          {dXe/dXt,    0   dXe/dXpe}  a 4x10 matrix. 
   * Xs=Tranform2D(ps)=Rot2D(Xps-Xt)  
   * Xps is the start point in the 'map' frame.
   * Xs=this->StartPoint.X is the start point in Senor frame.
   * this.X=Xr  (ie. We are the transformed line in the Sensor/Robot Frame)    
   * This is an embedded 2D transformations.
   * It is assumed type includes xytheta if z is there 
   * it will have a column of 0's.
   *
   * @param jacobian the answer is put here.
   * @param dR  2x2 matricies just do trans.get2DJac(dR), before calling this. 
   * @param type bits 0..5 set the transformation coordinates to use,
   * type = 63 will use all six x,y,z,theta,phi,psi,
   * type = 1  will only use x and so on, however the type is assumed 
   * to include xytheta (type=11) as a minimum. 
   */
  void get2DJacobian(Matrix& jacobian, Matrix dR[5],
		     int type=0)const;
};

} // namespace Cure

#endif // CURE_LINE2D_HH
