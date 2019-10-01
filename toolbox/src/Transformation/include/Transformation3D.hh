//    $Id: Transformation3D.hh,v 1.15 2008/10/29 16:32:25 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_TRANSFORMATION3D_HH
#define CURE_TRANSFORMATION3D_HH

#include "Transformation2D.hh"
#include "Rotation3D.hh"

namespace Cure{

/**
 * This does translation and rotation in 3D.
 * Always use Set... functions since information is calculated as needed.  
 * 
 * The Transformation of a point p=(x,y,z) is:
 *  
 * p'=Transformation(v) = R * (p - X).
 * 
 * The inverse is:
 * 
 * p = X + R^(-1) * p'.
 *
 * The  Transform2D are special for infinite lines in the z direction.
 * They give the intersection with the transformed xy plane.
 * This is what one needs to look at walls in the laser frame.
 * For this Transform2D takes you from the x,y in the map 
 * to x, y in the laser frame.  The InvTransform2D takes you back.
 * These are to transform xy points z unlimited
 * @see Rotation3D
 *
 * An 'addition' of these transforms is defined by the composition of
 * two transforms.  So if A and B are are functions that transform
 * a point in 3D x then the sum of the cooresponding Transformation3D objects
 * c = a + b is defined such that 
 *
 * C(x) =B(A(x))
 *
 *  This composition object c will have a translation and EulerAngles
 *  defined by A and B. These coordinates of c will then be a function
 *  of the coordinates of A and B.  The Jacobians of this function
 *  will have 6 rows and 12 columns.  This class will calculate that
 *  horrid Jacobian for you.
 *
 * @author John Folkesson
 */
class Transformation3D: public Transformation, public Rotation3D
{
public:
  /**
   * The constructor.
   */
  Transformation3D(){}
  /**
   * The copy constructor.
   * @param t the Transformation to be copied into this.
   */
  Transformation3D(const Transformation3D& t):Transformation(),Rotation3D()
  { Transformation3D::operator=(t);} 
  /**
   * A constructor that will initialize the values.
   * translation.
   *
   * @param t the Transformation to be copied into this.
   */
  Transformation3D(const Transformation2D& t){Transformation3D::operator=(t);} 
  virtual ~Transformation3D(){}
  
  //*******************SET FUNCTIONS***********************************
  /**
   * The copy operator.
   * @param t the Transformation to be copied into this.
   */
  virtual void operator = (const Transformation3D& t){
    Rotation3D::operator=(t);
    Transformation::operator=(t);
  }
  /**
   * The copy operator.
   * @param t the Transformation to be copied into this.
   */
  virtual void operator = (const Transformation2D& t){
    Rotation2D::operator=(t);
    Transformation::operator=(t);
  }
  /**
   * The copy operator.
   * @param t the rotation to be copied into this.
   */
  virtual void operator = (const Rotation3D& t){
    Rotation3D::operator=(t);
  }
  /**
   * This allows all six corrdinates to be set at once from a double array.
   * @param x A vector of x,y,z,theta, phi, psi
   */
  virtual void operator = (double x[6]){ 
    setAngles(x+3);
    Position=x;
  }

  /**
   * Set the 2D corrdiantes.
   * @param x A vector of x,y,theta
   */
 void setXYTheta(const double x[3]){
    Position.setXY(x);
    Rotation3D::setTheta(x[2]);
  }   

  /**
   * Set the 2D corrdiantes.
   * @param x A vector of x,y,theta
   */
 void setXYTheta(const double x, const double y, const double theta){
    Position.setX(x);
    Position.setY(y);
    Rotation3D::setTheta(theta);
  }   

  /*
   * This sets the uncertian part of the coordinates as
   * defined by covType.  It leaves the rest of the coordinates alone.
   * It does a bit more checking than the double array version.
   * @param coordinate A calcRows(CovType) by 1 matrix containing the new
   *                    uncertian part of the coordinates.
   * @param covType Bit flags that define the uncertain coordinates,
   *                0000 0000 00(psi)(phi) (theta)(z)(y)(x).
   * @return 1 if input has wrong length else 0.
   * @see Pose3D::setCovType
   */
  int setCovCoordinates(Matrix & coordinates, unsigned short covType);
  /*
   * This sets the uncertian part of the coordinates as
   * defined by CovType.  It leaves the rest of the coordinates alone.
   * @param coordinates An array of length calcRows(CovType) containing 
   * @param covType defines the uncertain coordinates bitwise,
   *                0000 0000 00(psi)(phi) (theta)(z)(y)(x).
   * the new uncertian part of the coordinates.
   * @see Pose3D::setCovType  
   */
  void setCovCoordinates(const double *coordinates, unsigned short covType);
  /*
   * This gets the uncertian part of the tranformation coordinates as
   * defined by CovType.  
   * @param coordinates An array of length calcRows(CovType) returning with 
   * @param covType Bit flags that define the uncertain coordinates,
   *                0000 0000 00(psi)(phi) (theta)(z)(y)(x).
   * the uncertian part of the coordinates.
   * @see Pose3D::setCovType.
   */
  void getCovCoordinates(double *coordinates, unsigned short covType) const;
  /*
   * This gets the uncertian part of the tranformation coordinates as
   * defined by CovType.  
   * @param coordinates A calcRows(CovType) by 1 matrix returning with 
   * @param covType Bit flags that define the uncertain coordinates,
   *                0000 0000 00(psi)(phi) (theta)(z)(y)(x).
   * the uncertian part of the coordinates.
   * @see Pose3D::setCovType.
   */
  void getCovCoordinates(Matrix & coordinates, unsigned short covType) const;

