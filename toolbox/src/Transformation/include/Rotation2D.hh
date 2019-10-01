//    $Id: Rotation2D.hh,v 1.7 2007/09/14 09:14:05 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_ROTATION2D_HH
#define CURE_ROTATION2D_HH

#include "Vector3D.hh"
#include <iostream>
#ifndef DEPEND
#include <math.h>
#endif

namespace Cure{

#define TODEG 180/M_PI
#define TORAD M_PI/180
#define TWO_PI 6.28318530717958647692
  

/**
 * This is a base class for the Tranformation family.
 * It stores some of the transform information and does 2D rotations
 * For this and its subclasses one can access the coordinates (EulerAngles)
 * Directly but if one wants to change them use the Member Methods.  
 * Do not just do EulerAngles[0]=2.1;
 * 
 * The reason is that there is a calculation state that needs to be reset.
 * When one asks for information from these objects they sometimes need to 
 * do some calculation to get it.  These calculations are saved and the state
 * set accordingly.
 *
 * @author John Folkesson
 */
class Rotation2D
{
public:
  /**
   * The values of cos and sin of the EulerAngles will be stored here
   * after being calculated the first time.  They are calcualted when
   * setTheta is called.
   *
   *  Trig[2n] is cos(EulerAngles[n]).
   * Trig[2n+1] is sin(EulerAngles[n]).
   */
  double Trig[6];         //cos(Theta),sin(Theta)...sin(Psi)
protected:
  /**
   * The Euler angles, theta, phi, and psi.  The rotation is defined
   * as theta around the z axis followed by phi around the y axis and
   * finally psi arount the x axis.
   */
  double EulerAngles[3];  //Theta, Phi and Psi 
 
  
protected:
  /**
   * The State is 0 untill the Rot2D is calcualted then it is set to 1.
   */
  short State;
  /**
   * This is the rotation matrix of the 2D transformation.
   */
  double Rot2D[4];        
  /**
   * This is int inverse of the 2D rotation.  
   */
  double Rot2Dinv[4];

public:
  /**
   * Constructor for Rotation2D.
   */
  Rotation2D();
  /**
   * Constructor for Rotation2D.
   */
  Rotation2D(double theta) { setTheta(theta); }
  /**
   * Copy Constructor.
   */
  Rotation2D(const Rotation2D& t);
  virtual ~Rotation2D();
  
  //*******************SET FUNCTIONS***********************************
  /*
   * Copies the Rotation object.
   */
  virtual void operator = (const Rotation2D& trans);
  /**
   * Sets the angle and calculates Trig.
   * @param x the value for the rotation angle theta.
   */
  void setTheta(const double x);   
  /**
   * Sets the angle and calculates Trig.
   * @param x the value for the rotation angle phi.
   */
  void setPhi(const double x);   
  /**
   * Sets the angle and calculates Trig.
   * @param x the value for the rotation angle psi.
   */
  void setPsi(const double x);   
  
  //*******************GET FUNCTIONS***********************************
  /**
   * Gets the values of the Euler angles stored in this object.
   * @param angles the values of the euler angles are returned here.
   */
  void getEulerAngles(double angles[3]) const;
  /*
   * Get the value of theta the rotation angle around the z axis.
   * @return the value of theta.
   */
  double getTheta()const{return EulerAngles[0];}   
  /*
   * Get the value of theta the rotation angle around the y axis.
   * @return the value of theta.
   */
  double getPhi()const{return EulerAngles[1];}   
  /*
   * Get the value of theta the rotation angle around the x axis.
   * @return the value of theta.
   */
  double getPsi()const{return EulerAngles[2];}   
  /**
   * Get the values of the rotation matrix. 
   * @param rotation the 2D rotation matrix is returned here.
   */
  void getRot2D(double rotation[4]);
  /**
   * Get the values of the inverse rotation matrix. 
   * @param rotationInverse the inverse of the 2D 
   *  rotation matrix is returned here.
   */
  void getRot2Dinv(double rotationInverse[4]);
  
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
  virtual int getJR2DxR2Dinv(double dR[4]);
  /**
   * Rotates a 2D point/vector using the rotation.
   * @param fromx the xy of the point to rotate can be same as tox.
   * @param tox the xy of the rotated point is returned here.
   */
  void rotate2D(const double fromx[2],double tox[2]);
  /**
   * Rotates a 2D point/vector using the inverse rotation.
   * @param fromx the xy of the point to rotate can be same as tox.
   * @param tox the xy of the rotated point is returned here.
   */
  void invRotate2D(const double fromx[2],double tox[2]);
  /**
   * Rotates a 2D point/vector using the rotation.
   * @param fromx the xy of the point to rotate can be same as tox.
   * @param tox the xy of the rotated point is returned here.
   */
  void rotate2D(const Vector3D& fromx,Vector3D& tox);
  /**
   * Rotates a 2D point/vector using the inverse rotation.
   * @param fromx the xy of the point to rotate can be same as tox.
   * @param tox the xy of the rotated point is returned here.
   */
  void invRotate2D(const Vector3D& fromx,Vector3D& tox);
  
