//    $Id: Pose3D.hh,v 1.15 2007/10/08 23:06:35 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_POSE3D_HH
#define CURE_POSE3D_HH

#include "Transformation3D.hh"
#include "include/Timestamp.hh"
#include "TimestampedData.hh"

//#include <sstream>  // istringstream


namespace Cure{

/**
 * This class is the Cure workhorse, the most used class aside form
 * Matrix.  It represents timestamped transformations and can carry
 * out some tricky computaions such as linearization of transforms
 * operation on transforms, lines points and so on. One can specify
 * just which of the 6dof are to be considered variables for computing
 * covariances and Jacobians.  This is done by a binay flag
 * interpretaion of the class subType member.  So if its 2D 3dof it
 * knows it by the flags set in its CovType which is contained in the
 * subType.
 *
 *
 * The Pose3D Class adds a timestamp, velocity, covariance matrix 
 * and type to the Transforamtion3D Class.  
 *
 * The Pose3D.Type is to tell how to add this. 
 * If we look at Type in binary
 *
 * Type=(I,NoTranslate,NoRotate,2D,C5,C4,C3,C2,C1,C0,V5,V4,V3,V2,V1,V0),
 *
 * First NoTranslate, NoRotate and 2D have to do with 'Packing' the Pose.
 * If you do not plan on making any PackedPose objects they don't matter.
 * If you must know anyhow:
 * Not setting these (ie =0) will pack all of xyz and theta Phi Psi,
 * 
 *setting the 2D bit will ignore z,phi and psi when packing (assumes they=0).
 *
 * setting the NoRotate will ignore theta, phi, psi.
 *
 * setting the NoTranslate will ignore xyz.
 *
 * setting the I bit indicates that it is incremental and not cumulative data
 *
 * (so for 2D translations, just pack xy with (#011############)
 *
 * For the Rest of Type:
 *
 * The coordinates that are present in Covariance will have 1 for Ci 
 *
 * and the velocities that are not present will have 0 for Vi.
 *
 * So for xy theta pose and xyztheta Velocity we have:
 * 
 * Type=(####001011001111)=1+2+4+8+64+128+512=719 
 *
 * One can use:
 *  
 * int calcType(int x, int y,int z,int theta,int phi,int psi );  
 * 
 * void SetCovType(int t);
 * 
 * void SetVelType(int t);
 * 
 * To set this if one wants.
 * 
 * Type=0 (default) makes this ignore Covariance and Velocity. 
 * 
 *  NOTES on UNDERSCORE:
 *
 * All the methods with an underscore at the end of the method name as
 * in add_(...)  are not value safe.  That is do not call them with 
 * the argument the same object as 'this' 
 *
 *
 * DO NOT DO objectA.methodB_(objectA) DO NOT DO
 *
 * 
 * Name_.  They take a Matrix& as an argument which can not be this.
 * 
 * Do NOT code:
 * 
 * a.add_(a,a);  
 * 
 * Do instead: 
 * 
 * Pose3D b(a);
 * a.add_(b,b);
 *     
 * Of course the good news is they are more efficient by not making 
 * intermeadiate copies all the time.
 *
 *  Notes on Transform Algerbra:
 * 
 *  'Adding' two transforms a + b (one usually puts a circle around
 *  these + and - signs:
 * 
 *  c = a + b, which means operating on a vector x:c(x) = b(a(x)).
 * 
 * More explictly:
 * 
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
 * The minus works similar but one uses the invers transforms...
 * 
 * 
 * @see Transformation3D
 * @author John Folkesson
 */  
class Pose3D : public Transformation3D, public TimestampedData
{
public:
  /** The uncertianty of the coordinates is stored in this matrix */
  Matrix Covariance;
  /** The velocity may be stored here*/
  double Velocity[6];
public:
  /** The constructor*/
  Pose3D():Transformation3D(){init();}
  /** 
   * The copy constructor
   * @param p the pose to copy to this.
   */
  Pose3D(const Pose3D& p);
  /**Called by the constructors*/
  void init();
  virtual ~Pose3D(){}
  /**
   * @return a pointer to this.
   */ 
  Pose3D * narrowPose3D(){return this;}
  //*******************SET FUNCTIONS***********************************

