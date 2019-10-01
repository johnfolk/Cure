// = AUTHOR(S)
//    John Folkesson
//    
//    August, 2004
 //
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_MEASUREMENTSET_HH
#define CURE_MEASUREMENTSET_HH


#include "Measurement.hh"
#include "TimestampedData.hh"
/**
 *
 * @author John Folkesson 
 */
class PackedMeasurementSet;
namespace Cure {

/**
 * This TimestampedData subclass contains a set of Measurement
 * objects.  The number of Measurement Objects stored here will be
 * coded as the SubType.  The ClassType is MEASUREMENTSET_TYPE.
 * Initially the MeasurementSet is created with no Measurements.  One
 * can then call setNumberOfElements or setSubType to create an array
 * of Measurements. The number of Elements can be changed later.  If
 * the number is set to a larger allocation the existing Measurements
 * array will be deleted after its elements are copied to a larger
 * allocated Measurement array.
 */
class MeasurementSet: public TimestampedData 
{
public:
  Measurement *Measurements;
  
protected:
  unsigned short MeasurementsSize;
public:
  
  friend class PackedMeasurementSet;
  /**
   * Create empty MeasurementSet
   */
  MeasurementSet(unsigned short id = 0);
  

  /**
   * Create empty MeasurementSet and setst the timestamp and id.
   * @param t the time 
   * @param id the ID
   */
  MeasurementSet(Timestamp &t,unsigned short id = 0);
  
  /**
   * Create a copy of a MeasurementSet
   * @param src the set to be copied
   */
  MeasurementSet(const MeasurementSet &src);
  /**
   * Sets the Type's ect
   */
  void init();
  /**
   * Destructor
   */
  virtual ~MeasurementSet();
  
  /*
   * This is how we (efficiently) get a MeasurementSet from a TimestampedData.
   */
  MeasurementSet * narrowMeasurementSet(){return this;}

  /**
   * Copy operator
   * @param src the set to be copied
   */
  MeasurementSet& operator=(const MeasurementSet &src);  

  void operator = (TimestampedData& p);

  /** 
  * @return the elements of the Measurments array.
   */
  Measurement& operator()(const unsigned short r) {  
    return( Measurements[r] );
  }

  /**
   * Access the individul Measurements
   * @param r the index of the measurement 
   * @return the elements of the m_Ranges array.
   */
  Measurement operator()(const unsigned short r) const {
    return( (*(MeasurementSet *)this).operator()(r) );
  }
  
  /**
   * Function used to set the number of elements in the
   * MeasurementSet. If you give it a larger size than what it has
   * had before it will allocate space for it. If the size if
   * decreased the space will not be release. That is, to avoid many
   * allocations and deallocations you can call this function with a
   * number larger than what you epect to get and there will be
   * only one allocation.
   *
   * This function calls setSubType
   */
  void setNumberOfElements(unsigned short t) { setSubType(t); }

  /**
   * This returns data based on the string value and the indcies
   * given in the ShortMatrix.  
   *
   * @param str So Z,W, V, CovV and BoundingBox 
   * are recongnized strings.
   * 
   * @param index Set the index as when you would call
   * Cure::Matrix.offset(roff,coff,r,c) with the integers
   * roff,coff,r,c placed in a the top row of index (1x5).
   * the 5th element in index is used to specify the Measurement. 
   * If index(0,4)==-1 then all the measurments are put in a 
   * matrix that is grown to fit by calling getMatrix on each Measurement 
   * in the set. @seeMeasurement::getMatrix.
   * 
   * @param mat the result is returned here in a r x c matrix
   * @return true if ok else false. 
   */
  virtual bool getMatrix(const std::string & str, ShortMatrix & index,
			 Matrix &mat);
  /**
   * This sets data based on the string value and the indcies
   * given in the ShortMatrix.  
   *
   * @param str So Z,W, V, CovV and BoundingBox 
   * are recongnized strings.
   * 
   * @param index Set the index as when you would call
   * Cure::Matrix.offset(roff,coff,r,c) with the integers
   * roff,coff,r,c placed in a the top row of index (1x5).
   * the 5th element in index is used to specify the Measurement. 
   * If index(0,4)==-1 then all the measurments are put in a 
   * matrix that is grown to fit by calling getMatrix on each Measurement 
   * in the set. @seeMeasurement::getMatrix.
   * 
   * @param mat the values to set are given here in a r x c matrix
   * @return true if ok else false. 
   */
  virtual bool setMatrix(const std::string & str, ShortMatrix & index,
			 Matrix &mat);

  
  /**
   * @return number of elements in the MeasurementSet
   *
   * Calls getSubType()
   */
  unsigned short getNumberOfElements() const { return getSubType(); }
  
  /**
   * Setting the subType also will allocatea new Measurements array,
   * (and delete the old one), if the current array  is smaller than
   * t..  After setting the SubType the set will have t elements. 
   * The SubType is the number of elements in the set not the
   * size of the Measurements array which could be larger.
   */
  void setSubType(unsigned short t); 
  
  /**
   * This copies the first Measurment into all the others in the 
   * set.  It is useful to initialize the Matricies in Measurements. 
   */
  void setupMeasurements();
 /**
   * This will assign the relavent parts of a DataDescriptor to
   * this object.
   * @param d the descriptor to assign to this objects.
   * @return 0 if ok 1 if wrong class. 
  */
  virtual  int setDescriptor(const Cure::DataDescriptor & d){
    if(getClassType()!=d.ClassType)return 1;
    setID(d.ID);
     if (Measurements)
      {
	for (int i=0; i<MeasurementsSize; i++)
	  {
	    Measurements[i].SensorType=d.SensorType;
	    Measurements[i].SensorID=d.SensorID;
	  }
      }
   return 0;
  }
  virtual void setSensorID(unsigned short id) {
	if (Measurements)
	  for (int i=0; i<MeasurementsSize; i++)
	    Measurements[i].SensorID=id;	
      }
  
  virtual void setSensorType(unsigned short type){
    if (Measurements)
      for (int i=0; i<MeasurementsSize; i++)
	Measurements[i].SensorType=type;	
  }

  /**Gets The DataDescriptor (not Name)*/
  virtual  void getDescriptor(Cure::DataDescriptor & d) const {
    d.ClassType=getClassType();
    d.ID=getID();
    if (Measurements)
      {
       d.SensorType=Measurements[0].SensorType;
       d.SensorID=Measurements[0].SensorID;
     }
    else
      {
	d.SensorType=0;
	d.SensorID=0;
      }
    d.Name="";
  }
  virtual unsigned short getSensorType() const{
    if (Measurements)
      return Measurements[0].SensorType;
    return 0;
  }
  virtual unsigned short getSensorID() const {
    if (Measurements)
      return Measurements[0].SensorID;
    return 0;
  }
   

  /**Display info*/
  virtual void print();
  
protected:
       
  };
}
#endif
