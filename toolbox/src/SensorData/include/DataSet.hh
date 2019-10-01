//    $Id: DataSet.hh,v 1.3 2007/10/10 10:18:20 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2005 John Folkesson
//    

#ifndef CURE_DATASET_HH
#define CURE_DATASET_HH

#include "SmartData.hh"
#include "TimestampedData.hh"


namespace Cure{
  /**
   * The is able to make and hold A set of TimestampedData objects of any type.
   * One can set this equal to any such object and it will correctly
   * copy all data to the internal objects.  It can then set any other 
   * object equal to the internal object to the extent the Classes match.
   *
   *
   * @author John Folkesson
   */
  class DataSet:public TimestampedData 
{
public:
  unsigned short SetSize;
protected:
  /**
   * Pointer to an object that this object wraps.
   */
  SmartData * Data;

public:
  /**Constructor of an empty Object*/
  DataSet() 
  {
    init();  
  }
  DataSet(const DataSet & d):TimestampedData() 
  {
    init();  
    (*this)=d;
  }
  /**Called by Constructors*/
  void init()
  {
    m_Packed = false;
    m_ClassType=DATASET_TYPE;
    m_SubType = 0;
    ID = 0;
    Data=0;
    SetSize=0;
  }
  /**Will Delete the held object.*/
  ~DataSet(){
    if (Data) delete [] Data;
    Data=0;
  }
  /**fast cast*/
  DataSet * narrowDataSet(){return this;}
  /**
   * Query the subtype.
   * @return the subType of the internal object.
   */
  unsigned short getDataSubType(int i){return Data[i].getTPointSubType();}
  /**
   * Query the classtype.
   * @return the ClassType of the internal object.
   */
  unsigned short getDataClassType(int i){return Data[i].getTPointClassType();}
  
  /**
   * This sets this up to hold len SmartData Objects
   * @param the length to allocate to Data (the SmartData Array)
   */
  void setSetSize(unsigned short len);
  /*
   * Changes Internal objects into a new object of the same type and value 
   * as t is holding.
   * @param t the DataSet holding the objects to copy.
   */
  void operator=(const DataSet & t)
  {
    setSetSize(t.SetSize);
    Time=t.Time;
    setSubType(t.getSubType());
    setID(t.getID());
    for (int i=0; i<SetSize; i++)
      (*this)(i)=t.Data[i];
  }  
  void add (DataSet  * a,const DataSet * b)
  {
    int m=0;
    if (b){
      m=b->SetSize;
      Time=b->Time;
    }
    int n=0;
    if (a){
      n=a->SetSize;
      Time=a->Time;
    } 
    setSetSize(n+m);
    int j=0;
    for (int i=0; i<n; j++,i++)
      (*this)(j)=a->Data[i];
    for (int i=0; i<m; j++,i++)
      (*this)(j)=b->Data[i];
  }  
  /**
   * Gets the reference to the ith SmartData object
   * @param i the index
   * @return the ref of object
   */
  SmartData & operator()(const int i) {  
    return( Data[i] );
  }


  /**copy the object*/
  void operator=(TimestampedData & t)
  {
    DataSet *s=t.narrowDataSet();
    if (s) {
      (*this)=(*s);
    }
    Time=t.Time;
  }  
  /**
   * Sets p equal to the internal object this points to,
   * packing/unpacking as needed.  So if p is not packed and the internal
   * object is the right type but packed then it is unpacked into p.
   * @param i the index of the internal object.
   * @param  p the object to copy the internal object into.
   */
  bool setEqual(unsigned int i,TimestampedData &p)
  {
    if (i<SetSize){
      Data[i].setEqual(p);
      return true;
    } 
    std::cerr<<"DataSet::SetEqual(i,p) i out of range\n";
    return false;
  }
  

  TimestampedData *getTPointer(unsigned int i){
    if (i<SetSize) return Data[i].getTPointer();
    return 0;
  }
   /*
   * Changes TPointers into new packed objects of the cooresponding to
   * the type and value it has.
   * @return 0 if ok else 1.
   */
  unsigned short pack(){
   for (int i=0; i<SetSize; i++)
     if ((*this)(i).pack())return 1;
   return 0;
  }

  
   /*
   * Changes TPointers into a new unpacked objects of the cooresponding to
   * the type and value it has.
   */
  void unpack(){
   for (int i=0; i<SetSize; i++)
     (*this)(i).unpack();
  }

