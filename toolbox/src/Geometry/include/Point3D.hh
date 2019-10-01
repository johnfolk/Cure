//    $Id: Point3D.hh,v 1.6 2007/09/14 09:13:52 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_POINT3D_HH
#define CURE_POINT3D_HH

#include "Transformation3D.hh"
#include "Point2D.hh"

namespace Cure{

/**
 * A 3D point class
 * @author John Folkesson
 */
class Point3D: public Point2D
{
 public:
  Point3D(double x, double y, double z):Point2D(x,y) { setZ(z); }
  Point3D():Point2D(){}
  Point3D(const Point3D& p):Point2D(p){} 
  virtual ~Point3D(){}

  virtual void operator = (const Point3D& p){Vector3D::operator=(p);} 
  virtual void operator = (const Point2D& p){Vector3D::operator=(p);} 
  virtual void operator = (const Vector3D& p){Vector3D::operator=(p);} 
  virtual void operator = (const double x[3]){Vector3D::operator=(x);} 
  virtual double getDistance(const Point3D& p)const;

  void transform(Transformation3D& trans,const Point3D& p);
  void invTransform(Transformation3D&  trans,const Point3D& p);
  void rotate(Rotation3D&  r,const Point3D& p);
  void invRotate(Rotation3D&  r,const Point3D& p);

  
  /**
   * This gives the pixels assuming the camera is pointing in the y (X[1])
   * direction.
   *
   * One might think this an odd choice but the camera may point at the 
   * ceiling causing the transformation to be singular if one chose z or x.
   *
   * The first argument, pixels, is just the image x and 'z' 
   * pixels[0]=-focalLength*X[0]/(X[1]-focalLength);
   * pixels[1]=-focalLength*X[2]/(X[1]-focalLength);
   *
   * @param pixels holds the returned pixels image x y coordinates
   * @return 
   * 0 if ok, 
   * 1 if  X[1]<=focalLength+1E-15; 
   */
  int getPixels(double pixels[2], double focalLength)const ;
  
  /*
   * The jacobian of the above pixels;
   *
   * jacobian={dpixels/dX}  a 2x3 matrix.
   *
   * The full Jac = PixelJacobian*XJacobian
   * @return 
   * 0 if ok, 
   * 1 if  X[1]<=focalLength+1E-15; 
   */
  int getPixelJacobian( Matrix& jacobian,
			 double focalLength)const;
    /**
   * This returns the jacobian of the point in the 
   * Point3D's frame with respect to of the sensor transformation 
   * and the map frame (x,y,z).
   * jacobian={dXp/dXt,dXp/dXm } 
   * Xs=Tranform(ps)=Rot(Xm-Xt)  
   * Xm is the point in the 'map' frame.
   * Xp=this->X is the point in Senor frame.
   * this.X=Xp  (ie. We are the transformed line in the Sensor/Robot Frame)    
   * It is assumed type includes xy at least. 
   *
   *
   * @param jacobian the answer is put here, a 3 x (5..9) Matrix.
   * @param dR  just do trans.getJac(dR), before calling this. 
   * @param type bits 0..5 set the transformation coordinates to use,
   * type = 63 will use all six x,y,z,theta,phi,psi,
   * type = 1  will only use x and so on, however the type is assumed 
   * to include xy (type=3) as a minimum. 
   */
  void getJacobian(Matrix& jacobian, Matrix dR[5],int type=11)const;

};

} // namespace Cure

#endif // CURE_POINT3D_HH
