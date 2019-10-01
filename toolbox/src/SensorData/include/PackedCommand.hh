// = AUTHOR(S)
//    John Folkesson 
//    
//    Copyright (c) 2005 John Folkesson
//    

#ifndef CURE_PACKEDCOMMAND_HH
#define CURE_PACKEDCOMMAND_HH

#include "Command.hh"
#include "PackedData.hh"
namespace Cure{

/**
 * Packed version of a Command object.
 * 
 * The SubType is the size of the ShortMatrix CmdFlags
 * 
 * The orginal layout of the ShortData was laid out as:
 * CmdFlags.Rows, CmdFlags.Columns, CmdData.Rows, CmdData.Columns,
 * CmdType, Service, Priority, Sender, ReplyAddress(high 2 bytes, low 2 Bytes)
 * CmdID (high, low) AwkID (high, low).
 * Then comes the CmdFlags across the first row then the second row
 * and so on for the whole Matrix
 *
 * @author John Folkesson, 
 */  
class PackedCommand: public PackedData 
{
  friend class Command;
public:
  /**
   * Version number that is used when packing unpacking data to make
   * sure that the same version of the code is used to pack/unpack the
   * data.
   */
  static int VERSION;

public:
  /**The Constructor*/
  PackedCommand();
  /**
   * The copy constructor.
   * @param p the set to copy
   */
  PackedCommand(PackedCommand& p);
  /**
   * The pack and copy constructor.
   * @param p the set to copy and pack
   */
  PackedCommand(Command& p);
  /**callsed by the constuctors*/ 
  virtual void init();
  virtual ~PackedCommand();
  /**fast cast*/
  PackedCommand * narrowPackedCommand(){return this;}
  //*******************SET FUNCTIONS***********************************
  /**
   * Copy operator 
   * If p is a Command or PackedCommand then this will pack it 
   * up.
   * @param p the object to copy.
   */
  virtual void operator=(TimestampedData& p);
  /**
   * Copy operator 
   * p is a Command that this will pack  
   * up.
   * @param p the object to copy.
   */
  void operator=(const Command& p);
  /**
   * If p is a Command then this will set p to be equal to the object
   * stored in this PackedCommand.
   * @param p the object to unpack into.
   */
  virtual void unpack(TimestampedData& p){
    Command *s=p.narrowCommand();
    if (s) unpack(*s);
  }

  
  virtual int setShortDataSize(unsigned long len)
  {
    if (len<14)return 1;
    m_SubType = (len-14);
    if (ShortDataSize==len)return 0;
    ShortDataSize = len;  
    if (ShortData) delete [] ShortData;
    if (ShortDataSize){
      ShortData = new unsigned short[len];
      memset(ShortData, 0, ShortDataSize*sizeof(unsigned short));
    } else ShortData=0; 
    return 0;
  }
  /**
   * This function will alter the size of the array ShortData but will
   * not copy the values from the old to the new memory location,
   * i.e. do not call this function unless you no longer need the data
   * @param t the new number of elements. 
   */
  void setSubType(unsigned short t){
    if (m_SubType == 0 || t != m_SubType) 
      setShortDataSize(t+14);
  }

  /**
   * This will set s to be equal to the object
   * stored in this PackedCommand.
   * @param s the object to unpack into.
   */
  void unpack(Command& s);
 
protected:
  virtual  TimestampedData * makeUnpack(){
    Command *p=new Command();
    unpack(*p);
    return p;
  }

};

} // namespace Cure

#endif // CURE_PACKEDMEASUREMENTSET_HH
