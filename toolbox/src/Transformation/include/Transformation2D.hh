//    $Id: Transformation2D.hh,v 1.6 2007/09/14 09:14:06 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_TRANSFORMATION2D_HH
#define CURE_TRANSFORMATION2D_HH

#include "Transformation.hh"
#include "Rotation2D.hh"

namespace Cure{

/**
 * These give transformations in the xy plane
 * (xt,yt)^T=R(xp-x,yp-y)^T, where xt yt are the transformed xp yp.  R
 * is the rotaion matrix and x y are the translation part.
 * 
 * Always use Set... functions since information is calculated as needed.    
 *
 * @author John Folkesson
 */
class Transformation2D: public Transformation, public Rotation2D
{
public:

public:
  /** 
   * The Constructor.
   */
  Transformation2D():Rotation2D(){}
  /**
   * The constructor with initail values.
   * @param x the initial displacement in the x direction.
   * @param y the initial displacement in the y direction.
   * @param a the initial rotation angle in radians.
   */
  Transformation2D(double x, double y, double a)
    :Rotation2D(a) 
  { setX(x); setY(y); }
  /**
   * The copy constructor.
   * @param t the transformation to be copied into this new object. 
   */
  Transformation2D(const Transformation2D& t):Transformation(),Rotation2D()
  { this->Transformation2D::operator=(t);} 
  virtual ~Transformation2D(){}
  
  //*******************SET FUNCTIONS***********************************
  /*
   * Copy a transformation2D to this object.
   * @param t the transformation to be copied into this object. 
  */
  virtual void operator = (const Transformation2D& t){
    Rotation2D::operator=(t);
    Position=t.Position;
  }

  /*
   * Copy a Rotation2D to this object.  The translation part remains unchanged.
   * @param r the rotation to be copied into this object. 
  */
  virtual void operator = (const Rotation2D& r){Rotation2D::operator=(r);}
  /**
   * This will set the 2D coordinates of this.
   * @param x The array [x,y,theta] to set the coordinates of this
   * transformation.
   */
  void setXYTheta(const double x[3]){
    Position.setXY(x);
    Rotation2D::setTheta(x[2]);
  }   

  /**
   * This will set the 2D coordinates of this.
   * @param x The array [x,y,theta] to set the coordinates of this
   * transformation.
   */
  void setXYTheta(const double x, const double y, const double theta){
    Position.setX(x);
    Position.setY(y);
    Rotation2D::setTheta(theta);
  }   

  //*******************GET FUNCTIONS***********************************
  /**
   * This will get the values of the 2D coordinates of this.
   * @param x The array [x,y,theta] to return the coordinates of this
   * transformation in.
   */
  void getXYTheta(double x[3])const{
    Position.getXY(x);
    x[2]=getTheta();
  }
  /**
   * This will get the 2D rotation matrix of this.
   * @param rotation The rotation matrix 2*r+c
   */
  void getRot2D(double rotation[4]){ Rotation2D::getRot2D(rotation);}   
  /**
   * This will get the 2D inverse rotation matrix of this.
   * @param rotation The inverse rotation matrix 2*r+c
   */
  void getRot2Dinv(double rotaInv[4]){Rotation2D::getRot2Dinv(rotaInv);}   

  /**
   * This is used to linearize the rotation matrix.  This is needed to
   * calculate, for instance, wall measurement jacobians The value of
   * the derivative of the rotation matrix is multiplied on the right
   * by the inverse of the rotation matrix.  This is simple convient for
   * some of the computations to be done with this.
   *
   * J=Jacobian for 2D rotations (Theta part)
   * 
   * J=(dRot2D/dTheta)
   * 
   * dR= J * (Rot2Dinv).
   * 
   * dR=(dRot2D/dTheta) * (Rot2Dinv).
   * @param dR the result is returned in this array
  */
  int getJR2DxR2Dinv(double dR[4]){return Rotation2D::getJR2DxR2Dinv(dR);}

  //************* TRANSFORMS ****************************************
  /**
   * Translate then rotate a 2D point/vector using the rotation and
   * translation(Position).  
   * @param fromx the xy of the point to
   * transform can be same as tox.  
   * @param tox the xy of the transformed  point is returned here.
   */
  void transform2D(const double fromx[2],double tox[2]);
  /**
   * Inverse rotate then inverse translate a 2D point/vector using the
   * rotation and translation(Position).
   *
   * @param fromx the xy of the point to
   * inverse transform can be same as tox.  
   * @param tox the xy of the inverse transformed  point is returned here.
   */
  void invTransform2D(const double fromx[2],double tox[2]);

  /**
   * This does 
   *  c = a + b, which means operating on a vector x:c(x) = b(a(x)).
   * 
   * More explictly:
   * c(x) = Rc*(x-Xc) = Rb*(Ra*(x-Xa)-Xb).
   * 
   * So for the rotations:
   *
   * Rc=Rb*Ra;
   *
   * and for the X:
   *
   * Xc = Xa +Ra^(-1)*Xb
   *
   * This matrix is a and c is the returned matrix.
   *
   * @param b the second transformation.
   * @return c =a+b
  */
  Transformation2D operator + (const Transformation2D& b)const;
  /**
   * This does 
   *  c = a - b, which means operating on a vector x:c(x) = b^(-1)(a(x)).
   * 
   * This matrix is a and c is the returned matrix.
   *
   * @param b the second transformation.
   * @return c =a-b
  */
  Transformation2D operator - (const Transformation2D& b)const;
  /**
   * The inverse of this transformation is returned.
   * This has the transform of the rotation matrix and a translation that is 
   * -Ra^(-1)*Xa, where this is a.
   *
   * c= -a
   * 
   * @return c=-a
   */
  Transformation2D inverse();

  /**
   * This does 
   *  a = a + b.
   *
   * This matrix is a which is set to a+b.
   *
   * @param b the second transformation.
  */
  void operator += (const Transformation2D& b){
    Rotation2D *r=this;
    Vector3D tmp;
    Transformation2D::invTransform2D(b.Position.X,tmp.X);
    Position=tmp;
    (r->Rotation2D::operator +=)(b);
  }
  
  /**
   * This does 
   *  a = a - b.
   *
   * This matrix is a which is set to a-b.
   *
   * @param b the second transformation.
  */
  void operator -= (const Transformation2D& b){
    Rotation2D *r=(this);
    (r->Rotation2D::operator -=)(b);
    Vector3D tmp;
    tmp-=b.Position;
    Transformation2D::invTransform2D(tmp.X,tmp.X);
    Position=tmp;
  }

  /*
   * Prints the object to the display screen.
   * @param verbose if >0 more info is printed
   */
  virtual void print();
};

} // namespace Cure

#endif // CURE_TRANSFORMATION2D_HH
