// = AUTHOR(S)
//    John Folkesson 
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_PACKEDMEASUREMENTSET_HH
#define CURE_PACKEDMEASUREMENTSET_HH

#include "MeasurementSet.hh"
#include "PackedData.hh"
namespace Cure{

/**
 * Packed version of a measurement set
 *
 * The SubType is the number of measurements as an unsigned short;
 * 
 * The orginal layout of the ShortData was laid out as:
 * DatasizeH, DataSizeL,
 * mtype,keyH,keyL,Z.Rows,Box.R/C,W.Rows,W.Columns,V.Rows
 * mtype,keyH,keyL,Z.Rows,Box.R/C,W.Rows,W.Columns,V.Rows
 * and so on for each measurmenet
 *
 * VERSION 2: ShortData is laid out as:
 * DatasizeH, DataSizeL,
 * mtype,sensType,sensID,keyH,keyL,Z.Rows,Box.R/C,W.Rows,W.Columns,V.Rows
 * mtype,sensType,sensID,keyH,keyL,Z.Rows,Box.R/C,W.Rows,W.Columns,V.Rows
 * and so on for each measurment
 *
 * VERSION 3: ShortData is laid out as: 
 * (DataSize written instead of DataLen in the "header" Type, Time, ...)
 * mtype,sensType,sensID,keyH,keyL,Z.Rows,Box.R/C,W.Rows,W.Columns,V.Rows
 * mtype,sensType,sensID,keyH,keyL,Z.Rows,Box.R/C,W.Rows,W.Columns,V.Rows
 * and so on for each measurment
 *
 * VERSION 4: ShortData is laid out as: 
 * 
 * mtype,sensType,sensID,keyH,keyL,Z.Rows,Box.R/C,W.Rows,W.Columns,V.Rows SW.Rows SW.Columns SW(0,0) SW(0,1),..SW(SW.Rows-1,SW.Columns-1)
 * mtype,sensType,sensID,keyH,keyL,Z.Rows,Box.R/C,W.Rows,W.Columns,V.Rows SW.Rows SW.Columns SW(0,0) SW(0,1),..SW(SW.Rows-1,SW.Columns-1)
 * and so on for each measurment
 *
 *
 *
 * @author John Folkesson, Patric Jensfelt 
 */  
class PackedMeasurementSet: public PackedData 
{
  friend class MeasurementSet;
public:
  /**
   * Version number that is used when packing unpacking data to make
   * sure that the same version of the code is used to pack/unpack the
   * data.
   */
  static int VERSION;

public:
  /**The Constructor*/
  PackedMeasurementSet();
  /**
   * The copy constructor.
   * @param p the set to copy
   */
  PackedMeasurementSet(PackedMeasurementSet& p);
  /**
   * The pack and copy constructor.
   * @param p the set to copy and pack
   */
  PackedMeasurementSet(MeasurementSet& p);
  /**callsed by the constuctors*/ 
  virtual void init();
  virtual ~PackedMeasurementSet();
  /**fast cast*/
  PackedMeasurementSet * narrowPackedMeasurementSet(){return this;}
  //*******************SET FUNCTIONS***********************************
  /**
   * Copy operator 
   * If p is a MeassurmentSet or PackedMeasurementSet then this will pack it 
   * up.
   * @param p the object to copy.
   */
  virtual void operator=(TimestampedData& p);
  /**
   * Copy operator 
   * p is a MeassurmentSet that this will pack  
   * up.
   * @param p the object to copy.
   */
  void operator=(const MeasurementSet& p);
  /**
   * If p is a MeasurementSet then this will set p to be equal to the object
   * stored in this PackedMeasurementSet.
   * @param p the object to unpack into.
   */
  virtual void unpack(TimestampedData& p){
    MeasurementSet *s=p.narrowMeasurementSet();
    if (s) unpack(*s);
  }
  /**
   * Get the Number of Measurments.
   * @return the number of Measurements in the MeasurementSet
   */
  unsigned short getNumberOfElements() const { return getSubType(); }
  /**
   * This allocates the ShortData and sets the SubType.
   * Old data is deleted if this must change size.
   * @param t the new number of elements.
   */
  void setNumberOfElements(unsigned short t) { setSubType(t); }

  /**
   * This function will alter the size of the array ShortData but will
   * not copy the values from the old to the new memory location,
   * i.e. do not call this function unless you no longer need the data
   * @param t the new number of elements. 
   */
  void setSubType(unsigned short t);
  /**
   * This converts the short data array from verion's meaning to this. 
   * @param vesion the version to convert from.
   * @return 1 if version too high
   */
  int convertShortData(int vers){
    if (vers==Version)return 0;
    return fixShortData(vers);
  }
  
  int setShortDataSize(unsigned long len); 

  /**
   * This will set s to be equal to the object
   * stored in this PackedMeasurementSet.
   * @param s the object to unpack into.
   */
  void unpack(MeasurementSet& s);
 
protected:
  int fixShortData(int vers);
  virtual  TimestampedData * makeUnpack(){
    MeasurementSet *p=new MeasurementSet();
    unpack(*p);
    return p;
  }
};

} // namespace Cure

#endif // CURE_PACKEDMEASUREMENTSET_HH
