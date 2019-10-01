//
// = LIBRARY
//
// = FILENAME
//    SICKDefines.h
//
// = AUTHOR(S)
//    Patric Jensfelt
//
// = COPYRIGHT
//    Copyright (c) 2004 Patric Jensfelt
//
/*----------------------------------------------------------------------*/

#ifndef CURE_SICKDEFINES_H
#define CURE_SICKDEFINES_H

/**
 * Enumeration of different setups for FOV and resolution. The first
 * value in the name refers to the FOV in degrees and the second to
 * the angular spacing in 1/100 of degrees.
 */
enum SICKLMSVariant {
  SICKLMSVariant_180_50 = 0,
  SICKLMSVariant_180_100,
  SICKLMSVariant_100_25
};

/**
 * The unit used both to set what using the sensor should work with
 * but also what units to deleiver the result in
 */
enum SICKMeasUnit {
  SICKMeasUnit_cm = 0,
  SICKMeasUnit_mm,
  SICKMeasUnit_dm,
  
  // NOT supported by the SICK as measurement unit
  SICKRangeUnit_m
};
  
/**
 * Enumeration of the different measurement modes for the SICK
 */
enum SICKMeasMode {
  SICKMeasMode_8m_FA_FB_dazzle   = 0,  // 0
  SICKMeasMode_8m_intensity,           // 1
  SICKMeasMode_8m_FA_FB_FC,            // 2
  SICKMeasMode_16m_intensity,          // 3
  SICKMeasMode_16m_FA_FB,              // 4
  SICKMeasMode_32m_intensity,          // 5
  SICKMeasMode_32m_FA,                 // 6
  SICKMeasMode_32m_brakefield   = 10,  // 10
  SICKMeasMode_32m_immediate    = 15,  // 15
  SICKMeasMode_Reflectivity     = 63   // 63
};


/**
 * Enumeration of the different operation modes for the SICK sensors
 */
enum SICKOpMode {
  SICKOpMode_Installation          = 0,
  SICKOpMode_Calibration,
  SICKOpMode_Diagnostic,
  SICKOpMode_Continous             = 0x24,
  SICKOpMode_ValuesOnRequest,
  SICKOpMode_ContinousPartialScans = 0x2A,
  SICKOpMode_ContinousPartialRefl  = 0x2B,
  SICKOpMode_9600baud              = 0x40,
  SICKOpMode_19200baud,
  SICKOpMode_38400baud,
  SICKOpMode_500000baud            = 0x48
};

/**
 * Enumeration of different responses to operation mode changes
 */
enum SICKOpModeChangeResponses {
  SICKOpModeChange_succ            = 0,
  SICKOpModeChange_fail_passwd,
  SICKOpModeChange_fail_fault
};

/**
 * Structure that comes from the SICK sensor when you ask for its status
 */
typedef struct {
  char swVersion[7];                    // 0 
  unsigned char opMode;                 // 7
  unsigned char status;                 // 8
  char prodCode[8];                     // 9 
  unsigned char varByte;                // 17
  unsigned short pollVal[8];            // 18
  unsigned short refPollVals[4];        // 34
  unsigned short pollChanCalVal[8];     // 42
  unsigned short refPollChanCalVal[4];  // 58
  unsigned short motorSpeed;            // 66
  unsigned short dummy1;                // 68
  unsigned short refChan1dark100;       // 70
  unsigned short dummy2;                // 72
  unsigned short refChan2dark100;       // 74
  unsigned short refChan1dark66;        // 76
  unsigned short dummy3;                // 78
  unsigned short refChan2dark66;        // 80
  unsigned short dummy4;                // 82
  unsigned short signalAmp;             // 84
  unsigned short peakThrshold;          // 86
  unsigned short angleMeas;             // 88
  unsigned short calValSignalAmp;       // 90
  unsigned short expStopThres;          // 92
  unsigned short expPeakThres;          // 94
  unsigned short actStopThres;          // 96
  unsigned short actPeakThres;          // 98
  unsigned char dummy9;                 // 100
  unsigned char measMode;               // 101
  unsigned short dummy10;               // 102
  unsigned short dummy11;               // 104
  unsigned short scanAngleDegs;         // 106
  unsigned short singleShotRes;         // 108
  unsigned char restartMode;            // 110
  unsigned short restartTime;           // 111
  unsigned char restartTest;            // 113
  unsigned char def3output;             // 114
  unsigned short baudRate;              // 115
  unsigned char evalNmb;                // 117
  unsigned char permBaudRate;           // 118
  unsigned char sensAddr;               // 119
  unsigned char fieldSetNo;             // 120
  unsigned char currentUnit;            // 121
  unsigned char laserSwitching;         // 122
  char swVersion2[7];                   // 123
  // There is more data here but I do not know how to interpret it anyway
} SICKStatusData;

/**
 * Stucture used to set/aks for the SICK sensor configuration
 */
typedef struct {
  unsigned short blanking;           // 0
  unsigned short stopThreshold;      // 2
  unsigned char fogCorr;             // 4
  unsigned char measMode;            // 5
  unsigned char measUnits;           // 6
  unsigned char tempField;           // 7
  unsigned char subFieldAB;          // 8
  unsigned char multEval;            // 9
  unsigned char restart;             // 10
  unsigned char restartTime;         // 11
  unsigned char restartTest;         // 12
  unsigned char contourAObjSize;     // 13
  unsigned char contourAPosTol;      // 14
  unsigned char contourANegTol;      // 15
  unsigned char contourAStartAngle;  // 16
  unsigned char contourAStopAngle;   // 17
  unsigned char contourBObjSize;     // 18
  unsigned char contourBPosTol;      // 19
  unsigned char contourBNegTol;      // 20
  unsigned char contourBStartAngle;  // 21
  unsigned char contourBStopAngle;   // 22
  unsigned char contourCObjSize;     // 23
  unsigned char contourCPosTol;      // 24
  unsigned char contourCNegTol;      // 25
  unsigned char contourCStartAngle;  // 26
  unsigned char contourCStopAngle;   // 27
  unsigned char pixelEval;           // 28
  unsigned char singleMeasMode;      // 29
  short zeroPointOffset;             // 30
} SICKConfigData;

#endif  // CURE_SICKDEFINES_H