  /** This takes the Value, CovType Cov from the stream*/
  int setupPose(std::string &cmd);
  /**
   * This does not set the ID to the same, only the SubTypes.
   * @param p the data to copy. 
  */
  void operator = (TimestampedData& p)
  {
    Pose3D *pp=p.narrowPose3D();
    if (pp)operator=(*pp);
    else TimestampedData::operator=(p);
  }
  /*
   * The copy operator.
   * @param p a pose to copy to this.
   */
  void operator = (const Pose3D& p);
  /**
   * Set the coordinates to values in a double array. 
   * @param x This is and array of [x,y,z,theta,phi,psi]
   */
  void operator = (double x[6]){ 
    Transformation3D::operator =(x);
  }
  /**
   * Copy the transformation data only.
   * @param t the transformation to copy.
   */
  void operator = (Transformation3D& t){
    Transformation3D::operator=(t);
  }
  /**
   * Gets the reference to the covarance elements.
   * @param r the row
   * @parm c the column.
   * @return the ref of elements of the Covariance Maatrix.
   */
  double& operator()(const int r, const int c) {  
    return( Covariance(r,c) );
  }
  /**
   * Gets the value of  the covarance elements.
   * @param r the row
   * @parm c the column.
   * @return the value of elements of the Covariance Maatrix.
   */
  double operator()(const int r, const int c) const {
    return( (*(Pose3D *)this).operator()(r,c) );
  }

  /**
   * Tell what kind of Pose this is, cumulative ,translation, rotation,  3D, 
   * vs. incremental, no-translation, no-rotation,  2D. 
   *here t is from 0..15 and sets the 4 most significant bits of Type.
   * 
   * @param t the coordinate flages are ordered (increamental,no-translate,
   * no-rotate,2D), so cumulative 2D rotations would be (0b0101=0x5).
   */
  void setCoordinateType(unsigned short t);
  /**
   * Tell which of the 6 coordinates are included in the covariance
   * estimate.  The CovType should be viewed as a set of binary flags.
   * The least significant bit of CovType is the x flag.  If the x
   * flag is set, (CovType is then an odd number), that means that x
   * is to be included as part of the uncertainty estimate described
   * by the Covariance matrix. In particular x will be coorespond to
   * the first row and column of the Covariance.  The 2's bit is for y
   * and so on up to the 6th bit position (decimal 32) which is for
   * the third Euler angle psi.
   *
   *
   * This also allocates the Covariance Matrix.
   * @param t the covariance flages are ordered (psi,phi,theta,z,y,x)
   * with psi the MSB (32 decimal or 0b100000 or 0x20) and x the LSB. 
   *  @see Pose3D::setSubType.   
  */
  void setCovType(unsigned short t);
  /**
   * Query which of the 6 coordinates are included in the covariance estimate.
   * 
   * @return the covariance flages are ordered (psi,phi,theta,z,y,x)
   * with psi the MSB (32 decimal or 0b100000 or 0x20) and x the LSB. 
   * @see Pose3D::setCovType.  
 */

  /**
   * This returns data based on the string value and the indcies
   * given in the ShortMatrix.  
   *
   * @param str So x, Vel,Velocity, Cov and Covariance
   * Are recongnized strings.
   * 
   * @param index Set the index as when you would call
   * Cure::Matrix.offset(roff,coff,r,c) with the integers
   * roff,coff,r,c placed in a the top row of index (1x4).
   * 
   * @param mat the result is returned here in a r x c matrix
   * @return true if ok else false. 
   */
  virtual bool getMatrix(const std::string & str, ShortMatrix & index,
			 Matrix &mat);

  /**
   * This sets data based on the string value and the indcies
   * given in the ShortMatrix.  
   *
   * @param str So x, Vel,Velocity, Cov and Covariance
   * Are recongnized strings plus the TimestampedData Base strings.
   * Be sure to first setSubType before trying to put values into
   * the covariance unless you know that it is correct  already. 
   *
   *
   * @param index Set the index as when you would call
   * Cure::Matrix.offset(roff,coff,r,c) with the integers
   * roff,coff,r,c placed in a the top row of index (1x4).
   * 
   * @param mat the value to set to is  here in a r x c matrix
   * @return true if ok else false. 
   */
  virtual bool setMatrix(const std::string & str, ShortMatrix & index,
			 Matrix &mat);


