// = AUTHOR(S)
//    John Folkesson 
//    
//    Copyright (c) 2005 John Folkesson
//    

#ifndef CURE_PACKEDVISION_HH
#define CURE_PACKEDVISION_HH

#include "VisionData.hh"
#include "PackedData.hh"
namespace Cure{

/**
 * I don't know what this will look like yet.
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 *
 * @author John Folkesson,
 */  
class PackedVision: public PackedData 
{
  friend class VisionData;
public:
  /**
   * Version number that is used when packing unpacking data to make
   * sure that the same version of the code is used to pack/unpack the
   * data.
   */
  static int VERSION;

public:
  /**The Constructor*/
  PackedVision();
  /**
   * The copy constructor.
   * @param p the set to copy
   */
  PackedVision(PackedVision& p);
  /**
   * The pack and copy constructor.
   * @param p the set to copy and pack
   */
  PackedVision(VisionData& p);
  /**callsed by the constuctors*/ 
  virtual void init();
  virtual ~PackedVision();
  /**fast cast*/
  PackedVision * narrowPackedVision(){return this;}
  //*******************SET FUNCTIONS***********************************
  /**
   * Copy operator 
   * If p is a Vision or PackedVision then this will pack it 
   * up.
   * @param p the object to copy.
   */
  virtual void operator=(TimestampedData& p);
  /**
   * Copy operator 
   * p is a Vision that this will pack  
   * up.
   * @param p the object to copy.
   */
  void operator=(const VisionData& p);
  /**
   * If p is a Vision then this will set p to be equal to the object
   * stored in this PackedVision.
   * @param p the object to unpack into.
   */
  virtual void unpack(TimestampedData& p){
    VisionData *s=p.narrowVisionData();
    if (s) unpack(*s);
  }
  /**
   * This does not change the size of the array.
   * @param t the new number of elements. 
   */
  void setSubType(unsigned short t);
  /**
   * This function will change the size of the  array ShortData (if
   * needed). Effects ShortData and ShortDataSize.
   *
   * NOTE: Whatever is in the array ShortData will be copied
   * if it fits in the new array.
   *
   * This will reallocate
   * the short array.
   *
   * @param len the size of the new array.
   * @return 0.
  */
  int setShortDataSize(unsigned long len);
  
  /**
   * This will set s to be equal to the object
   * stored in this PackedVision.
   * @param s the object to unpack into.
   */
  void unpack(VisionData& s);
 /**
   * This will assign the relavent parts of a DataDescriptor to
   * this object.
   * @param d the descriptor to assign to this objects.
   * @return 0 if ok 1 if wrong class. 
  */
  virtual  int setDescriptor(const Cure::DataDescriptor & d){
    if(getClassType()!=d.ClassType)return 1;
    if (SensorData::SENSORTYPE_CAMERA==d.SensorType)return 1;
    setID(d.ID);
    ShortData[0]=d.SensorID;
    return 0;
   }
  /**Gets the Descriptor*/
  virtual  void getDescriptor(Cure::DataDescriptor & d) const {
    d.ClassType=getClassType();
    d.ID=getID();
    d.SensorType=SensorData::SENSORTYPE_CAMERA;
    d.SensorID=ShortData[0];
    d.Name="";
  }
 
protected:
  virtual  TimestampedData * makeUnpack(){
    VisionData *p=new VisionData();
    unpack(*p);
    return p;
  }
};

} // namespace Cure

#endif // CURE_PACKEDMEASUREMENTSET_HH
