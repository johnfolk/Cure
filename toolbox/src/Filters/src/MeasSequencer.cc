//
// = LIBRARY
//
// = FILENAME
//    MeasSequencer.cc
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

#include "MeasSequencer.hh"
#include "MutexHelper.hh"
#include "SensorData.hh"
#include "CureDebug.hh"

#ifndef DEPEND
#include <iomanip>
#endif

namespace Cure {

MeasSequencer::MeasSequencer()
  :Cure::DataFilter( 1, 1, false),
   m_LastOutputTime(0),
   m_TriggerType(Cure::SensorData::SENSORTYPE_SICK),
   m_TriggerTimeMargin(0)
{
  m_Input = 0;

  FilterName = "MeasSequencer";
}

MeasSequencer::~MeasSequencer()
{}

void 
MeasSequencer::setTriggerType(unsigned short triggerType)
{
  m_TriggerType = triggerType;
}

void
MeasSequencer::setTriggerTimeMargin(double t)
{
  m_TriggerTimeMargin = t;
}

Cure::TimestampedData* 
MeasSequencer::data(int port)
{
  CureCERR(60) << "data(" << port << ") size=" << m_Meas.size() << std::endl;
  if (port == 0) return m_Input;
  else if (port == 1) {
    if (m_Meas.empty()) return 0;
    else return &(m_Meas.front());
  }

  CureCERR(20) << "MeasSequencer::data(" << port << ") port out of bounds\n";
  return 0;
}

unsigned short 
MeasSequencer::write(Cure::TimestampedData& p, const int port)
{
  if (port != 0) {
    CureCERR(20)  << "MeasSequencer::write(...) Can only write to port 0, "
                   << "not " << port << std::endl;
    return ADDRESS_INVALID;
  }

  Cure::MeasurementSet *ms = p.narrowMeasurementSet();
  if (ms == 0) {
    CureCERR(20) << "This filter currently ONLY accepts unpacked "
                 << "MeasurementSets\n";
    return TYPE_ERROR;
  }

  Cure::MutexHelper guard(m_Mutex);

  if (p.Time < m_LastOutputTime) {
    CureDO(20) {
      fprintf(stderr, "\n\n             WARNING: Skipping too old data\n"
                      "             t=%.6f type=%d dt=%f\n\n", 
              p.Time.getDouble(), p.getClassType(),
              m_LastOutputTime.getDouble() - p.Time.getDouble());
    }
    return TIMESTAMP_ERROR;
  }

  CureCERR(50) << "Got " << ms->getNumberOfElements()
               << " meas from sensor " 
               << (ms->getNumberOfElements()>0 ? 
                   ms->Measurements[0].SensorType : -1)
               << " with time " << ms->Time 
               << " already had " << m_Meas.size() << " sets\n";
  
  // Put the new measurement set into the list in the right order wrt time
  std::list<Cure::MeasurementSet>::iterator mi;
  for (mi = m_Meas.begin(); mi != m_Meas.end(); mi++) {
    if (p.Time < mi->Time) {
      mi = m_Meas.insert(mi, *ms);
      m_Input = &(*mi);
      break;
    }
  }

  // Either no mesaurements or this measurement is the newest one and
  // should go to the back of the list
  if (mi == m_Meas.end()) {
    m_Meas.push_back(*ms);
    m_Input = &(m_Meas.back());
  }

  CureCERR(60) << "Now have " << m_Meas.size() << " meas sets\n";
  CureDO(60) {
    int i = 0;
    for (std::list<Cure::MeasurementSet>::iterator mi = m_Meas.begin();
         mi != m_Meas.end(); mi++) {
      fprintf(stderr, "%d: t=%.6f %d\n", 
              i++, mi->Time.getDouble(), 
              (mi->getNumberOfElements()>0?mi->Measurements[0].SensorType:-1));
    }
  }

  if ((ms->getNumberOfElements() > 0 &&
       ms->Measurements[0].SensorType == m_TriggerType) ||
      (ms->getNumberOfElements() == 0 &&
       m_TriggerType == Cure::SensorData::SENSORTYPE_SICK)) {

    Cure::Timestamp trigTime = ms->getTime();
    trigTime -= m_TriggerTimeMargin;

    CureCERR(60) << "Output triggered with " << m_Meas.size() << " sets\n";
    while (!m_Meas.empty() &&  m_Meas.front().Time <= trigTime) {

      CureCERR(50) << "Calling setOutputs with " << m_Meas.size() 
                   << " meas in list, curr meas has time "
                   << m_Meas.front().Time
                   << std::endl;
      
      int ret = Ports[1].pushData(m_Meas.front());
      if (ret & ADDRESS_INVALID) {
        CureCERR(20) << "MeasSequencer tried to write to an Invalid Address. "
                     << "Disconnected" << std::endl;
      }

      if (ret) { CureCERR(30) << "ERROR in setOutput = " << ret << "\n"; }
      m_LastOutputTime = m_Meas.front().Time;
      m_Meas.pop_front();
      CureCERR(60) << "Called setOutputs, now " << m_Meas.size() 
                    << " meas in set\n";
    }
  }

  return 0;
}

}
