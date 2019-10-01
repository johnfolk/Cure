//
// = FUNCTION
//
// = RCSID
//    $Id: RoboLookProxy.cc,v 1.16 2007/09/14 09:14:06 johnf Exp $
//
// = AUTHOR(S)
//    Patric Jensfelt
//
// = VERSION
//    $Revision: 1.16 $
//
// = DATE RELEASED 
//    $Date: 2007/09/14 09:14:06 $
//
// = COPYRIGHT
//    Copyright (c) 2002 Patric Jensfelt
/*----------------------------------------------------------------------*/

#include "RoboLookProxy.h"

#ifndef DEPEND
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#endif

#define START_SEND_BUF_SIZE 10000

/*
double getCurrentTime()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (tv.tv_sec + 1.0e-6*tv.tv_usec);
}
*/

RoboLookProxy::RoboLookProxy()
  :fd_(-1),
   retryInterval_(-1),
   server_(NULL),
   send_buf_(NULL)
{
  send_buf_size_ = 0;
  getMem(START_SEND_BUF_SIZE);
}

RoboLookProxy::~RoboLookProxy()
{
  fprintf(stderr, "Destructor of RoboLookProxy\n");
  free(send_buf_);
  free(server_);
}

int RoboLookProxy::Connect(const char* server, short port, 
                           double retryInterval) 
{
  struct sockaddr_in servername;
  struct hostent *hostinfo;
  struct timeval tv;

  retryInterval_ = retryInterval;

  if (connected())
      Disconnect();

  server_ = strdup(server);
  port_ = port;

  //fprintf(stderr,"Attempting to connect to %s at port %d\n", server_, port_);

  gettimeofday(&tv, NULL);
  lastTry_ = tv.tv_sec + 1.0e-6*tv.tv_usec;

  /* Create the socket. */
  fd_ = socket(PF_INET, SOCK_STREAM, 0);
  if (fd_ < 0) {
    perror ("socket (client)");
    exit (EXIT_FAILURE);
  }
  

  servername.sin_family = AF_INET;
  hostinfo = gethostbyname(server);
  if (hostinfo == NULL) {
    fprintf (stderr, "Unknown host \"%s\"!. Failed to connect\n", server);
    fd_ = -1;
    return -1;
  }
  servername.sin_addr = *(struct in_addr *) hostinfo->h_addr;

  bool done = false;
  while (!done && port < (port_ + 5)) { 
    servername.sin_port = htons (port);
    //fprintf(stderr, "Trying port %d\n", port);
    if (connect(fd_, (struct sockaddr *) &servername,
                sizeof (servername)) < 0) {
      //perror ("connect (client)");
    } else {
      // We are connected to something, but we really wnat to see the
      // magic message from the server before we really believe that
      // it is the RoboLook server
      int selret = timedSelect(1,0);
      if (selret > 0) {
        // We actually got data
        char buf[64]; 
        char *magicMsg = "You have connected to the RoboLook server";
        int ret = read(fd_, buf, 64);
        //fprintf(stderr, "ret = %d\n", ret);
        if (ret > 0 && strncmp(buf, magicMsg, ret) == 0) {
          fprintf(stderr, "Connected to the RoboLook server!\n");
          done = true;
        } else {
          //fprintf(stderr, "Not a RoboLook server, got \"%s\" on port %d\n", 
          //      buf, port);
        }
      } else if (selret == 0) {
        fprintf(stderr, "Timeout! This is not a RoboLook server\n");
      } else {
        perror("Select");
      }
    }
    if (!done) {
      port++;
    }
  }

  if (done)
    return 0;
  else {
    fd_ = -1;
    return -1;
  }
}

void RoboLookProxy::Disconnect()
{
  if (connected()) {
    if (fd_ > 0)
      close (fd_);
    retryInterval_ = -1;
  }
}

int RoboLookProxy::timedSelect(long sec, long usec)
{
    fd_set mask;
    FD_ZERO(&mask);
    FD_SET(fd_, &mask);

    struct timeval tv;
    tv.tv_sec = sec;
    tv.tv_usec = usec;

    int selret = select(FD_SETSIZE, &mask, NULL, NULL, &tv);
    return selret;
}

unsigned char* RoboLookProxy::getMem(unsigned int len)
{
  if (len > send_buf_size_) {
    // For now we ignore all errors
    void *ptr = malloc(len);
    if (ptr != NULL) {
      free(send_buf_);
      send_buf_ = (unsigned char*)ptr;
      send_buf_size_ = len;
      memset(send_buf_, 0, send_buf_size_);
    } else {
      fprintf(stderr, "ERROR: Realloc from %d to %d bytes failed!\n",
              send_buf_size_, len);
    }
  }

  return send_buf_;
}

int RoboLookProxy::verifyConnection()
{
  // If we think we are connected we check if it seems ok still
  if (connected()) {
    int selret = timedSelect(0,0);
    if (selret > 0) {
      // We actually got data
      int ret;
      char buf[32];
      ret = read(fd_, buf, 32);
      //fprintf(stderr, "ret = %d\n", ret);
      if (ret == 0) {
        // The connection is closed
        fd_ = -1;
        fprintf(stderr, "Connection broken to RoboLook server!\n");
        return 0;
      } else if (ret > 0) {
        fprintf(stderr, "Got something from the server!\n");
        return 1;
      } else {
        perror("verifyConnection read");
        return 0;
      }
    } else if (selret == 0) {
      // We had a timeout
      //fprintf(stderr, "Timeout, connection still ok\n");
      return 1;
    } else {
      //perror("Select");
      return 0;
    }
  } else {
    // We do not even think that we are connected, so probably we are not
    //fprintf(stderr, "Not connected\n");
    return 0;
  }
}

