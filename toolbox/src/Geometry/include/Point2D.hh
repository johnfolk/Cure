//    $Id: Point2D.hh,v 1.5 2007/09/14 09:13:52 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_POINT2D_HH
#define CURE_POINT2D_HH

#include "Pose3D.hh"
#include "Vector3D.hh"

namespace Cure{

// Forward declaration
class Point3D;
  
/**
 * A 2D point class that uses the first 2 positions of the Vector3D class data
 * 
 * @author John Folkesson
 */
class Point2D:public Vector3D
{
public:
  Point2D(double x, double y):Vector3D(x,y,0){}
  Point2D():Vector3D(){}
  Point2D(const Point2D& p):Vector3D(p){} 
  virtual ~Point2D(){}

  virtual void operator = (const Point2D& p){Vector3D::operator=(p);}
  virtual void operator = (const Vector3D& p){Vector3D::operator=(p);}
  virtual double getDistance(const Point2D& p)const;
  /*
    Use this to convert to polar coordinates;
    return:
  */
  void getPolarValues(double thetaRho[2])const;  
  
  /*
    Here the idea is that this is a point in the sensor frame
    and the measurement of this point was a theta/rho measurement (Range)
    Then the jacobian of this measurment relative to the transformation 
    and map coordinates is returned in a 2x5 matrix:
    
    jacobian={ dTheta/dXt,  dTheta/dXmap }  
    {   drho/dXt,  drho/dXmap   }  
    
    Notice that if trans is a full 3D Transformation then
    trans->Transform2D  does an embedded 2D rotation.
    (see the Transformation class).
  */
  void transform2D(Transformation3D&  trans,const Point2D& p);
  void invTransform2D(Transformation3D&  trans,const Point2D& p);
  void rotate2D(Rotation2D&  trans,const Point2D& p);
  void invRotate2D(Rotation2D&  trans,const Point2D& p);
  
    /**
   * This returns the jacobian of the point in the 
   * Point2D's frame with respect to of the sensor transformation 
   * and the map frame (x,y).
   * jacobian={dXp/dXt,dXp/dXm } 
   * Xs=Tranform2D(ps)=Rot2D(Xm-Xt)  
   * Xm is the point in the 'map' frame.
   * Xp=this->X is the point in Senor frame.
   * this.X=Xp  (ie. We are the transformed line in the Sensor/Robot Frame)    
   * This is an embedded 2D transformations.
   * It is assumed type includes xytheta if z is there 
   * it will have a column of 0's.
   *
   * @param jacobian the answer is put here.
   * @param dR  just do trans.get2DJac(dR), before calling this. 
   * @param type bits 0..5 set the transformation coordinates to use,
   * type = 63 will use all six x,y,z,theta,phi,psi,
   * type = 1  will only use x and so on, however the type is assumed 
   * to include xytheta (type=11) as a minimum. 
   */
  void get2DJacobian(Matrix& jacobian, Matrix dR[5],int type=11)const;
  /**
   * This returns the jacobian of the point parameters in the 
   * Point2D's own frame.
   *
   * @param jacobian the 2x2  jacobian matrix
   */
  void getPolarJacobian(Matrix& jacobian)const;
       
protected:
  
};

} // namespace Cure

#endif // CURE_POINT2D_HH