  /**
   *   Add two rotations.  
   *    c = a + b means c(x) = b(a(x)) for the rotations
   *    Rc=Rb*Ra;
   *
   * @param r the rotation to add to this from the right.
   *  @return the sum of this rotation and r
   */
  virtual Rotation2D operator + (const Rotation2D& r)const;
  /**
   *   Subtract two rotations.  
   *    c = a - b means c(x) = invb(a(x)) for the rotations
   *    Rc=invRb*Ra;
   *
   *  @param r the rotation to subtract from this from the right.
   *  @return this rotation minus r
   */
  virtual Rotation2D operator - (const Rotation2D& r)const;
  /**
   *   Add two rotations and become the result.  
   *    c = a + b means c(x) = b(a(x)) for the rotations
   *    Rc=Rb*Ra;
   *
   * @param r the rotation to add to this from the right.
   */
  virtual void operator += (const Rotation2D& r){
    setTheta(EulerAngles[0]+r.EulerAngles[0]);  
  }
  /**
   *   Subtract two rotations and become the result.  
   *    c = a - b means c(x) = invb(a(x)) for the rotations
   *    Rc=invRb*Ra;
   *
   *  @param r the rotation to subtract from this from the right.
    */
  virtual void operator -= (const Rotation2D& r){
  setTheta(EulerAngles[0]-r.EulerAngles[0]); 
  }
  
  /**    
   * This does an add and calculates the jacobian of the Euler angles 
   * with repect to the Euler angles of a and b.
   * this=a+b; 
   *
   * Rot=b.Rot*a.Rot;
   * 
   * Jacobian is a 1 x 2 matrix
   * 
   * jacobian = {dTheta/da.Theta,dEulerAngles/dTheta}
   * 
   * This is a bit silly here as it is just (1,1) but 3D is more.
   * @param a the one of the rotations.
   * @param b the one of the rotations.
   * @param jacobian = {dTheta/da.Theta,dEulerAngles/dTheta}  
  */
  virtual void doAplusB(Rotation2D& a, Rotation2D& b, Matrix& jacobian);
  /**    
   * This does a subtract and calculates the jacobian of the Euler angles 
   * with repect to the Euler angles of a and b.
   * this=a-b; 
   *
   * Rot=invb.Rot*a.Rot;
   * 
   * Jacobian is a 1 x 2 matrix
   * 
   * jacobian = {dTheta/da.Theta,dEulerAngles/dTheta}
   * 
   * This is a bit silly here as it is just (1,-1) but 3D is more.
   * @param a the one of the rotations.
   * @param b the one of the rotations.
   * @param jacobian = {dTheta/da.Theta,dEulerAngles/dTheta}  
  */
  virtual void doAminusB(Rotation2D& a, Rotation2D& b, Matrix& jacobian);
  /**    
   * This does a subtract and calculates the jacobian of the Euler angles 
   * with repect to the Euler angles of a and b.
   * this=-a+b; 
   *
   * Rot=b.Rot*inva.Rot;
   * 
   * Jacobian is a 1 x 2 matrix
   * 
   * jacobian = {dTheta/da.Theta,dEulerAngles/dTheta}
   * 
   * This is a bit silly here as it is just (-1,1) but 3D is more.
   * @param a the one of the rotations.
   * @param b the one of the rotations.
   * @param jacobian = {dTheta/da.Theta,dEulerAngles/dTheta}  
  */
  virtual void dominusAplusB(Rotation2D& a, Rotation2D& b, Matrix& jacobian);
  /**    
   * This does a subtract and calculates the jacobian of the Euler angles 
   * with repect to the Euler angles of a and b.
   * this=-a-b; 
   *
   * Rot=invb.Rot*inva.Rot;
   * 
   * Jacobian is a 1 x 2 matrix
   * This is a bit silly here as it is just (-1,-1) but 3D is more.
   *
   * @param a the one of the rotations.
   * @param b the one of the rotations.
   * @param jacobian = {dTheta/da.Theta,dEulerAngles/dTheta}  
  */
  virtual void dominusAminusB(Rotation2D& a, Rotation2D& b, Matrix& jacobian);
  
  /**
   * This limits theta to (-Pi, Pi]
  */
  virtual void limitAngles(){
    limitPlusMinusPi(EulerAngles[0]);
  }
  void limitPlusMinusPi(double a)
  {
    while (a>M_PI)a-=TWO_PI;
    while (a<-M_PI)a+=TWO_PI;
  }
  /*
   * Prints the object to the display screen.
   * @param verbose if >0 more info is printed
   */
  virtual void print(int verbose=0);

protected:
  /**
   * This sets the State to 1 and calculates the Rotation matrix.
   */
  virtual void calcRot2D();
};

} // namespace Cure

std::ostream& operator<<(std::ostream& os, const Cure::Rotation2D& r);

#endif // CURE_ROTATION2D_HH
