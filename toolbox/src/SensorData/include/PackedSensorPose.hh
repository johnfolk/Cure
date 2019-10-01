// = LIBRARY
//
// = FILENAME
//    PackedSensorPose.hh
//
// = AUTHOR(S)
//    Patric Jensfelt
//
// = COPYRIGHT
//    Copyright (c) 2005 Patric Jensfelt
//
/*----------------------------------------------------------------------*/

#ifndef CURE_PACKEDSENSORPOSE_HH
#define CURE_PACKEDSENSORPOSE_HH

#include "PackedPose.hh"
#include "SensorPose.hh"

namespace Cure{

/**
 * A PackedSensorPose is a compressed SensorPose. It uses the type to
 * store only the information needed to unpack to a SensorPose.  It
 * does not have the full functionality of a SensorPose, so if you
 * need that you must unpack it.
 *
 *
 *
 *
 * @author Patric Jensfelt
 */  
class PackedSensorPose: public PackedPose 
{
  friend class SensorPose;
public:

  /**
   * Version number that is used when packing unpacking data to make
   * sure that the same version of the code is used to pack/unpack the
   * data.
   */
  static int VERSION;
  
  private:

public:
  PackedSensorPose();
  PackedSensorPose(PackedSensorPose& p);
  PackedSensorPose(SensorPose& p); 

  virtual ~PackedSensorPose();

  PackedSensorPose * narrowPackedSensorPose(){ return this; }

  //*******************SET FUNCTIONS***********************************

  virtual void operator=(TimestampedData& p);
  void operator=(const SensorPose& sp);

  virtual void unpack(TimestampedData& p);

  /**
   * This function will alter the size of the arrays Data and
   * ShortData but will not copy the values from the old to the new
   * memory location, i.e. do not call this function unless you no
   * longer need the data
   */
  virtual void setSubType(unsigned short t);
  
  void unpack(SensorPose& scan);

  /**
   * This will assign the relavent parts of a DataDescriptor to
   * this object.
   * @param d the descriptor to assign to this objects.
   * @return 0 if ok 1 if wrong class. 
  */
  virtual  int setDescriptor(const Cure::DataDescriptor & d){
    if(getClassType()!=d.ClassType)return 1;
    ShortData[0]=d.SensorType;
    setID(d.ID);
    ShortData[0]=d.SensorID;
    return 0;
   }
  /**Gets the Descriptor*/
  virtual  void getDescriptor(Cure::DataDescriptor & d) const {
    d.ClassType=getClassType();
    d.ID=getID();
    d.SensorType=ShortData[0];;
    d.SensorID=ShortData[1];;
    d.Name="";
  }
  virtual void print();

protected:
  virtual  TimestampedData * makeUnpack(){
    SensorPose *p=new SensorPose();
    unpack(*p);
    return p;
  }
  virtual void init();
};

} // namespace Cure

#endif // CURE_PACKEDSENSORPOSE_HH