  /**
   * Set td to an unpacked copy of TPointer;
   * @param i the index of the internal object.
   * @param td the object to unpack into.
   * @return 0 if ok else 1.
   */
  unsigned short unpack(unsigned short i,TimestampedData & td){
    if (i<SetSize){
      (((*this)(i)).unpack(td));
      return 0;
    }
    return 1;
  }
  
  /**
   * This sets the internal object as specified.
   * @param i the index of the internal object.
   * @param classType the ClassType of the object you want 
   *               (ie POSE3D_TYPE ect).
   * @param subType the subtype to give the new object.
   * @param pckd wether to make a packed version of the object or normal. 
   *   
   * @return 0 if ok else 1.
   * @see Cure::makeTimestampedData
   */
  unsigned short setup(unsigned short i, unsigned char classType, 
                       unsigned short subType = 0,
		       bool pckd = false){
    if (i<SetSize)
      return (*this)(i).setup(classType,subType,pckd);
    return 1;
  }
  virtual void setTime(const Timestamp &t){
    Time=t;
    for (int i=0;i<SetSize;i++)
      (*this)(i).setTime(t);
  }

  /**
   * This returns data based on the string value and the indcies
   * given in the ShortMatrix.  
   *
   * @param str So Whatever the elements of the set understandd
   * are recongnized strings.
   * 
   * @param index Set the index as when you would call
   * if SetSize>index(0,0)>-1 then getMatrix is called on the
   * index(0,0)element with the index matrix offset one col.
   * if (index(0,0)==-1 the entire data set is
   * called with index offset by one col.  Each call
   * offsets mat by one row after discovering the size 
   * of the matrix by assumeing all get matrix calls
   * will return 1-row matrix of the same number of col 
   * 
   *
   * 
   * @param mat the result is returned here in a r x c matrix
   * @return true if ok else false. 
   */
  virtual bool getMatrix(const std::string & str, ShortMatrix & index,
			 Matrix &mat);
  /**
   * This sets data values based on the string value and the indcies
   * given in the ShortMatrix.  
   *
   * @param str So Whatever the elements of the set understandd
   * are recongnized strings.
   * 
   * @param index Set the index as when you would call
   * if SetSize>index(0,0)>-1 then getMatrix is called on the
   * index(0,0)element with the index matrix offset one col.
   * if (index(0,0)==-1 the entire data set is
   * called with index offset by one col.  Each call
   * offsets mat by one row after discovering the size 
   * of the matrix by assumeing all get matrix calls
   * will return 1-row matrix of the same number of col 
   * 
   *
   * 
   * @param mat the value to set is given here in a r x c matrix
   * @return true if ok else false. 
   */
  virtual bool setMatrix(const std::string & str, ShortMatrix & index,
			 Matrix &mat);

  /**
   * This interpolate works even for PackedData.  It will
   * 'change' into a'a classType if needed.  
   * They must be DataSets of the same form as this.
   * @param a one object
   * @param b another object
   * @param t the interpolation time
   */
  void interpolate_(TimestampedData& a, TimestampedData& b, 
		    Timestamp t);
  /**
   * This interpolate works even for PackedData.  It will
   * 'change' into a'a classType if needed. The Data
   * Sets must be the same size and the right types. 
   *
   *
   * @param a one object
   * @param b another object
   * @t the interpolation time
   */
  void interpolate_(DataSet& a, DataSet& b, 
		    Timestamp t);
  /**Display info*/
  void print(){

    TimestampedData::print();
    std::cerr<<"DataSet SetSize:"<<SetSize<<" ";
    for (unsigned int i=0; i<SetSize; i++)getTPointer(i)->print();
  }
  void setAllTimes(const Timestamp &t){
    Time=t;
    for (int i=0; i<SetSize; i++)(*this)(i).setTime(t);
  }
};

} // namespace Cure

#endif 