int RoboLookProxy::checkConn()
{
  if (verifyConnection())
    return 0;

  //fprintf(stderr, "Might try to (re-)connect\n");
  struct timeval tv;
  gettimeofday(&tv, NULL);
  double time = tv.tv_sec + 1.0e-6*tv.tv_usec;

  if ((retryInterval_ >= 0) && 
      ((time - lastTry_) > retryInterval_)) {
    //fprintf(stderr, "Trying to (re-)connect\n");
    return Connect(server_, port_, retryInterval_);
  } else {
    //fprintf(stderr, "Did not try to connect again, waiting %.3fs\n",
    //        retryInterval_ - (time - lastTry_));
    return 1;
  }
}

int RoboLookProxy::sendSendBuffer(int n)
{
  if (checkConn()) return 1;

  int ret = 0;

  // This debug printout assumes that the message starts with a MsgHead
  //fprintf(stderr, "Sending message of type %d with %d bytes\n", 
  //    head->id, head->len);
  //for (int ii = 0; ii < 20 && ii < head->len; ii++) {
  //fprintf(stderr, "send_buf_[%02d]=%02X\n", ii, send_buf_[ii]);
  //}

  int nbytes = write(fd_, send_buf_, n);
  if (nbytes < 0) {
    perror ("write");
    fd_ = -1;
    Disconnect();
    ret = 1;
  } else if (nbytes != n) {
    fprintf(stderr, "Sent %d but expected to send %d bytes\n", 
            nbytes, n);
    ret = 1;
  } 
  
  // This debug printout assumes that the message starts with a MsgHead
  //fprintf(stderr, "Successfully sent msg type %d with %d bytes\n", 
  //      head->id, nbytes);

  return ret;  
}

void RoboLookProxy::mapNormValueToRGB(double normDist, unsigned char *color, 
                                      bool swapOrder, bool trimEndsOff)
{
  if (swapOrder) normDist = 1.0 - normDist;

  if (trimEndsOff) {
    // The ends of the color scale are quite similar (it is a
    // "circle"). To avoid this I map the normDist which is 0-1 to 0.05-0.95
    // instead.
    normDist = 0.05 + 0.9*normDist;
  }

  double tmp;

  // R
  tmp = normDist + 2.0 / 8.0;
  if (tmp > 1.0) tmp -= 1.0;
  lookupColorValue(tmp, color[0]);

  // G
  lookupColorValue(normDist, color[1]);

  // B
  tmp = normDist - 2.0 / 8.0;
  if (tmp < 0) tmp += 1.0;
  lookupColorValue(tmp, color[2]);  
}

void RoboLookProxy::lookupColorValue(double d, unsigned char &color)
{
  double c;
  
  // These values were obtained from matlab by looking at the
  // colormap. It was a 64 valued lookup table where the below values
  // are for the green channel. The red and the blue are shifted 16
  // steps wrt green.

  if (d > 55.0 / 64.0) {
    c = 0;
  } else if (d > 39.0 / 64.0) {
    c = 1.0 - (d - 39.0 / 64.0) * 4.0;
  } else if (d > 23.0 / 64.0) {
    c = 1.0;
  } else if (d > 7.0 / 64.0) {
    c = (d - 7.0 / 64.0) * 4.0;
  } else {
    c = 0;
  }

  color = (int)(255.0 * c);
}

int RoboLookProxy::setSceneUpdateInterval(double interval)
{
  if (checkConn()) return 1;

  RL_MsgSceneUpdateInterval *msg = 
    (RL_MsgSceneUpdateInterval*)getMem(sizeof(RL_MsgSceneUpdateInterval));
  RL_addSTX(&msg->head);

  msg->head.id = RL_MsgSceneUpdateInterval_ID;
  msg->head.len = sizeof(RL_MsgSceneUpdateInterval);
  msg->head.crc = 0;
  
  msg->interval = interval;

  return sendSendBuffer(msg->head.len);
}

int RoboLookProxy::forceScreenUpdate()
{
  if (checkConn()) return 1;

  RL_MsgHead *msg = (RL_MsgHead*)getMem(sizeof(RL_MsgHead));
  RL_addSTX(msg);

  msg->id = RL_MsgForceScreenUpdate_ID;
  msg->len = sizeof(RL_MsgHead);
  msg->crc = 0;
  
  return sendSendBuffer(msg->len);
}

int RoboLookProxy::setRobotPose(int rid, double x, double y, 
                                double z, double a)
{
  if (checkConn()) return 1;

  //fprintf(stderr, "%.6f: Sending pose message %f %f %f %f\n", 
  //      getCurrentTime(), x, y, z, a);

  RL_MsgRobotPose *msg = (RL_MsgRobotPose*)getMem(sizeof(RL_MsgRobotPose));
  RL_addSTX(&msg->head);
  
  msg->head.id = RL_MsgRobotPose_ID;
  msg->head.len = sizeof(RL_MsgRobotPose);
  msg->head.crc = 0;
  
  msg->rid = rid;
  msg->x = x;
  msg->y = y;
  msg->z = z;
  msg->a = a;
  
  return sendSendBuffer(msg->head.len);
}

int RoboLookProxy::setExtraRobotDOFs(int rid, int n, int offset, double *value)
{
  if (checkConn()) return 1;

  if (n < 0 || n > 30) {
    fprintf(stderr, "RoboLookProxy::setExtraRobotDOFs n=%d is invalid\n",n);
    return 1;
  }

  //fprintf(stderr, "Sending extra robot dofs message\n");

  RL_MsgExtraRobotDOFs *msg = 
    (RL_MsgExtraRobotDOFs*)getMem(sizeof(RL_MsgExtraRobotDOFs));
  RL_addSTX(&msg->head);
  
  msg->head.id = RL_MsgExtraRobotDOFs_ID;
  msg->head.len = sizeof(RL_MsgExtraRobotDOFs);
  msg->head.crc = 0;
  
  msg->rid = rid;
  msg->n = n;
  msg->offset = offset;
  memcpy(msg->value, value, n*sizeof(double));

  return sendSendBuffer(msg->head.len);
}

