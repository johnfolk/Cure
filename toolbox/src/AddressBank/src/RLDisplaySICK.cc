//
// = LIBRARY
//
// = FILENAME
//    RLDisplaySICK.cc
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

#include "RLDisplaySICK.hh"

#include "CureDebug.hh"
#include "AddressBank.hh"
#include "RoboLookProxy.h"
#include "DataFilterAddress.hh"

#ifndef DEPEND
#endif

namespace Cure {

RLDisplaySICK::RLDisplaySICK(RoboLookProxy *rlp)
  :DataFilter( 1, 0, true),
   m_Rlp(rlp)
{
  m_TurnedOff=false;
  FilterName = "RLDisplaySICK";
  in()->setInterpolate(false);
}

RLDisplaySICK::~RLDisplaySICK()
{}
 
unsigned short 
RLDisplaySICK::calc()
{
  if (m_Rlp == 0) return RESOURCE_ERROR;
  if (m_TurnedOff)return 0;
  return display(m_Scan, m_Rlp);
}

unsigned short
RLDisplaySICK::display(Cure::SICKScan &scan, RoboLookProxy *rlp)
{
  if (rlp == 0) return RESOURCE_ERROR;

  CureCERR(60) << "Displaying SICK scan with time "
               << scan.getTime()
               << std::endl;
  rlp->setLaserScan(rlp->lastRid(), 0, scan.getNPts(),
                    scan.getStartAngle(), scan.getAngleStep(),
                    0, scan.getRanges(), 1.0);

  return 0;
}
unsigned short 
RLDisplaySICK::typeCheck(int port, TimestampedData * inputData)
{
  if (port == 0) {
    if (inputData) {
      if (inputData->getClassType() == SICKSCAN_TYPE) {
        if (inputData->isPacked()) {
          inputData->narrowPackedData()->unpack(*data(port));
        }	else *data(port)=*inputData;   
        return 0;
      } else if (inputData->narrowSmartData()) {
        SmartData *sd = inputData->narrowSmartData();
        if (sd->getTPointer()->getClassType() == SICKSCAN_TYPE) {
          sd->setEqual(*data(port));
          return 0;
        }
      }      
      std::cerr << "\nRLDisplaySICK: WARNING Wrong type of input "
                << (int)inputData->getClassType() << " written to port "
                << port << "\n";
      return TYPE_ERROR;
    }
  }
  return 0;
}

Cure::TimestampedData* 
RLDisplaySICK::data(int port)
{
  if (port != 0) {
    std::cerr << "\nRLDisplaySICK: WARNING Can only access data on port 0, "
              << "not port " << port << std::endl;
    return 0;
  }

  return &m_Scan;
}

} // namespace Cure
