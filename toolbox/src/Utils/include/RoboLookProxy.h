//
// = FUNCTION
//
// = RCSID
//    $Id: RoboLookProxy.h,v 1.13 2007/09/14 09:14:06 johnf Exp $
//
// = AUTHOR(S)
//    Patric Jensfelt
//
// = VERSION
//    $Revision: 1.13 $
//
// = DATE RELEASED 
//    $Date: 2007/09/14 09:14:06 $
//
// = COPYRIGHT
//    Copyright (c) 2002 Patric Jensfelt
/*----------------------------------------------------------------------*/

#ifndef RoboLookProxy_h
#define RoboLookProxy_h

#include "RoboLookMsg.h"

#ifndef DEPEND
#include <string>
#endif

/**
 * Class that provides a socket interface to the the RoboLook server
 *
 * @author Patric Jensfelt
 * @see
 */
class RoboLookProxy {
 public:
  /**
   * Constructor
   */
  RoboLookProxy();

  /**
   * Destructor
   */
  ~RoboLookProxy();

  /**
   * @param server name of the host machine where the server is running
   * @param port the port numbe where the server can be found
   * @param retryInterval the interval with which we try to reconnect
   * @return 0 on success, else 1
   */
  int Connect(const char* server, short port = 5555, double retryInterval=-1);

  /**
   * Same as above but with string arg for server
   */
  int Connect(const std::string server, short port=5555, 
              double retryInterval = -1)
    {return Connect(server.c_str(),port,retryInterval);};

  /**
   * @return 0 on success, else 1
   */
  void Disconnect();

  /**
   * @return true when connected, else false
   */
  bool connected() { return fd_ != -1; }

  /**
   * Specify how often you want to retry to open the connection to the
   * server in case it is not up when the first open was issued.
   *
   * @param secs number of seconds between connection attempts
   */
  void setRetryIntervall(double secs);

  //==================================================
  // =         MESSAGE SENDING METHODS
  //==================================================

  /**
   * Change the update interval for the scene
   * @param interval interval between scene update for server
   * @return 0 if sending ok, else 1
   */
  int setSceneUpdateInterval(double interval);

  /** 
   * Force and update of the screen of the RoboLook server
   * Might be ignored by the server
   *
   * @return 0 if sending ok, else 1
   */
  int forceScreenUpdate();

  /**
   * Create a robot in environment env (RL_ENV_TRUE, RL_ENV_EST, ...)
   * with a given id (rid) and of a certain kind ("Goofy",
   * "Louie"). If the kind is not recognized a robot like Goofy will
   * be created. The robot id given here will be the one to use when
   * steting the pose of it for example.
   *
   * @return 0 if sending ok, else 1 
   */
  int createRobot(int env, int rid, const char* type);

  /**
   * @return the rid of the last robot that was created
   */
  int lastRid() const { return lastRid_; }

  /**
   * Deletes the robot with id rid. If several robots have the same
   * id, the first robot to be encountered will be deleted.
   * @return 0 if sending ok, else 1 
   */
  int deleteRobot(int rid);


  /**
   * Use this function to delete all robots in the environment
   * @return 0 if sending ok, else 1 
   */
  int deleteAllRobots();

  /**
   * Clears all robots from environment env
   * @param env environment id 
   * @return 0 if sending ok, else 1 
   */
  int clearRobots(int env);

  /*
   * Will not do anything if the robot id (rid) does not match any of
   * the ones in the environment. If several robots match this id the
   * first to be encountered will be used.
   *
   * @param rid id of the robot
   * @param x x coordinate [m]
   * @param y y coordinate [m]
   * @param a orientation [rad]
   *
   * @return 0 if sending ok, else 1 
   */
  int setRobotPose(int rid, double x, double y, double z, double a);


  /**
   * Function that lets you set up to 20 extra robot dofs. It is up to
   * the robot to interpret what those dofs are.
   *
   * @param rid id of the robot
   * @param n number of dofs to set
   * @param offset used to start with dofs 3 instead of 0
   * @param value array of values for the extra dofs
   *
   * @return 0 if sending ok, else 1 
   */
  int setExtraRobotDOFs(int rid, int n, int offset, double *value);

  /**
   * Changes the transformation used for a particular environment when
   * it is being draw. Cna be used to make the estimated environment
   * overlap with the true one for exmaple.Will not do anything if the
   * environment id (env) does not match any in use.
   *
   * @param env environmental id
   * @param x x coordinate
   * @param y y coordinate
   * @param z z coordinate
   * @param a orientation
   * @return 0 if sending ok, else 1 
   */
  int setEnvPose(int env, double x, double y, double z, double a);