  //*******************GET FUNCTIONS***********************************
  /**
   * Set the 2D corrdiantes.
   * @param x A vector of x,y,theta
   */
  void getXYTheta(double x[3])const{
    Position.getXY(x);
    x[2]=getTheta();
  }
  /**
   * This allows all six corrdinates to be gotten at once into a double array.
   * @param x A vector to hold [x,y,z,theta, phi, psi].
   */
  void getCoordinates(double x[6])const{
    Position.getXYZ(x);
    Rotation3D::getAngles(x+3);
  }
  /**
   * Get the values of the rotation matrix. 
   * @param rotation the 2D rotation matrix is returned here.
   */
  void getRot2D(double rotation[4]) { Rotation3D::getRot2D(rotation);}   
   /**
   * Get the values of the inverse rotation matrix. 
   * @param rotaInv the inverse of the 2D 
   *  rotation matrix is returned here.
   */
 void getRot2Dinv(double rotaInv[4]){Rotation3D::getRot2Dinv(rotaInv);}   

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
int getJR2DxR2Dinv(double dR[4]){return Rotation3D::getJR2DxR2Dinv(dR);}
  
  //************* TRANSFORMS ****************************************
  /**
   * Translate then rotate a 2D point/vector using the rotation(R) and
   * translation(Position).  
   * 
   * tox=R(fromx-Position)  
   *
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
   * Translate then rotate a 3D point/vector using the rotation(R) and
   * translation(Position).  
   * 
   * tox=R(fromx-Position)  
   *
   * @param fromx the xy of the point to
   * transform can be same as tox.  
   * @param tox the xy of the transformed  point is returned here.
   */
  void transform(const Vector3D& fromx, Vector3D& tox){
    Vector3D temp=fromx;
    Vector3D tmp;
    temp-=Position;
    rotate(temp,tmp);
    tox=tmp;
  }
  /**
   * Inverse rotate then inverse translate a 3D point/vector using the
   * rotation and translation(Position).
   *
   * @param fromx the xy of the point to
   * inverse transform can be same as tox.  
   * @param tox the xy of the inverse transformed  point is returned here.
   */
  void invTransform(const Vector3D& fromx, Vector3D& tox){
    Vector3D tmp;
    invRotate(fromx,tmp);
    tmp+=Position;
    tox=tmp;
  }

  /**
   * Transform xyz points using this transformation
   *   
   * @param fromx position before the transform (the input)
   * @param tox position after the transform (the output)
   *
   * NOTE: fromx and tox do not need to be disjoint in memory,
   * i.e. the can be the same or partially overlap in memory
   */
  void transform(const double fromx[3],double tox[3]);

  /**
   * Transform xyz points using the inverse of this transformation
   *   
   * @param fromx position before the transform (the input)
   * @param tox position after the transform (the output)
   *
   * NOTE: fromx and tox do not need to be disjoint in memory,
   * i.e. the can be the same or partially overlap in memory
   */
  void invTransform(const double fromx[3],double tox[3]);
  
  //************* ALGERBRA ****************************************
  

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
  Transformation3D operator + (Transformation3D& b) const;

  /**
   * This does 
   *  a = a + b.
   *
   * This matrix is a which is set to a+b.
   *
   * @param b the second transformation.
  */
  void operator += (Transformation3D& b){
    Vector3D tmp;
    invTransform(b.Position,tmp);
    Position=tmp;
    Rotation3D::operator +=(b);
  }
  /**
   * This does 
   *  c = a - b, which means operating on a vector x:c(x) = b^(-1)(a(x)).
   * 
   * This matrix is a and c is the returned matrix.
   *
   * @param b the second transformation.
   * @return c =a-b
  */
  Transformation3D operator - (Transformation3D& b) const;
  

  /**
   * This does 
   *  a = a - b.
   *
   * This matrix is a which is set to a-b.
   *
   * @param b the second transformation.
  */
  void operator -= (Transformation3D& b){
    Rotation3D::operator -=(b);
    Vector3D tmp;
    tmp-=b.Position;
    invTransform(tmp,tmp);
    Position=tmp;
  }
  Transformation3D inverse();
  
