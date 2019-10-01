//    $Id: Vector3D.hh,v 1.9 2007/09/14 09:14:02 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_VECTOR3D_HH
#define CURE_VECTOR3D_HH

#include "Matrix.hh"

namespace Cure {

  
/**
 * Base class for classes that have 2 or 3 coordinates.
 *
 * @author John Folkesson
 */
class Vector3D
{
public:
  /**The (x y z) values*/
  double X[3];

public:
  /**Constructor */
  Vector3D(double x, double y, double z){
    X[0]=x;
    X[1]=y;
    X[2]=z;
  }

  /**Constructor set X=0*/
  Vector3D(){
    X[0]=0;
    X[1]=0;
    X[2]=0;
  }

  /**
   *Constructor set X[i]=t.X[i].
   * @param t the vector to copy.
   */
  Vector3D(const Vector3D& t){*this=t;}
  
  /**
   * Constructor set X[i]=x[i].
   * @param x the xyz to copy.
   */
  Vector3D(const double x[3]){*this=x;}
  virtual ~Vector3D(){}
  /**
   * Sets X[i]=b.X[i].
   * @param b the vector to copy.
   */  
  void operator = (const Vector3D& b){
    X[0]=b.X[0];
    X[1]=b.X[1];
    X[2]=b.X[2];
  }
  /**
   * Sets X[i]=px[i].
   * @param px the values to copy.
   */  
  void operator = (const double px[3]){
    X[0]=px[0];
    X[1]=px[1];
    X[2]=px[2];
  }
  /**
   * Sets X[0]=x.
   * @param x the value to copy.
   */  
  void setX(double x) { X[0] = x; }
  /**
   * Sets X[1]=y.
   * @param y the value to copy.
   */  
  void setY(double y) { X[1] = y; }
   /**
   * Sets X[2]=z.
   * @param z the value to copy.
   */  
  void setZ(double z) { X[2] = z; }

  /**
   * Gets X[0].
   * @return the x the value.
   */  
  double getX() const { return X[0]; }
  /**
   * Gets X[1].
   * @return the y the value.
   */  
  double getY() const { return X[1]; }
    /**
   * Gets X[2].
   * @return the z the value.
   */  
  double getZ() const { return X[2]; }

  /**
   * Sets X[i]=ax[i] i=0,1.
   * @param ax the values to copy.
   */  
  void setXY(const double ax[2]){
    X[0]=ax[0];
    X[1]=ax[1];
  }

  /**
   * Sets X[0]=x and X[1]=y
   */  
  void setXY(const double x, const double y){
    X[0]=x;
    X[1]=y;
  }

  /**
   * Sets X[i]=ax[i], i=0,1,2.
   * @param ax the values to copy.
   */  
  void setXYZ(const double ax[3]){*this=ax;}


  /**
   * Sets X[0]=x, X{1]=y, X[2]=z
   */  
  void setXYZ(const double x, const double y, const double z)
  {
    X[0] = x;    
    X[1] = y;    
    X[2] = z;
  }
              

  /**
   * Gets X[i], i=0,1.
   * @param ax the values are copied into this.
   */  
  void getXY(double ax[2])const{
    ax[0]=X[0];
    ax[1]=X[1];
  }

  /**
   * Gets X[i], i=0,1,2.
   * @param ax the values are copied into this.
   */  
  void getXYZ(double ax[3])const{
    ax[0]=X[0];
    ax[1]=X[1];
    ax[2]=X[2];
  }

  /**
   * Gets a ref to the array X+i
   * @param i the index into xyz.
   */  
  double& operator() (const int i){return X[i];}
  /**
   * Gets the value in the array X+i
   * @param i the index into xyz.
   */
  double operator() (const int i)const{
    return ((*(Vector3D *)this).operator()(i));
  }
  /**
   * This adds the components of b to this and stores the result in this.
   * @param b the vector to add.
   */
  void operator += (const Vector3D& b){
    X[0]+=b.X[0];
    X[1]+=b.X[1];
    X[2]+=b.X[2];
  }
  /**
   * This subtracts the components of b from this and stores the
   * result in this.
   * @param b the vector to subtract.
   */
  void operator -= (const Vector3D& b){
    X[0]-=b.X[0];
    X[1]-=b.X[1];
    X[2]-=b.X[2];
  }
  /**
   * This adds the components of b to this and returns the result.
   * @param b the vector to add.
   * @return (this+b)
   */
  Vector3D operator + (const Vector3D m)const{
    Vector3D res;
    res.X[0]=m.X[0]+X[0];
    res.X[1]=m.X[1]+X[1];
    res.X[2]=m.X[2]+X[2];
    return (res);
  }
  /**
   * This subtracts the components of b from this and returns the result.
   * @param b the vector to subtract.
   * @return (this-b)
   */
  Vector3D operator - (const Vector3D m)const{
    Vector3D res;
    res.X[0]=X[0]-m.X[0];
    res.X[1]=X[1]-m.X[1];
    res.X[2]=X[2]-m.X[2];
    return (res);
  }
  /**
   * This multiplies the components of this by m and returns the result.
   * Treats the Vector3D as a row vector.
   * @param m the Matrix to multiply (3 x 3).
   * @return (m*this)
   */
  Vector3D leftMultBy(const Matrix& m)const;
  /**
   * This multiplies the components of this by m and returns the result.
   * Treats the Vector3D as a column vector.
   * @param m the Matrix to multiply (3 x 3).
   * @return (this*m)
   */
  Vector3D operator * (const Matrix& m)const;
  /**
   * This multiplies the components of this by d.
   * @param d the number to multiply by.
   */ 
  void operator *= (const double d){
    X[0]*=d;
    X[1]*=d;
    X[2]*=d;
  }
  /**
   * This divides the components of this by d.
   * @param d the number to divide by.
   */ 
  void operator /= (const double d){
    X[0]/=d;
    X[1]/=d;
    X[2]/=d;
  }
  /**
   * Takes the dot product with another vector.
   * @param b the vector to dot with.
   * @return (this 'dot' b)
   */
  double operator * (const Vector3D& b)const{
    return (X[0]*b.X[0]+X[1]*b.X[1]+X[2]*b.X[2]);
  }
  /**
   * Equal test.
   * @param b the vector to compare to.
   * @return 0 if different else 1.
   */
  int operator == (const Vector3D& b)const{
    if (X[0]!=b.X[0])return 0;
    if (X[1]!=b.X[1])return 0;
    if (X[2]!=b.X[2])return 0;
    return 1;
  }
  /**
   * Takes the cross product with another vector.
   * @param b the vector to cross with.
   * @return (this 'x' b)
   */
  Vector3D cross(const Vector3D & a)
  {
    Vector3D res;
    res.X[0]=X[1]*a(2)-X[2]*a(1);
    res.X[1]=X[2]*a(0)-X[0]*a(2);
    res.X[2]=X[0]*a(1)-X[1]*a(0);    
    return res;
  }
  /**
   * Takes the sqroot of dot product with itself
    * @return sqrt(this * this)
   */
  double magnitude()
  {
    return sqrt((*this)*(*this));
  }
  /**Print info to display*/
  virtual void print();
};

} // namespace Cure

std::ostream& operator<<(std::ostream &os, const Cure::Vector3D &v);

#endif // CURE_VECTOR3D_HH