int RoboLookProxy::createRobot(int env, int rid, const char *type)
{
  if (checkConn()) return 1;
  
  //fprintf(stderr, "Creating robot of type %s\n", type);
  
  RL_MsgCreateRobot *msg = 
    (RL_MsgCreateRobot*)getMem(sizeof(RL_MsgCreateRobot));
  RL_addSTX(&msg->head);
  
  msg->head.id = RL_MsgCreateRobot_ID;
  msg->head.len = sizeof(RL_MsgCreateRobot);
  msg->head.crc = 0;
  
  msg->env = env;
  msg->rid = rid;

  lastRid_ = rid;

  strncpy(msg->type, type, 64);

  return sendSendBuffer(msg->head.len);
}

int RoboLookProxy::deleteRobot(int rid)
{
  if (checkConn()) return 1;
  
  //fprintf(stderr, "Deleting robot %d\n", rid);
  
  RL_MsgDeleteRobot *msg = 
    (RL_MsgDeleteRobot*)getMem(sizeof(RL_MsgDeleteRobot));
  RL_addSTX(&msg->head);
  
  msg->head.id = RL_MsgDeleteRobot_ID;
  msg->head.len = sizeof(RL_MsgDeleteRobot);
  msg->head.crc = 0;
  
  msg->rid = rid;

  return sendSendBuffer(msg->head.len);
}

int RoboLookProxy::deleteAllRobots()
{
  if (checkConn()) return 1;
  
  RL_MsgHead *head = (RL_MsgHead*)getMem(sizeof(RL_MsgHead));
  RL_addSTX(head);
  
  head->id = RL_MsgDeleteAllRobots_ID;
  head->len = sizeof(RL_MsgHead);
  head->crc = 0;

  return sendSendBuffer(head->len);
}

int RoboLookProxy::clearRobots(int env)
{
  if (checkConn()) return 1;
  
  //fprintf(stderr, "Clearing robots in env %d\n", env);
  
  RL_MsgClearRobots *msg = 
    (RL_MsgClearRobots*)getMem(sizeof(RL_MsgClearRobots));
  RL_addSTX(&msg->head);
  
  msg->head.id = RL_MsgClearRobots_ID;
  msg->head.len = sizeof(RL_MsgClearRobots);
  msg->head.crc = 0;
  
  msg->env = env;

  return sendSendBuffer(msg->head.len);
}

int RoboLookProxy::setEnvPose(int env, double x, double y, double z, double a)
{
  if (checkConn()) return 1;

  //fprintf(stderr, "Sending env pose message\n");

  RL_MsgEnvPose *msg = (RL_MsgEnvPose*)getMem(sizeof(RL_MsgEnvPose));
  RL_addSTX(&msg->head);
  
  msg->head.id = RL_MsgEnvPose_ID;
  msg->head.len = sizeof(RL_MsgEnvPose);
  msg->head.crc = 0;
  
  msg->env = env;
  msg->x = x;
  msg->y = y;
  msg->z = z;
  msg->a = a;
  
  return sendSendBuffer(msg->head.len);
}


int RoboLookProxy::setShowLaserScan(int rid, int lid, bool show) 
{
  if (checkConn()) return 1;

  RL_MsgShowLaserScan *msg = 
    (RL_MsgShowLaserScan*)getMem(sizeof(RL_MsgShowLaserScan));
  RL_addSTX(&msg->head);
  
  msg->head.id = RL_MsgShowLaserScan_ID;
  msg->head.len = sizeof(RL_MsgShowLaserScan);
  msg->head.crc = 0;
  
  msg->rid = rid;
  msg->lid = lid;
  if (show)
    msg->show = 1;
  else 
    msg->show = 0;

  return sendSendBuffer(msg->head.len);    
}

int RoboLookProxy::setShowScanCoords(int rid, int lid, bool show) 
{
  if (checkConn()) return 1;

  RL_MsgShowLaserScan *msg = 
    (RL_MsgShowLaserScan*)getMem(sizeof(RL_MsgShowLaserScan));
  RL_addSTX(&msg->head);
  
  msg->head.id = RL_MsgShowScanCoords_ID;
  msg->head.len = sizeof(RL_MsgShowLaserScan);
  msg->head.crc = 0;
  
  msg->rid = rid;
  msg->lid = lid;
  if (show)
    msg->show = 1;
  else 
    msg->show = 0;

  return sendSendBuffer(msg->head.len);    
}

int RoboLookProxy::setShowScanLines(int rid, int lid, bool show) 
{
  if (checkConn()) return 1;

  RL_MsgShowLaserScan *msg = 
    (RL_MsgShowLaserScan*)getMem(sizeof(RL_MsgShowLaserScan));
  RL_addSTX(&msg->head);
  
  msg->head.id = RL_MsgShowScanLines_ID;
  msg->head.len = sizeof(RL_MsgShowLaserScan);
  msg->head.crc = 0;
  
  msg->rid = rid;
  msg->lid = lid;
  if (show)
    msg->show = 1;
  else 
    msg->show = 0;

  return sendSendBuffer(msg->head.len);    
}

int RoboLookProxy::setLaserScanColor(int rid, int lid, int color)
{
  if (checkConn()) return 1;

  RL_MsgLaserScanColor *msg = 
    (RL_MsgLaserScanColor*)getMem(sizeof(RL_MsgLaserScanColor));
  RL_addSTX(&msg->head);
  
  msg->head.id = RL_MsgLaserScanColor_ID;
  msg->head.len = sizeof(RL_MsgLaserScanColor);
  msg->head.crc = 0;
  
  msg->rid = rid;
  msg->lid = lid;
  msg->color = color;

  return sendSendBuffer(msg->head.len);    
}

