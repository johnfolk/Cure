//    $Id: CureDefines.hh,v 1.3 2008/10/04 15:47:26 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2005 John Folkesson
//    

#ifndef CURE_DEFINES_HH
#define CURE_DEFINES_HH


/** Code for different data types (ie m_ClassType)*/
#define POSE3D_TYPE 0
#define TIMESTAMP_TYPE 1
#define SICKSCAN_TYPE 2
#define ODOMETRY_TYPE 3
#define RANGE_TYPE 4
#define SONAR_TYPE 5
#define GENERIC_TYPE 6
#define MEASUREMENTSET_TYPE 7
#define VISION_TYPE 8
#define SMARTDATA_TYPE 9
#define SENSORPOSE_TYPE 10
#define COMMAND_TYPE 11
#define GRID_TYPE 12
#define CONTACT_TYPE 13
#define DATASET_TYPE 14
#define STATE_TYPE 15
#define STRING_TYPE 16

/** ERROR Codes */

#define RESOURCE_ERROR       0x0001
#define ADDRESS_INVALID      0x0002
#define TYPE_ERROR           0x0004
#define OBJECT_BUSY          0x0008
#define TIMESTAMP_ERROR      0x0010
#define NO_DATA_AVAILABLE    0x0020
#define TIMESTAMP_TOO_OLD    0x0040  
#define NO_INTERPOLATE       0x0080
#define TIMESTAMP_IN_FUTURE  0x0100
#define THREAD_ERROR      0x0100


/**
 * Error Codes
 * These error types need to be bit or-able. 
 * The errors can be of 3 sorts: 
 *   RESOURCE_ERROR
 *   TIMESTAMP_ERROR
 *   something else
 *
 *  RESOURCE_ERROR indicates that something relatively serious happened
 *  such as:
 *     ADDRESS_INVALID The read or write failed due to address not registered 
 *                     with the AddressBank.  This causes a disconnect.
 *                     Or the read was attempted on an input (or write to an
 *                     output).
 *         TYPE_ERROR  The data was of the wrong type so that it could not
 *                     be used.
 *
 *        OBJECT_BUSY For Multithreading if the object is being used
 *                    this will be returned.
 *
 *   
 *  TIMESTAMP_ERROR indicates some problem matching a requested timestamp 
 *  such as
 * 
 *  NO_DATA_AVAILABLE The timestamp was too young so that the data has not yet
 *                    arrived.  Try again later.
 *  TIMESTAMP_TOO_OLD This is worse than too young since the data will 
 *                    presumably never be available so some action is needed
 *                    to handle not getting stuck tring this timestamp 
 *                    over and over.
 *     NO_INTERPOLATE The request was for an exactly interpolated timestamp
 *                    but the object is not able to comply since it can not 
 *                    interpolate data.  This is also a RESOURCE_ERROR
 *
 * Something else such as 
 *       THREAD_ERROR Patric can write what this means.
 *
 *
 * Some error codes are allways caught (handled and then changed to a less
 * specific code which is passed on to the calling function)
 *
 *  So if an object pulls data (calls read on a Pull Client) from an
 *  Address and gets ADDRESS_INVALID the object disconnects and passes
 *  on an RESOURCE_ERROR to the caller (so that the caller doesn't
 *  mistakenly disconect this objects Address from its (push/pull)
 *  client list.
 *
 * Similarly TYPE_ERRORS are changed to RESOURCE_ERROR but how the
 * object handles the error is up to the object.  Normally only an
 * error message is printed.  This might then happen repeatedly.
 *
 * NO_DATA_AVAILABLE might be caught but it might be passed.  If
 * caught it is changed to TIMSTAMP_ERROR or 0 depending on the
 * context.  Some objects can handle this by for instance setting up a
 * Queue of pending calculations that will be tried later.
 *
 * Normally the pull error codes will be passed on a write (so error in getting
 * the needed input  data.  Push error codes are normally not returned to 
 * the caller of write.
 */

/**
 * For SensorTypes @see SensorData::SensorTypes.
 *  SensorData::SensorTypes 
 * enum
 *     SENSORTYPE_UNKNOWN = 0,
 *     SENSORTYPE_SICK,
 *     SENSORTYPE_CAMERA,
 *     SENSORTYPE_SONAR,
 *     SENSORTYPE_CONTACT,
 *     SENSORTYPE_GPS,
 *     SENSORTYPE_INERTIAL,
 *     SENSORTYPE_RANGEBEARING,
 *     SENSORTYPE_COMPASS,
 *     SENSORTYPE_ROBOT,
 *     SENSORTYPE_ODOMETRY,
 *     SENSORTYPE_POSITION,
 *     SENSORTYPE_ACTUATION,
 *     SENSORTYPE_LONG_BASELINE,
 *     SENSORTYPE_ALTITUDE,
 *     SENSORTYPE_DEPTH,
 *     SENSORTYPE_VELOCITY,
 *     SENSORTYPE_DVL,
 *     SENSORTYPE_FORCE,
 * };
 */

/**
 * For CmdTypes @see Command::CmdTypes.
 *  SensorData::SensorTypes 
 *
 * Defines different types of commands
 *
 * enum CmdTypes {
 *   STATUS = 1,
 *   INIT, 
 *   CONFIGURE,
 *   STOP,
 *   START,
 *   SPEED,
 *   GOTO,
 *   SAVE,
 *   LOAD,
 *   RESET,
 *   GET,
 *   SET,
 *   REPLY,
 *   RELEASE,
 *  };
 *
 */
/**
 * For ServiceTypes @see Command::SerciceTypes
 *
 * Values from 1 to 128 will be reserved for CURE 
 * Values from 128 to 256 will be left free to the users to define
 * enum ServiceTypes {
 *   SERVICE_SLAM = 1,
 *   SERVICE_LOCALIZATION,
 *   SERVICE_GOTO,
 *   SERVICE_DRIVE,
 *   SERVICE_AVOIDANCE,
 *   SERVICE_FOLLOW,
 *   SERVICE_POSE,
 *   SERVICE_PAN_TILT,
 *   SERVICE_ARM,
 *   SERVICE_SICK,
 *   SERVICE_SONAR,
 *   SERVICE_GPS,
 *   SERVICE_INERTIAL,
 *   SERVICE_CAMERA ,
 *   SERVICE_ROBOT,
 *   SERVICE_UNKNOWN,
 *   SERVICE_PATH_PLANNING,
 *   SERVICE_EXPLORE,
 * };
 */

#endif 
