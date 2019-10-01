// = AUTHOR(S)
//    John Folkesson
//
// = COPYRIGHT
//    Copyright (c) 2005 John Folkesson
//
/*----------------------------------------------------------------------*/

#ifndef CURE_GENERICDATA_hh
#define CURE_GENERICDATA_hh

#include "TimestampedData.hh"
#include "PoseTree.hh"
#include "Matrix.hh" 
#include "ShortMatrix.hh" 

#ifndef DEPEND
#include <iostream>  // ostream
#endif

// Forward declaration(s)
class PackedGeneric;
namespace Cure {


/**
 * This class is used to send anything that
 * does not fit some predifined TimestampedData Class
 *
 * The ShortData should always be allocated with forceShortDataSize
 * The SubType for this is 
 * the size of ShortData, (Rows*Columns) 
 *
 * @author John Folkesson 
 */
class GenericData :  public TimestampedData 
{  
public:

public:
  
  friend class PackedGeneric;
  /**
   * Create empty GenericData
   * @param id the id of this
   */
  GenericData(unsigned short id = 0);
  /**
   * Create empty GenericData
   * @param t the time
   * @param id the id of this
   */
  GenericData(Timestamp &t,unsigned short id = 0);

  /**
   * Create a copy of a GenericData
   * @param s The object to copy
   */
  GenericData(const GenericData &s);
  /**called by consturctors*/
  void init();
  /**
   * Destructor
   */
  virtual ~GenericData();
  /*
   * This is how we (efficiently) get a GenericData from a TimestampedData.
   */
  GenericData * narrowGenericData(){return this;}
  /**
   * Copy operator
   * @param src The object to copy
   */
  GenericData& operator=(const GenericData &src);  
  /**
   * Store a tree in this.
   * @param tree The object to copy
   */
  void operator=(PoseTree &tree);  
  /**
   * @param tree the PoseTree stored in this is returned here
   * @return 1 if fails esle 0
   */
  int getTree(PoseTree &tree);
  
  /**
   * Copy operator
   * @param p The object to copy
   */
  void operator = (TimestampedData& p)
  {
    GenericData *ss=p.narrowGenericData();
    if (ss)operator=(*ss);
    else TimestampedData::operator=(p);
  }
  /**
   * Returns the elements of the Data Matrix
   */
  double& operator()(const unsigned short r, const unsigned short c) {  
    return  Data(r,c);
  }
  /**
   * Returns the elements of the Data Matrix.
   */
  double operator()(const unsigned short r, const unsigned short c) const {
    return( (*(GenericData *)this).operator()(r,c) );
  }

  void setGenericType(unsigned short r) {GenericType=r; }
  /** 
   * Sets the subtype, if t is not equal to the size of ShortData then
   * ShortData is set to a row vector of that length.
   * @param t the new subtype/length of ShortData
   */ 
 void setSubType(unsigned short t);
  /** 
   * This sets up a new ShortData of the given size, with all zeros
   * and sets the SubType.
   */
 void setShortDataSize(int rows, int columns);
  /** 
   * This is what should always be used to change the shortData size
   * This will grow the ShortData to the given size, with existing
   *  data intact as far as it overlaps the new size and sets the SubType.
   * 
   */
  void forceShortDataSize(int rows, int columns);
  /** Display info on the Object*/
  void print(); 
public:
  /**
   * This is the Generic Data.
   */
  Cure::Matrix Data;
  /**
   * This can be used to help interprete the data. Only the msb is checked
   * in this class.  So if GenericType&0x8000 is true then the Data in
   * Data is interpolated.  Otherwise the closest timestamp is used as the 
   * interpolated value.    
   *.
   */
  unsigned short GenericType;
  /**
   * This can hold whatever number of intergers is needed.
   */
  Cure::ShortMatrix ShortData;
  /**
   * This returns data based on the string value and the indcies
   * given in the ShortMatrix.  
   *
   * @param str So Data and ShortData are recongnized strings.
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
   * This sets data based on the string value and the indecies given
   * in the ShortMatrix.  The Data and ShortData matrixes are grown to
   * fit the size indicated by index if they are too small initially.
   *
   * @param str So Data, ShortData,ShortDataRows,ShortDataColumns, and
   * GenericType are recongnized strings, plus the base class strings.
   * 
   * @param index Set the index as when you would call
   * Cure::Matrix.offset(roff,coff,r,c) with the integers
   * roff,coff,r,c placed in a the top row of index (1x4).
   * 
   * @param mat the result is returned here in a r x c matrix
   * @return true if ok else false. 
  */
  virtual bool setMatrix(const std::string & str, ShortMatrix & index,
			 Matrix &mat);


      
  /**
   * Interpolate between two GenericDatas to become a GenericData at
   * t.  The GenericType msb being set causes this to interpolate The
   * Data between a and b.  The ShortData is taken as the most recent.
   *
   * If msb of GenericType is not set then this returns the closest of
   * a and b to t.
   *
   * @param a data at one timestamp NOT this.
   * @param b data at another timestamp, NOT this.
   * @param t the time to interpolate this to between a and b.
   * @see Pose3D::interpolate_ 
  */
  virtual void interpolate_(TimestampedData& a, TimestampedData& b, 
			    Timestamp t){
    GenericData *pa=a.narrowGenericData();
    GenericData *pb=b.narrowGenericData();
    if ((pa!=0)&(pb!=0))return GenericData::interpolate_(*pa,*pb,t);
    TimestampedData::interpolate_(a,b,t);
  } 
  /**
   * @param a data at one timestamp NOT this.
   * @param b data at another timestamp, NOT this.
   * @param t the time to interpolate this to between a and b.
   * 
  */
  void interpolate_(GenericData& a, GenericData& b, Timestamp t);
  /**
   * @param a data at one timestamp even this.
   * @param b data at another timestamp, even this.
   * @param t the time to interpolate this to between a and b.
   * @see GenericData::interpolate_ 
  */
  void interpolate(GenericData a, GenericData b, Timestamp t){
    interpolate_(a,b, t);}
  void setInterpolate(){GenericType=(GenericType|0x8000);}
  
protected:
  
private:
};
}

std::ostream& operator << (std::ostream& os, const Cure::GenericData &cmd);

#endif // GenericData_hh
