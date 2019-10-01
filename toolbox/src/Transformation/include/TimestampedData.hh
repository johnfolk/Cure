//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_TIMESTAMPEDDATA_HH
#define CURE_TIMESTAMPEDDATA_HH




/**
 * Code for different data types @see DataDescriptor.hh
 * @see CureDefines.hh
*/ 

#include "Timestamp.hh"
#include "ShortMatrix.hh"
#include "DataDescriptor.hh"
#include "CureDefines.hh"

namespace Cure{


  class SICKScan; 
  class Odometry;
  class Pose3D;
  class RangeData;
  class GenericData;
  class MeasurementSet;
  class SmartData;
  class SensorData;
  class SensorPose;
  class SonarData;
  class Command;
  class GenericData;
  class VisionData;
  class ContactData;
  class GridData;
  class PackedDataSet;
  class DataSet;
  class PackedGrid;
  class PackedContact;
  class PackedVision;
  class PackedGeneric;
  class PackedCommand;
  class PackedSonar;
  class PackedData;
  class PackedPose;
  class PackedSICK;
  class PackedOdometry;
  class PackedRange;
  class PackedGeneric;
  class PackedMeasurementSet;
  class PackedSensorPose;
  class StateData;
  class PackedState;
  class TimestampedString;
  class PackedTimestampedString;
/**
 * The TimestampedData base class gives a time, a classType, a subType,
 * an ID and a packed flag.
 *
 * ClassType - 8 bits that can be for example SICKSCAN_TYPE, e.g. 0x02
 * would be a SICK scan and 0x07 is a MeasurementSet
 * 
 * SubType - 16 bits used by the subclass to further specify the
 * interpretation of the data. This could for example contain the
 * number of range readings for a SICKScan or the type of pose data.
 *
 * ID - 8 bits that give an arbitrary id number to the data.  This can
 * be used to specify the sensor in an array of sensors or a special
 * pose such as dead reckoning position, etc.
 *
 * There are packed versions of some of the subclasses, these contain
 * the same data but in a more compact form that is ideal for writing
 * to file or transmitting over a socket. These classes should set the
 * Packed flag.
 *
 * If you use ClassType = 0x01 you get just timestamp
 *
 * @author John Folkesson
 */
class TimestampedData 
{
public:
  /**
   * Here is where the time is kept.
   */ 
 Timestamp Time;
protected:
  /**
   * Flag that is set for the packed version of a certain TimestamedData type
   * @see Cure::PackedData. 
  */
  bool m_Packed;

  /**
   * The type of data.  Each sub class will have a unique type. 
   */
  unsigned char m_ClassType;
  
  /**
   * Each subclass of TimestampedData can use this for amore fine
   * grained description of the type of data within the subclass.
   */
  unsigned short m_SubType;

  /**
   * Can give an arbitrary id number to the data
   */
  unsigned short ID;
  
public:
  /**
   * The constructor.
   */
  TimestampedData();
  
  virtual ~TimestampedData();

  /**
   * This is called by constructor to do subclass specific
   * initialization such as setting the class type.
   */
  virtual void init()
  {
    m_Packed = false;
    m_ClassType=TIMESTAMP_TYPE;
    m_SubType = 0;
    ID = 0;
  }

