//
// = FUNCTION
//
// = RCSID
//    $Id: RoboLookMsg.h,v 1.11 2006/04/30 14:53:01 patric Exp $
//
// = AUTHOR(S)
//    Patric Jensfelt
//
// = VERSION
//    $Revision: 1.11 $
//
// = DATE RELEASED 
//    $Date: 2006/04/30 14:53:01 $
//
// = COPYRIGHT
//    Copyright (c) 2002 Patric Jensfelt
/*----------------------------------------------------------------------*/

#ifndef RoboLookMsg_h
#define RoboLookMsg_h

#define RL_STX_1      0x42
#define RL_STX_2      0x07

#define RL_ENV_TRUE      0  // NOTE that the first must be 0!! Index in array
#define RL_ENV_EST       1

/***************************************************************************/
/**************************** Message IDs **********************************/
/***************************************************************************/

/**
 * enum that lists the different message IDs
 */
enum {
  RL_MsgSceneUpdateInterval_ID = 1,
  RL_MsgCreateRobot_ID,           //  2
  RL_MsgDeleteRobot_ID,           //  3
  RL_MsgClearRobots_ID,           //  4
  RL_MsgRobotPose_ID,             //  5
  RL_MsgEnvPose_ID,               //  6
  RL_MsgShowLaserScan_ID,         //  7
  RL_MsgLaserScan_ID,             //  8
  RL_MsgAddWalls_ID,              //  9
  RL_MsgClearWalls_ID,            // 10
  RL_MsgAddDoors_ID,              // 11
  RL_MsgClearDoors_ID,            // 12
  RL_MsgRobotAlpha_ID,            // 13
  RL_MsgAddEllipses_ID,           // 14
  RL_MsgAddRectangles_ID,         // 15
  RL_MsgForceScreenUpdate_ID,     // 16
  RL_MsgCloseCamRobot_ID,         // 17
  RL_MsgLaserLines_ID,            // 18
  RL_MsgMarkedScanPoints_ID,      // 19  
  RL_MsgClearMarkedScanPoints_ID, // 20
  RL_MsgAddMarkers_ID,            // 21
  RL_MsgClearMarkers_ID,          // 22
  RL_MsgAddLines_ID,              // 23
  RL_MsgClearLines_ID,            // 24
  RL_MsgClearRectangles_ID,       // 25
  RL_MsgAddCylPillars_ID,         // 26
  RL_MsgClearCylPillars_ID,       // 27
  RL_MsgScanColors_ID,            // 28
  RL_MsgSonarRanges_ID,           // 29
  RL_MsgBumperState_ID,           // 30
  RL_MsgClearEllipses_ID,         // 31
  RL_MsgLaserScanColor_ID,        // 32
  RL_MsgAddCubes_ID,              // 33
  RL_MsgClearCubes_ID,            // 34
  RL_MsgAddStars_ID,              // 35
  RL_MsgClearStars_ID,            // 36
  RL_MsgExtraRobotDOFs_ID,        // 37
  RL_MsgShowScanCoords_ID,        // 38
  RL_MsgGrabSingleFrame_ID,       // 39
  RL_MsgStartGrabbing_ID,         // 40
  RL_MsgStopGrabbing_ID,          // 41
  RL_MsgResetFrameCount_ID,       // 42
  RL_MsgSetGhostState_ID,         // 43
  RL_MsgDeleteAllRobots_ID,       // 44
  RL_MsgShowScanLines_ID,         // 45
  RL_MsgAddEllipsoids_ID,         // 46
  RL_MsgClearEllipsoids_ID,       // 47
  RL_MsgAddPoints_ID,             // 48
  RL_MsgClearPoints_ID,           // 49
  RL_MsgAddTriangles_ID,          // 50
  RL_MsgClearTriangles_ID,        // 51
  RL_MsgAddRGBPoints_ID,          // 52
};

/***************************************************************************/
/****************************** Message Head *******************************/
/***************************************************************************/
/**
 * Message head for all messages
 */
typedef struct {
  unsigned short STX;
  unsigned short id;
  int len;
  unsigned short crc;
} RL_MsgHead;

/***************************************************************************/
/******************************* Functions *********************************/
/***************************************************************************/

/**
 * Added STX to the messages
 */
inline void RL_addSTX(RL_MsgHead *head) {
  *((char*)head) = RL_STX_1;
  *(((char*)head) + 1) = RL_STX_2;
}

/***************************************************************************/
/**************************** Message structs ******************************/
/***************************************************************************/

/**
 * Message to set RoboLook Server to update interval for screen
 */
typedef struct {
  RL_MsgHead head;
  double interval;
} RL_MsgSceneUpdateInterval;

/**
 * Create a robot with a certain id and type
 */
typedef struct {
  RL_MsgHead head;
  int env;
  int rid;
  char type[64];
} RL_MsgCreateRobot;

/**
 * Delete a robot with a certain id
 */
typedef struct {
  RL_MsgHead head;
  int rid;
} RL_MsgDeleteRobot;

/**
 * Set the ghost state of a robot
 */
typedef struct {
  RL_MsgHead head;
  int rid;
  short on;
} RL_MsgGhostState;

