//    $Id: Rotation3D.hh,v 1.10 2007/09/14 09:14:05 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_ROTATION_HH
#define CURE_ROTATION_HH

#include "include/Rotation2D.hh"
namespace Cure{  
  
/**
 * This is the heavy calulation class for rotations. 
 * The EulerAngles are defined as;
 * Theta = EulerAngles[0] about the z-axis followed by 
 * Phi   = EulerAngles[1] about the y axis followed by
 * Psi   = EulerAngles[2] about the x axis.
 *  
 * It is important to call the set... (or =) functions to change the 
 * values as these will update the calculation State.
 *
 * The Rotation3D of a point p= (x,y,z) is:
 * p'=Rotation3D(v) = Rot*(p).
 * The inverse is:
 * p =  Rot^(-1) * p = (p' * Rot)' in Matlab notation.
 * 
 *  
 * The Rotation Matrix is defined:
 *
 *  Rot = 
 *
 *              (c0*c1)              (s0*c1)            (-s1)
 *
 *       (-s0*c2 + c0*s1*s2)   (c0*c2 + s0*s1*s2)      (c1*s2)       
 *
 *        (s0*s2 + c0*s1*c2)   (-c0*s2 + s0*s1*c2)     (c1*c2)
 *
 * Where s0 stands for sine(EulerAngles[0]) ect.
 *
 * The  Rotate2D are special for infinite lines in the z direction.
 * They are '3D aware' in the following sense: 
 * They give the intersection with the transformed xy plane.
 * This is what one needs to look at walls in the laser frame.
 * For this Rotate2D takes you from the x,y in the map 
 * to x, y in the laser frame for such a (x, y, -) point.  
 * The InvRotate2D takes you back.
 * These are to transform xy points z unlimited
 *  
 * The Rot2D matrix is not a SO(2) matrix due to the pitch and roll.
 * That is why you need to have a seperate
 * Rot2Dinv != transpose(Rot2D);
 * 
 * It should be clear that the EulerAngles of composite rotations,
 * a + b, ( Ra*Rb), are rather complicated functions of the angles 
 * for a and b.  This is why jacobians are needed when working with
 * them.
 *  
 * The jacobians of the EulerAngles get singular at pitch angles of Pi/2.
 * The angles themselves get weird but are defined such that Psi=0.
 * That is why one should try to avoid defining transformation that 
 * require a rotation of 90 degrees about the y axis.
 *  
 * It is advisable to use the y-axis as the axis that ends up
 * verticle.  Thus a pan/tilt unit should rotate around the z and x
 * axis, (theta, psi).  The laser or camera frame then should also
 * have the y-axis as the camera 'focal axis' (One often calls this z
 * for the camera).  By chosing y instead one saves much agony when
 * the tilt becomes Pi/2.
 *  
 * @author John Folkesson
 */
class Rotation3D:public Rotation2D
{
protected:
  /** The 3D rotation matrix.*/
  double Rot[9];  

public:
  /**
   * Constructor for Rotation3D.
   */
  Rotation3D();
  /**
   * Copy constructor for Rotation3D.
   */
  Rotation3D(const Rotation3D& t):Rotation2D(){*this=t;} 
  virtual ~Rotation3D();

  //*******************SET FUNCTIONS***********************************
   /**
    * Copy a Rotation into another.
    * @param r the Rotation3D to copy.
    */
  virtual void operator = (const Rotation3D& r);
   /**
    * Copy a Rotation into another.
    * @param r the Rotation2D to copy.
    */
  virtual void operator = (const Rotation2D& r){
      Rotation3D::operator=(r);
  }

  /**
   * Sets the angles and calculates Trig.
   * 
   *@param inertialAngles the value for the rotation angles theta, phi and psi.
   */
  void setAngles(const double inertialAngles[3]);
  /**
   * This sets the angles by extracting them from a rotation matrix 
   * @param nR the rotation matrix that is copied to this object.
   */
  void setR(const double nR[9]);
  
