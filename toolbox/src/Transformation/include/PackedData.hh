//    $Id: PackedData.hh,v 1.17 2008/05/05 16:55:30 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_PACKEDDATA_HH
#define CURE_PACKEDDATA_HH

#include "TimestampedData.hh"

namespace Cure{

/**
 * A PackedData is a compressed TimestampedData.  It uses the type to store 
 * only the information needed to unpack to a TimestampedData.  It does not 
 * have the full functionality of a unpacked type, so if you need that 
 * you must unpack it.
 *
 * The idea is to pack everying into two arrays, ShortData and Data.
 * These two arrays then become more or less piles of data that can be
 * handled without any deep understanding of what they represent.
 *
 * setShortDataSize() and setDataSize() will allocate the arrays
 * setSubType() will set the subType and for some classes set the Data size.  
 *
 * @author John Folkesson
 */  
class PackedData : public TimestampedData
{
public:

  /**
   * Array of short integers that is used for parameters such as keys,
   * types, etc
   */
  unsigned short* ShortData;

  /**
   * Array of doubles containing the data that requires double
   * precision such as matrices, measurement values, etc
   */
  double* Data;

  /**
   * Optional version number that can be used reading from file to
   * make sure that the reader of the data has the same version as the
   * writer.
   */
  int Version;
private:
  
  public:
  /**The constructor.*/
  PackedData();
  /**
   * The copy constructor.
   * @param p the data to copy into this. 
   */
  PackedData(PackedData& p);
  /**
   * The copy and pack constructor.
   * @param p the data to copy and pack into this. 
   */
  PackedData(TimestampedData& p);
  /**called by constructors to initialize members.*/
  virtual void init();
  virtual ~PackedData();
  /**cast to a PackedData object*/
  PackedData * narrowPackedData(){return this;}
  //*******************SET FUNCTIONS***********************************

  /**
   * The copy operator.
   * @param p the data to copy into this. 
   */
  virtual void operator=(const PackedData& p);
  /**
   * The copy and pack operator.
   * @param p the data to copy and pack into this. 
   */
  virtual void operator=(TimestampedData& p){
    m_ClassType=p.getClassType();    
    TimestampedData::operator=(p);
  }
  /**
   * Sets p equal as possible to this,
   * 
   * 
   * 
   * @param  p the object to copy the internal object into.
   */
  virtual void setEqual(TimestampedData &p){
    if (p.isPacked())
      p=(*this);
    else unpack(p);
  }
    virtual bool getMatrix(const std::string & str, ShortMatrix & index,
			 Matrix &mat);

  virtual bool setMatrix(const std::string & str, ShortMatrix & index,
			 Matrix &mat);

  /**
   * Test equality.
   * @param p the object to compare to this.
   * @return 1 if p is identical to this. 
   */ 
  virtual int operator==(const PackedData& p)const;
  /**Print information about this to the display.*/
  virtual void print();
  /**
   * Version number that can be used reading from file to
   * make sure that the reader of the data has the same version as the
   * writer.
   */
  virtual int version(){return Version;}
/**
   * This function will change the size of the  array ShortData (if
   * needed). Effects ShortData and ShortDataSize.
   *
   *
   *
   *
   * 
   *
   *
   *
   * @param len the size of the new array.
   * @return 1 if failed 0 else, 
  */
  virtual int setShortDataSize(unsigned long len)
  {
    if (ShortDataSize==len)return 0;
    if (ShortData) delete []ShortData;
    ShortDataSize=len;
    if (ShortDataSize){
      ShortData = new unsigned short[len];
      memset(Data, 0, DataSize*sizeof(unsigned short));
    } else ShortData=0;
    return 1;
  }

  /**
   * This function will change the size of the  array ShortData (if
   * needed). Effects ShortData and ShortDataSize.
   * It iwll convert older version ShortData that was loaded
   * directly into the ShortDataArray 
   * into the current version for the subclass 
   *
   *
   * @param vers the version currently stored in ShortDatao.
   * @return 1 if failed 0 else, 
  */
  virtual int convertShortData(int vers)
  {
    if (vers<=version())return 0;
    return 1;
  }

  /**
   * This function will change the size of the double array Data (if
   * needed). Effects Data and DataLen.
   *
   * NOTE: This function relies on DataLen really being the number of
   * doubles allocated for
   *
   * NOTE: Whatever is in the array Data will be lost after a call to
   * this function if the length changes
   *
   * @param len the size of the new array.
   * @return number of double values in Data
   */
  unsigned long setDataSize(unsigned long len);

  /**
   * This function will change the size of the  array Data (if
   * needed). Effects ShortData and DataSize.
   * It iwll convert older version ShortData that was loaded
   * directly into the ShortDataArray 
   * into the current version for the subclass 
   *
   *
   * @param vers the version currently stored in ShortDatao.
   * @return 1 if failed 0 else, 
  */
  virtual int convertData(int vers)
  {
    if (vers<=version())return 0;
    return 1;
  }
  
  /**    
   * @return number of doubles in Data array
   */
  unsigned long getDataSize() const { return DataSize; }

  /**
   *
   * @return size of the ShortData Array.   
   */
  unsigned long getShortDataSize()const{return ShortDataSize;}


protected:
  /**
   * Number of unsigned shorts allocated for in ShortData.
   */
  unsigned long ShortDataSize;
  virtual  TimestampedData * makeUnpack(){
    TimestampedData *td=new TimestampedData();
    unpack(*td);
    return td;
  }

  /**
   * Number of doubles allocated for in Data.
   */
  unsigned long DataSize;
};

} // namespace Cure

#endif // CURE_PACKEDPOSE_HH