  unsigned short getCovType() const 
  {return (unsigned short)((0x3F)&(m_SubType>>6));}
   /**
   * Tell which of the 6 coordinates are included in the velocity estimate.
   * 
   * @param t the velocity flages are ordered (psi,phi,theta,z,y,x)
   * with psi the MSB (32 decimal or 0b100000 or 0x20) and x the LSB. 
   * @see Pose3D::setCovType.  
   */
  void setVelType(unsigned short t);
 /**
   * Query which of the 6 coordinates are included in the velocity estimate.
   * 
   * @return the velocity flages are ordered (psi,phi,theta,z,y,x)
   * with psi the MSB (32 decimal or 0b100000 or 0x20) and x the LSB. 
   * @see Pose3D::setCovType.  
   */
  unsigned short getVelType(){return (unsigned short) (m_SubType&0x3F);}

  /**
   * The Pose3D.m_SubType, (we will call this just type below), is to
   * tell how to add the poses together.  So the covariance that
   * results from the composition of two transformations ('adding
   * them') will depend on what parts of the transformations were
   * uncertain.  This information is coded in the type so that the
   * objects will automatically know how to do the linearizations
   * needed to calculate the covariance for the answer.
   *
   * If we look at type in binary:
   *
   * type=(I,NoTranslate,NoRotate,2D,C5,C4,C3,C2,C1,C0,V5,V4,V3,V2,V1,V0),
   *
   * First NoTranslate, NoRotate and 2D have to do with 'Packing' the Pose.
   * If you do not plan on making any PackedPose objects they don't matter.
   * If you must know anyhow:
   * Not setting these (ie =0) will pack all of xyz and theta Phi Psi,
   * setting the 2D bit will ignore z,phi and psi when packing (assumes they=0).
   * setting the NoRotate will ignore theta, phi, psi.
   * setting the NoTranslate will ignore xyz.
   * setting the I bit indicates that it is incremental and not cumulative data
   * (so for 2D translations, just pack xy with (#011############)
   *
   * For the rest of type:
   * The coordinates that are present in Covariance will have 1 for Ci 
   * and the velocities that are not present will have 0 for Vi.
   * So for xy theta pose and xyztheta Velocity we have:
   * 
   * type=(####001011001111)=1+2+4+8+64+128+512=719 
   *
   * One can use:
   *  
   * int calcType(int x, int y,int z,int theta,int phi,int psi );  
   * void setCovType(int t);
   * void setVelType(int t);
   * 
   * To set this if one wants.
   * type=0 makes this ignore Covariance and Velocity. 
   *
   * @param t new sub type
   * @see Pose3D::setCovType, @see Pose3D::setVelType
   */
  void setSubType(unsigned short t);
  /**
   * This sets the values of the covarance matrix.
   * No check is made to see if the number of rows is consistent 
   * with the CovType.
   * @param p the covariance values.
   */
  void setCovariance(Matrix& p){Covariance=p;}
  /**
   * The coordinates, velocity, time and covariance are all set to 0.
   */ 
  void zero();
  /**
   * Set the velocity values.
   * @param v the velocity values [vx, vy,vz,vtheta,vphi,vpsi]
   */
  void setVelocity(const double v[6]);  

  /**
   * Get the velocity values.
   * @param v the velocity values [vx, vy,vz,vtheta,vphi,vpsi] are
   * returned here
   */
  void getVelocity(double v[6])const;

  /**
   * Returns the value of the Covariance maatrix.
   */
  Matrix getCovariance()const{return Covariance;}

  void separateVelocity(Matrix &jacC,Matrix &jacV,Matrix &jac,
			Pose3D &a, Pose3D &b);
    
  //*********************NOT VALUE SAFE*****************************
  /**
   * All the methods below are not value safe.  
   * Signaled by the '_' in the Name_.
   * They take a Matrix& as an argument which can not be this.
   * 
   * Do NOT code:
   * 
   * a.Add_(a,a);  
   * 
   * Do instead: 
   * 
   * Pose3D b(a);
   * a.Add_(b,b);
   *     
   * Of course the good news is they are more efficient by not making 
   * intermeadiate copies all the time.
   */
    