  //*******************GET FUNCTIONS***********************************
  /**
   * Get the values of the Euler Angles.
   * @param inertialAngles the three Euler angles are return here
   */
 void getAngles(double inertialAngles[3])const;
  /**
   * Return the rotation matrix.  This will cause a calculation if it hasn't 
   * already been calculated.
   * @param rotation the rotation matrix is returned here (r,c)=3*r+c.
   */ 
  void getR(double rotation[9]){
    calcRot();
    for (int i=0; i<9;i++)rotation[i]=Rot[i];
  }

  /**
   * Return the inverse rotation matrix.  This will cause a
   * calculation if it hasn't already been calculated.
   *
   * @param invrotation the invers rotation matrix is returned here
   * (r,c)=3*r+c.
   */ 
  void getRinv(double invrotation[9]){
    calcRot();
    invrotation[0]=Rot[0];
    invrotation[3]=Rot[1];
    invrotation[6]=Rot[2];
    invrotation[1]=Rot[3];
    invrotation[4]=Rot[4];
    invrotation[7]=Rot[5];
    invrotation[2]=Rot[6];
    invrotation[5]=Rot[7];
    invrotation[8]=Rot[8];
  }
  
  /**
   * This special method is used in some other methods that need to calculate 
   * transformation jacobians.  It takes the derivative of Rot2D wrt theta
   * and then multiplies it by Rot2D's inverse from the right.
   *
   * J=Jacobian for 2D rotations (Theta part).
   *  
   *  J=(dRot2D/dTheta)
   *  
   *  dR= J * (Rot2Dinv).
   *  
   *  dR=(dRot2D/dTheta) * (Rot2Dinv).
   *  
   *  This is needed to calculate for instance wall measurement jacobians
   * @param dR the product above is returned here. 
  */
  int getJR2DxR2Dinv(double dR[4]);
  
  /**
   *  This is as above but returns the full jacobian, a vector of matricies.
   * So the difference is the derivatives wrt phi and psi are also returned.
   *  
   *  dR= J * (Rot2Dinv). (Theta,Phi and Psi variations)
   *  
   *  dR[0] = (dRot2D/dTheta) * (Rot2Dinv)
   *  dR[1] = (dRot2D/dPhi)   * (Rot2Dinv)
   *  dR[2] = (dRot2D/dPsi)   * (Rot2Dinv)
   *  
   *  These are 2x2 Matricies.
   * @param dR an array of matricies giving the quantities explained.
  */ 
  int getJR2DxR2Dinv(Matrix dR[3]);
  

  /**
   *  This is as above but add the rot2D matrix and its negative.
   *  
   *  dR= J * (Rot2Dinv). (Theta,Phi and Psi variations)
   *  
   *  dR[0] = (dRot2D/dTheta) * (Rot2Dinv)
   *  dR[1] = (dRot2D/dPhi)   * (Rot2Dinv)
   *  dR[2] = (dRot2D/dPsi)   * (Rot2Dinv)
   *  dR[3] = (Rot2D)
   *  dR[4] = (-Rot2D)
   *  
   *  These are 2x2 Matricies.
   * @param dR an array of matricies giving the quantities explained.
  */ 
  void get2DJac(Matrix dR[5]);
  
  /**
   *  The Full Jacobian and full variation.
   *  
   *  dR = J * (Rot)^-1 = J * (Rotinv) = J * (Transpose Rot).
   *  
   *  dR[0] = (dRot/dTheta) * (Rot^T)
   *  dR[1] = (dRot/dPhi)   * (Rot^T)
   *  dR[2] = (dRot/dPsi)   * (Rot^T)
   *  
   *  These are 3x3 Matricies.
   *  
   *  Also notice that:
   *  
   *  (Jacobian of R)*Rinv = -R*(Jacobain of Rinv).
   *  
   * @param dR an array of matricies giving the quantities explained.
  */ 
  void getJRxRinv(Matrix dR[3]);

