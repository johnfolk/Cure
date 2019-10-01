//    $Id: DataPort.hh,v 1.9 2008/05/05 16:55:27 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson, Patric Jensfelt
//    
//    Copyright (c) 2005 John Folkesson
//    

#ifndef CURE_DATAPORT_HH
#define CURE_DATAPORT_HH

#include "DataSlotAddress.hh"

namespace Cure{

/**
 * A DataPort is a Dataslot input/output port.
 * It can be used to buffer input and output data.
 * It has its Own DataDescriptor to identify its Data. 
 *
 * This can either be a transparent door to a set of filters
 * of a buffer for the input and output.  If pull is set this will 
 * simply read the pull client on any read and thus the data written to the
 * slot is lost.
 *
 * @author John Folkesson 
 */
class DataPort: public DataSlotAddress
{
public:

  /**
   * The index of the pull Address.
   */
  unsigned long PullClient;
  /**
   * Tells if there is any subscribed pull client.
   * If this is true the address will read PullClient on pullData(...).
   */
  bool IsPull;
  /**
   * Set to true for debuging.
   */
  bool Verbose;
protected:
  bool Hidden;
  
  virtual void disconnectAddress(); 
public:

  DataPort(const unsigned short thread=0);

  /**
   * Note that if this is to be set to pull then anything written to
   * the slot will not be read.  So then the depth should be set to 0.
   * Also The classType is always checked against the Descriptor but
   * not neccessarily the subType unless dotypecheck is true.

   * @param depth buffer size
   * @param des type of data that slot is intended for. Storage for
   * this kind of data will be allocted.
   * @param subType fine grained description of the type of data
   * specific to each data type, for example what type of pose
   * @param compressed if you want data to be packed in the slot 
   * @param dotypecheck true if you want to make sure that you can only
   * write data with the type you specified,
   */
  DataPort( short depth,DataDescriptor &des, 
	    unsigned short subType=0,
	    bool compressed=false, 
	    bool dotypecheck=false,
	    const unsigned short thread=0);
  
  ~DataPort();
  /**
   *   
   * Call write on the DataPort.  It checks the ClassType and
   * SensorType against the Descriptor and returns TYPE_ERROR if it
   * doesn't match.  Otherwise it sets the ID and SensorID to match
   * and writes this to the Slot and any push clients. 
   * 

   * @param p Data to be copied
   * @return 0 if ok,
   */
  unsigned short write(TimestampedData& p);
  
  /**
   *
   * Call read on the DataPort.  If the port is set to Pull then This
   * pulls the data from the PullClient.  It checks the ClassType and
   * SensorType against the Descriptor and returns TYPE_ERROR if it
   * doesn't match.  Otherwise it sets the ID and SensorID to match.
   * If there is no Pull Client then this calls read as a
   * DataSlotAddress on the Slot.
   *
   *
   * @see DataSlotAddress::read
   * @param result Data to be copied into
   * @param t requested time will be interpreted based on interpolate 
   * @param interpolate flag for requesting interpolation to t.
   * @return 0 if ok, else TIMESTAMP_ERROR, NO_INTERPOLATE
   *                NO_DATA_AVAILABLE  TIMESTAMP_TOO_OLD or TYPE_ERROR.
   */
  virtual unsigned short read(TimestampedData& result, 
			      int interpolate[1], const Timestamp t=0);

  virtual unsigned short read(TimestampedData& result, const Timestamp t=0,
		   const int interpolate=0){
    int i=interpolate;
    return read(result,&i,t);
  }
  
  /*
   * Removes Data Address 'pa' from the Notify list
   * @param pa the DataAddress 
   * @return 0. 
   */
  virtual unsigned short disconnect(Address *pa);
  unsigned short pull(Address * pa);
protected:
private:
};
  
} // namespace Cure

#endif // CURE_DATASLOTADDRESS_HH