int RoboLookProxy::setLaserScan(int rid, int lid, int n, 
                                double startAngle, double angleStep, 
                                double timeStamp, float *r)
{
  if (checkConn()) return 1;

  //fprintf(stderr, "%.6f: Sending scan message\n", getCurrentTime());

  int nbytes = sizeof(RL_MsgLaserScan) + n * sizeof(float);
  RL_MsgLaserScan *msg = (RL_MsgLaserScan*)getMem(nbytes);
  RL_addSTX(&msg->head);
  
  msg->head.id = RL_MsgLaserScan_ID;
  msg->head.len = nbytes;
  msg->head.crc = 0;
  
  msg->rid = rid;
  msg->lid = lid;
  msg->n = n;
  msg->startAngle = startAngle;
  msg->angleStep = angleStep;
  msg->timeStamp = timeStamp;
  
  memcpy((char*)msg + sizeof(RL_MsgLaserScan), r, n * sizeof(float));

  return sendSendBuffer(msg->head.len);
}

int RoboLookProxy::setLaserScan(int rid, int lid, int n, 
                                double startAngle, double angleStep, 
                                double timeStamp, float *r,
                                double scaleFactor2m)
{
  for (int i = 0; i < n; i++)
    tmpRange_[i] = scaleFactor2m * r[i];
  
  return setLaserScan(rid, lid, n, startAngle, angleStep, timeStamp,
                      (float*)tmpRange_);
}

int RoboLookProxy::setLaserScan(int rid, int lid, int n, 
                                double startAngle, double angleStep, 
                                double timeStamp, double *r,
                                double scaleFactor2m)
{
  for (int i = 0; i < n; i++)
    tmpRange_[i] = scaleFactor2m * r[i];
  
  return setLaserScan(rid, lid, n, startAngle, angleStep, timeStamp,
                      (float*)tmpRange_);
}

int RoboLookProxy::setLaserLines(int rid, int lid, int n, RL_LineItem *line)
{
  if (checkConn()) return 1;

  size_t nbytes = sizeof(RL_MsgLaserLines) + n * sizeof(RL_LineItem);

  //fprintf(stderr, "Sending scan lines message (%d bytes=%d+%d*%d)\n", 
  //      nbytes, sizeof(RL_MsgLaserLines), n, sizeof(RL_LineItem));

  RL_MsgLaserLines *msg = (RL_MsgLaserLines*)getMem(nbytes);
  memset(msg, 0, nbytes);

  RL_addSTX(&msg->head);
  
  msg->head.id = RL_MsgLaserLines_ID;
  msg->head.len = nbytes;
  msg->head.crc = 0;
  
  msg->rid = rid;
  msg->lid = lid;
  msg->n = n;

  if (n > 0)
    memcpy(msg + 1, line, n * sizeof(RL_LineItem));
  
  return sendSendBuffer(msg->head.len);
}

int RoboLookProxy::setMarkedScanPoints(int rid, int lid, int color,
                                       int n, unsigned short *pts)
{
  if (checkConn()) return 1;

  size_t nbytes = sizeof(RL_MsgMarkedScanPoints) + n * sizeof(unsigned short);

  RL_MsgMarkedScanPoints *msg = (RL_MsgMarkedScanPoints*)getMem(nbytes);

  RL_addSTX(&msg->head);
  
  msg->head.id = RL_MsgMarkedScanPoints_ID;
  msg->head.len = nbytes;
  msg->head.crc = 0;
  
  msg->rid = rid;
  msg->lid = lid;
  msg->color = color;
  msg->n = n;

  memcpy(msg + 1, pts, n * sizeof(unsigned short));
  
  return sendSendBuffer(msg->head.len);
}

int RoboLookProxy::setScanColors(int rid, int lid, 
                                 int n, unsigned short *c)
{
  if (checkConn()) return 1;

  size_t nbytes = sizeof(RL_MsgScanColors) + n * sizeof(unsigned short);

  RL_MsgScanColors *msg = (RL_MsgScanColors*)getMem(nbytes);

  RL_addSTX(&msg->head);
  
  msg->head.id = RL_MsgScanColors_ID;
  msg->head.len = nbytes;
  msg->head.crc = 0;
  
  msg->rid = rid;
  msg->lid = lid;
  msg->n = n;

  memcpy(msg + 1, c, n * sizeof(unsigned short));
  
  return sendSendBuffer(msg->head.len);
}

int RoboLookProxy::clearMarkedScanPoints(int rid, int lid)
{
  if (checkConn()) return 1;

  RL_MsgClearMarkedScanPoints *msg = 
    (RL_MsgClearMarkedScanPoints*)getMem(sizeof(RL_MsgClearMarkedScanPoints));
  RL_addSTX(&msg->head);
  
  msg->head.id = RL_MsgClearMarkedScanPoints_ID;
  msg->head.len = sizeof(RL_MsgClearMarkedScanPoints);
  msg->head.crc = 0;

  msg->rid = rid;
  msg->lid = lid;

  return sendSendBuffer(msg->head.len);
}


int RoboLookProxy::setRobotAlpha(int rid, float alpha)
{
  if (checkConn()) return 1;

  //fprintf(stderr, "Sending alpha message\n");

  RL_MsgRobotAlpha *msg = (RL_MsgRobotAlpha*)getMem(sizeof(RL_MsgRobotAlpha));
  RL_addSTX(&msg->head);
  
  msg->head.id = RL_MsgRobotAlpha_ID;
  msg->head.len = sizeof(RL_MsgRobotAlpha);
  msg->head.crc = 0;
  
  msg->rid = rid;
  msg->alpha = alpha;
  
  return sendSendBuffer(msg->head.len);
}