  /** 
   * This does the composition of two transformations and returns the result
   * in this rotation. 
   *  
   *  A(x)=A(B(x))
   *
   *  Where a and b are Transformation3Ds.
   *  This is also writen 
   *  a=b+a
   *  
   *  a=this transformation,
   * 
   * @param b the second transformation.
   */
  void leftAdd(Transformation3D& b){
    b.invTransform(Position,Position);
    Rotation3D::leftAdd(b);
  }
  
  /** c=This Become b = -a+c*/
  /** 
   * This does the composition of two transformations and returns the result
   * in this rotation. 
   *  
   *  A(x)=A(invB(x))
   *
   *  Where a and b are Transformation3Ds.
   *  This is also writen 
   *  a=-b+a
   *  
   *  a=this transformation,
   * 
   * @param b the second transformation.
   */
  void leftSubtract(Transformation3D& b){
    b.transform(Position,Position);
    Rotation3D::leftSubtract(b);
}
  



  /*
    doAplusB(...) does an add and calculates the jacobian
    
    this=a+b; 
    
    Rot = b.Rot * a.Rot;
    
    X = b.X + a.Rot^T * b.X;
    
    jacobian is a 6 x 12 matrix
    
    jacobian = {Ja,Jb}
    
    
    The default is calculate everything.
    
    Note that jacobian ends up bigger than you need so you must delete the
    extra rows and columns yourself
    
      Set deleteExtra to 1 if you want them deleted automatically.
      This leaves the jacobian.Element bigger than needed 
      (it gets deleted when jacobian is). 
  */
  
  /**
   *  This does an add and calculates the jacobian.
   *  
   *  this=a+b; 
   *  
   *  Rot=b.Rot*a.Rot;
   *
   * X = b.X + a.Rot^T * b.X;
   *  
   *  jacobian is a 6 x 6 matrix
   *  
   * jacobian = {Ja,Jb}
   *
   * 
   * Select The columns and rows that you are interested in using by:
   * 
   * rows=sum 2^i
   *
   * where the sum is includes only the rows i that you want.
   * 
   * So rows 0 1 and 3 only (x, y and theta) give rows=1+2+8=11.
   *
   * The same for cols only then there are 12 columns to chose 
   *  from  ie. (a.x...b.psi).
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
   *  The default is calculate everything.
   *  
   *  Note that jacobian ends up bigger than you need so you may need
   *  to delete the extra rows and columns yourself.
   *  This way it is clear what the elements of jacobian mean. 
   *  Set deleteExtra to 1 if you want them deleted automatically. 
   *
   * @param a the first transformation.
   * @param b the other transformation.
   * 
   * @param jacobian the dervatives of the resulting coordinates wrt
   * the coordinates of a and b,
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
   * @see Rotation3D::doAplusB
  */
  void doAplusB(Transformation3D& a, Transformation3D& b,
		Matrix& jacobian,
		int rows=63, int cols=4095, int deleteExtra=0);
  /**
   * this=a-b.
   * @param a the first transformation.
   * @param b the other transformation.
   * 
   * @param jacobian the dervatives of the resulting coordinates wrt
   * the coordinates of a and b,
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
   * @see Transformation3D::doAplusB
  */
  void doAminusB(Transformation3D& a, Transformation3D& b,
		 Matrix& jacobian,
		 int shichRows=63, int whichCols=4095, int deleteExtra=0);
  /**
   * this=-a+b.
   * @param a the first transformation.
   * @param b the other transformation.
   * 
   * @param jacobian the dervatives of the resulting coordinates wrt
   * the coordinates of a and b,
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
   * @see Transformation3D::doAplusB
   *
  */
  void dominusAplusB(Transformation3D& a, Transformation3D& b,
		     Matrix& jacobian,
		     int rows=63, int cols=4095, int deleteExtra=0);
  /**
   * this=-a-b.
   * @param a the first transformation.
   * @param b the other transformation.
   * 
   * @param jacobian the dervatives of the resulting coordinates wrt
   * the coordinates of a and b,
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
   * @see Transformation3D::doAplusB
   */
  void dominusAminusB(Transformation3D& a, Transformation3D& b,
		      Matrix& jacobian,
		      int rows=63, int cols=4095, int deleteExtra=0);
 
  /*
   * Prints the object to the display screen.
   * @param verbose if >0 more info is printed
   */
  virtual void print();

private:  
};

/**
 * This function returns the number of rows needed by to make a
 * covarance matrix of this CovType.  
 * @param type the CovType flags.
 * @see Pose3D::setCovType.
 */
static inline unsigned short calcRows(unsigned short type)
{
  unsigned short r=0;
  for (type=(type&0x003F);type; type=(type>>1)) if (type&1)r++;
  return r;
}

} // namespace Cure

std::ostream& operator<<(std::ostream& os, const Cure::Transformation3D& t);

#endif // CURE_TRANSFORMATION3D_HH