  /**
   *  The Full Jacobian and full variation.
   *  
   *  dR = J * (Rot)^-1 = J * (Rotinv) = J * (Transpose Rot).
   *  
   *  dR[0] = (dRot/dTheta) * (Rot^T)
   *  dR[1] = (dRot/dPhi)   * (Rot^T)
   *  dR[2] = (dRot/dPsi)   * (Rot^T)
   *  dR[3] = (Rot)
   *  dR[4] = (-Rot)
   *  
   *  These are 3x3 Matricies.
   *  
   *  Also notice that:
   *  
   *  (Jacobian of R)*Rinv = -R*(Jacobain of Rinv).
   *  
   * @param dR an array of matricies giving the quantities explained.
  */ 
  void getJac(Matrix dR[5]);
  
  /**
   *  The Full Jacobian:
   *  
   *  jacobian for 3D rotatations
   *
   *  
   *  jacobian[0] = (dRot/dTheta);
   *  jacobian[1] = (dRot/dPhi);
   *  jacobian[2] = (dRot/dPsi);
   *  
   *  These are 3x3 Matricies.
   * @param jacobian an array of matricies giving the quantities explained.
  */
  void getJacobianR(Matrix jacobian[3]);
  
  /**
   *  The Full Hessian:
   *  
   *  hessian[0] = (d^2Rot/dTheta dTheta);
   *  hessian[1] = (dRot^2/dTheta dPhi);
   *  hessian[2] = (dRot^2 /dTheta dPsi);
   *   and so on.
   *  These are 3x3 Matricies.
   * @param hessian an array of matricies giving the quantities explained.
  */
  void getHessianR(Matrix hessian[9]);  
  /**
   *  This calculates the time derviatives of the Euler angles given
   *  the angular velocity in the rotated frame.  This is the angular
   *  velocity measured by an IMU for instance. So an IMU gyro
   *  velocity can be incomega as a column matrix 3x1 and the implied
   *  time derviatives of the Euler angles will be returned in
   *  absomega.  The linear relationship is returned as well in a. 
   *  If jac is not 0 the jacobian of this transformation is returned there.
   *
   * @param incomega the input angular velocity of an incremental rotaion.
   * @param absomega returns the angular velocity in the euler angles.
   * @param a returns the tranformation matrix for the rotation velocity 
   *          from the 'robot' frame to 'earth', absomega=a(incomega).
   * @param jac an array of 3 Matrix's to hold the returned 
   *            values of (da/dtheta, da/dphi ,da/dpsi)
   * @return 0 if ok else 1 if pitch is 90 degrees.
   */
  int transformedAngluarVel(Matrix &incomega,
				      Matrix &absomega, Matrix & a, 
			    Matrix *jac=0);

  /**
   *  These do the actual rotation of vectors/points.
   *  Notice that the Rotate2D ect. are inherited and do not need to 
   *  be redefined, although the protected CalcRot2D() does.
   * 
   * @param fromx the xyz of the point to rotate can be same as tox.
   * @param tox the xyz of the rotated point is returned here.
  */
  void rotate(const double fromx[3],double tox[3]);
  /**
   *  These do the actual inverse rotation of vectors/points.
   * 
   * @param fromx the xyz of the point to inverse rotate can be same as tox.
   * @param tox the xyz of the inverse rotated point is returned here.
  */
  void invRotate(const double fromx[3],double tox[3]);
  /**
   *  These do the actual rotation of vectors/points.
   *  Notice that the Rotate2D ect. are inherited and do not need to 
   *  be redefined, although the protected CalcRot2D() does.
   * 
   * @param fromx the xyz of the point to rotate can be same as tox.
   * @param tox the xyz of the rotated point is returned here.
  */
  void rotate(const Vector3D& fromx,Vector3D& tox);
  /**
   *  These do the actual inverse rotation of vectors/points.
   * 
   * @param fromx the xyz of the point to inverse rotate can be same as tox.
   * @param tox the xyz of the inverse rotated point is returned here.
  */
  void invRotate(const Vector3D& fromx,Vector3D& tox);
  
  /**
   *  These operators on transforms concern this equation:
   *  C(x)=B(A(x))
   *  Where a, b and c are Rotation3Ds.
   *  This is also writen 
   *  c=a+b
   *  or
   *  a=c-b
   *  or
   *  b=-a+c 
  */  
  
