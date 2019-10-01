//
// = LIBRARY
//
// = FILENAME
//    SICKScan.cc
//
// = AUTHOR(S)
//    Patric Jensfelt
//
// = COPYRIGHT
//    Copyright (c) 2004 Patric Jensfelt
//
/*----------------------------------------------------------------------*/

#include "SICKScan.hh"
#include "SICKDefines.h"

#ifndef DEPEND
#include <string.h>  // memcpy
#include <math.h>    // M_PI
#include <stdlib.h>  // srand, rand, RAND_MAX
#include <time.h>    // time
#include <stdio.h>   // fprintf
#endif

namespace Cure {

SICKScan::SICKScan(unsigned short id)
  :SensorData(SensorData::SENSORTYPE_SICK, id)
{
  init();
}

SICKScan::SICKScan(Timestamp &t, unsigned short id)
  :SensorData(SensorData::SENSORTYPE_SICK, id)
{
  init();
  Time=t;
}

/**
 * Create a copy of a SICKScan
 */
SICKScan::SICKScan(const SICKScan &src)
  :SensorData(SensorData::SENSORTYPE_SICK, src.SensorID)
{
  init();
  (*this) = src;
}

SICKScan::~SICKScan()
{}

void SICKScan::init()
{
  m_Packed =false;
  m_ClassType=(SICKSCAN_TYPE);
  setSubType(0);
  setID(0);
  SensorType=SENSORTYPE_SICK;
  m_ScannerType = SCANNER_TYPE_LMS200;
  m_MaxRange = 32;
  m_StartAngle = 0;
  m_AngleStep = 0;
  m_RangeResolution = 0.001;
  m_NPts=0;
  m_NFlags=0;
  m_NIntensityLevels=0;

  calcSubType();
}

SICKScan& SICKScan::operator=(const SICKScan &src)
{
  Time=src.Time;
  setSubType(src.getSubType());
  setID(src.getID());
  m_NPts = src.m_NPts;

  m_AngleStep = src.m_AngleStep;
  m_StartAngle = src.m_StartAngle;
  m_MaxRange = src.m_MaxRange;
  m_RangeResolution = src.m_RangeResolution;

  m_NFlags = src.m_NFlags;

  m_NIntensityLevels = src.m_NIntensityLevels;

  if (src.m_NPts > 0) {
    memcpy(m_Ranges, src.m_Ranges, sizeof(double) * src.m_NPts);
    memcpy(m_Flags, src.m_Flags, sizeof(unsigned char) * src.m_NPts);
    memcpy(m_Intensities, src.m_Intensities, sizeof(unsigned char)*src.m_NPts);
  }

  return (*this);
}

void
SICKScan::setNumberOfPoints(unsigned short num)
{
  m_NPts = num;
  calcSubType();
}

void SICKScan::setSubType(unsigned short t)
{
  m_NPts=(t&0x3FF);
  if (m_NPts>721){
    t=(t&0xFC00);	   
    m_NPts=721;
    t=(t|m_NPts);
  }
  m_SubType = t;
}
void SICKScan::calcSubType()
{
  if (m_NPts>721)m_NPts=721;
  unsigned long t=0;
  if (m_NFlags>0) t++;
  if (m_NIntensityLevels>0) t+=2;
  t=(t<<10);
  t+=m_NPts;
  m_SubType = t;
}

bool 
SICKScan::import(Timestamp timestamp,
                 int nPts,
                 double *ranges,
                 double angleStep,
                 double startAngle,
                 double maxRange,
                 double rangeResolution,
                 int nFlags,
                 int nIntensityLevels,
                 unsigned char *flags,
                 unsigned char *intensities )
{
  Time = timestamp;
  m_NPts = nPts;
  m_AngleStep = angleStep;
  m_StartAngle = startAngle;
  m_MaxRange = maxRange;
  m_RangeResolution = rangeResolution;

  memcpy(m_Ranges, ranges, nPts * sizeof(double));

  m_NFlags = nFlags;
  if (flags != NULL){
    memcpy(m_Flags, flags, nPts);
  }
  
  m_NIntensityLevels = nIntensityLevels;
  if (intensities != NULL) {
    memcpy(m_Intensities, intensities, nPts);
  }
  calcSubType();
  return true;
}

bool
SICKScan::createRandomMeasurement(double minRange, double maxRange)
{
  Time = 123.456789;

  m_NPts = 361;
  m_AngleStep = 0.5 * M_PI / 180.0;
  m_StartAngle = 0;
  m_MaxRange = maxRange;
  m_RangeResolution = 0.001;

  memset(m_Flags, 0, m_NPts);
  memset(m_Intensities, 0, m_NPts);

  m_NIntensityLevels = 0;
  m_NFlags = 0;

  // "Inter point noise"
  const double ipn = 0.15;
  const double newStructProb = 0.01;
  
  srand(time(0));
  m_Ranges[0] = minRange + ((maxRange - minRange) * rand() / (RAND_MAX + 1.0));
  for (int j = 1; j < 361; j++) {
    if (rand() / (RAND_MAX + 1.0) > newStructProb) {
      m_Ranges[j] = m_Ranges[j-1] + (ipn - (2.0*ipn * rand()/(RAND_MAX+1.0)));
    } else {
      m_Ranges[j] = minRange + ((maxRange - minRange) * rand()/(RAND_MAX+1.0));
    }
    if (m_Ranges[j] < minRange) m_Ranges[j] = minRange;
    else if (m_Ranges[j] > maxRange) m_Ranges[j] = maxRange;
  }
  calcSubType();
  return true;
}


bool SICKScan::interpretMeas(// Inputs
                             bool PLS,
                             int nPts, 
                             const unsigned short *meas,
                             short partial,
                             short scanPart,
                             short scanVariant,
                             short measMode,
                             short measUnit,
                             short outUnit,
                             // Outputs
                             double &angleStep,
                             double &startAngle,
                             double &maxRange,
                             double &rangeResolution,
                             double *ranges,
                             int &intensityLevels,
                             int &nFlags,
                             unsigned char *intensities,
                             unsigned char *flags)
{
  if (PLS) {
    fprintf(stderr, "Not implemented interpretMeas for PLS\n");
    return false;
  }

  double maxRangeInM = 8;
  unsigned char *rawBuf = (unsigned char*)meas;

  switch(measMode) {
  case SICKMeasMode_8m_FA_FB_dazzle:
    maxRangeInM = 8;
    intensityLevels = 0;
    nFlags = 2;
    for (int i = 0; i < nPts; i++) ranges[i] = meas[i] & 0x1FFF;
    if (intensities != NULL) 
      memset(intensities, 0, sizeof(unsigned char) * nPts);
    if (flags != NULL) 
      for (int i = 0; i < nPts; i++) 
        flags[i] = (rawBuf[i * 2 + 1] >> 5);
    break;
  case SICKMeasMode_8m_intensity:
    maxRangeInM = 8;
    intensityLevels = 8;
    nFlags = 0;
    for (int i = 0; i < nPts; i++) ranges[i] = meas[i] & 0x1FFF;
    if (intensities != NULL) 
      for (int i = 0; i < nPts; i++) 
        intensities[i] = (rawBuf[i * 2 + 1] >> 5);
    if (flags != NULL) 
      memset(flags, 0, sizeof(unsigned char) * nPts);
    break;
  case SICKMeasMode_8m_FA_FB_FC:
    maxRangeInM = 8;
    intensityLevels = 0;
    nFlags = 3;
    for (int i = 0; i < nPts; i++) ranges[i] = meas[i] & 0x1FFF;
    if (intensities != NULL) 
      memset(intensities, 0, sizeof(unsigned char) * nPts);
    if (flags != NULL) 
      for (int i = 0; i < nPts; i++) 
        flags[i] = (rawBuf[i * 2 + 1] >> 5);
    break;
  case SICKMeasMode_16m_intensity:
    maxRangeInM = 16;
    intensityLevels = 4;
    nFlags = 0;
    for (int i = 0; i < nPts; i++) ranges[i] = meas[i] & 0x3FFF;
    if (intensities != NULL) 
      for (int i = 0; i < nPts; i++) 
        intensities[i] = (rawBuf[i * 2 + 1] >> 6);
    if (flags != NULL) 
      memset(flags, 0, sizeof(unsigned char) * nPts);
    break;
  case SICKMeasMode_16m_FA_FB:
    maxRangeInM = 16;
    intensityLevels = 0;
    nFlags = 2;
    for (int i = 0; i < nPts; i++) ranges[i] = meas[i] & 0x3FFF;
    if (intensities != NULL) 
      memset(intensities, 0, sizeof(unsigned char) * nPts);
    if (flags != NULL) 
      for (int i = 0; i < nPts; i++) 
        flags[i] = (rawBuf[i * 2 + 1] >> 6);
    break;
  case SICKMeasMode_32m_intensity:
    maxRangeInM = 32;
    intensityLevels = 2;
    nFlags = 0;
    for (int i = 0; i < nPts; i++) ranges[i] = meas[i] & 0x7FFF;
    if (intensities != NULL) 
      for (int i = 0; i < nPts; i++) 
        intensities[i] = (rawBuf[i * 2 + 1] >> 7);
    if (flags != NULL) 
      memset(flags, 0, sizeof(unsigned char) * nPts);
    break;
  case SICKMeasMode_32m_FA:
    maxRangeInM = 32;
    intensityLevels = 0;
    nFlags = 1;
    for (int i = 0; i < nPts; i++) ranges[i] = meas[i] & 0x7FFF;
    if (intensities != NULL) 
      memset(intensities, 0, sizeof(unsigned char) * nPts);
    if (flags != NULL) 
      for (int i = 0; i < nPts; i++) flags[i] = (rawBuf[i * 2 + 1] >> 7);
    break;
  case SICKMeasMode_32m_brakefield:
    maxRangeInM = 32;
    intensityLevels = 0;
    nFlags = 1;
    for (int i = 0; i < nPts; i++) ranges[i] = meas[i] & 0x7FFF;
    if (intensities != NULL) 
      memset(intensities, 0, sizeof(unsigned char) * nPts);
    if (flags != NULL) 
      for (int i = 0; i < nPts; i++) flags[i] = (rawBuf[i * 2 + 1] >> 7);
    break;
  case SICKMeasMode_32m_immediate:
    maxRangeInM = 32;
    intensityLevels = 0;
    nFlags = 0;
    for (int i = 0; i < nPts; i++) ranges[i] = meas[i] & 0x7FFF;
    if (intensities != NULL) 
      memset(intensities, 0, sizeof(unsigned char) * nPts);
    if (flags != NULL) 
      memset(flags, 0, sizeof(unsigned char) * nPts);
    break;
  default:
    fprintf(stderr, "SICKHelpFunctions::interpretMeas() "
            "Not handling meas mode 0x%X\n", measMode);
    return false;
  }

  switch (outUnit) {
  case SICKMeasUnit_mm:
    maxRange = 1000.0 * maxRangeInM;
    rangeResolution = 0.001;
    break;
  case SICKMeasUnit_cm:
    maxRange = 100.0 * maxRangeInM;
    rangeResolution = 0.01;
    break;
  case SICKMeasUnit_dm:
    maxRange = 10.0 * maxRangeInM;
    rangeResolution = 0.1;
    break;
  case SICKRangeUnit_m:
    maxRange = maxRangeInM;
    rangeResolution = 1.0;
    break;
  }

  if (outUnit != measUnit) {
    double scaleFactor = 1.0;
    switch (measUnit) {
    case SICKMeasUnit_mm:
      rangeResolution = 0.001;
      if (outUnit == SICKMeasUnit_cm)
        scaleFactor = 0.1;
      else if (outUnit == SICKMeasUnit_dm)
        scaleFactor = 0.01;
      else if (outUnit == SICKRangeUnit_m)
        scaleFactor = 0.001;
      break;
    case SICKMeasUnit_cm:
      rangeResolution = 0.01;
      if (outUnit == SICKMeasUnit_mm)
        scaleFactor = 10.0;
      else if (outUnit == SICKMeasUnit_dm)
        scaleFactor = 0.1;
      else if (outUnit == SICKRangeUnit_m)
        scaleFactor = 0.01;
      break;
    case SICKMeasUnit_dm:
      rangeResolution = 0.1;
      if (outUnit == SICKMeasUnit_mm)
        scaleFactor = 100.0;
      else if (outUnit == SICKMeasUnit_cm)
        scaleFactor = 10.0;
      else if (outUnit == SICKRangeUnit_m)
        scaleFactor = 0.1;
      break;
    default:
      fprintf(stderr, "SICKHelpFunctions::interpretMeas() "
              "Unknown meas unit %d\n", measUnit);
      calcSubType();
      return false;
    };
    for (int i = 0; i < nPts; i++)
      ranges[i] *= scaleFactor;
  }

  if (partial) { 
    fprintf(stderr, "SICKHelpFunctions::interpretMeas() "
            "Not handling partial scans yet\n");
    calcSubType();
    return false;
  }

  if (scanVariant == SICKLMSVariant_180_50) {
    startAngle = 0;
    angleStep = 0.5 * M_PI / 180.0;
  } else {
    fprintf(stderr, "Not handling scanVariant=%d yet\n", scanVariant);
    calcSubType();
    return false;
  }
  calcSubType();
  return true;
}

} // namespace Cure

std::ostream& 
operator << (std::ostream& os, const Cure::SICKScan &scan)
{
  os << "n=" << scan.getNPts() << std::endl
     << "StartAngle=" << scan.getStartAngle() << std::endl
     << "AngleStep=" << scan.getAngleStep() << std::endl
     << "nFlags=" << scan.getNFlags() << std::endl
     << "nIntensityLevels=" << scan.getNIntensityLevels() << std::endl
     << "ranges = [";
  for (int i = 0; i < scan.getNPts(); i++) {
    os << scan.getRange(i) << " ";
  }
  os << "]";

  return os;
}
