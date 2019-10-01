// = AUTHOR(S)
//    John Folkesson 
//    
//    Copyright (c) 2005 John Folkesson
//    

#ifndef CURE_PACKEDGRID_HH
#define CURE_PACKEDGRID_HH

#include "GridData.hh"
#include "PackedData.hh"
namespace Cure{

/**
 * Packed version of a grid data objects
 * 
 * The SubType is the size of the ShortMatrix ShortData
 * 
 * The orginal layout of the ShortData was laid out as:
 * ShortData.Rows, ShortData.Columns, Data.Rows, Data.Columns,
 * GridType
 * Then comes the ShortData across the first row then the second row
 * and so on for the whole Matrix
 *
 * @author John Folkesson,
 */  
class PackedGrid: public PackedData 
{
  friend class GridData;
public:
  /**
   * Version number that is used when packing unpacking data to make
   * sure that the same version of the code is used to pack/unpack the
   * data.
   */
  static int VERSION;

public:
  /**The Constructor*/
  PackedGrid();
  /**
   * The copy constructor.
   * @param p the set to copy
   */
  PackedGrid(PackedGrid& p);
  /**
   * The pack and copy constructor.
   * @param p the set to copy and pack
   */
  PackedGrid(GridData& p);
  /**callsed by the constuctors*/ 
  virtual void init();
  virtual ~PackedGrid();
  /**fast cast*/
  PackedGrid * narrowPackedGrid(){return this;}
  //*******************SET FUNCTIONS***********************************
  /**
   * Copy operator 
   * If p is a Grid or PackedGrid then this will pack it 
   * up.
   * @param p the object to copy.
   */
  virtual void operator=(TimestampedData& p);
  /**
   * Copy operator 
   * p is a Grid that this will pack  
   * up.
   * @param p the object to copy.
   */
  void operator=(const GridData& p);
  /**
   * If p is a Grid then this will set p to be equal to the object
   * stored in this PackedGrid.
   * @param p the object to unpack into.
   */
  virtual void unpack(TimestampedData& p){
    GridData *s=p.narrowGridData();
    if (s) unpack(*s);
  }
  /**
   * This function will alter the size of the array ShortData but will
   * not copy the values from the old to the new memory location,
   * i.e. do not call this function unless you no longer need the data
   * @param t the new number of elements. 
   */
  void setSubType(unsigned short t);
  
  /**
   * This function will change the size of the  array ShortData (if
   * needed). Effects ShortData and ShortDataSize.
   *
   * 
   * This will reallocate the short array.
   *
   * @param len the size of the new array.
   * @return 1 if failed 0 else, 
  */
  virtual int setShortDataSize(unsigned long len);

  /**
   * This will set s to be equal to the object
   * stored in this PackedGrid.
   * @param s the object to unpack into.
   */
  void unpack(GridData& s);
 
protected:
  virtual  TimestampedData * makeUnpack(){
    GridData *p=new GridData();
    unpack(*p);
    return p;
  }
};

} // namespace Cure

#endif // CURE_PACKEDMEASUREMENTSET_HH