  /** 
   * This does the composition of two rotations and returns the result. 
   *  
   *  C(x)=B(A(x))
   *
   *  Where a, b and c are Rotation3Ds.
   *  This is also writen 
   *  c=a+b
   *  
   *   a=this rotation, returns c
   *
   * @param b the second rotation.
   */
  virtual Rotation3D operator + ( Rotation3D &b);
  
  /** 
   * This does the composition of two rotations and returns the result. 
   *  
   *  C(x)=B(A(x))  -> A(x)=invB(C(x))
   *
   *  Where a, b and c are Rotation3Ds.
   *  
   *  This is also writen 
   *  a=c-b
   *  
   *  c=this rotation, returns a
   *
   * @param b the second rotation.
   */
  virtual Rotation3D operator - (Rotation3D& b);
  
  /** 
   * This does the composition of two rotations and returns the result
   * in this rotation. 
   *  
   *  A(x)=B(A(x))
   *
   *  Where a and b are Rotation3Ds.
   *  This is also writen 
   *  a=a+b
   *  
   *  a=this rotation,
   * 
   * @param b the second rotation.
   */
  virtual void operator += (Rotation3D& b);
  
  /** 
   * This does the composition of two rotations and returns the result. 
   *  
   *  A(x)=invB(A(x))
   *
   *  Where a, and b are Rotation3Ds.
   *  
   *  This is also writen 
   *  a=a-b
   *  
   *  a=this rotation, 
   *
   * @param b the second rotation.
   */
  virtual void operator -= (Rotation3D& b);
  
  /** 
   * This does the composition of two rotations and returns the result
   * in this rotation. 
   *  
   *  A(x)=A(B(x))
   *
   *  Where a and b are Rotation3Ds.
   *  This is also writen 
   *  a=b+a
   *  
   *  a=this rotation,
   * 
   * @param b the second rotation.
   */
  virtual void leftAdd(Rotation3D& b);
  /** 
   * This does the composition of two rotations and returns the result
   * in this rotation. 
   *  
   *  A(x)=B(A(x))
   *
   *  Where a and b are Rotation3Ds.
   *  This is also written 
   *  a=a+b
   *  
   *  a=this rotation,
   * 
   * @param b the second rotation.
   */
  virtual void rightAdd(Rotation3D& b){*this+=(b);}
  /** 
   * This does the composition of two rotations and returns the result. 
   *  
   *  A(x)=A(invB(x))
   *
   *  Where a, and b are Rotation3Ds.
   *  
   *  This is also written 
   *  a=-b+a
   *  
   *  a=this rotation, 
   *
   * @param b the second rotation.
   */
  virtual void leftSubtract(Rotation3D& b);
   /** 
   * This does the composition of two rotations and returns the result. 
   *  
   *  A(x)=invB(A(x))
   *
   *  Where a, and b are Rotation3Ds.
   *  
   *  This is also writen 
   *  a=a-b
   *  
   *  a=this rotation, 
   *
   * @param b the second rotation.
   */
  virtual void rightSubtract(Rotation3D& b){*this-=(b);}

