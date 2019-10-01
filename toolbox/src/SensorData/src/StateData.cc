//
// = LIBRARY
//
// = FILENAME
//    StateData.cc
//
// = AUTHOR(S)
//    john Folkesson
//
// = COPYRIGHT
//    Copyright (c) 2006 John Folkesson
//
/*----------------------------------------------------------------------*/
#include <ctype.h>  // isdigit
#include "StateData.hh"
#include "Pose3D.hh"

namespace Cure {

StateData::StateData(unsigned short id)
{
  
  init();
  setID(id);
}

StateData::StateData(Timestamp &t, unsigned short id)
{
  init();
  Time=t;
  setID(id);
}

/**
 * Create a copy of a StateData
 */
StateData::StateData(const StateData &src)
{
  init();
  (*this) = src;
}

StateData::~StateData()
{}


void StateData::init()
{
  m_Packed =false;
  m_ClassType=(STATE_TYPE);
  setSubType(0);
  setID(0);
  m_Flags=0;
  StateType=0;
  Covariance.reallocate(0);
  Jacobian.reallocate(0);
  StateID.reallocate(0,1);
  State.reallocate(0);
}

StateData & StateData::operator=(const StateData &src)
{
  Time=src.Time;
  setSubType(src.getSubType());
  setID(src.getID());
  setSensorID(src.getSensorID());
  setSensorType(src.getSensorType());
  setIncrementalFlag(src.getIncrementalFlag());
  setDerivativeType(  src.getDerivativeType());
  State=src.State;
  Covariance=src.Covariance;
  Jacobian=src.Jacobian;
  StateID=src.StateID;
  StateType=src.StateType;
  return (*this);
}
void StateData::setCovariance()
{
  Covariance.reallocate(State.Rows);
  Covariance=0;
}


void StateData::setDim(unsigned short dim)
{
  State.reallocateZero(dim,1);
  State=0;
}
void StateData::setIDDim(unsigned short rows,unsigned short col)
{
  if (col==0){
    col=StateID.Columns;
    if (col<1)col=1;
  }
  StateID.reallocate(rows,col);
  StateID=0;
  m_SubType=rows*col;
}

int StateData::getMatchedState(StateData &sd)
{
  int k=0;
  for (int i=0;i<StateID.Rows;i++)
    for (int j=0;j<sd.StateID.Rows;j++){
      if (StateID(i,0)==sd.StateID(j,0))
	{
	  State(i,0)=sd.State(j,0);
	  k++;
	}
    }
  return k;
}
/**
 * The StateID is ignored here.  The Jacobians of innov wrt this and
 * measured are in those objects.  The Covariance of this and measured
 * are also in those objects.  The Covariance of innov is then
 * computed and used with the value of the innovation to uppdate the
 * State and Covariance of this.
 */
bool StateData::update(StateData &innov, StateData &measured)
{
  innov.getCov((*this),measured);
  if (innov.Covariance.invert())return false;
  Matrix w; 
  w.multiply_(innov.Jacobian,innov.Covariance);
  State.subtractProduct_(w,innov.State);
  Matrix dc;
  dc.multiply_(w,Covariance);
  Covariance-=dc;
  return true;
}

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
 */
bool StateData::getCov(StateData &a, StateData &b)
{
  int  r=a.Jacobian.Rows;
  if (r!=b.Jacobian.Rows)return false;
  Covariance.reallocateZero(r);
  Jacobian.reallocateZero(r,a.Covariance.Rows);
  Matrix cjb(r,b.Covariance.Rows);
  Jacobian.multTranspose_(a.Covariance,a.Jacobian,2);
  cjb.multTranspose_(b.Covariance,b.Jacobian,2);
  Covariance.multiply_(a.Jacobian,Jacobian);
  Covariance.addProduct_(b.Jacobian,cjb);
  return true;
}



/**
 * This calculates a.C a.J^T, where a.J is the jacobian in a and a.C
 * is the covariance.  The number in StateID of this should index into
 * the rows of a.Covariance .  The part of this->state that the this
 * doesn't depend on a.State needs no ID set at all.  The Columns of
 * a.Jacobian are not the same number or order as a.Covariance.
 * Instead they are the number and order of this->StateID The result
 * is stored in this->Jacobian.
 *
 * So when calling:
 *
 *  a.Jacobian has a row for each dim of this 
 *
 *  a.Jacobian has a column for each row of a's dim that this depends on
 *  
 *  this->StateID(i,0) has the row of a.C that cooressponds to column
 *  i of a.Jacobian.
 *
 *  On return: 
 *
 *  this->Jacobian has a row for each dim of a.Covariance and a column
 *  for each dim of this.
 *
 * @return false if fail
 */
/*
bool StateData::multiply_(StateData &a)
{
  if (a.Jacobian.Columns>StateID.Rows)return false;
  Jacobian.reallocateZero(a.Covariance.Rows,a.Jacobian.Rows);
  for (int k=0;k<(int)StateID.Rows; k++)
    for (int i=0; i<Jacobian.Rows; i++)
      for (int j=0; j<Jacobian.Columns; j++)
	    Jacobian(i,j)+=(a.Covariance(i,StateID(k,0))*
	      a.Jacobian(j,k));
  return true;
}
*/
/*
bool StateData::multiply_(Matrix *a,ShortMatrix *Index, Matrix &result)
{
  if (!a)result=&Covariance;
  if (!a)a=&Jacobain
  if (a.Jacobian.Columns>StateID.Rows)return false;
  Jacobian.reallocateZero(a.Covariance.Rows,a.Jacobian.Rows);
  for (int k=0;k<StateID.Rows; k++)
    for (int i=0; i<Jacobian.Rows; i++)
      for (int j=0; j<Jacobian.Columns; j++)
	    Jacobian(i,j)+=(a.Covariance(i,StateID(k,0))*
	      a.Jacobian(j,k));
  return true;
}
*/
/**
 * Here the dx has an incremental change to this objects state in its
 * state, the covariance of that change in its covariance,the
 * jacobian between that change and this State in its jacobian and
 * the StateID set up as in *=. 
 * 
 *
 *
 *
 *
 */
/*
bool StateData::operator+=(StateData &dx)
{
  //  if (StateID.Rows>State.Rows) return false;
  //if (src.StateID.Rows>src.State.Rows) return false;
  //if (!StateData::operator*=(src))return false;
  multiply_(dx);
  dx.Covariance.transpose(Jacobian);
  multiply_(dx);
  for (int i=0;i<dx.StateID.Rows;i++)
    {
      State(dx.StateID(i,0),0)+=dx(i);
      for (intj=0;j<dx.StateID.Rows;j++)
	Covariance(dx.StateID.(i,0),dx.StateID.(j,0))=Jacobian(i,j)+dx(i,j);
    }
  return true;
}
*/
/*
bool StateData::update(StateData &est)
{
  (*this)*=est
}
*/
/**
 * This calculates JC, where J is the jacobian in src and C is the
 * covariance of this object.  The StateID should agree with the
 * def. of the StateID in this object. (For the relevant rows) The
 * part of the state that the innovation doesn't depend on needs no ID
 * set at all.  The ID index this objects Covariance and the columns
 * of src's Jacobian.  
 *
 */
/*
bool StateData::operator*=(const StateData &src)
{
  if (src.StateID.Rows>src.Jacobian.Columns)return false;
  if (StateID.Rows>Covariance.Rows)return false;
  Jacobian.reallocate(src.Jacobian.Rows,Covariance.Rows);
  Jacobian=0;
  for (int k=0;k<src.StateID.Rows; k++)
    for (int h=0;h<StateID.Rows; h++)
      if ((StateID(h,0)>0)&&(src.StateID(k,0)==StateID(h,0)))
	for (int i=0; i<src.Jacobian.Rows; i++)
	  for (int j=0; j<Covariance.Rows; j++)
	    Jacobian(i,j)+=src.Jacobian(i,k)*Covariance(h,j);
  return true;
}

*/
/**
 * Predict.
 * Here the src has an incremental change to this objects state in its
 * state, the covariance of that change in its covariance,the
 * jacobian between that change and this State in its jacobian and
 * the StateID of the columns of its jacaobian in its StateID.  These
 * StateID should agree with the def. of the StateID in this
 * object. (For the relevant rows) The part of the state that the
 * increment doesn't depend on needs no ID set at all.  The rows
 * of src's jacobian coorespond to the rows of its state vector.
 * these have to agree with its stateID as well.
 */
/*
bool StateData::operator+=(const StateData &src)
{
  if (StateID.Rows>State.Rows) return false;
  if (src.StateID.Rows>src.State.Rows) return false;
  if (!StateData::operator*=(src))return false;
  Matrix jcj=src.Covariance;
  for (int k=0;k<src.StateID.Rows; k++)
    for (int h=0;h<StateID.Rows; h++)
      if ((StateID(h,0)>0)&&(src.StateID(k,0)==StateID(h,0))){
	State(h,0)+=src.State(k,0);
	for (int i=0; i<src.Jacobian.Rows; i++)
	  for (int j=0; j<jcj.Rows; j++)
	    jcj(j,i)+=src.Jacobian(i,k)*Jacobian(j,h);
      }
  for (int k=0;k<src.StateID.Rows; k++)
    for (int h=0;h<StateID.Rows; h++)
      if (src.StateID(k,0)==StateID(h,0)){
	for (int j=0;j<Covariance.Rows; j++){
	  Covariance(h,j)+=Jacobian(k,j);
	  Covariance(j,h)+=Jacobian(k,j);
	}
	for (int k2=0;k2<src.StateID.Rows; k2++)
	  for (int h2=0;h2<StateID.Rows; h2++)
	    if (src.StateID(k2,0)==StateID(h2,0)){
	      Covariance(h,h2)+=jcj(k,k2);
	    }
      }
  return true;
}
*/
/**
 * Here the src has an innovation in its state, the covariance of that
 * innovation in its covariance,the jacobian between that innovation and 
 * this State in its jacobian and the StateID of the columns of
 * the jacaobian in its StateID.  These StateID should agree with
 * the def. of the StateID in this object. (For the relevant rows)
 * The part of the state that the innovation doesn't depend on
 * needs no ID set at all.  The rows of src's jacobian are in
 * the same order and number as the rows of its innovation.
 */
/*
bool StateData::update(StateData &src)
{
  if (!StateData::operator*=(src))return false;
  Matrix jcj=src.Covariance;
  for (int k=0;k<src.StateID.Rows; k++)
    for (int h=0;h<StateID.Rows; h++)
      if ((StateID(h,0)>0)&&(src.StateID(k,0)==StateID(h,0))){
	for (int i=0; i<src.Jacobian.Rows; i++)
	  for (int j=0; j<jcj.Rows; j++)
	    jcj(j,i)+=src.Jacobian(i,k)*Jacobian(j,h);
      }
  if (jcj.invert())return false;
  Matrix w;
  w.multTranspose_(Jacobian,jcj,1);
  State.subtractProduct_(w,src.State);
  Covariance.subtractProduct_(w,Jacobian);
  return true;
}
*/
void StateData::interpolate_(StateData& a, StateData& b, Timestamp t)
{
  if (a.Time==t){
    *this=a;
    return;
  }
  if (b.Time==t){
    *this=b;
     return;
  }
  if (b.Time==a.Time){
    *this=b;
     return;
  }

  if (a.getDerivativeType()!=b.getDerivativeType())return;
  if (a.getIncrementalFlag()!=b.getIncrementalFlag())return;
  if (a.State.Rows!=b.State.Rows)return;
  if (a.Covariance.Rows!=b.Covariance.Rows)return;
  unsigned short rule=interpolateRule() ;
  StateData *ap=&a;
  StateData *bp=&b;
  if(a.Time>b.Time)
    {
      ap=&b;
      bp=&a;
    }
  (*this)=(*ap);
  Time=t;
  t-=ap->Time;
  double w2=t.getDouble();
  if (w2<1E-6)return;
  t=bp->Time;
  t-=ap->Time;
  double dt=t.getDouble();
  if (dt<1E-6)return;
  w2/=dt;
  double w=1.0-w2;
  State*=(w);
  Covariance*=w;
  if (rule==0){
    Matrix adata=bp->State;
    adata*=(w2);
    State+=adata;
    adata=bp->Covariance;
    Covariance*=(w);
    adata*=(1-w);
    Covariance+=adata;
    adata=bp->Jacobian;
    Jacobian*=(w);
    adata*=(1-w);
    Jacobian+=adata;
  }else if (rule==1){
    //Case of incremental
  }else if (rule==2){
    //case velocity  
    Matrix adata=bp->State;
    adata*=(w2);
    State+=adata;
    adata=bp->Covariance;
    Covariance*=(w);
    adata*=(1-w);
    Covariance+=adata;
    dt=w;
    for (int i=1; i<getDerivativeType();i++)dt*=w;
    Covariance/=dt;
    adata=bp->Jacobian;
    Jacobian*=(w);
    adata*=(1-w);
    Jacobian=adata;
  }
}
bool StateData::getMatrix(const std::string & str, ShortMatrix & index,
			  Cure::Matrix & mat)
{
  if ((index.Rows<1)||(index.Columns<4))
    return TimestampedData::getMatrix(str,index,mat);
  if ((index(0,0)<0)||(index(0,1)<0))
    return TimestampedData::getMatrix(str,index,mat);
  if ((index(0,2)<0)||(index(0,3)<0))
    return TimestampedData::getMatrix(str,index,mat);
  int n=0;
  ShortMatrix offsets;
  offsets=index;
  bool rowwise=false;
  if (index.Columns>4)
    if (index(0,4)==-1){
      rowwise=true;      
  }
  int r=offsets(0,0)+offsets(0,2);
  int c=offsets(0,1)+offsets(0,3);

  if ((str=="Value")||(str=="State")||(str=="state")){
    n=State.Rows*State.Columns;
    if (rowwise)r=State.Rows;
    if (r==0){
      mat.reallocateZero(offsets(0,2),offsets(0,3));
      return false;
    }
    if ((r*c)>n){
      r=(int)((double)n/(double)c);
      offsets(0,2)=r-offsets(0,0);
      if (offsets(0,2)<0){
	offsets=0;
      }
    }
    if (rowwise){
      mat.reallocateZero(offsets(0,2),index(0,3));
      int k=offsets(0,0);
      for (int i=0;i<offsets(0,2);i++)
	for (int j=0;j<index(0,3); j++,k++)
	  mat(i,j)=State(k,0);
      return true;
    }
    r=State.Rows;
    c=State.Columns;
    State.offset(offsets(0,0),offsets(0,1),offsets(0,2),offsets(0,3));
    mat=State;
    State.offset(-offsets(0,0),-offsets(0,1),r,c);
    return true;
  } else if (str=="StateID"||str=="stateID" || str=="stateid"){
    n=StateID.Rows*StateID.Columns;
    if (rowwise)r=StateID.Rows; 
    if (r*c>n){
      r=n/c;
      offsets(0,2)=r-offsets(0,0);
      if (offsets(0,2)<0){
	offsets=0;
      }
    }
    if (rowwise){
      mat.reallocateZero(offsets(0,2),index(0,3));
      int k=offsets(0,0);
      for (int i=0;i<offsets(0,2);i++)
	for (int j=0;j<index(0,3); j++,k++)
	  mat(i,j)=StateID(k,0);
      return true;
    }
    r=StateID.Rows;
    StateID.offset(offsets(0,0),offsets(0,1),offsets(0,2),offsets(0,3));
    mat.reallocate(StateID.Rows,StateID.Columns);
    for (int i=0;i<StateID.Rows;i++)
      for (int j=0;j<StateID.Columns;j++)
	mat(i,j)=StateID(i,j);
    StateID.offset(-offsets(0,0),-offsets(0,1),r,c);
    return true;
  } else if (str=="Cov"||str=="Covariance" || str=="covariance"){
    n=Covariance.Rows*Covariance.Columns;
    if (r*c>n){
      r=n/c;
      offsets(0,2)=r-offsets(0,0);
      if (offsets(0,2)<0){
	offsets=0;
      }
    }
    r=Covariance.Rows;
    c=Covariance.Columns;
    Covariance.offset(offsets(0,0),offsets(0,1),offsets(0,2),offsets(0,3));
    mat=Covariance;
    Covariance.offset(-offsets(0,0),-offsets(0,1),r,c);
    return true;
  } else if (str=="Jac"||str=="jacobian" || str=="Jacobain"){
    n=Jacobian.Rows*Jacobian.Columns;
    if (r*c>n){
      r=n/c;
      offsets(0,2)=r-offsets(0,0);
      if (offsets(0,2)<0){
	offsets=0;
      }
    }
    r=Jacobian.Rows;
    c=Jacobian.Columns;
    Jacobian.offset(offsets(0,0),offsets(0,1),offsets(0,2),offsets(0,3));
    mat=Jacobian;
    Jacobian.offset(-offsets(0,0),-offsets(0,1),r,c);
    return true;
  } else if (str=="StateType"){
    mat.reallocate(1);
    mat=StateType;
    return true;
  }
  return TimestampedData::getMatrix(str,index,mat);  
}
bool StateData::setMatrix(const std::string & str, ShortMatrix & index,
			  Cure::Matrix & mat)
{
  if ((index.Rows<1)||(index.Columns<4))
    return TimestampedData::setMatrix(str,index,mat);
  if ((index(0,0)<0)||(index(0,1)<0))
    return TimestampedData::setMatrix(str,index,mat);
  if ((index(0,2)<0)||(index(0,3)<0))
    return TimestampedData::setMatrix(str,index,mat);
  ShortMatrix offsets;
  offsets=index;
  bool rowwise=false;
  if (index.Columns>4)
    if (index(0,4)==-1){
      rowwise=true;      
  }
  int r=offsets(0,0)+offsets(0,2);
  int c=offsets(0,1)+offsets(0,3);
  if (r==0)return false;
  if (c==0)return false;
  if ((str=="Value")||(str=="State")||(str=="state")){
    if (rowwise){
      r*=c;
      c=1;
    }
    if (r>State.Rows)State.grow(r,State.Columns);
    if (c>State.Columns)State.grow(State.Rows,c);
    if (rowwise){
      int k=offsets(0,0);
      for (int i=0;i<offsets(0,2);i++)
	for (int j=0;j<index(0,3); j++,k++)
	  State(k,0)=mat(i,j);
      return true;
    }
    r=State.Rows;
    c=State.Columns;
    State.offset(offsets(0,0),offsets(0,1),offsets(0,2),offsets(0,3));
    State=mat;
    State.offset(-offsets(0,0),-offsets(0,1),r,c);
    return true;
  } else if (str=="StateID"||str=="stateID" || str=="stateid"){
    if (rowwise){
      r*=c;
      c=1;
    }
    if (r>StateID.Rows)StateID.grow(r,StateID.Columns);
    if (c>StateID.Columns)StateID.grow(StateID.Rows,c);
    if (rowwise){
      int k=offsets(0,0);
      for (int i=0;i<offsets(0,2);i++)
	for (int j=0;j<index(0,3); j++,k++)
	  StateID(k,0)=(short)mat(i,j);
      return true;
    }
    r=StateID.Rows;
    c=StateID.Columns;
    StateID.offset(offsets(0,0),offsets(0,1),offsets(0,2),offsets(0,3));
    for (int i=0;i<offsets(0,2);i++)
	for (int j=0;j<index(0,3); j++)
	  StateID(i,j)=(short)mat(i,j);
    StateID.offset(-offsets(0,0),-offsets(0,1),r,c);
    return true;
  }else if (str=="Cov"||str=="Covariance" || str=="covariance"){
    if (r>Covariance.Rows)Covariance.grow(r,Covariance.Columns);
    if (c>Covariance.Columns)Covariance.grow(Covariance.Rows,c);
    r=Covariance.Rows;
    c=Covariance.Columns;
    Covariance.offset(offsets(0,0),offsets(0,1),offsets(0,2),offsets(0,3));
    Covariance=mat;
    Covariance.offset(-offsets(0,0),-offsets(0,1),r,c);
    return true;
  } else if (str=="Jac"||str=="jacobian" || str=="Jacobain"){
    if (r>Jacobian.Rows)Jacobian.grow(r,Jacobian.Columns);
    if (c>Jacobian.Columns)Jacobian.grow(Jacobian.Rows,c);
    r=Jacobian.Rows;
    c=Jacobian.Columns;
    Jacobian.offset(offsets(0,0),offsets(0,1),offsets(0,2),offsets(0,3));
    Jacobian=mat;
    Jacobian.offset(-offsets(0,0),-offsets(0,1),r,c);
    return true;
  } else if (str=="StateType"){
    if ((mat.Rows!=1)||(mat.Columns!=1))return false;
    StateType=(unsigned short)mat(0,0);
    return true;
  }
  return TimestampedData::setMatrix(str,index,mat);
}
void StateData::setStateID(short key)
{
  setIDDim(State.Rows);
  for (int i=0;i<StateID.Rows;i++)
    StateID(i,0)=key+i;
}
int StateData::operator=(const std::string &params)
{
  std::istringstream strdes(params);
  std::string cmd, nextcmd;
  if (!(strdes>>cmd))return 1;
  while (1){
    Matrix x;
    int i=0;  
    bool loop=true;
    while (loop){
      std::string tmp; 
      if (strdes >> tmp){
	if (isdigit(tmp[0]) || 
	 tmp[0] == '-' || tmp[0] == '+' ||
	    tmp[0] == '.') {
	x.grow(i+1,1);
	x(i,0)=strtod(tmp.c_str(),(char**)NULL);
	//x(i,0)= atof(tmp.c_str());
	i++;
	}else {
	 nextcmd=tmp;
	loop=false;
	}
      } else {
	loop=false;
	nextcmd=" ";
      }
    }
    if ((cmd=="Value")||(cmd=="State")||(cmd=="state")){
      setDim(x.Rows);
      State=x;
    }else if ((cmd=="Grow")||(cmd=="grow")){
      int r=dim();
      State.grow(r+x.Rows,1);
      State.offset(r,0,x.Rows,1);
      State=x;
      State.offset(-r,0,x.Rows+r,1);
    }else if (cmd=="Cov"||cmd=="Covariance"||cmd=="cov"||cmd=="covariance"){
      Covariance.reallocate(State.Rows);
      for (int j=0; ((j< (*this).Covariance.Rows)&(j<i));
	   j++)
	(*this).Covariance(j,j)=x(j,0);
      for(int j=i; j<(*this).Covariance.Rows;j++)
	(*this).Covariance(j,j)=x(i-1,0);
    }else if (cmd=="Row"||cmd=="row"){
      int r=(*this).Covariance.Rows;
      (*this).Covariance.grow(r+1,State.Rows);      
      for (int j=0; ((j< (*this).Covariance.Columns)&(j<i));
	   j++)
	(*this).Covariance(r,j)=x(j,0);
      }
    x.reallocate(0,0);
    cmd=nextcmd;
    if (cmd==" ")
      return 0;
  }
  return 0;
}
 void StateData::setPose(Pose3D &p,short type)
 {
   p.Time=Time;
   double x[6];
   for (int i=0;i<6;i++)
     if (i<State.Rows)x[i]=State(i,0);
     else x[i]=0;
   p=x;
   if (type<0)type=(short)p.getCovType();
   p.setCovType((unsigned short) type);
   unsigned short t=type;
   int row=Covariance.Rows;
   int r=0;
   for (int i=0; i<row; i++, t=(t>>1))
     {
       if (t&1){
	 unsigned short t2=type;
	 int c=0;
	 for (int j=0;j<row; j++,t2=(t2>>1))
	   if (t2&1){
	     p(r,c)=Covariance(i,j);
	     c++;
	   }
	 r++;
       }
     }
 }
void StateData::print()
{
  TimestampedData::print();
  std::cerr<<" Transposed State= ";
  Matrix s(State);
  s.Rows=State.Columns;
  s.Columns=State.Rows;
  s.RowInc=s.Columns;
  s.print();
}

} // namespace Cure

std::ostream& 
operator << (std::ostream& os, const Cure::StateData &cmd)
{
  os << "Flags=" << cmd.m_Flags<< std::endl;
  return os;
}
