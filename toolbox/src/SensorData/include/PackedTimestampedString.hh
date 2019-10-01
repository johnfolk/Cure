// = AUTHOR(S)
//    John Folkesson 
//    
//    Copyright (c) 2007 John Folkesson
//    

#ifndef CURE_PACKEDTIMESTAMPEDSTRING_HH
#define CURE_PACKEDTIMESTAMPEDSTRING_HH

#include "TimestampedString.hh"
#include "PackedData.hh"
namespace Cure{

/**
 * Packed version of TimestampedString
 * 
 * The SubType is the size of the ShortMatrix ShortData
 * 
 * The orginal layout of the ShortData was laid out as:
 * ShortData.Rows, ShortData.Columns, Data.Rows, Data.Columns,
 * GenericType
 * Then comes the ShortData across the first row then the second row
 * and so on for the whole Matrix
 *
 * @author John Folkesson,
 */  
class PackedTimestampedString: public PackedData 
{
  friend class TimestampedString;
public:
  /**
   * Version number that is used when packing unpacking data to make
   * sure that the same version of the code is used to pack/unpack the
   * data.
   */
  static int VERSION;

public:
  /**The Constructor*/
  PackedTimestampedString();
  /**
   * The copy constructor.
   * @param p the set to copy
   */
  PackedTimestampedString(PackedTimestampedString& p);
  /**
   * The pack and copy constructor.
   * @param p the set to copy and pack
   */
  PackedTimestampedString(TimestampedString& p);
  /**callsed by the constuctors*/ 
  virtual void init();
  virtual ~PackedTimestampedString();
  /**fast cast*/
  PackedTimestampedString * narrowPackedTimestampedString(){return this;}
  //*******************SET FUNCTIONS***********************************
  /**
   * Copy operator 
   * If p is a Generic or PackedTimestampedString then this will pack it 
   * up.
   * @param p the object to copy.
   */
  virtual void operator=(TimestampedData& p);
  /**
   * Copy operator 
   * p is a Generic that this will pack  
   * up.
   * @param p the object to copy.
   */
  void operator=(const TimestampedString& p);
  /**
   * If p is a Generic then this will set p to be equal to the object
   * stored in this PackedTimestampedString.
   * @param p the object to unpack into.
   */
  virtual void unpack(TimestampedData& p){
    TimestampedString *s=p.narrowTimestampedString();
    if (s) unpack(*s);
  }
  void setSubType(unsigned short t);  
  int setShortDataSize(unsigned long len);
  /**
   * This will set s to be equal to the object
   * stored in this PackedTimestampedString.
   * @param s the object to unpack into.
   */
  void unpack(TimestampedString& s);
  void get(std::string &str){
    
    str=(char*)&ShortData[1];
  }

  unsigned short getStringType(){return (unsigned short)ShortData[0];}
protected:
  virtual  TimestampedData * makeUnpack(){
    TimestampedString *p=new TimestampedString();
    unpack(*p);
    return p;
  }
};

} // namespace Cure

#endif // CURE_PACKEDMEASUREMENTSET_HH