  //******************** ALGERBRA *****************************************
    
  /**
   *  This adds  two poses and calculates the covariance and Jacobians.
   * 
   * this = b + a
   * 
   * if Type==0 it stops after calculating the new transform x
   * if Type!=0 it looks at the types of a, b and this to 
   * calculate Velocity and Covariance using the Jacobians
   * 
   * Velocity = jb * b.Velocity + ja * a.Velocity 
   * 
   * Covariance = jb*b.Covariance*jb^T + ja*a.Covariance*ja^T 
   * 
   * for those components that are valid according to 
   * a.Type, b.Type and Type.
   * 
   * The jacobian for just the Covariance Types (unused rows and
   * columns not present) can be returned in 'jacobian'.  The caller
   * will need to figure out what coordinates the rows and columns refer to.
   * 
   * this = c = a + b
   * jacobian={ Jca, Jcb } maximum a 6 x 12 matrix
   * so the rows are [x y z theta phi psi] with unused coordinates deleted.
   * similarly for the columns.
   * @param a a Pose to add, not this Pose3D object.
   * @param b another pose to add, not this Pose3D object..
   * @param jacobian if not 0 the Jacobian matrix is returned here.
  */
  void add_(Pose3D& a,Pose3D& b, Matrix *jacobian=0); 
  /**
   *  this=a+b but the a or b can be this.
   * @see Pose3D::add_
   * @param a a Pose to add. 
   * @param b another pose to add, 
   * @param jacobian if not 0 the Jacobian matrix is returned here.
   */ 
  void add(Pose3D a,Pose3D b, Matrix *jacobian=0){
    add_(a, b, jacobian); 
  }
  /**
   * this=a-b, the a or b can NOT be this.
   * @see Pose3D::add_
   * @param a a Pose to subtract from, not this Pose3D object.
   * @param b another pose to subtract, not this Pose3D object..
   * @param jacobian if not 0 the Jacobian matrix is returned here.
   */ 
  void subtract_(Pose3D& a,Pose3D& b, Matrix *jacobian=0); 
  /**
   *  this=a-b but the a or b can be this.
   * @see Pose3D::add_
   * @param a a Pose to subtract from,
   * @param b another pose to subtract, 
   * @param jacobian if not 0 the Jacobian matrix is returned here.
   */ 
  void subtract(Pose3D a,Pose3D b, Matrix *jacobian=0){
    subtract_(a,b,jacobian); 
  }
  /**
   *  this=-a+b but the a or b can NOT be this.
   * @see Pose3D::add_
   * @param a a Pose, NOT this,
   * @param b another pose, NOT this, 
   * @param jacobian if not 0 the Jacobian matrix is returned here.
   */ 
  void minusPlus_(Pose3D& a,Pose3D& b, Matrix *jacobian=0); 
  /**
   *  this=-a+b but the a or b can be this.
   * @see Pose3D::add_
   * @param a a Pose,
   * @param b another pose.
   * @param jacobian if not 0 the Jacobian matrix is returned here.
   */ 
  void minusPlus(Pose3D a,Pose3D b, Matrix *jacobian=0){
    minusPlus_(a,b,jacobian);
  } 
  
  /**
   *  this=-a-b but the a or b can NOT be this.
   * @see Pose3D::add_
   * @param a a Pose, NOT this,
   * @param b another pose, NOT this, 
   * @param jacobian if not 0 the Jacobian matrix is returned here.
   */ 
  void minusMinus_(Pose3D& a,Pose3D& b, Matrix *jacobian=0); 
  /**
   *  this=-a-b but the a or b can be this.
   * @see Pose3D::add_
   * @param a a Pose,
   * @param b another pose.
   * @param jacobian if not 0 the Jacobian matrix is returned here.
   */ 
  void minusMinus(Pose3D a,Pose3D b, Matrix *jacobian=0){
    minusMinus_(a,b, jacobian);
  } 

