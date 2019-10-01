// = AUTHOR(S)
//    John Folkesson 
//    
//    Copyright (c) 2005 John Folkesson
//    

#ifndef CURE_PACKEDGENERIC_HH
#define CURE_PACKEDGENERIC_HH

#include "GenericData.hh"
#include "PackedData.hh"
namespace Cure{

/**
 * Packed version of GenericData
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
class PackedGeneric: public PackedData 
{
  friend class GenericData;
public:
  /**
   * Version number that is used when packing unpacking data to make
   * sure that the same version of the code is used to pack/unpack the
   * data.
   */
  static int VERSION;

public:
  /**The Constructor*/
  PackedGeneric();
  /**
   * The copy constructor.
   * @param p the set to copy
   */
  PackedGeneric(PackedGeneric& p);
  /**
   * The pack and copy constructor.
   * @param p the set to copy and pack
   */
  PackedGeneric(GenericData& p);
  /**callsed by the constuctors*/ 
  virtual void init();
  virtual ~PackedGeneric();
  /**fast cast*/
  PackedGeneric * narrowPackedGeneric(){return this;}
  //*******************SET FUNCTIONS***********************************
  /**
   * Copy operator 
   * If p is a Generic or PackedGeneric then this will pack it 
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
  void operator=(const GenericData& p);
  /**
   * If p is a Generic then this will set p to be equal to the object
   * stored in this PackedGeneric.
   * @param p the object to unpack into.
   */
  virtual void unpack(TimestampedData& p){
    GenericData *s=p.narrowGenericData();
    if (s) unpack(*s);
  }
  
  virtual int setShortDataSize(unsigned long len)
  {
    if (len<5)len=5;;
    m_SubType = (len-5);
    if (ShortDataSize==len)return 0;
    ShortDataSize = len;  
    if (ShortData) delete [] ShortData;
    if (ShortDataSize){
      ShortData = new unsigned short[len];
      memset(ShortData, 0, ShortDataSize*sizeof(unsigned short));
    } else ShortData=0; 
    return 0;
  }
/**
   * This function will alter the size of the array ShortData but will
   * not copy the values from the old to the new memory location,
   * i.e. do not call this function unless you no longer need the data
   * @param t the new number of elements. 
   */
    void setSubType(unsigned short t){
      if (m_SubType == 0 || t != m_SubType) 
	setShortDataSize(t+5);
    }
  

  /**
   * This will set s to be equal to the object
   * stored in this PackedGeneric.
   * @param s the object to unpack into.
   */
  void unpack(GenericData& s);
 
void print()
{
  Cure::PackedData::print();
  std::cerr<<"PackedGeneric Data: ";
  for (int i=0;i<(int)DataSize;i++)std::cerr<<Data[i]<<" ";
  std::cerr<<"\n";
  for (int i=0;i<(int)ShortDataSize;i++)std::cerr<<ShortData[i]<<" ";
  std::cerr<<"\n";
}
protected:
  virtual  TimestampedData * makeUnpack(){
    GenericData *p=new GenericData();
    unpack(*p);
    return p;
  }
};

} // namespace Cure

#endif // CURE_PACKEDMEASUREMENTSET_HH
