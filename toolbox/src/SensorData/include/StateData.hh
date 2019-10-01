// = AUTHOR(S)
//    John Folkesson
//
// = COPYRIGHT
//    Copyright (c) 2006 John Folkesson
//
/*----------------------------------------------------------------------*/

#ifndef CURE_STATEDATA_hh
#define CURE_STATEDATA_hh


#include "Matrix.hh" 
#include "ShortMatrix.hh" 
#include "SensorData.hh" 
#include <sstream>  // istringstream
#ifndef DEPEND
#include <iostream>  // ostream
#endif

// Forward declaration(s)
class PackedState;
namespace Cure {


/**
 * This class is used to send anything that
 * does not fit some predifined TimestampedData Class
 *
 *
 * The SubType for this is 
 * the size of ShortData, (Rows*Columns) 
 *
 * @author John Folkesson 
 */
class StateData :  public SensorData 
{  
public:

public:
  
  friend class PackedState;
  /**
   * Create empty StateData
   * @param id the id of this
   */
  StateData(unsigned short id = 0);
  /**
   * Create empty StateData
   * @param t the time
   * @param id the id of this
   */
  StateData(Timestamp &t,unsigned short id = 0);

  /**
   * Create a copy of a StateData
   * @param s The object to copy
   */
  StateData(const StateData &s);
  /**called by consturctors*/
  void init();
  /**
   * Destructor
   */
  virtual ~StateData();
  /*
   * This is how we (efficiently) get a StateData from a TimestampedData.
   */
  StateData * narrowStateData(){return this;}
  /**
   * Copy operator
   * @param src The object to copy
   */
  StateData& operator=(const StateData &src);  
  /**
   * Copy operator
   * @param p The object to copy
   */
  void operator = (TimestampedData& p)
  {
    StateData *ss=p.narrowStateData();
    if (ss)operator=(*ss);
    else TimestampedData::operator=(p);
  }
  int operator=(const std::string &cmd);
  void setStateID(short key);
  void zero()
  {
    State=0;
    Covariance=0;
    Jacobian=0;
  }
  /**
   * Returns the elements of the Covariance Matrix
   */
  double& operator()(const unsigned short r, const unsigned short c) {  
    return  Covariance(r,c);
  }
  /**
   * Returns the elements of the Covariance Matrix.
   */
  double operator()(const unsigned short r, const unsigned short c) const {
    return( (*(StateData *)this).operator()(r,c) );
  }
  /**
   * Returns the elements of the State vector
   */
  double& operator()(const unsigned short r) {  
    return  State((int)r,0);
  }
  /**
   * Returns the elements of the State vector.
   */
  double operator()(const unsigned short r) const {
    return( (*(StateData *)this).operator()(r) );
  }
  /** 
   * This puts as much of the statea and Covariance that fits into p
   * after changing its CovType to type.
   * It does not set CovType of p if type<0. 
  */
  void setPose(Pose3D &p, short type);
  unsigned short getDerivativeType()const {
    return((m_Flags&0x00FF)); }
  bool getIncrementalFlag()const{
    return((m_Flags&0x8000)); }
  unsigned short dim(){return State.Rows;}
unsigned short dimID(){return StateID.Rows;}
  void setDerivativeType(unsigned short r) {
    m_Flags=((m_Flags&0xFF00)|((0xFF00)&r)); }
  void setIncrementalFlag(bool r){
    if (r) m_Flags=(m_Flags|(0x8000)); 
    else m_Flags=(m_Flags&(0x7FFF)); 
  }
  void grow(unsigned short dim){
    State.grow(dim,1);
  }
  void growID(unsigned short dim){
    short c=StateID.Columns;
    if (c<1)c=1;
    StateID.grow(dim,c);
  }
  void growCov(unsigned short dim){
    Covariance.grow(dim,dim);
  }
  void growJac(unsigned short dim){
    Jacobian.grow(dim,dim);
  }
  void setSubType(unsigned short dim){
    if (StateID.Rows*StateID.Columns!=dim)
      setIDDim(dim);
    else m_SubType=dim;
  }
  void setFlags(unsigned short flag){
    setIncrementalFlag(flag&0x8000);
    setDerivativeType((flag&0x00FF));
  }
  void offset(int r, int dim){
    State.offset(r,0,dim,1);
    short c=StateID.Columns;
    if (c<1)c=1;
    StateID.offset(r,0,dim,c);
    Covariance.offset(r,dim);
  }
  void getValues(StateData &sd){
    State=sd.State;
    StateID=sd.StateID;
    Covariance=sd.Covariance;
  }
  /** 
   * This sets the Covariance size and fills it with 0.
   * The size is State.Rows 
   */
  void setCovariance();

  /** 
   * This sets the Covariance size and fills it with 0.
   * 
   */
  void setCovarianceDim(unsigned short r){
    Covariance.reallocate(r);
    Covariance=0;
  }
  void setJacobianDim(unsigned short r,unsigned short c){
    Covariance.reallocate(r,c);
    Covariance=0;
  }
  //  int setupState(std::istringstream &strdes, short key);
  
  /** 
   * This sets up a new State of the given size, with all zeros.
   *
   *
   */
  void setDim(unsigned short dim);
  /** 
   * This sets up a new StateID of the given size, with all zeros.
   * The subType==dim
   * @param rows the rows for StateID
   * @param columns if 0 then the number of current columns is unchanged
   * else StateID.Columns is set to columns.
   */
  void setIDDim(unsigned short rows, unsigned short columns=0);


  /**
   * This sets the State vector elements that match StatID 
   * It overwrites just that data and leaves the rest of the state unchanged.  
  */
  int getMatchedState(StateData &sd);
    
  /** Display info on the Object*/
 
