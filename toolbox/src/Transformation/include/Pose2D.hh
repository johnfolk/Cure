//    $Id: Pose2D.hh,v 1.3 2007/09/14 09:14:05 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_POSE2D_HH
#define CURE_POSE2D_HH

#include "Transformation2D.hh"
#include "Timestamp.hh"

namespace Cure{  

/**
 * The Pose2D Class adds a timestamp, velocity, covariance matrix 
 * and type to the Transforamtion2D Class. 
 *
 * The Pose2D.m_SubType is to tell how to add this. 
 * If we look at type in binary
 *
 * type=(#,#,#,#,#,#,#,#,#,#,C2,C1,C0,V2,V1,V0)
 *
 * where the coordinates that are present in the Covariance
 * will have 1 for Ci 
 * and the velocities that are NOT present 
 * will have 0 for Vi
 * so for xy theta Coariance and xy Velocity we have 
 * m_SubType=(##########111011)=1+2+8+16+32=?? 
 *
 * @author John Folkesson
 */  
class Pose2D : public Transformation2D
{
public:
  Matrix Covariance;
  double Velocity[3];
  Timestamp Time;
public:
  Pose2D();
  Pose2D(Pose2D& p):Transformation2D()
  {this->Pose2D::operator=(p);}
  virtual ~Pose2D(){}
  
  //*******************SET FUNCTIONS***********************************
  void operator=(Pose2D& p);
  void operator=(Transformation2D& t){
    Transformation2D::operator=(t);
  }
  double& operator()(const int r, const int c) {
    return( Covariance.Element[r*Covariance.RowInc+c] );
  }
  double operator()(const int r, const int c) const{
    return( (*(Pose2D *)this).operator()(r,c) );
  }  

  unsigned short getSubType() const { return m_SubType; }
  void getSubType(unsigned short t) { m_SubType = t; }

  void setTime(Timestamp t){Time=t;}
  void setTime(const double t){Time=t;}
  void setVelocity(const double v[3]);
  void setCovariance(Matrix& p){Covariance=p;}
  
  //*******************GET FUNCTIONS***********************************
  Timestamp getTime()const{return Time;}
  double getDoubleTime()const{return Time.getDouble();}
  void getVelocity(double v[3])const;
  Matrix getCovariance()const{return Covariance;}
  
  //*********************NOT VALUE SAFE*****************************
  /*
    All these  are not value safe.  Signaled by the '_' in the Name_.
    They take a Matrix& as an argument which can not be this.
    
    Do NOT code:
    
    a.Add_(a,a);  
    
    Do instead: 
    
    Pose2D b(a);
    a.Add(b,b);
    
    Of course the good news is they are more efficient by not making 
    intermeadiate copies all the time.
    
    void Add_(Pose2D& b,Pose2D& a)
    
    c=b+a
    
    if Type==0 it stops after calculating the new transform x
    if Type!=0 it looks at the types of a, b and c to 
    calculate Velocity and Covariance
    
    c.v=jb*b.Velocity+ja*a.Velocity 
    
    c.Covariance=jb*b.Covariance*jb^T+ja*a.Covariance*ja^T 
    
    for those components that are valid according to 
    a.Type, b.Type and c.type
  */  
  //this = c = a + b
  void add_(Pose2D& a,Pose2D& b); 
  void add(Pose2D a,Pose2D b){
    add_(a,b); 
  }
  
  //this = a = c - b
  void subtract_(Pose2D& c,Pose2D& b);
  void subtract(Pose2D c,Pose2D b){
    subtract_(c,b);
  }
  
  //this = b = -a + c
  void minusPlus_(Pose2D& a,Pose2D& c);
  void minusPlus(Pose2D a,Pose2D c){
    minusPlus_(a,c);
  }
  
  //this = c = -a - b
  void minusMinus_(Pose2D& a,Pose2D& b);
  void minusMinus(Pose2D a,Pose2D b){
    minusMinus_(a,b);
  }
  
  //this = a = -b = I-b 
  void minus_(Pose2D& b);
  void minus(Pose2D b){
    minus_(b);
  }
  /*
    Interpolate between two Pose2Ds to become a Pose2D at t,
    If you think this is trivial its not so trivial.
    I have got this wrong enough to want this function.
    Think about the average Pi and -Pi = Pi not zero...
  */
  void interpolate_(Pose2D& a, Pose2D& b, Timestamp t);
  void interpolate(Pose2D a, Pose2D b, Timestamp t){
    interpolate_(a,b,t);
  }
  
  virtual void print();

protected:
    unsigned int m_SubType;
};

} // namespace Cure

#endif // CURE_POSE2D_HH