/**
 * Remove all robots in a given environment
 */
typedef struct {
  RL_MsgHead head;
  int env;
} RL_MsgClearRobots;

/**
 * Set the pose of a given robot
 */
typedef struct {
  RL_MsgHead head;
  int env;  // environment identifier
  int rid;  // robot id
  double x;
  double y;
  double z;
  double a;
} RL_MsgRobotPose;

/**
 * Set the position of a given environment. Is handy when you want to
 * have an estimated environment overlay an existing one.
 */
typedef struct {
  RL_MsgHead head;
  int env;  // environment identifier
  double x;
  double y;
  double z;
  double a;
} RL_MsgEnvPose;

/**
 * Give the laser scan for a certain robot and laser sensor
 */
typedef struct {
  RL_MsgHead head;
  int rid;  // robot id
  int lid;  // laser id
  int n;
  double startAngle;
  double angleStep;
  double timeStamp;
  // Here follows the laser range data in the form of an array of
  // floats, ranges expressed in m
} RL_MsgLaserScan;

/**
 * Mark some of the laser scan points with a certain color for a
 * certain robot and laser sensor
 */
typedef struct {
  RL_MsgHead head;
  int rid;  // robot id
  int lid;  // laser id
  int color;
  int n;
  // Here follows the marked points as an array of unsigned shorts
} RL_MsgMarkedScanPoints;

/**
 * Mark all laser scan points in colors specified by the 
 */
typedef struct {
  RL_MsgHead head;
  int rid;  // robot id
  int lid;  // laser id
  int n;
  // Here follows the an array of unsigned shorts with the color code
  // of the scan points
} RL_MsgScanColors;

/**
 * Make all laser scan points have the defualt color again
 */
typedef struct {
  RL_MsgHead head;
  int rid;  // robot id
  int lid;  // laser id
} RL_MsgClearMarkedScanPoints;

/**
 * Turn show laser scan on/off for a certain robot and laser scanner
 */
typedef struct {
  RL_MsgHead head;
  int rid;  // robot id
  int lid;  // laser id
  int show;
} RL_MsgShowLaserScan;

/**
 * Set color for laser scan
 */
typedef struct {
  RL_MsgHead head;
  int rid;  // robot id
  int lid;  // laser id
  int color;// color for laser scan, colors defined in RLColors.cc
} RL_MsgLaserScanColor;

/**
 * Define some laser lines that are drawn in the sensor coordinate system
 */
typedef struct {
  RL_MsgHead head;
  int rid;
  int lid;
  int n;
  // Here after follows an array of RL_LineItems
} RL_MsgLaserLines;

/**
 * Remove all wall items from a certain environment
 */
typedef struct {
  RL_MsgHead head;
  int env;
} RL_MsgClearWalls;

/**
 * Remove all door items from a certain envrionment
 */
typedef struct {
  RL_MsgHead head;
  int env;
} RL_MsgClearDoors;

/**
 * Set the blening factor for a certain robot. Can be used to make it
 * transparent
 */
typedef struct {
  RL_MsgHead head;
  int rid;
  float alpha;
} RL_MsgRobotAlpha;

/**
 * Connect a camera close to a certain robot
 */
typedef struct {
  RL_MsgHead head;
  int rid;
} RL_MsgCloseCamRobot;

/**
 * This message is used to add items, e.g. doors, walls, to a certain
 * environment. The id in the head tells what items to expect
 */
typedef struct {
  RL_MsgHead head;
  int env;
  int replace;
  int n;
  // Here after follows an array of some RL_*Items
} RL_MsgAddItems;

/**
 * Clear items of a certain type from a certain ennvironment. The id
 * in the head tells what items to clear.
 */
typedef struct {
  RL_MsgHead head;
  int env;
} RL_MsgClearItems;

/**
 * Message with sonar range readings
 */
typedef struct {
  RL_MsgHead head;
  int rid;
  int n;
  short gotIDs;
  // Here follows two arrays
  // 1) an array of (double) with ranges
  // 2) an array of (unsigned short) that holds in indices of the sonars 
  // If withIDs=0 the second array is not included and it assumed that
  // the ranges come for sonar 0, 1, 2, ..., n
} RL_MsgSonarRanges;

/**
 * Message with bumper data
 */
typedef struct {
  RL_MsgHead head;
  int rid;
  int state;
} RL_MsgBumperState;

/**
 * Message that lets you set up to 30 extra d.o.f. at a time for a robot. 
 */
typedef struct {
  RL_MsgHead head;
  int rid;
  int n; // number of dofs that are set
  int offset;
  double value[30];
} RL_MsgExtraRobotDOFs;


/***************************************************************************/
/****************************** DATA ITEMS *********************************/
/***************************************************************************/


/**
 * Structure for a 3D point
 */
struct _RL_PointItem {
  
  _RL_PointItem()
  {
    x = 0;
    y = 0;
    z = 0;
    size = 1.0;
    color = 1;
  }

  float x;
  float y;
  float z;
  float size;
  unsigned short color;
};
typedef struct _RL_PointItem RL_PointItem;