  /**
   *  This does an add and calculates the jacobian.
   *  
   *  this=a+b; 
   *  
   *  Rot=b.Rot*a.Rot;
   *  
   *  jacobian is a 3 x 6 matrix
   *  
   *  jacobian = {dEulerAngles/da.EulerAngles,dEulerAngles/db.EulerAngles}
   *  
   *  Select The columns and rows that you are interested in using by:
   *  
   *  whichRows = sum 2^i  
   *  
   *  where the sum is includes only the rows i that you want
   *  
   *  So, for example, rows 0 and 1 only (theta and phi) give whichRows=3
   *  
   *  The same for whichColumns only then there are 6 columns to chose 
   *  from  ie. (a.theta...b.psi)
   *  
   *  The default is calcualte everything.
   *  
   *  Note that jacobian ends up bigger than you need so you may need
   *  to delete the extra rows and columns yourself.
   *  This way it is clear what the elements of jacobian mean. 
   *  Set deleteExtra to 1 if you want them deleted automatically. 
   *
   * @param a the first rotation.
   * @param b the other rotation.
   * 
   * @param jacobian the dervatives of the resulting Euler angles wrt
   * the Euler angles of a and b,
   *
   * @param whichRows Flags that let you specify to not bother
   * calculating some of the rows of the jacobian (default all rows
   * calculated).
   * 
   * @param whichCols Flags to skip some of the columns, (default calc all)
   *
   * @param deleteExtra if set this will delete the rows and columns
   * not calculated from the jacobian matrix. (default is to leave
   * them alone).
  */
  virtual void doAplusB(Rotation3D& a, Rotation3D& b, 
				    Matrix& jacobian,
				    int whichRows=7,int whichCols=63,
				    int deleteExtra=0);
  /**
   * this=a-b.
   * @see Rotation3D::doAplusB
   * @param a the first rotation.
   * @param b the other rotation.
   * 
   * @param jacobian the dervatives of the resulting Euler angles wrt
   * the Euler angles of a and b,
   *
   * @param whichRows Flags that let you specify to not bother
   * calculating some of the rows of the jacobian (default all rows
   * calculated).
   * 
   * @param whichCols Flags to skip some of the columns, (default calc all)
   *
   * @param deleteExtra if set this will delete the rows and columns
   * not calculated from the jacobian matrix. (default is to leave
   * them alone).
  */
  virtual void doAminusB(Rotation3D& a, Rotation3D& b, 
				     Matrix& jacobian,
				     int whichRows=7, int whichCols=63, 
				     int deleteExtra=0);
  /**
   * this=-a+b.
   * @see Rotation3D::doAplusB
   * @param a the first rotation.
   * @param b the other rotation.
   * 
   * @param jacobian the dervatives of the resulting Euler angles wrt
   * the Euler angles of a and b,
   *
   * @param whichRows Flags that let you specify to not bother
   * calculating some of the rows of the jacobian (default all rows
   * calculated).
   * 
   * @param whichCols Flags to skip some of the columns, (default calc all)
   *
   * @param deleteExtra if set this will delete the rows and columns
   * not calculated from the jacobian matrix. (default is to leave
   * them alone).
  */
  virtual void dominusAplusB(Rotation3D& a, Rotation3D& b, 
					 Matrix& jacobian, int whichRows=7, 
					 int whichCols=63, int deleteExtra=0);
  /**
   * this=-a-b.
   * @see Rotation3D::doAplusB
   * @param a the first rotation.
   * @param b the other rotation.
   * 
   * @param jacobian the dervatives of the resulting Euler angles wrt
   * the Euler angles of a and b,
   *
   * @param whichRows Flags that let you specify to not bother
   * calculating some of the rows of the jacobian (default all rows
   * calculated).
   * 
   * @param whichCols Flags to skip some of the columns, (default calc all)
   *
   * @param deleteExtra if set this will delete the rows and columns
   * not calculated from the jacobian matrix. (default is to leave
   * them alone).
  */
  virtual void dominusAminusB(Rotation3D& a, Rotation3D& b, 
					  Matrix& jacobian, int whichRows=7, 
					  int whichCols=63, int deleteExtra=0);
  
 
   /*
   * Prints the object to the display screen.
   * @param verbose if >0 more info is printed
   */
 virtual void print(int verbose=0);
  
  /**
   * This restricts
   * theta to [-pi, pi],
   * phi   to [-pi/2, pi/2],
   * psi   to [-pi, pi].
  */
  void limitAngles();
  
  /**
   * Used to get unique Euler angles from any rotation matrix.
   * We make a choice for the phi=Pi/2 degenerate case.
   * In: nR[9] the 3D rotation matrix.
   * out: x[3] the theta phi and psi.
   * @param nr the rotation matrix.
   * @param x the resulting euler angles cooresponding to nr. 
  */
  static void extractAngles(double nr[9],double x[3]);
  
protected:
  double Beta[6];  //needed to calculate jacobians.

  void calcDR2D();
  void calcRot2D();
  void calcRot();
};

} // namespace Cure

std::ostream& operator<<(std::ostream& os, const Cure::Rotation3D& r);

#endif // CURE_ROTATION3D_HH