  void print(); 
public:
  /**
   * This is the State Data.
   */
  Cure::Matrix State;
  Cure::Matrix Covariance;
  Cure::Matrix Jacobian;
  /**
   * This can be used to help interprete the data. Only the msb is checked
   * in this class.  So if StateType&0x8000 is true then the Data in
   * Data is interpolated as if incremental.  
   * if Flag is 0 the  timestamps are used to prorate  the 
   * interpolated value.    
   *.
   */
  unsigned short m_Flags;
  /** This is up to the user to define and use */
  unsigned short StateType;
  /**
   * This can hold whatever number of intergers is needed.
   */
  Cure::ShortMatrix StateID;
  /**
   * This returns data based on the string value and the indcies
   * given in the ShortMatrix.  
   *
   * @param str So State and Covariance are recongnized strings.
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
   * @param str So State and Covariance are recongnized strings.
   * 
   * @param index Set the index as when you would call
   * Cure::Matrix.offset(roff,coff,r,c) with the integers
   * roff,coff,r,c placed in a the top row of index (1x4).
   * 
   * @param mat the value here in a r x c matrix
   * @return true if ok else false. 
  */
  virtual bool setMatrix(const std::string & str, ShortMatrix & index,
			 Matrix &mat);



  virtual int interpolateRule(){
    if (getIncrementalFlag())return 1;
    if (getDerivativeType())return 2;
    return 0;
  }
      
  /**
   * Interpolate between two StateDatas to become a StateData at
   * t.  This depends on Derivative type and IncrementalFlag  
   * 
   *
   *
   * @param a data at one timestamp NOT this.
   * @param b data at another timestamp, NOT this.
   * @param t the time to interpolate this to between a and b.
   * @see Pose3D::interpolate_ 
  */
  virtual void interpolate_(TimestampedData& a, TimestampedData& b, 
			    Timestamp t){
    StateData *pa=a.narrowStateData();
    StateData *pb=b.narrowStateData();
    if ((pa!=0)&(pb!=0))return StateData::interpolate_(*pa,*pb,t);
    TimestampedData::interpolate_(a,b,t);
  } 
  /**
   * @param a data at one timestamp NOT this.
   * @param b data at another timestamp, NOT this.
   * @param t the time to interpolate this to between a and b.
   * 
  */
  void interpolate_(StateData& a, StateData& b, Timestamp t);
  /**
   * @param a data at one timestamp even this.
   * @param b data at another timestamp, even this.
   * @param t the time to interpolate this to between a and b.
   * @see StateData::interpolate_ 
  */
  void interpolate(StateData a, StateData b, Timestamp t){
    interpolate_(a,b, t);}

  /**
   * Update.
   * @param src 
   * Here the src has an innovation in its state, the covariance of that
   * innovation in its covariance,the jacobian between that innovation and 
   * this State in its jacobian and the StateID of the columns of
   * the jacaobian in its StateID.  These StateID should agree with
   * the def. of the StateID in this object. (For the relevant rows)
   * The part of the state that the innovation doesn't depend on
   * needs no ID set at all.  The rows of src's jacobian are in
   * the same order and number as the rows of its innovation.
   * When matching StateID only ID>0 are matched.
   *   
   * 
   * @return true if successful else false.   
   */
  //bool update(StateData &src);
  
  /**
   * The StateID is ignored here.  The Jacobians of innov wrt this and
   * measured are in those objects.  The Covariance of this and
   * measured are also in those objects.  The Covariance of innov is
   * then computed (@see getCov()) and used with the value of the
   * innovation to uppdate the State and Covariance of this.
   *
   * @return true if successful else false.   
   */
  bool update(StateData &innov, StateData &measured);

  /**
   * The StateID is ignored here.  Here this is a function of a and b.
   * a and b have the Jacobians between this and them in Jacobian.  This
   * Covariance is computed as the sum of the two independant
   * Covariances a and b projected with the jacobians.  The rows of the
   * (a and b)Jacobians must be set to match one another.  We also grab
   * the useful product JC for a in our Jacobian.  Our state does not
   * change.
   *
   * Our Jacobian and Covariance get set to:
   *  
   *  Jacobian = a.C  a.Jacobian^T  
   *
   *  Covaraince = a.Jacobian  a.C a.Jacobian^T + b.Jacobian  b.C b.Jacobian^T
   *
   * @return true if successful else false.   
   */
  bool getCov(StateData &a, StateData &b);

  /**
   * Predict.
   *
   * @param src Here the src has an incremental change to this objects
   * state in its state, the covariance of that change in its
   * covariance,the jacobian between that change and this State in its
   * jacobian and the StateID of the columns of its jacaobian in its
   * StateID.  These StateID should agree with the def. of the StateID
   * in this object. (For the relevant rows) The part of the state
   * that the increment doesn't depend on needs no ID set at all.  The
   * rows of src's jacobian coorespond to the rows of its state
   * vector.  these have to agree with its stateID as well.
   * When matching StateID only ID>0 are matched.
   * 
   * @return true if successful else false.   
   */
  // bool operator+=(const StateData &src);

  /**
   * This calculates JC.
   *
   * @param src where J is the jacobian in src and C is the covariance
   * of this object.  The StateID should agree with the def. of the
   * StateID in this object. (For the relevant rows) The part of the
   * state that the innovation doesn't depend on needs no ID set at
   * all.  The ID index this objects Covariance and the columns of
   * src's Jacobian.
   * When matching StateID only ID>0 are matched.
   *
   * @return true if successful else false.   
   */
  //  bool StateData::operator*=(const StateData &src);

    
protected:
  
private:
};
}

std::ostream& operator << (std::ostream& os, const Cure::StateData &cmd);

#endif // StateData_hh