  /**narrow functions for easy casting */
  virtual SmartData * narrowSmartData(){return 0;}
  /**narrow functions for easy casting */
  virtual SensorData * narrowSensorData(){return 0;}
  /**narrow functions for easy casting */ 
 virtual Pose3D * narrowPose3D() {return 0;}  
  /**narrow functions for easy casting */
  virtual PackedPose * narrowPackedPose(){return 0;}
  /**narrow functions for easy casting */
  virtual SICKScan * narrowSICKScan(){return 0;}
  /**narrow functions for easy casting */
  virtual PackedSICK * narrowPackedSICK(){return 0;}
  /**narrow functions for easy casting */
  virtual Odometry * narrowOdometry(){return 0;}
   /**narrow functions for easy casting */
  virtual PackedOdometry * narrowPackedOdometry(){return 0;}
  /**narrow functions for easy casting */
  virtual MeasurementSet * narrowMeasurementSet(){return 0;}
  /**narrow functions for easy casting */
  virtual PackedMeasurementSet * narrowPackedMeasurementSet(){return 0;}
  /**narrow functions for easy casting */
  virtual SensorPose * narrowSensorPose(){return 0;}
   /**narrow functions for easy casting */
  virtual PackedSensorPose * narrowPackedSensorPose(){return 0;}
  /**narrow functions for easy casting */
  virtual PackedData * narrowPackedData(){return 0;}
  /**narrow functions for easy casting */
  virtual SonarData * narrowSonarData(){return 0;}
  /**narrow functions for easy casting */
  virtual PackedSonar * narrowPackedSonar(){return 0;}
  /**narrow functions for easy casting */
  virtual Command * narrowCommand(){return 0;}
  /**narrow functions for easy casting */
  virtual PackedCommand * narrowPackedCommand(){return 0;}
  /**narrow functions for easy casting */
  virtual GridData * narrowGridData(){return 0;}
  /**narrow functions for easy casting */
  virtual PackedGrid * narrowPackedGrid(){return 0;}
  /**narrow functions for easy casting */
  virtual GenericData * narrowGenericData(){return 0;}
  /**narrow functions for easy casting */
  virtual PackedGeneric * narrowPackedGeneric(){return 0;}
  /**narrow functions for easy casting */
  virtual VisionData * narrowVisionData(){return 0;}
  /**narrow functions for easy casting */
  virtual PackedVision * narrowPackedVision(){return 0;}
  /**narrow functions for easy casting */
  virtual ContactData * narrowContactData(){return 0;}
  /**narrow functions for easy casting */
  virtual PackedContact * narrowPackedContact(){return 0;}
  /**narrow functions for easy casting */
  virtual RangeData * narrowRangeData(){return 0;}
  /**narrow functions for easy casting */
  virtual PackedRange * narrowPackedRange(){return 0;}
  virtual DataSet * narrowDataSet(){return 0;}
  virtual PackedDataSet * narrowPackedDataSet(){return 0;}
  virtual StateData * narrowStateData(){return 0;}
  virtual PackedState * narrowPackedState(){return 0;}
  virtual TimestampedString *narrowTimestampedString(){return 0;}
  virtual PackedTimestampedString *narrowPackedTimestampedString(){return 0;}
  /** 
   * The get function will get a pointer to an object of the ClassType
   * if it can.  It will unpack it if it can.  The pointer will either
   * be to the same as this or an object inside this.  Probably only 
   * overwritten in SmartData
   *
   * @param classType the class to match (packed classTypes=unpacked
   *                  ones so there are two possible return types.
   * @return a pointer to an object with the class type or 0 if not 
   *        possible.  
   */
  virtual TimestampedData *getClassPointer(unsigned short classType){
    if (m_ClassType==classType)return this;
    return 0;
  }
  /** 
   * The set function will return a pointer to an object of the
   * ClassType if it can.  It will unpack it if it can and change the
   * internal object if it is a SmartData
   * The pointer will either be to the same as this
   * or an object inside this.  Probably only overwritten in SmartData
   *
   * @param classType the class to match (packed classTypes=unpacked
   *                  ones so there are two possible return types.
   * @return a pointer to an object with the class type or 0 if not 
   *        possible.  
   */
  virtual TimestampedData *setClassPointer(unsigned short classType){
    if (m_ClassType==classType)return this;
    return 0;
  }
  /**
   * Just copies if this is not packed.  Packed subclasses will do
   * more work.
   *
   * @param td the object to copy to.
   */
  virtual void unpack(TimestampedData &td){
    td=(*this);
  }
  /**
   * This will copy if it can match somehow the classType to this and
   * td.  If td is unpacked or SmartData the it will become an
   * unpacked.  If td.isPacked() it will remain so.  The case
   * PackedData=SmartData->PackedData is somewhat inefficient as the
   * SmartData will unpack and then the PackedData will pack.
   * 
   * @param td the object to copy to.  It must be able to 
   *           be an object of the classType. (ie it
   *           either is one already or is a SmartData).
   * @param classType the class to match (packed classTypes=unpacked
   *                  ones so there are two possible return types.
   * @return true if sucessfully copied eles false (nothing done)
   */
  virtual bool copyIfClass(unsigned short classType, 
			   TimestampedData &td){
    TimestampedData *p=getClassPointer(classType);
    if (!p)return false;
    TimestampedData *p2=td.setClassPointer(classType);
    if (!p2)return false;
    if (p2->isPacked())(*p2)=(*p);
    p->unpack(*p2);
    return true;
  }
  
  //*******************SET FUNCTIONS***********************************