int 
RoboLookProxy::setSonarRanges(int rid, int n, double *ranges, 
                              unsigned short *ids)
{
  if (checkConn()) return 1;

  size_t nbytes = sizeof(RL_MsgSonarRanges) + n * sizeof(double);
  if (ids != NULL) nbytes += n * sizeof(unsigned short);

  RL_MsgSonarRanges *msg = (RL_MsgSonarRanges*)getMem(nbytes);

  RL_addSTX(&msg->head);
  
  msg->head.id = RL_MsgSonarRanges_ID;
  msg->head.len = nbytes;
  msg->head.crc = 0;
  
  msg->rid = rid;
  msg->n = n;
  if (ids == NULL) msg->gotIDs = 0; else msg->gotIDs = 1;

  memcpy(msg + 1, ranges, n * sizeof(double));
  if (ids != NULL) {
    memcpy(((char*)(msg + 1)) + n * sizeof(double), ids, 
           n * sizeof(unsigned short));
  }
  
  return sendSendBuffer(msg->head.len);
}

int
RoboLookProxy::setBumperState(int rid, int state)
{
  if (checkConn()) return 1;

  size_t nbytes = sizeof(RL_MsgBumperState);

  RL_MsgBumperState *msg = (RL_MsgBumperState*)getMem(nbytes);

  RL_addSTX(&msg->head);

  msg->head.id = RL_MsgBumperState_ID;
  msg->head.len = nbytes;
  msg->head.crc = 0;

  msg->rid = rid;
  msg->state = state;

  return sendSendBuffer(msg->head.len);
}

int RoboLookProxy::addWalls(int env, RL_WallItem *wall, int n, bool replace)
{
  if (checkConn()) return 1;
  
  //fprintf(stderr, "%.6f: Sending add walls message %d\n",
  //      getCurrentTime(), n);

  size_t nbytes = sizeof(RL_MsgAddItems) + n * sizeof(RL_WallItem);
  RL_MsgAddItems *msg = (RL_MsgAddItems*)getMem(nbytes);

  RL_addSTX(&(msg->head));
  msg->head.id = RL_MsgAddWalls_ID;
  msg->head.len = nbytes;
  msg->head.crc = 0;
  
  msg->env = env;
  msg->replace = (int)replace;
  msg->n = n;

  memcpy(((char*)msg) + sizeof(RL_MsgAddItems), wall, 
         n * sizeof(RL_WallItem));
  
  return sendSendBuffer(msg->head.len);
}

int RoboLookProxy::clearWalls(int env)
{
  if (checkConn()) return 1;

  //fprintf(stderr, "%.6f: Sending clear walls for env %d\n", 
  //      getCurrentTime(), env);

  RL_MsgClearWalls *msg = (RL_MsgClearWalls*)getMem(sizeof(RL_MsgClearWalls));
  RL_addSTX(&msg->head);
  
  msg->head.id = RL_MsgClearWalls_ID;
  msg->head.len = sizeof(RL_MsgClearWalls);
  msg->head.crc = 0;
  
  msg->env = env;

  return sendSendBuffer(msg->head.len);
}

int RoboLookProxy::addDoors(int env, RL_DoorItem *door, int n, bool replace)
{
  if (checkConn()) return 1;

  //fprintf(stderr, "Sending door message\n");

  int nbytes = sizeof(RL_MsgAddItems) + n * sizeof(RL_DoorItem);
  RL_MsgAddItems *msg = (RL_MsgAddItems*)getMem(nbytes);

  RL_addSTX(&msg->head);
  msg->head.id = RL_MsgAddDoors_ID;
  msg->head.len = nbytes;
  msg->head.crc = 0;
  
  msg->env = env;
  msg->replace = (int)replace;
  msg->n = n;
  
  memcpy(((char*)msg) + sizeof(RL_MsgAddItems), door, 
         n * sizeof(RL_DoorItem));
  
  return sendSendBuffer(msg->head.len);
}

int RoboLookProxy::clearDoors(int env)
{
  if (checkConn()) return 1;

  //fprintf(stderr, "%.6f: Sending clear doors for env %d\n", 
  //      getCurrentTime(), env);

  RL_MsgClearDoors *msg = (RL_MsgClearDoors*)getMem(sizeof(RL_MsgClearDoors));
  RL_addSTX(&msg->head);
  
  msg->head.id = RL_MsgClearDoors_ID;
  msg->head.len = sizeof(RL_MsgClearDoors);
  msg->head.crc = 0;
  
  msg->env = env;

  return sendSendBuffer(msg->head.len);
}

int RoboLookProxy::addCylPillars(int env, RL_CylPillarItem *pillar, 
                                 int n, bool replace)
{
  if (checkConn()) return 1;
  
  //fprintf(stderr, "Sending add cyl pillars message\n");

  size_t nbytes = sizeof(RL_MsgAddItems) + n * sizeof(RL_CylPillarItem);
  RL_MsgAddItems *msg = (RL_MsgAddItems*)getMem(nbytes);

  RL_addSTX(&(msg->head));
  msg->head.id = RL_MsgAddCylPillars_ID;
  msg->head.len = nbytes;
  msg->head.crc = 0;
  
  msg->env = env;
  msg->replace = (int)replace;
  msg->n = n;

  memcpy(((char*)msg) + sizeof(RL_MsgAddItems), pillar, 
         n * sizeof(RL_CylPillarItem));
  
  return sendSendBuffer(msg->head.len);
}

int RoboLookProxy::clearCylPillars(int env)
{
  if (checkConn()) return 1;

  //fprintf(stderr, "Sending clear cyl pillars for env %d\n", env);

  RL_MsgClearItems *msg = (RL_MsgClearItems*)getMem(sizeof(RL_MsgClearItems));
  RL_addSTX(&msg->head);
  
  msg->head.id = RL_MsgClearCylPillars_ID;
  msg->head.len = sizeof(RL_MsgClearItems);
  msg->head.crc = 0;
  
  msg->env = env;

  return sendSendBuffer(msg->head.len);
}