/**
 * Structure for a 3D point with RGB colors
 */
struct _RL_RGBPointItem {
  
  _RL_RGBPointItem()
  {
    x = 0;
    y = 0;
    z = 0;
    size = 1.0;
    color[0] = 0;
    color[1] = 0;
    color[2] = 0;
  }

  float x;
  float y;
  float z;
  float size;
  unsigned char color[3]; // RGB values
};
typedef struct _RL_RGBPointItem RL_RGBPointItem;

/**
 * Defines a wall which is a plane that stretches from zMin to zMax in
 * height. All values in m and rad
 */
typedef struct {
  double xS;
  double yS;
  double xE;
  double yE;
  double zMin;  // z-coordinate relative to the floor
  double zMax;  // z-coordinate relative to the floor

  double lineHeight;         // height of line approx of the wall above ground
  double lineWidth;          // width of line approx of the wall
  unsigned short lineColor;  // color of line approx of the wall

  // Can be used to turn on and off certain items depending on their quality
  unsigned short quality;    

  short floor; // Not used unless turned on in RLEnvironment.cc
} RL_WallItem;

/**
 * A Line. All values in m and rad
 */
typedef struct {
  double xS;
  double yS;
  double zS;  
  double xE;
  double yE;
  double zE;  

  unsigned short color;
  float width;

  short floor; // Not used unless turned on in RLEnvironment.cc
} RL_LineItem;

/**
 * A door item. All values in m and rad
 */
typedef struct {
  double xC;
  double yC;
  double angle;
  double width;
  double zMin;  // z-coordinate relative to the floor
  double zMax;  // z-coordinate relative to the floor
  short hasLeaf;
  short rightSideHinges;
  double leafAngle;

  short floor; // Not used unless turned on in RLEnvironment.cc
} RL_DoorItem;

/**
 * A cylindrical pillar
 */
typedef struct {
  float x;
  float y;
  float z;
  float h;  
  float r;  
  short color;  

  double lineHeight;  // height of line approx of the cyl pillar above ground

  short floor; // Not used unless turned on in RLEnvironment.cc
} RL_CylPillarItem;

/**
 * An ellipse defined by its minor and major axis. All values in m and rad
 */
typedef struct {
  float xC;
  float yC;
  float z;
  float major;
  float minor;
  float majorAngle;
  short style;
  short color;

  short floor; // Not used unless turned on in RLEnvironment.cc
} RL_EllipseItem;

/**
 * An ellipsoid 
 */
struct _RL_EllipsoidItem {

  _RL_EllipsoidItem()
  {
    xC = 0;
    yC = 0;
    zC = 0;

    axis[0] = 0;
    axis[1] = 0;
    axis[2] = 0;

    eulerAngle[0] = 0;
    eulerAngle[1] = 0;
    eulerAngle[2] = 0;

    color = 1;
    color2 = 2;

    floor = 0;
  }

  float xC;
  float yC;
  float zC;

  float axis[3];
  float eulerAngle[3];

  short color;
  short color2;

  short floor; // Not used unless turned on in RLEnvironment.cc
};
typedef struct _RL_EllipsoidItem RL_EllipsoidItem;

/**
 * A rectangle. All values in m and rad
 */
typedef struct {
  float xC;
  float yC;
  float z;
  float xLen;
  float yLen;
  float xAngle;
  short style;
  short color;

  short floor; // Not used unless turned on in RLEnvironment.cc
} RL_RectangleItem;

/**
 * A marker which is used to mark things such as places to go to or
 * places we have been to, nodes in a graph etc. All values in m and rad
 */
typedef struct {
  float x;
  float y;
  float z;
  float w;  // half width of the square marker
  short color;  

  short floor; // Not used unless turned on in RLEnvironment.cc
} RL_MarkerItem;

/**
 * A cube which can be used to mark point features for example. All
 * values in m and rad
 */
typedef struct {
  float x;
  float y;
  float z;
  float w;     // half length of the side of the cube
  short color;  

  short floor; // Not used unless turned on in RLEnvironment.cc
} RL_CubeItem;

/**
 * A cube which can be used to mark point features for example. All
 * values in m and rad
 */
typedef struct {
  float x;
  float y;
  float z;
  float w;     // half width of the star
  short color;  

  short floor; // Not used unless turned on in RLEnvironment.cc
} RL_StarItem;

/**
 * A triangle defined by 3 points. The points should be defined such
 * that they are arranged counterclockwise when looking at the front
 * side of the triangle.
 */
struct _RL_TriangleItem {
  _RL_TriangleItem()
  {
    x1 = 0;    y1 = 0;    z1 = 0;
    x2 = 0;    y2 = 0;    z2 = 0;
    x3 = 0;    y3 = 0;    z3 = 0;
    color[0] = 255; color[1] = 0; color[2] = 0; color[3] = 0;
  }

  float x1;
  float y1;
  float z1;
  float x2;
  float y2;
  float z2;
  float x3;
  float y3;
  float z3;

  unsigned char color[4];  // RGB + shininess
}; 
typedef _RL_TriangleItem RL_TriangleItem;



#endif