  /**
   * Will not do anything if the robot id (rid) does not match any of
   * the ones in the environment. If several robots match this id the
   * first to be encountered will be used.
   *
   * @param rid id of the robot
   * @param alpha blending factor for the robot (to make it transparent)
   * @return 0 if sending ok, else 1
   */
  int setRobotAlpha(int rid, float alpha);

  /**
   * Tell whether or not to show the scan from this scanner
   *
   * @param rid robot id
   * @param lid laser id
   * @return 0 if sending ok, else 1
   */
  int setShowLaserScan(int rid, int lid, bool show);

  /**
   * Tell whether or not to show the scan coordinates for the scan
   * from a certain scanner
   *
   * @param rid robot id
   * @param lid laser id
   * @return 0 if sending ok, else 1
   */
  int setShowScanCoords(int rid, int lid, bool show);

  /**
   * Tell whether or not to show the lines that have been added to the
   * scan with setLaserLines().
   *
   * @param rid robot id
   * @param lid laser id
   * @return 0 if sending ok, else 1
   *
   * @see RoboLookProxy::setLaserLines
   */
  int setShowScanLines(int rid, int lid, bool show);

  /**
   * Set the color when showing the laser scan point
   */
  int setLaserScanColor(int rid, int lid, int color);

  /**
   * Will not do anything if the robot id (rid) does not match any of
   * the ones in the environment. If several robots match this id the
   * first to be encountered will be used.
   *
   * @param rid robot id
   * @param lid laser id
   * @param startAngle the angle for the first laser beam
   * @param 
   * @param 
   * @param 
   * @param 
   * @return 0 if sending ok, else 1
   */
  int setLaserScan(int rid, int lid, int n, 
                   double startAngle, double angleStep, double timeStamp,
                   float *r);

  /**
   * Will not do anything if the robot id (rid) does not match any of
   * the ones in the environment. If several robots match this id the
   * first to be encountered will be used.
   *
   * @param rid robot id
   * @param lid laser id
   * @return 0 if sending ok, else 1
   */
  int setLaserScan(int rid, int lid, int n, 
                   double startAngle, double angleStep, double timeStamp,
                   float *r, double scaleFactor2m);

  /**
   * Will not do anything if the robot id (rid) does not match any of
   * the ones in the environment. If several robots match this id the
   * first to be encountered will be used.
   *
   * @param rid robot id
   * @param lid laser id
   * @return 0 if sending ok, else 1
   */
  int setLaserScan(int rid, int lid, int n, 
                   double startAngle, double angleStep, double timeStamp,
                   double *r, double scaleFactor2m);

  /**
   * Add lines detected from a scan
   * @param rid robot id
   * @param lid laser id
   * @param n number of line items
   * @param line array of line items
   * @return 0 if sending ok, else 1
   */
  int setLaserLines(int rid, int lid, int n, RL_LineItem *line);
  
  /**
   * Define the points that should be marked with some color
   * @param rid robot id
   * @param lid laser id
   * @param color color code (see RlColors.cc)
   * @param n number of points to mark
   * @param pts array of indices of scan points to mark
   * @return 0 if sending ok, else 1
   */
  int setMarkedScanPoints(int rid, int lid, int color,
                          int n, unsigned short *pts);

  /**
   * Specify the color of the n first scan points
   *
   * @param rid robot id
   * @param lid laser id
   * @param n number of points to mark
   * @param c array of color codes (see RlColors.cc)
   *
   * @return 0 if sending ok, else 1
   */
  int setScanColors(int rid, int lid, int n, unsigned short *c);

  /**
   * Clear all marked scan points
   * @param rid robot id
   * @param lid laser id
   * @return 0 if sending ok, else 1
   */
  int clearMarkedScanPoints(int rid, int lid);

  /**
   * Update the ranges for some sonar sensors
   *
   * @param rid robot id
   * @param n number of sonars to update range for
   * @param ranges the ranges
   * @param ids the indices. If NULL ids are assumed to be 0,1,2,3,...
   *
   * @return 0 if sending ok, else 1
   */
  int setSonarRanges(int rid, int n,
                     double *ranges, unsigned short *ids = 0);

  /**
   * Update the state of the bumpers
   *
   * @param rid robot id
   * @param state a bitmask for the bumper states
   *
   * @return 0 if sending ok, else 1
   */
  int setBumperState(int rid, int state);

  /**
   * @param env environment id
   * @param wall array of wall items
   * @param n number of wall items
   * @param replace will replace all existing walls if it is set to true 
   * else the walls will be added.
   * @return 0 if sending ok, else 1
   */
  int addWalls(int env, RL_WallItem* wall, int n, bool replace = false);
  
