//
// = LIBRARY
//
// = FILENAME
//    ScanLineFilter.cc
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

#include "ScanLineFilter.hh"
#include "CureDebug.hh"
#include "SICKScan.hh"
#include "MeasurementSet.hh"
#include "DataFilterAddress.hh"
#include "RangeHough.hh"

#ifndef DEPEND
#include <sstream>
#endif

namespace Cure {

ScanLineFilter::ScanLineFilter()
  :Cure::DataFilter(1, 1, false)
{
  PassDuplicateTimeStamps=false;  
  Ports[0].setInterpolate(false);
  Ports[0].setDataNeeded(false);
  FilterName = "ScanLineFilter";

  m_Hough.setThresholds(0.7, 20, 0.03, 0.0002, 0.0075, 0.017, 0.2, 30, 1);
  m_Hough.setBiasThreshold(3e-3);
}

Cure::TimestampedData* 
ScanLineFilter::data(int port)
{
  //std::cerr << "ScanLineFilter::data(" << port << ")\n";
  if (port == 0) return &m_Scan;
  else return &m_Hough.Lines;
}
unsigned short ScanLineFilter::setOutputs()
{
  if (!PushOut)return 0;
  unsigned short r=0;
  for (int i=Ins; i<(Ins+Outs);i++)
    r=(r|Ports[i].pushData(*data(i)));
  return r;
}

void ScanLineFilter::config( double minlength,
			    short minPointsPerLine,
			    double rhoThreshold,
			    double varRhoThreshold,
			    double halfWidth,
			    double dGamma,
			    double maxGap,
			    double maxrange,
			    short scanResHalfDegrees,
			    double rhoBiasTreshold)
{
  m_Hough.setThresholds(minlength, minPointsPerLine, 
			rhoThreshold, varRhoThreshold,
			halfWidth, dGamma, maxGap,
			maxrange, scanResHalfDegrees);
  m_Hough.setBiasThreshold(rhoBiasTreshold);
}

int 
ScanLineFilter::config(const std::string &arglist)
{
  return m_Hough.config(arglist);
}
void
ScanLineFilter::printConfiguration()
{
  m_Hough.printConfiguration();
}
unsigned short
ScanLineFilter::calc()
{
  CureDO(60) {
    std::cerr << "ScanLineFilter::calc() Time="
              << m_Scan.Time.Seconds << "."
              << m_Scan.Time.Microsec << " "
              << m_Scan.getNPts() << " "
              << m_Scan.getMaxRange() << " "
              << m_Scan.getRangeResolution() << " "
              << m_Scan.getStartAngle() << " "
              << m_Scan.getAngleStep() << " "
              << m_Scan.getScannerType() << " "
              << std::endl;
  }

  /*
  if (PLUTO) {   
    m_Hough.setThresholds(1, 5, 0.09, 0.0027, 0.0075, 0.017, 2.0, 60, 2);
  } else if (GOOFY) {
    m_Hough.setThresholds(0.7, 20, 0.03, 0.0002, 0.0075, 0.017, 0.2, 30, 1);
    m_Hough.setBiasThreshold(3e-3);
  } else if (HOLOS) {
    m_Hough.setThresholds(0.7, 20, 0.03, 0.0002, 0.0075, 0.017, 0.2, 30, 1);
    m_Hough.setBiasThreshold(3e-3);
  }
  */

  PushOut=true;
  if (PassDuplicateTimeStamps) 
    m_Hough.findLines(&m_Scan);
  else{ 
    if (!(m_Scan.Time==LastTime))
      m_Hough.findLines(&m_Scan);
    else PushOut=false;
  } 
  LastTime=m_Scan.Time;
  CureCERR(40) << "Produced "
                << m_Hough.Lines.getNumberOfElements() 
                << " wall measurements from sick scan at time "
                << m_Hough.Lines.Time << std::endl;

  return 0;
}

}