  /**
   * This assignment operator must be overidden in subclass for proper
   * read/write operation.
   */
  virtual void operator = (TimestampedData& p)
  {
    Time = p.Time;
    if (getClassType() != p.getClassType()) return;

    // We do not set the m_Packed flag since we want to be able copy
    // packed to unpacked and vice versa

    setID(p.getID());
    setSubType(p.getSubType());
  }
  /**
   * This just sets the time.
   * @param t the tiem to copy.
   */
  void operator = (const Timestamp t){ 
    Time =(t);
  }
  /**
   * Sets p equal as possible to this,
   * 
   * 
   * 
   * @param  p the object to copy the internal object into.
   */
  virtual void setEqual(TimestampedData &p){
    p=(*this);
  }
  /**
   * Set the SubType.
   * @param st the subtype value to set.
   */
  virtual void setSubType(unsigned short st) { m_SubType = st; }
  /**
   * Set the SubID.
   * @param id the ID value to set.
   */
  virtual void setID(unsigned short id) { ID = id; }

  
  /**
   * This will assign the relavent parts of a DataDescriptor to
   * this object.
   * @param d the descriptor to assign to this objects.
   * @return 0 if ok 1 if wrong class. 
  */
  virtual  int setDescriptor(const Cure::DataDescriptor & d){
    if(getClassType()!=d.ClassType)return 1;
    setID(d.ID);
    setSensorID(d.SensorID);
    setSensorType(d.SensorType);
    return 0;
   }
  virtual void setSensorID(unsigned short) {}
  virtual void setSensorType(unsigned short) {}
  /**
   * This just sets the time.
   * @param t the time value to set.
   */
  virtual void setTime(const Timestamp &t){ Time=t;  }
 

 //******************** GETS *****************************************
 
  /**
   * This just gets the time value.
   * @return the time value.
   */
  virtual Timestamp getTime() const {return Time;}
  /**
   * This just gets the time value.
   * @return the time value.
   */
  virtual double getDoubleTime()const {return Time.getDouble();}
  /**
   * This returns data based on the string value and the indcies given
   * in the ShortMatrix.  The implementations differ by subclass but
   * they all use only the first row of index.
   */
  virtual bool getMatrix(const std::string & str, ShortMatrix & index,
			 Matrix &mat);
  /**
   * This sets data based on the string value and the indcies given
   * in the ShortMatrix.  The implementations differ by subclass but
   * they all use only the first row of index.
   */
  virtual bool setMatrix(const std::string & str, ShortMatrix & index,
			 Matrix &mat);
  /**
   * This just discovers if this is a packed version.
   * @return true if a PackedData else false.
   */
  bool isPacked() const { return m_Packed; }
  /**
   * Gets the Class Type.
   * @return the ClassType of this object.
   */
  unsigned char getClassType() const { return m_ClassType; }
  /**
   * Gets the  SubType.
   * @return the SubType of this object.
   */
  unsigned short getSubType() const { return m_SubType; }
  /**
   * Gets the  ID.
   * @return the ID of this object.
   */
  unsigned short getID() const { return ID; }
  virtual  void getDescriptor(Cure::DataDescriptor & d) const {
    d.ClassType=getClassType();
    d.ID=getID();
    d.SensorType=getSensorType();
    d.SensorID=getSensorID();
    d.Name="";
  }
  virtual unsigned short getSensorType() const {return 0;}
  virtual unsigned short getSensorID() const {return 0;}
  /**
   * Version number that can be used reading from file to
   * make sure that the reader of the data has the same version as the
   * writer.
   */
  virtual int version(){return 1;}

  /**
   * This just gets the time value.
   * @param t the time value is returned here.
   */
  virtual void getTime(Timestamp &t){t=Time;}
  /**
   * Interpolate between two TimestampedData's to time t.
   *
   * The object becomes the interpolated object.  
   *
   * This virtual method just takes the data closest to t:
   * A subclass might override this to do better.
   *
   * PackedData objects should not be interpolated as they 
   * generally do not override this. 
   *
   * This method is not value safe,(signified by _ ), which means
   * You can not call this with itself as an argument (ie. 
   * DO NOT DO THIS: A.interpolate_(A,B,t)).
   *
   * @param  a One TimestampedData near time t. 
   * @param  b Another TimestampedData near time t.
   * @param t The time you want to interpolate to. 
   */
  virtual void interpolate_(TimestampedData& a, TimestampedData& b, 
			    Timestamp t);

  /**
   * Prints a message about the object to the display.
   */
  virtual void print();

};
} // namespace Cure

std::ostream& operator<<(std::ostream& os, const Cure::TimestampedData& td);

#endif // CURE_TIMESTAMPEDDATA_HH