  /**
   * @param env environment id
   * @return 0 if sending ok, else 1
   */
  int clearWalls(int env);

  /**
   * @param env environment id
   * @param door array of door items
   * @param n number of door items
   * @param replace will replace all existing doors if it is set to true 
   * else the doors will be added. 
   * @return 0 if sending ok, else 1
   */
  int addDoors(int env, RL_DoorItem* door, int n, bool replace = false);
  
  /**
   * @param env environment id
   * @param pillar array of cylindrical pillar items
   * @param n number of pillar items
   * @param replace will replace all existing pillars if it is set to true 
   * else the pillars will be added.
   * @return 0 if sending ok, else 1
   */
  int addCylPillars(int env, RL_CylPillarItem* pillar, int n, 
                    bool replace = false);
  
  /**
   * @param env environment id
   * @return 0 if sending ok, else 1
   */
  int clearCylPillars(int env);

  /**
   * @param env environment id
   * @param ellipse array of ellipse items
   * @param n number of ellipse items
   * @param replace will replace all existing ellipses if it is set to true 
   * else the ellipses will be added.
   * @return 0 if sending ok, else 1
   */
  int addEllipses(int env, RL_EllipseItem* ellipse, int n, 
		  bool replace = false);

  /**
   * @param env environment id
   * @return 0 if sending ok, else 1
   */
  int clearEllipses(int env);

  /**
   * @param env environment id
   * @param ellipsoid array of ellipsoid items
   * @param n number of ellipsoid items
   * @param replace will replace all existing ellipsoids if it is set to true 
   * else the ellipsoids will be added.
   * @return 0 if sending ok, else 1
   */
  int addEllipsoids(int env, RL_EllipsoidItem* ellipsoid, int n, 
                    bool replace = false);

  /**
   * @param env environment id
   * @return 0 if sending ok, else 1
   */
  int clearEllipsoids(int env);

  /**
   * @param env environment id
   * @param rectangle array of rectangle items
   * @param n number of rectangle items
   * @param replace will replace all existing rectangles if it is set to true 
   * else the rectangles will be added.
   * @return 0 if sending ok, else 1
   */
  int addRectangles(int env, RL_RectangleItem* rectangle, int n,
		    bool replace = false);

  /**
   * @param env environment id
   * @return 0 if sending ok, else 1
   */
  int clearRectangles(int env);

  /**
   * @param env environment id
   * @param line array of line items
   * @param n number of line items
   * @param replace will replace all existing lines if it is set to true 
   * else the lines will be added.
   * @return 0 if sending ok, else 1
   */
  int addLines(int env, RL_LineItem* line, int n,
               bool replace = false);
  
  /**
   * @param env environment id
   * @param marker array of marker items
   * @param n number of marker items
   * @param replace will replace all existing markers if it is set to true 
   * else the markers will be added.
   * @return 0 if sending ok, else 1
   */
  int addMarkers(int env, RL_MarkerItem* marker, int n,
                 bool replace = false);

  /**
   * @param env environment id
   * @param cube array of cube items
   * @param n number of cube items
   * @param replace will replace all existing cubes if it is set to true 
   * else the cubes will be added.
   * @return 0 if sending ok, else 1
   */
  int addCubes(int env, RL_CubeItem* cube, int n,
                 bool replace = false);

  /**
   * @param env environment id
   * @param star array of star items
   * @param n number of star items
   * @param replace will replace all existing stars if it is set to true 
   * else the stars will be added.
   * @return 0 if sending ok, else 1
   */
  int addStars(int env, RL_StarItem* star, int n,
                 bool replace = false);

  /**
   * @param env environment id
   * @param point array of point items
   * @param n number of point items
   * @param replace will replace all existing points if it is set to true 
   * else the points will be added.
   * @return 0 if sending ok, else 1
   */
  int addPoints(int env, RL_PointItem* point, int n,
                bool replace = false);

  /**
   * @param env environment id
   * @param point array of point items
   * @param n number of point items
   * @param replace will replace all existing points if it is set to true 
   * else the points will be added.
   * @return 0 if sending ok, else 1
   */
  int addPoints(int env, RL_RGBPointItem* point, int n,
                bool replace = false);

  /**
   * @param env environment id
   * @param triangles array of triangle items
   * @param n number of triangle items
   * @param replace will replace all existing triangles if it is set to true 
   * else the triangles will be added.
   * @return 0 if sending ok, else 1
   */
  int addTriangles(int env, RL_TriangleItem* triangles, int n,
                   bool replace = false);