int RoboLookProxy::addEllipses(int env, RL_EllipseItem *ellipse, int n, 
			       bool replace)
{
  if (checkConn()) return 1;

  //fprintf(stderr, "Sending ellipse message\n");

  int nbytes = sizeof(RL_MsgAddItems) + n * sizeof(RL_EllipseItem);
  RL_MsgAddItems *msg = (RL_MsgAddItems*)getMem(nbytes);

  RL_addSTX(&msg->head);
  msg->head.id = RL_MsgAddEllipses_ID;
  msg->head.len = nbytes;
  msg->head.crc = 0;
  
  msg->env = env;
  msg->replace = (int)replace;
  msg->n = n;
  
  memcpy(((char*)msg) + sizeof(RL_MsgAddItems), ellipse, 
         n * sizeof(RL_EllipseItem));
  
  return sendSendBuffer(msg->head.len);
}

int RoboLookProxy::clearEllipses(int env)
{
  if (checkConn()) return 1;

  //fprintf(stderr, "%.6f: Sending clear ellipses for env %d\n", 
  //      getCurrentTime(), env);

  RL_MsgClearItems *msg = 
    (RL_MsgClearItems*)getMem(sizeof(RL_MsgClearItems));
  RL_addSTX(&msg->head);
  
  msg->head.id = RL_MsgClearEllipses_ID;
  msg->head.len = sizeof(RL_MsgClearItems);
  msg->head.crc = 0;
  
  msg->env = env;

  return sendSendBuffer(msg->head.len);
}

int RoboLookProxy::addEllipsoids(int env, RL_EllipsoidItem *ellipsoid, int n, 
			       bool replace)
{
  if (checkConn()) return 1;

  //fprintf(stderr, "Sending ellipsoid message\n");

  int nbytes = sizeof(RL_MsgAddItems) + n * sizeof(RL_EllipsoidItem);
  RL_MsgAddItems *msg = (RL_MsgAddItems*)getMem(nbytes);

  RL_addSTX(&msg->head);
  msg->head.id = RL_MsgAddEllipsoids_ID;
  msg->head.len = nbytes;
  msg->head.crc = 0;
  
  msg->env = env;
  msg->replace = (int)replace;
  msg->n = n;
  
  memcpy(((char*)msg) + sizeof(RL_MsgAddItems), ellipsoid, 
         n * sizeof(RL_EllipsoidItem));
  
  return sendSendBuffer(msg->head.len);
}

int RoboLookProxy::clearEllipsoids(int env)
{
  if (checkConn()) return 1;

  //fprintf(stderr, "%.6f: Sending clear ellipsoids for env %d\n", 
  //      getCurrentTime(), env);

  RL_MsgClearItems *msg = 
    (RL_MsgClearItems*)getMem(sizeof(RL_MsgClearItems));
  RL_addSTX(&msg->head);
  
  msg->head.id = RL_MsgClearEllipsoids_ID;
  msg->head.len = sizeof(RL_MsgClearItems);
  msg->head.crc = 0;
  
  msg->env = env;

  return sendSendBuffer(msg->head.len);
}

int RoboLookProxy::addRectangles(int env, RL_RectangleItem *rectangle, int n, 
				 bool replace)
{
  if (checkConn()) return 1;

  //fprintf(stderr, "Sending rectangle message\n");

  int nbytes = sizeof(RL_MsgAddItems) + n * sizeof(RL_RectangleItem);
  RL_MsgAddItems *msg = (RL_MsgAddItems*)getMem(nbytes);

  RL_addSTX(&msg->head);
  msg->head.id = RL_MsgAddRectangles_ID;
  msg->head.len = nbytes;
  msg->head.crc = 0;
  
  msg->env = env;
  msg->replace = (int)replace;
  msg->n = n;
  
  memcpy(((char*)msg) + sizeof(RL_MsgAddItems), rectangle, 
         n * sizeof(RL_RectangleItem));
  
  return sendSendBuffer(msg->head.len);
}

int RoboLookProxy::clearRectangles(int env)
{
  if (checkConn()) return 1;

  //fprintf(stderr, "%.6f: Sending clear rectangles for env %d\n", 
  //      getCurrentTime(), env);

  RL_MsgClearItems *msg = 
    (RL_MsgClearItems*)getMem(sizeof(RL_MsgClearItems));
  RL_addSTX(&msg->head);
  
  msg->head.id = RL_MsgClearRectangles_ID;
  msg->head.len = sizeof(RL_MsgClearItems);
  msg->head.crc = 0;
  
  msg->env = env;

  return sendSendBuffer(msg->head.len);
}

int RoboLookProxy::connectCloseCamToRobot(int rid)
{
  if (checkConn()) return 1;

  //fprintf(stderr, "Sending clear doors for env %d\n", env);

  RL_MsgCloseCamRobot *msg = 
    (RL_MsgCloseCamRobot*)getMem(sizeof(RL_MsgCloseCamRobot));
  RL_addSTX(&msg->head);
  
  msg->head.id = RL_MsgCloseCamRobot_ID;
  msg->head.len = sizeof(RL_MsgCloseCamRobot);
  msg->head.crc = 0;
  
  msg->rid = rid;

  return sendSendBuffer(msg->head.len);
}

