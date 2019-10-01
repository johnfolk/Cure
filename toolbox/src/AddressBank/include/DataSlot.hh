//    $Id: DataSlot.hh,v 1.14 2007/09/16 09:04:41 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_DATASLOT_HH
#define CURE_DATASLOT_HH

#include "PackedData.hh"
#include "AddressBank.hh"
#include <string>

namespace Cure{
  
/**
 * DataSlot is a History of Data data from which one can request data at
 * a given time in the past.  If compressed is set when creating the 
 * DataSlot then it stores PackedData objects instead of TimestampedData
 * objects (much smaller but need to be packed and unpacked to use).  
 * In that case, the calculated values are not saved, only the numbers 
 * needed to recalculate them are saved.
 * 
 * 
 * DataSlot has one input/output port, a DataSlotAddress.  One typically 
 * creates the DataSlotAddress rather than the DataSlot directly.  
 * This Address can then be used to read/write/pushTo to/from the slot.
 *
 * Any data can be written to the slot but if one knows in advance the
 * Types one can allocate them at once.  Eventually if one constantly
 * writes the same type the pointers will be allocated anyhow.  One
 * can read the data with a SmartData without haveing to know the
 * type.  By setting DoTypeCheck one can require that the Types match
 * a given data type.  In that case only data of the right type or
 * SmartData are considered the type that they point to and PackedData
 * have the type of the unpacked version.
 *
 * The purpose of this is to store sequential data and then do look
 * ups based on the Time.  If the data is written out of sequence it
 * will be sorted into the correct location but not effieciently.
 * Also some functionality like tring to read all the data
 * sequentially using interpolate flag=1 will end up not working
 * correctly if the data is not written in time order.  Most likely
 * you just miss some data when you think you are reading everything.
 *
 * @author John Folkesson
 */
class DataSlot
{
public:
  /**
   * This name is used for debug print outs
   * The ClassType is used for TypeChecks 
  */
  DataDescriptor Descriptor;
 /**If not 0 the data will be stored in packed form, (default 0). */
  bool Packed;
  /**
   * This is to allow reads to an empty slot and writes out of
   * sequence to print errors.  default true.  If you don't want
   * these anoying messages set this to false.
   *
  */
  bool  BugMe;
  /**Chose to check the Data type on read and write. default false.*/
  bool DoTypeCheck;
  /**The subtype of the data to be stored here for type checking*/
  unsigned short SubType;

protected:
  /** The circular array for storing the data*/
  SmartData *History;
  /** A helper object used when interpolating*/
  SmartData Interpolator;
  /**Index of the most recent data*/
  long Last;
  /**Ths size of the History array*/
  long Size;
  /**
   * The amount of data being stored.
   * Starts out at 0 and grows to Size but no more
   */
  long NumItems;  
public:
  /** The constructor*/
  DataSlot();
  /**
   * The constructor
   * @param depth the size of storage array, maximu number of items stored.
   * @param classType For type checking on read and write if
   * (dotypecheck=true) and for allocating the memory for the objects
   * at once.
   * @param subType For type checking on read and write if
   * (dotypecheck=true) and for allocating the memory for the objects
   * at once.
   * @compressed if true the data is stored as packed data objects (smaller)
   * @dottypecheck If true the data will be checked on read and write
   * to see if it is the right type (smartdata is has the object
   * pointed to checked).
  */
  DataSlot(const long depth, 
           const unsigned char classType=POSE3D_TYPE,
           const unsigned short subtype=0,
           bool compressed=false, bool dotypecheck=false);
  /**
   * Called by the constructor to set up this slot.
   * @param depth the size of storage array, maximu number of items stored.
   * @param classType For type checking on read and write if (dotypecheck=true)
   * @param subType For type checking on read and write if (dotypecheck=true)
   * @compressed if true the data is stored as packed data objects (smaller)
   * @dottypecheck If true the data will be checked on read and write
   * to see if it is the right type (smartdata is has the object
   * pointed to checked).
   * @bugme set BugMe to this (true prints debug message on
   * read to an empty slot and write out of sequence).
  */
  void setup(const long depth, 
           const unsigned char classType=POSE3D_TYPE,
            const  unsigned short subtype=0,
	     bool compressed=false,bool dotypecheck=false);
  virtual ~DataSlot();
  /**
   * return the Name of the Descriptor
   */
  std::string &   slotName(){
    return Descriptor.Name;
  }

  /** Get number of items in the Slot (written to it) */
  long getNumItems() const;

  /**
   * This will write a TimestampedData 'p' to the slot.  Items should
   * be written in time order but if not the data is sorted into the
   * right position.  Writing out of time order might cause read with
   * interpolate!=0 and t!=0 to not work correctly.
   *
   * @param p Data to be copied
   * @return 0 if ok or TYPE_ERROR, TIMESTAMP_ERROR;.
   */
   virtual unsigned short write(TimestampedData& p);

  /**
   * Copy a TimestampedData into result.
   * if t=0 return the most recent Data available.
   * 
   * if interpolate=k >0 Go to first data written that was >= t and then
   * k more steps through the data, so the kth one written after the 1st
   * one >= t.
   * if interpolate=k <0 Go to last data written that was <= t and then
   * k more steps back through the data, so the kth one written before the
   * last one <= t.  This means that there needs to be some data with
   * >t inorder to know where to start.
   * if interpolate =  0 interpolate between the two Datas before and 
   * after t (or just take the data at exactly t).  
   *
   * Simply said:
   * if interpolate == +k return the kth Data 'after' t.
   * if interpolate == -k return the kth Data 'before' t.
   * if interpolate =  0 interpolate between the two Datas before and 
   *  
   * Upon return intepolate will have been set for reading the next
   * Data (before or after depending on its sign)
   * 
   * So one would call read with the the timestamp t=prior results Time
   * and *interpolate = the prior returned intepolate.
   *
   * If the data is not available as requested:
   * return TIMSTAMP_ERROR  (note: there may be something useful in p)
   * 
   *
   * @param result Data to be copied into
   * @param t requested time will be interpreted based on interpolate 
   * @param interpolate flag for requesting interpolation to t.
   * @return 0                 if ok, 
   *       TIMESTAMP_ERROR | NO_DATA_AVAILABLE if slot empty
   *       TIMESTAMP_ERROR | NO_DATA_AVAILABLE | TIMESTAMP_IN_FUTURE if 
   *                        in future
   *       TIMESTAMP_ERROR | TIMESTAMP_TOO_OLD if request is for 
   *                        data older than oldest stored data.
   *       TYPE_ERROR if DoTypeCheck is true and the class of result is wrong.
   *       TIMESTAMP_ERROR if first and last time has same timestamp
   *           
   */
   virtual unsigned short read(TimestampedData& result,
		      int interpolate[1],const Timestamp t=0);
  /**
   *  Same as read(result interpolate,t) without return in interpolate.
   */
  virtual unsigned short read(TimestampedData& result,const Timestamp t=0, 
		      const int interpolate=0)
  {
    int i=interpolate;
    return read(result,&i,t);
  }
  
};

inline long DataSlot::getNumItems() const { return NumItems; }

} // namespace Cure

#endif // CURE_DATASLOT_HH