  /**
   * Clears all doors in a certain environment
   * @param env environment id
   * @return 0 if sending ok, else 1
   */
  int clearDoors(int env);

  /**
   * Clears all markers in a certain environment
   * @param env environment id
   * @return 0 if sending ok, else 1
   */
  int clearMarkers(int env);

  /**
   * Clears all cubes in a certain environment
   * @param env environment id
   * @return 0 if sending ok, else 1
   */
  int clearCubes(int env);

  /**
   * Clears all stars in a certain environment
   * @param env environment id
   * @return 0 if sending ok, else 1
   */
  int clearStars(int env);

  /**
   * Clears all lines in a certain environment
   * @param env environment id
   * @return 0 if sending ok, else 1
   */
  int clearLines(int env);

  /**
   * Clears all points in a certain environment (both RL_PointItem and
   * RL_RGBPointItem point)
   *
   * @param env environment id
   * @return 0 if sending ok, else 1
   */
  int clearPoints(int env);

  /**
   * Clears all triangles in a certain environment
   * @param env environment id
   * @return 0 if sending ok, else 1
   */
  int clearTriangles(int env);

  /**
   * Use this function to let RoboLook write the current buffer to
   * file. This function will force the model to be drawn on the
   * screen so you can be sure that data that you have sent will be
   * shown in the grabbed frame. The file will be called
   * rlsnapshot-<time>.ppm
   *
   * @return 0 if sending ok, else 1
   */
  int writeSingleFrame();

  /**
   * Use this function to tell RoboLook to write the buffer to file
   * every time it is updated. It will take time and slow things down
   * significantly. The files will be called rlframe-<counter>-<time>.ppm
   *
   * @return 0 if sending ok, else 1
   */
  int startWritingFrames();

  /**
   * Use this function to tell RoboLook to stop writing buffer on paint
   * @return 0 if sending ok, else 1
   */
  int stopWritingFrames();

  /**
   * Set the ghost state of the robot to on/off. In ghost state the
   * robot is drawn transparently so that it is not as clearly
   * visible as the other robots. Not all robots support this feature.
   * @return 0 if sending ok, else 1
   */
  int setGhostState(int rid, bool on);

  /**
   * @return the file descriptor for the socket
   */
  int fd() { return fd_; }  

  /**
   * @param rid robot id
   * @return 0 if sending ok, else 1
   */
  int connectCloseCamToRobot(int rid);

  /**
   * This function can be used to map a normalized value 0-1 to an RGB
   * color (3 long array of unsigned char 0-255). The default is that
   * 0 is mapped to redish and 1 to blueish but you can choose to swap
   * the orders of the colors if you wish. You can also choose to
   * remove the extreme values which are very similar in the two ends
   * by mapping 0-1 to 0.05-0.95.
   *
   * @param norm that normalized value to map to RGB
   * @param color the 3 long array of RGB values
   * @param swapOrder if true color order will be swapped
   * @param trimEndsOff if true norm value will be remapped to remove ends
   */
  void mapNormValueToRGB(double norm, unsigned char *color, 
                         bool swapOrder=false, bool trimEndsOff=false);

 protected:
  int fd_;
  double retryInterval_;
  double lastTry_;
  char *server_;
  short port_;
  unsigned char *send_buf_;
  unsigned int send_buf_size_;
  float tmpRange_[722];
  int lastRid_;

  /**   
   * Checks the connection to the server. If we are not connected but
   * have requested to retry the connection this function will try
   * again to connect.
   */
  int checkConn();
  
  /**
   * Function by verifies that the connection is still ok (if it was
   * up before)
   *
   * @return 1 if (still connected), else 0
   */
  int verifyConnection();
  
  /**
   * Use select to wait for activity on the socket connection to RoboLook
   *
   * @param sec number of seconds to wait
   * @param usec number of micro seconds to wait
   */
  int timedSelect(long sec, long usec);

  /*
   * @param len memory area size
   * @return a pointer to a memory area that is big enough to handle a
   * message of size len
   */
  unsigned char* getMem(unsigned int len);
  
  /*
   * Sends the message that is in send_buf_ with a certain length. We
   * pass on the length so that we do not have to assume that the
   * send_buf_ starts with a message head that has the len of the
   * data.
   */
  int sendSendBuffer(int n);

  /**
   * This is the function that maps a normalized value to one of the
   * channels RGB. Without modification this function maps the Green
   * channel. The Red channel is shifted 0.25 back (d+0.25) and the
   * Blue channel forward 0.25 (d-0.25). The shifting also includes
   * making sure that the value is 0-1.
   */
  void lookupColorValue(double d, unsigned char &color);
};

#endif