  /**
   *  this=-b but the  b can NOT be this.
   * @see Pose3D::add_
   * @param b another pose, NOT this, 
   * @param jacobian if not 0 the Jacobian matrix is returned here.
   */ 
  void minus_(Pose3D& b, Matrix *jacobian=0); 
  /**
   *  this=-b but the  b can be this.
   * @see Pose3D::add_
   * @param b another pose.
   * @param jacobian if not 0 the Jacobian matrix is returned here.
   */ 
  void minus(Pose3D b, Matrix *jacobian=0){
    minus_(b,jacobian); 
  }
    
  /**
   * Interpolate between two Pose3Ds to become a Pose3D at t.
   * If you think this is trivial its not so trivial.
   * I have got this wrong enough to want this function.
   * Think about the average Pi and -Pi = Pi not zero...
   * @param a data at one timestamp NOT this.
   * @param b data at another timestamp, NOT this.
   * @param t the time to interpolate this to between a and b.
   * @see Pose3D::interpolate_ 
  */
  virtual void interpolate_(TimestampedData& a, TimestampedData& b, 
			    Timestamp t){
    Pose3D *pa=a.narrowPose3D();
    Pose3D *pb=b.narrowPose3D();
    if ((pa!=0)&(pb!=0))Pose3D::interpolate_(*pa,*pb,t);
    else TimestampedData::interpolate_(a,b,t);
  } 
  /**
   * If pose a is not incremental (coorrdinateType = (0###))
   * Then this it a linear interpolation between a and b for some 
   * time t between a.Time and b.Time.
   * If a is incremental then this takes a fraction of a based on the interval 
   * (t-b.Time)/(a.Time-b.Time)*(b'a incremental coordinate changes) 
   * (for b.Time<t<a.Time) 
   * The order doesn't mataer as it will sort a and b by time first.
   * So notice that it is the larger time pose that gets interpolated 
   * (reduced) for incremental Poses. 
   * @param a data at one timestamp NOT this.
   * @param b data at another timestamp, NOT this.
   * @param t the time to interpolate this to between a and b.
   * @see Pose3D::interpolate_ 
  */
  void interpolate_(Pose3D& a, Pose3D& b, Timestamp t);
  /**
   * If pose a is not incremental (coorrdinateType = (0###))
   * Then this it a linear interpolation between a and b for some 
   * time t between a.Time and b.Time.
   * If a is incremental then this takes a fraction of a based on the interval 
   * (t-b.Time)/(a.Time-b.Time)*(b'a incremental coordinate changes) 
   * (for b.Time<t<a.Time) 
   * The order doesn't mataer as it will sort a and b by time first.
   * So notice that it is the larger time pose that gets interpolated 
   * (reduced) for incremental Poses. 
   * @param a data at one timestamp even this.
   * @param b data at another timestamp, even this.
   * @param t the time to interpolate this to between a and b.
   * @see Pose3D::interpolate_ 
  */
  void interpolate(Pose3D a, Pose3D b, Timestamp t){
    interpolate_(a,b, t);}
  /**
   * prints info to the display.
   */
  virtual void print();

  /**
   * Use this if you can't manage to figure out type;
   *  Use to make a covariance or a velocity type;
   * @return the cooresponding type flags. 
   * @param x if 1 it will include x in returned type, 0 to leave out x.
   * @param y if 1 it will include y in returned type, 0 to leave out y.
   * @param z if 1 it will include z in returned type, 0 to leave out z.
   * @param theta if 1 it will include theta in returned type, ...
   * @param phi ...
   * @param psi ...
   */
  static unsigned short calcType(int x, int y,int z,int theta,int phi,int psi );  
  /**
   * This doesn't care about ID Covarance or Velocity, so just Type,
   * coordinates and time.
   *
   * @param t the data to compare.
   * @return 1 if identical else 0.
   */
  unsigned short operator == (const Pose3D& t)const;
  
};
  /**
   * Takes the or between the VelType and CovType parts of type and
   * returns the result. 
   * @param type holds the VelType and CovType.
   * @param ptype returns with only the CovType in the lowest 6 bits.
   * @return the or of VelType and CovType.
   */
static inline unsigned short getTypeUnion(const unsigned short type,
                                         unsigned short& ptype)
{
  ptype=(type>>6);
  ptype=(ptype&63);
  return (ptype|(type&63));
}
  
} // namespace Cure

std::ostream& operator<< (std::ostream& os, const Cure::Pose3D &p);

#endif // CURE_POSE3D_HH