int RoboLookProxy::addMarkers(int env, RL_MarkerItem* marker, int n,
                              bool replace)
{
  if (checkConn()) return 1;

  //fprintf(stderr, "Sending marker message\n");

  int nbytes = sizeof(RL_MsgAddItems) + n * sizeof(RL_MarkerItem);
  RL_MsgAddItems *msg = (RL_MsgAddItems*)getMem(nbytes);

  RL_addSTX(&msg->head);
  msg->head.id = RL_MsgAddMarkers_ID;
  msg->head.len = nbytes;
  msg->head.crc = 0;
  
  msg->env = env;
  msg->replace = (int)replace;
  msg->n = n;
  
  memcpy(((char*)msg) + sizeof(RL_MsgAddItems), marker, 
         n * sizeof(RL_MarkerItem));
  
  return sendSendBuffer(msg->head.len);
}

int RoboLookProxy::clearMarkers(int env)
{
  if (checkConn()) return 1;

  //fprintf(stderr, "%.6f: Sending clear markers for env %d\n", 
  //      getCurrentTime(), env);

  RL_MsgClearItems *msg = 
    (RL_MsgClearItems*)getMem(sizeof(RL_MsgClearItems));
  RL_addSTX(&msg->head);
  
  msg->head.id = RL_MsgClearMarkers_ID;
  msg->head.len = sizeof(RL_MsgClearItems);
  msg->head.crc = 0;
  
  msg->env = env;

  return sendSendBuffer(msg->head.len);
}

int RoboLookProxy::addCubes(int env, RL_CubeItem* cube, int n,
                              bool replace)
{
  if (checkConn()) return 1;

  //fprintf(stderr, "Sending cube message\n");

  int nbytes = sizeof(RL_MsgAddItems) + n * sizeof(RL_CubeItem);
  RL_MsgAddItems *msg = (RL_MsgAddItems*)getMem(nbytes);

  RL_addSTX(&msg->head);
  msg->head.id = RL_MsgAddCubes_ID;
  msg->head.len = nbytes;
  msg->head.crc = 0;
  
  msg->env = env;
  msg->replace = (int)replace;
  msg->n = n;
  
  memcpy(((char*)msg) + sizeof(RL_MsgAddItems), cube, 
         n * sizeof(RL_CubeItem));
  
  return sendSendBuffer(msg->head.len);
}

int RoboLookProxy::clearCubes(int env)
{
  if (checkConn()) return 1;

  //fprintf(stderr, "%.6f: Sending clear doors for env %d\n", 
  //      getCurrentTime(), env);

  RL_MsgClearItems *msg = 
    (RL_MsgClearItems*)getMem(sizeof(RL_MsgClearItems));
  RL_addSTX(&msg->head);
  
  msg->head.id = RL_MsgClearCubes_ID;
  msg->head.len = sizeof(RL_MsgClearItems);
  msg->head.crc = 0;
  
  msg->env = env;

  return sendSendBuffer(msg->head.len);
}

int RoboLookProxy::addStars(int env, RL_StarItem* star, int n,
                              bool replace)
{
  if (checkConn()) return 1;

  //fprintf(stderr, "%.6f: Sending star message %d\n",
  //      getCurrentTime(), n);

  int nbytes = sizeof(RL_MsgAddItems) + n * sizeof(RL_StarItem);
  RL_MsgAddItems *msg = (RL_MsgAddItems*)getMem(nbytes);

  RL_addSTX(&msg->head);
  msg->head.id = RL_MsgAddStars_ID;
  msg->head.len = nbytes;
  msg->head.crc = 0;
  
  msg->env = env;
  msg->replace = (int)replace;
  msg->n = n;
  
  memcpy(((char*)msg) + sizeof(RL_MsgAddItems), star, 
         n * sizeof(RL_StarItem));
  
  return sendSendBuffer(msg->head.len);
}

int RoboLookProxy::clearStars(int env)
{
  if (checkConn()) return 1;

  //fprintf(stderr, "%.6f: Sending clear stars for env %d\n", 
  //      getCurrentTime(), env);

  RL_MsgClearItems *msg = 
    (RL_MsgClearItems*)getMem(sizeof(RL_MsgClearItems));
  RL_addSTX(&msg->head);
  
  msg->head.id = RL_MsgClearStars_ID;
  msg->head.len = sizeof(RL_MsgClearItems);
  msg->head.crc = 0;
  
  msg->env = env;

  return sendSendBuffer(msg->head.len);
}

int RoboLookProxy::addLines(int env, RL_LineItem* line, int n,
                            bool replace)
{
  if (checkConn()) return 1;
  
  //fprintf(stderr, "%.6f: Sending line message %d\n",
  //      getCurrentTime(), n);
  
  int nbytes = sizeof(RL_MsgAddItems) + n * sizeof(RL_LineItem);
  RL_MsgAddItems *msg = (RL_MsgAddItems*)getMem(nbytes);
  
  RL_addSTX(&msg->head);
  msg->head.id = RL_MsgAddLines_ID;
  msg->head.len = nbytes;
  msg->head.crc = 0;
  
  msg->env = env;
  msg->replace = (int)replace;
  msg->n = n;
  
  RL_LineItem *lptr = (RL_LineItem*)(msg + 1);

  memcpy(lptr, line, n * sizeof(RL_LineItem));

  return sendSendBuffer(msg->head.len);
}

// Returns 0 if sending ok, else 1
int RoboLookProxy::clearLines(int env)
{
  if (checkConn()) return 1;
  
  //fprintf(stderr, "%.6f: Sending clear lines for env %d\n", 
  //      getCurrentTime(), env);

  RL_MsgClearItems *msg = 
    (RL_MsgClearItems*)getMem(sizeof(RL_MsgClearItems));
  RL_addSTX(&msg->head);
  
  msg->head.id = RL_MsgClearLines_ID;
  msg->head.len = sizeof(RL_MsgClearItems);
  msg->head.crc = 0;
  
  msg->env = env;

  return sendSendBuffer(msg->head.len);
}

