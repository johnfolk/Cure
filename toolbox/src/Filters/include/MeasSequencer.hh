//
// = LIBRARY
//
// = FILENAME
//    MeasSequencer.hh
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

#ifndef MeasSequencer_hh
#define MeasSequencer_hh

#include "DataFilter.hh"
#include "DataFilterAddress.hh"
#include "MutexWrapper.hh"

#ifndef DEPEND
#include <list>
#endif


namespace Cure {

// Forward declaration(s)

/**
 * This filter helps sequenze measurements so that they come out
 * ordered by their timestamp on the output. A write to the filter
 * might thus not cause any output from the filter. The filter is
 * configured by telling it which measurement type has the biggest
 * delay. When measurements of that type arrives the measurements that
 * have come in before that but have timestamps older than this will
 * be output. 
 *
 * Users of this filter can be pushed one measurement for each
 * measurement that triggers an output, but it is then up to the user
 * to read the rest manually.
 * 
 * We assume that all measurements in a set is of the same type
 *
 * @author Patric Jensfelt 
 * @see
 */
class MeasSequencer : public Cure::DataFilter {
public:
  /**
   * 
   * 
   */
  MeasSequencer();
  ~MeasSequencer();

  /**
   * Specify which measurement type should trigger the sequenzer to
   * start outputting stuff on the output side. For example if you have
   * lines extracted from a laser scan that arrives to the computer
   * 200ms after the scan was acquired due to a slow serial link you
   * might already have received other types of measurements such as
   * from vision. To make life easier for the SLAM algorithm you want
   * them sequenzed in time.
   */
  void setTriggerType(unsigned short triggerType);

  /**
   * In cases where you have multiple sensors coming in with
   * information with approximately the same delay you might want to
   * add a bit of time margin, T, to the triggering. When sensors data of
   * the trigger type arrives all data up to the time of the trigger
   * data minus T will be output.
   */
  void setTriggerTimeMargin(double t);

  /**
   * Add a measurement set to the sequenzer. If the measurement is of
   * the type that we synch with we write something to the output,
   * otherwise we just put it in the list of yet unprocessed
   * measurements.
   *
   * @param p Data to be copied
   * @param port must be an input port number
   * @return 0 if ok, else ADDRESS_INVALID.
   */
  virtual unsigned short write( Cure::TimestampedData& p,const int port=0);

protected:

  Cure::TimestampedData* data(int port);
protected:
  /**
   * The list of measurements that have not yet been output
   */
  std::list<Cure::MeasurementSet> m_Meas;

  /**
   * The timestamp for the last measurement that was read on the
   * output
   */
  Cure::Timestamp m_LastOutputTime;

  /**
   * The type of measurement that we trigger an output on.
   */
  unsigned short m_TriggerType;

  /**
   * The time margin for triggering. When data of m_TriggerType is
   * written to the input all data up to its timestamp -
   * m_TriggerTimeMargin is output.
   */
  Cure::Timestamp m_TriggerTimeMargin;

private:
  Cure::MeasurementSet *m_Input;

  Cure::MutexWrapper m_Mutex;

}; // class MeasSequencer

}; // class Cure

#endif // MeasSequencer_hh
