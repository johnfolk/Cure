//
// = LIBRARY
//
// = FILENAME
//    RLDisplaySICK.hh
//
// = FUNCTION
//
// = AUTHOR(S)
//    Patric Jensfelt
//
// = DESCRIPTION
//    
// = COPYRIGHT
//    Copyright (c) 2004 Patric Jensfelt
//
/*----------------------------------------------------------------------*/

#ifndef RLDisplaySICK_hh
#define RLDisplaySICK_hh

#include "DataFilter.hh"

#ifndef DEPEND
#endif

// Forward declaration(s)
class RoboLookProxy;

namespace Cure {

/**
 * Class that can display a SICKScan. You can display in two ways:
 * 1: Call the display method with the scan to display
 * 2: You use the write mechanism either by directly writing a
 * SICKScan to the input (with the write function), connect it to some
 * producer of SICKScans with a push or you can write/push any
 * TimestampedData to the trigger port and a SICKScan will be pulled
 * on the input that in this case must be connected to a Slot with
 * SICKScans.
 * 
 * @author Patric Jensfelt 
 * @see
 */
class RLDisplaySICK : public Cure::DataFilter {
public:

  /**
   * Constructor
   *
   * @param rlp pointer to RoboLookProxy to use for the communication
   */
  RLDisplaySICK( RoboLookProxy *rlp);

  /**
   * Destructor
   */
  ~RLDisplaySICK();

  /**
   * Dislays the scan in RoboLook assuming that you have given a valid
   * RoboLookProxy pointer.
   *
   * @return 0 if OK
   */
  unsigned short calc();

  /**
   * Does the actual displaying. It is made static so that you can
   * call the function without having to instatiate an object
   * 
   * @param scan scan to display
   * @param rlp pointer to RoboLookProxy to use for the communication
   *
   * @return 0 if OK
   */
  static unsigned short display(Cure::SICKScan &scan, RoboLookProxy *rlp);

  /**
   * Must write a SICKScan data on the input or a TimestampedData on
   * the trigger port.
   *
   * @param port the port to check the type on
   * @param inputData the data written to the port
   * 
   * @return 0 if OK
   */
  unsigned short typeCheck(int port,TimestampedData * inputData);

  /**
   * Returns a pointer to the object on the input (assuming port=0),
   * for all other ports 0 is returned.
   */
  Cure::TimestampedData* data(int port);

public:

  /// True if displaying data written to this object is turned off
  bool m_TurnedOff;

protected:

  /// Pointer to RoboLookProxy used for the communication
  RoboLookProxy *m_Rlp;

  /// Scan last written/displayed so that you can read it
  SICKScan m_Scan;
private:

}; // class RLDisplaySICK

} // namespace Cure

#endif // RLDisplaySICK_hh
