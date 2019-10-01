//
// = AUTHOR(S)
//    John Folkesson 
//    
//    Copyright (c) 2005 John Folkesson
//    

#ifndef CURE_PACKEDSONAR_HH
#define CURE_PACKEDSONAR_HH

#include "SonarData.hh"
#include "PackedData.hh"

namespace Cure{
/**
 * A PackedSonar is a compressed SonarData.  It uses the type to
 * store only the information needed to unpack to a SonarData.  It
 * does not have the full functionality of a SonarData, so if you
 * need that you must unpack it.
 *
 *
 * @author John Folkesson
 */  
class PackedSonar: public PackedData 
{
  friend class SonarData;
public:

  /**
   * Version number that is used when packing unpacking data to make
   * sure that the same version of the code is used to pack/unpack the
   * data.
   */
  static int VERSION;

private:

public:
  /**The Constructor*/
  PackedSonar();
  /**
   * The copy constructor.
   * @param p the object to copy.
   */ 
  PackedSonar(PackedSonar& p);
  /**
   * The copy/pack constructor.
   * @param p the object to copy/pack.
   */ 
  PackedSonar(SonarData& p); 
  /**Called by constructors*/
  virtual void init();
  virtual ~PackedSonar();
  /**fast cast*/
  PackedSonar * narrowPackedSonar(){return this;}
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
  void operator=(const SonarData& p);

  /**
   * The objects data will be copied to the object p if it is a SonarData.
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
   * The objects data will be copied to the object s.
   * @param s the object to unpack into.
   */ 
  void unpack(SonarData& s);
  /**
   * This will assign the relavent parts of a DataDescriptor to
   * this object.
   * @param d the descriptor to assign to this objects.
   * @return 0 if ok 1 if wrong class. 
  */
  virtual  int setDescriptor(const Cure::DataDescriptor & d){
    if(getClassType()!=d.ClassType)return 1;
    if (SensorData::SENSORTYPE_SONAR==d.SensorType)return 1;
    setID(d.ID);
    ShortData[0]=d.SensorID;
    return 0;
   }
  /**Gets the Descriptor*/
  virtual  void getDescriptor(Cure::DataDescriptor & d) const {
    d.ClassType=getClassType();
    d.ID=getID();
    d.SensorType=SensorData::SENSORTYPE_SONAR;
    d.SensorID=ShortData[0];
    d.Name="";
  }

protected:

unsigned long getShortSizeFromSubType(unsigned short) const
  {
    return 1;
  }

  unsigned long getDataSizeFromSubType(unsigned short)const
  {
    return (4);
  }

  virtual  TimestampedData * makeUnpack(){
    SonarData *p=new SonarData();
    unpack(*p);
    return p;
  }
};

} // namespace Cure

#endif 