int RoboLookProxy::addPoints(int env, RL_PointItem* point, int n,
                              bool replace)
{
  if (checkConn()) return 1;
  
  //fprintf(stderr, "%.6f: Sending point message %d\n",
  //      getCurrentTime(), n);

  int nbytes = sizeof(RL_MsgAddItems) + n * sizeof(RL_PointItem);
  RL_MsgAddItems *msg = (RL_MsgAddItems*)getMem(nbytes);

  RL_addSTX(&msg->head);
  msg->head.id = RL_MsgAddPoints_ID;
  msg->head.len = nbytes;
  msg->head.crc = 0;
  
  msg->env = env;
  msg->replace = (int)replace;
  msg->n = n;
  
  memcpy(((char*)msg) + sizeof(RL_MsgAddItems), point, 
         n * sizeof(RL_PointItem));
  
  return sendSendBuffer(msg->head.len);
}

int RoboLookProxy::addPoints(int env, RL_RGBPointItem* point, int n,
                             bool replace)
{
  if (checkConn()) return 1;
  
  //fprintf(stderr, "%.6f: Sending point message %d\n",
  //      getCurrentTime(), n);

  int nbytes = sizeof(RL_MsgAddItems) + n * sizeof(RL_RGBPointItem);
  RL_MsgAddItems *msg = (RL_MsgAddItems*)getMem(nbytes);

  RL_addSTX(&msg->head);
  msg->head.id = RL_MsgAddRGBPoints_ID;
  msg->head.len = nbytes;
  msg->head.crc = 0;
  
  msg->env = env;
  msg->replace = (int)replace;
  msg->n = n;
  
  memcpy(((char*)msg) + sizeof(RL_MsgAddItems), point, 
         n * sizeof(RL_RGBPointItem));
  
  return sendSendBuffer(msg->head.len);
}

// Returns 0 if sending ok, else 1
int RoboLookProxy::clearPoints(int env)
{
  if (checkConn()) return 1;
  
  //fprintf(stderr, "%.6f: Sending clear points for env %d\n", 
  //      getCurrentTime(), env);

  RL_MsgClearItems *msg = 
    (RL_MsgClearItems*)getMem(sizeof(RL_MsgClearItems));
  RL_addSTX(&msg->head);
  
  msg->head.id = RL_MsgClearPoints_ID;
  msg->head.len = sizeof(RL_MsgClearItems);
  msg->head.crc = 0;
  
  msg->env = env;

  return sendSendBuffer(msg->head.len);
}

int RoboLookProxy::addTriangles(int env, RL_TriangleItem* triangles, int n,
                                bool replace)
{
  if (checkConn()) return 1;
  
  //fprintf(stderr, "Sending triangle message %d\n", n);

  int nbytes = sizeof(RL_MsgAddItems) + n * sizeof(RL_TriangleItem);
  RL_MsgAddItems *msg = (RL_MsgAddItems*)getMem(nbytes);

  RL_addSTX(&msg->head);
  msg->head.id = RL_MsgAddTriangles_ID;
  msg->head.len = nbytes;
  msg->head.crc = 0;
  
  msg->env = env;
  msg->replace = (int)replace;
  msg->n = n;
  
  memcpy(((char*)msg) + sizeof(RL_MsgAddItems), triangles, 
         n * sizeof(RL_TriangleItem));
  
  return sendSendBuffer(msg->head.len);
}

// Returns 0 if sending ok, else 1
int RoboLookProxy::clearTriangles(int env)
{
  if (checkConn()) return 1;
  
  //fprintf(stderr, "%.6f: Sending clear triangles for env %d\n", 
  //      getCurrentTime(), env);

  RL_MsgClearItems *msg = 
    (RL_MsgClearItems*)getMem(sizeof(RL_MsgClearItems));
  RL_addSTX(&msg->head);
  
  msg->head.id = RL_MsgClearTriangles_ID;
  msg->head.len = sizeof(RL_MsgClearItems);
  msg->head.crc = 0;
  
  msg->env = env;

  return sendSendBuffer(msg->head.len);
}

int RoboLookProxy::writeSingleFrame()
{
  if (checkConn()) return 1;
  
  RL_MsgHead *head = (RL_MsgHead*)getMem(sizeof(RL_MsgHead));
  RL_addSTX(head);
  
  head->id = RL_MsgGrabSingleFrame_ID;
  head->len = sizeof(RL_MsgHead);
  head->crc = 0;

  return sendSendBuffer(head->len);
}

int RoboLookProxy::startWritingFrames()
{
  if (checkConn()) return 1;
  
  RL_MsgHead *head = (RL_MsgHead*)getMem(sizeof(RL_MsgHead));
  RL_addSTX(head);
  
  head->id = RL_MsgStartGrabbing_ID;
  head->len = sizeof(RL_MsgHead);
  head->crc = 0;

  return sendSendBuffer(head->len);
}

int RoboLookProxy::stopWritingFrames()
{
  if (checkConn()) return 1;
  
  RL_MsgHead *head = (RL_MsgHead*)getMem(sizeof(RL_MsgHead));
  RL_addSTX(head);
  
  head->id = RL_MsgStopGrabbing_ID;
  head->len = sizeof(RL_MsgHead);
  head->crc = 0;

  return sendSendBuffer(head->len);
}

int RoboLookProxy::setGhostState(int rid, bool on)
{
  if (checkConn()) return 1;
  
  RL_MsgGhostState *msg = (RL_MsgGhostState*)getMem(sizeof(RL_MsgGhostState));
  RL_addSTX(&msg->head);
  
  msg->head.id = RL_MsgSetGhostState_ID;
  msg->head.len = sizeof(RL_MsgGhostState);
  msg->head.crc = 0;

  msg->rid = rid;
  msg->on = short(on);

  return sendSendBuffer(msg->head.len);
}
