//    $Id: PackedSICK.hh,v 1.5 2007/09/14 09:14:03 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson and Patric Jensfelt
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_PACKEDSICK_HH
#define CURE_PACKEDSICK_HH

#include "SICKScan.hh"
#include "PackedData.hh"
namespace Cure{

/**
 * A PackedSICK is a compressed SICKScan.  It uses the type to store only the 
 * information needed to unpack to a SICKScan.  It does not have the 
 * full functionality of a SICKScan, so if you need that you must unpack it.
 *
 * SubType:
 * The 10 LSB are the m_NPts as an int;
 * bit 10 is 1 if m_NFlags>0;
 * bit 11 is 1 if m_NIntensityLevels>0;
 *
 *
 * @author John Folkesson and Patric Jensfelt
 */  
class PackedSICK: public PackedData 
{
  friend class SICKScan;
public:

  /**
   * Version number that is used when packing unpacking data to make
   * sure that the same version of the code is used to pack/unpack the
   * data.
   *
   * Version 4 added the SensorID as ShortData[3]
   * run ConverSICK2ver4 to conver version3 to version 4. 
   */
  static int VERSION;


private:

public:
  /**The Constructor*/
  PackedSICK();
  /**
   * The copy constructor.
   * @param p the object to copy.
   */ 
  PackedSICK(PackedSICK& p);
  /**
   * The copy/pack constructor.
   * @param p the object to copy/pack.
   */ 
  PackedSICK(SICKScan& p); 
  /**Called by constructors*/
  virtual void init();
  virtual ~PackedSICK();
  /**fast cast*/
  PackedSICK * narrowPackedSICK(){return this;}
  //*******************SET FUNCTIONS***********************************

  /**
   * The copy/pack operator.
   * @param p the object to copy/pack.
   */ 
  virtual void operator=(TimestampedData& p);
  /**
   * The copy/pack operator.
   * @param p the object to copy/pack.
   */ 
  void operator=(const SICKScan& p);

  /**
   * The objects data will be copied to the object p if it is a SICKScan.
   * @param p the object to unpack into.
   */ 
  virtual void unpack(TimestampedData& p);

  /**
   * This function will alter the size of the arrays Data and
   * ShortData but will not copy the values from the old to the new
   * memory location, i.e. do not call this function unless you no
   * longer need the data
   * @param t the new SubType
   */
  void setSubType(unsigned short t);
  
  /**
   * The objects data will be copied to the object scan.
   * @param scan the object to unpack into.
   */ 
  void unpack(SICKScan& scan);

  /**
   * This will assign the relavent parts of a DataDescriptor to
   * this object.
   * @param d the descriptor to assign to this objects.
   * @return 0 if ok 1 if wrong class. 
  */
  virtual  int setDescriptor(const Cure::DataDescriptor & d){
    if(getClassType()!=d.ClassType)return 1;
    if (SensorData::SENSORTYPE_SICK==d.SensorType)return 1;
    setID(d.ID);
    ShortData[0]=d.SensorID;
    return 0;
   }
  /**Gets the Descriptor*/
  virtual  void getDescriptor(Cure::DataDescriptor & d) const {
    d.ClassType=getClassType();
    d.ID=getID();
    d.SensorType=SensorData::SENSORTYPE_SICK;
    d.SensorID=ShortData[3];
    d.Name="";
  }

protected:
  unsigned long getShortSizeFromSubType(unsigned short type) const;
  unsigned long getDataSizeFromSubType(unsigned short type) const;
  virtual  TimestampedData * makeUnpack(){
    SICKScan *p=new SICKScan();
    unpack(*p);
    return p;
  }
};

} // namespace Cure

#endif // CURE_PACKEDPOSE_HH
