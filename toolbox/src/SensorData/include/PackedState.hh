// = AUTHOR(S)
//    John Folkesson 
//    
//    Copyright (c) 2006 John Folkesson
//    

#ifndef CURE_PACKEDSTATE_HH
#define CURE_PACKEDSTATE_HH

#include "StateData.hh"
#include "PackedData.hh"
namespace Cure{

/**
 * Packed version of StateData
 * 
 * The SubType is the size of the ShortMatrix ShortData
 * 
 * The orginal layout of the ShortData was laid out as:
 * ShortData.Rows, ShortData.Columns, Data.Rows, Data.Columns,
 * StateType
 * Then comes the ShortData across the first row then the second row
 * and so on for the whole Matrix
 *
 * @author John Folkesson,
 */  
class PackedState: public PackedData 
{
  friend class StateData;
public:
  /**
   * Version number that is used when packing unpacking data to make
   * sure that the same version of the code is used to pack/unpack the
   * data.
   */
  static int VERSION;

public:
  /**The Constructor*/
  PackedState();
  /**
   * The copy constructor.
   * @param p the set to copy
   */
  PackedState(PackedState& p);
  /**
   * The pack and copy constructor.
   * @param p the set to copy and pack
   */
  PackedState(StateData& p);
  /**callsed by the constuctors*/ 
  virtual void init();
  virtual ~PackedState();
  /**fast cast*/
  PackedState * narrowPackedState(){return this;}
  //*******************SET FUNCTIONS***********************************
  /**
   * Copy operator 
   * If p is a State or PackedState then this will pack it 
   * up.
   * @param p the object to copy.
   */
  virtual void operator=(TimestampedData& p);
  /**
   * Copy operator 
   * p is a State that this will pack  
   * up.
   * @param p the object to copy.
   */
  void operator=(const StateData& p);
  /**
   * If p is a State then this will set p to be equal to the object
   * stored in this PackedState.
   * @param p the object to unpack into.
   */
  virtual void unpack(TimestampedData& p){
    StateData *s=p.narrowStateData();
    if (s) unpack(*s);
  }
  /**
   * This function will alter the size of the array ShortData but will
   * not copy the values from the old to the new memory location,
   * i.e. do not call this function unless you no longer need the data
   * @param t the new number of elements. 
   */
  void setSubType(unsigned short t);
  int setShortDataSize(unsigned long len);  
  /**
   * This will set s to be equal to the object
   * stored in this PackedState.
   * @param s the object to unpack into.
   */
  void unpack(StateData& s);
 
protected:
  virtual  TimestampedData * makeUnpack(){
    StateData *p=new StateData();
    unpack(*p);
    return p;
  }
};

} // namespace Cure

#endif // CURE_PACKEDMEASUREMENTSET_HH
