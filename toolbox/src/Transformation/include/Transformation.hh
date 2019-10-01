//    $Id: Transformation.hh,v 1.6 2007/09/14 09:14:06 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_TRANSFORMATION_HH
#define CURE_TRANSFORMATION_HH

#include "Rotation3D.hh"
#include "Vector3D.hh"

namespace Cure{

/**
 * This is the base Class for transforamtions 
 * T(x)=R(x-Position).
 * Really just has translation functionality.
 * To get a real Transformation one needs to inherit the Rotation part.
 *
 * @author John Folkesson
 */  
class Transformation
{
public:
  /**
   * This give the translation part of the transformation. 
   */
  Vector3D Position;  
public:
  /**
   * Constructor for a Transformation.
   */
  Transformation(){}
  /**
   * Copy constructor for a Transformation.
   * @param t the transformation to be copied into the new object.
   */
  Transformation(const Transformation& t){Transformation::operator=(t);} 
  /**
   * Copy constructor for a Transformation.
   */
  virtual ~Transformation(){}

  //*******************SET FUNCTIONS***********************************
  /**
   * Copy operator.
   * @param t the transformation to be copied into this object.
   */
  virtual void operator = (const Transformation& t){  
    Position=t.Position;
  }
  /**
   * Sets the x component of the 3D translation vector.
   * @param x the value for the x component;
   */
  void setX(const double x){ Position.setX(x); } 
  /**
   * Sets the y component of the 3D translation vector.
   * @param y the value for the y component;
   */
  void setY(const double y){ Position.setY(y); } 
  /**
   * Sets the z component of the 3D translation vector.
   * @param z the value for the z component;
   */
  void setZ(const double z){ Position.setZ(z); } 
  /**
   * Sets the x and y components of the 3D translation vector.
   * @param x the values for the x and y components;
   */
  void setXY(const double x[2]){Position.setXY(x);  } 
  /**
   * Sets the x,y and z components of the 3D translation vector.
   * @param x the values for the x, y and z components;
   */
  void setXYZ(const double x[3]){Position=x;  } 

  //*******************GET FUNCTIONS***********************************
  /**
   * Gets the x and y components of the 3D translation vector.
   * @param x the values for the x and y components are returned here;
   */
  void getXY(double x[2])const{Position.getXY(x);}
  /**
   * Gets the x, y and z components of the 3D translation vector.
   * @param x the values for the x, y and z components are returned here;
   */
  void getXYZ(double x[3])const{Position.getXYZ(x);}

  /**
   * Gets the x  component of the 3D translation vector.
   * @param x the values for the x component is returned here;
   */
  double getX() const { return Position.getX(); } 
  /**
   * Gets the y  component of the 3D translation vector.
   * @param y the values for the y component is returned here;
   */
  double getY() const { return Position.getY(); } 
  /**
   * Gets the z  component of the 3D translation vector.
   * @param z the values for the z component is returned here;
   */
  double getZ() const { return Position.getZ(); } 
};
  
} // namespace Cure

#endif // CURE_TRANSFORMATION_HH
