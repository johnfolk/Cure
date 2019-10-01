// = AUTHOR(S)
//    John Folkesson
//
// = COPYRIGHT
//    Copyright (c) 2005 John Folkesson
//
/*----------------------------------------------------------------------*/

#ifndef CURE_GRIDDATA_hh
#define CURE_GRIDDATA_hh


#include "Pose3D.hh"
#include "BinaryMatrix.hh" 
#include "Matrix.hh" 
#include "SensorData.hh"
#include "SonarData.hh"
#include "SICKScan.hh"

#ifndef DEPEND
#include <iostream>  // ostream
#endif

// Forward declaration(s)
class PackedGrid;
namespace Cure {


/**
 * Timestamped data containing a binary grid that can be used for
 * occupied/free space grids and also have some relavent functionality.
 *
 * Patric has written a simple X-Display viewer to see the grids.
 *
 *  if ((gd.Grid.Rows>0)&(gd.Grid.Columns>0))(
 *
 *      Cure::XDisplayBinaryMatrix disp(gd.Grid, 0);
 *
 *	 usleep(200000);
 *
 *	 disp.updateDisplay();
 *
 *       getchar();
 *    }
 *
 * will make a window pop up untill you hit enter.  You need to
 * call this updateDisplay after some delay apparently to give the
 * X-server time to set up the window first???  Also don't do it without
 * checking that there are cells in the grid.  So the XDisplayBinaryMatrix is 
 * not always built (if you haven't got X11).
 *
 * Otherwise this class is quite useful allowing the fast combining of grids 
 * with the offsets and grid sizes automatically adjusted.  The BinaryMatrix
 * class implements binaries as bits in an unsigned long array and is very 
 * efficient at doing these combining operations.
 *
 * The grids are set up with a cell width and xzy offset. 
 *
 * Summary of use:
 *
 *  Here is one way I have used these:
 *  I need to create a GridData with the scaned region of a set of 
 *  'sonars' that represent the 'seen' volumn of space at a given robot pose.
 * 
 *  GridData gd(m_GridCellWidth); 
 *
 *   gd.setSubType(8);//ie 1 means sonar hit grid cell 
 *
 *    gd.Offset[0]=0;  //This will get set automatically
 *
 *    gd.Offset[1]=0;  //This will get set automatically
 *
 *    gd.Offset[2]=-3.5; //Have to give a height (it matters)
 * 
 *   gd.Time=robotpose.Time;
 *
 *    SonarData sd;
 *
 *    Transformation3D sonpose;
 *
 *    for (int i=0;i<m_NumberOfSonars;i++){
 *
 *            sonpose=robotpose;
 *
 *	      sonpose+=m_SensorOffsets[i]; //offsets form the robot frame
 *
 *	      sd.Range=40
 * 
 *	      sd.BeamWidth=m_BeamWidth[i];  /the angle from side to side
 *
 *            sd.MaxRange=60;
 *
 *	     addSonar(sd,sonpose)
 *
 *  }
 *  
 * That will end up with gd as the union of all the intersections of
 * sonar beams with the z=-3.5 plane.  The offsets and
 * Grid.Rows/Columns are set automatically based on the pose and
 * Range/BeamWidth. These take account of the 32 bit boundaries of the 
 * longs of the BinaryMatrix for optimal operation.
 *  
 * The sonar can point in an arbitrary direction and will scan
 * a cone out to its range in that direction.  The conic section with 
 * the horizontal grid plane is calculated for both the scaned region
 * and the arc of possible object locations on the arc. So different 
 * SubTypes will give the free space or possibly filled etc..
 *
 * There are lots more possiblities this is an easy one.
 *
 * @author John Folkesson 
 */
class GridData :  public TimestampedData 
{  
public:
public:
  /**
   * The Grid is a binary representation of some spatial data.
   * x=rows y=coluns
   */
  Cure::BinaryMatrix Grid;

  /**
   * This sets the width of each grid cell in meters.
   */
  double CellWidth;

  /**
   * This is the xyz position of the center of the (0,0) cell in some
   * frame of reference (meters). Rotation is assumed to be 0.
   *
   */
  double Offset[3];

  /**
   * This holds the type of grid.  
   *
   * FreeSpace Grid is 1: The zeros are belived to be free of obstacles
   * 
   * Occupancy Grid is 2: The Ones are believed to contain obstacles.
   * 
   * Contains Occupied is 4 We believe that one of the cells with ones
   * is occupied but we can not tell which.
   * 
   */
  //unsigned short GridType;

  /**
   * The sensor that created this data.  @see Cure::SensorData.hh
   * SENSORTYPE_SICK, SENSORTYPE_CAMERA, SENSORTYPE_SONAR,
   * SENSORTYPE_CONTACT,SICK,SONAR It is of course possible that this
   * grid uses several sensors or SENSORTYPE_UNKNOWN = 0,.
   */
  unsigned short SensorType;
  
  /**
   * This tells how old the grid is as oppossed to Time which indicates
   * when it was last updated.
   */
  Cure::Timestamp Birthday;
  

public:
  
  friend class PackedGrid;

  /**
   * Create empty GridData
   * @param id the id of this
   */
  GridData(double cellwidth=.03125, unsigned short id = 0);

  /**
   * Create empty GridData
   * @param t the time
   * @param id the id of this
   */
  GridData(Timestamp &t,unsigned short id = 0);

  /**
   * Create a copy of a GridData
   * @param s The object to copy
   */
  GridData(const GridData &s);

  /**called by consturctors*/
  void init();

  /**
   * Destructor
   */
  virtual ~GridData();

  /*
   * This is how we (efficiently) get a GridData from a TimestampedData.
   */
  GridData * narrowGridData(){return this;}

  /**
   * Copy operator
   * @param src The object to copy
   */
  GridData& operator=(const GridData &src);  

  /**
   * Copy operator
   * @param p The object to copy
   */
  void operator = (TimestampedData& p)
  {
    GridData *ss=p.narrowGridData();
    if (ss)operator=(*ss);
    else TimestampedData::operator=(p);
  }


  /**
   * Assign all cells in the grid true or false (remeber that it is a
   * binary grid).
   * @param v value to assign to all cells
   */
  void operator = (bool v)
  {
    Grid=v;
  }  

  /**
   * Assign the cells in this grid the "negation" of what is in
   * another grid
   *
   * @param gd the othe rgrid to negate
   */
  void equalsNot(GridData & gd);


  /**
   * Clear the grid
   */
  void clear(){
    Grid=0;
    Grid.reallocate(0);
    Offset[0]=0;
    Offset[1]=0;
    Offset[2]=0;
  }

  /**
   * This holds the type of grid.  
   *
   * FreeSpace Grid is 1: The zeros are belived to be free of obstacles
   * 
   * Occupancy Grid is 2: The Ones are believed to contain obstacles.
   * 
   * Contains Occupied is 4 We believe that one of the cells with ones
   * is occupied but we can not tell which.
   * 
   * @param t the new subtype
   */ 
  void setSubType(unsigned short t);

  /**
   * 
   * @param rows the number of rows for the grid.
   * @param columns the number of columnss for the grid (mult of 32 is best).
   * @param cellwidth the width of the cells in meters. 
   * @param offset the offset of the center of the (0,0) cell in meters.
   * @param t the time for the birthday.
   * @param gtype the SubType to create.
   */
  void setupGrid(unsigned short rows, unsigned short columns, 
		 double cellwidth, double offset[3], 
		 Cure::Timestamp t, unsigned short gtype);
 
 /**
   * The grids are adjusted based on the Offsets to the overlap.
   * The grids must have the same CellWidth.
   * This method does different types of updates
   * on the grid depending on the SubType
   *
   * For gd and this both type=1 (free space grids):
   *
   * We simply add the free space (0's) from the overlapp of gd with this.
   *
   * For this type 2 and gd type 2 (or 4)(occupancy grids):
   *
   * We simply add the occupied cells from gd to this
   *
   * For this type 2 (or 4) and  gd type 1:
   *
   * We remove the free space from this.  If this ends up empty
   * (returns 0) We should add an original copy of this to gd as
   * something labeled as free must be occupied.  If this ends up with
   * only a small cluster of cells then we have a localized object.
   *
   * For this type 1 and gd type 2 (or 4):
   *
   * We add the occupied cells from gd to this.  This might be after
   * the case above returned no explaination.
   *
   *
   * @param gd the grid to update with.
   * @param givereturn if 0 this skips counting the cells; 
   * @return the number of set cells in the overlap region if givereturn.
   */
  int update(GridData &gd,int givereturn=0);

  /**
   * Here we add the information collected on this GridData to 
   * accumulators stored on a ShortMatrix object gd.  The gd is
   * treated similar to our Grid with a 2D offset gdoffset.
   * the z offsets are assumed to be equal and the 
   * relative rotation is 0.  One can rotate this grid prior to
   * calling this if needed.  CellWidths are assumed the same.
   * If type&1  we add one to gd's cells for each of our empty cells.
   * For type&6  we add one for each full cell. 
   * The difference in offsets is simply rounded off and no interpolation is 
   * done.  We grow gd if needed to be able to hold all our grid as
   * wel as adjusting the gdoffset 
   *
   * @param gd the accumulator matrix
   * @param gdoffset the xy of the gcenter of cell (0,0)
   */
  void accumulate(ShortMatrix &gd,double gdoffset[2]);

  void trim(int amount);

  void trimRows(int amount);

  void trimColumns(int amount);
  /**
   * Fills all empty cells that are reachable from seed by 4-adjacency.
   *
   * @param gd the filled grid is returned here.
   * @param seed the coordiantes for the seed in  meters
   * @param radius the grid is grown by this amount first.  
   * @return -1 if seed not in range else the number of cells filled.
   */  
  int cluster(GridData &gd, double seed[2],double radius=0);
  /**
   * Trims the grid by an amount from all sides. 
   * @param shrinkage the amount to trim grid from all sides
   */
  void shrink(double shrinkage);
  /**
   * This clears the grid and makes a new one with 1's in
   * the xy positions of the list. Then it grows the region from the seed
   * seting all bits that are 4-neighbors to a set bit stoping at the 
   * edge.  The rest of the grid will be 0's.
   * @param edge a list of xy points in meters each row should
   * be (x, y) and there should be no 4 adjacency gaps.
   * @param seed the point to fill first (inside the region)
   */
  void fillPoints(Cure::Matrix &edge, double seed[2]);
  /**
   * This clears the grid and makes a new one with val in
   * the xy positions of the list and not val everywhere else.
   *
   * @param val the value to pu in the cells
   * @param edge a list of xy points in meters each row should
   * be (x, y) and there should be no 4 adjacency gaps.
   *
   */
  void setPoints(Cure::Matrix &edge, bool val);
  /**
   * Trims the grid by an amount from all sides. 
   * @param gd the shrunken grid is returned here
   * @param shrinkage the amount to trim grid from all sides
   */
  void shrinkInto(GridData &gd, double shrinkage);

  /**
   * Change the size of the grid to cover a region. 
   * Type 1 will fill with 1's if needed. 
   * Types 2 nad 4 with 0's.
   * @param gd the resized grid is returned here
   * @param box the new region in format 2x2, row 0=(minx miny) row 1=
   * maxx maxy.
   */
  void  resizeGrid(GridData &gd, Cure::Matrix &box );
 
  /**
   * This will make a grid with a circlular disk either cleared or set.
   * This will clear any information in this GridData
   * @param radius of the disk in meters.
   * @param diskPose the pose of the disk center
   * @param gtype the SubType to create. gtype&1 will make a 
   *  disk of 0 surrounded by  1's else make a disk of 1's surrounded by 0s.
   */
  void disk(double radius, Cure::Pose3D &diskPose, 
	    unsigned short gtype);
  
  /**
   * This will make a grid with a circle of cells either cleared or set.
   * This will clear any information in this GridData
   * @param radius of the circle in meters.
   * @param diskPose the pose of the circle center
   * @param gtype the SubType to create. gtype&1 will make a 
   *  circle of 0 surrounded by  1's else make a circle
   *  of 1's surrounded by 0s.
   */
  void circle(double radius, Cure::Pose3D &diskPose, 
	      unsigned short gtype);

  /**
   * Finds the nearest point to a given center point in a given direction.
   * @center the point (x,y) to calc the distance from in meters.
   * @param points the closest point is returned in a matrix of
   *        x y value.
   * @param direction a unit vector in the search direction;
   * @param maxdistance the limit of how far to look for points (m)
   * @return the distance to the nearest point (if none then maxdistance) 
   */
  double nearest(double center[2], Cure::Matrix &point, 
		 double direction[2],double maxdistance);

  /**
   * Finds the nearest clusters of points to a given center point.
   * @center the point (x,y) to calc the distance from in meters.
   * @param points the closest point in each cluster is returned in a matrix of
   *        x y values.
   * @param maxdistance the limit of how far to look for points (m)
   * @param mindistance the lower limit of where to look (m).
   * @return the distance to the nearest point (if none then maxdistance) 
   */
  double nearest(double center[2], Cure::Matrix &points, 
		 double maxdistance,
		 double mindistance=0);

  /**
   * This will make a grid of the robot shape.  If the
   * verticies.Rows<3 the robot is assumed to be a circle of radius
   * verticies(0,0).  Further, if verticies.Columns>2 the xy
   * displacement of the center of the circle relative to robotPose is
   * taken a verticies(0,1) and (0,2).  If Columns is 2 then verticies(0,1)
   * is taken as x and y=0. 
   * 
   * This will clear any information in this GridData
   * 
   * @param verticies The list of polygon veticies in the RobotFrame (meters).
   *                  These must go counter-clockwise around the robot.
   * @param robotPose the pose of the robot (frame for verticies)
   * @param gtype the SubType to create.
   */
  void robot(Cure::Matrix &verticies, Cure::Pose3D &robotPose, 
	     unsigned short gtype);
  
  /**
   * This will make a grid of the robot shape.  If the
   * verticies.Rows<3 the robot is assumed to be a circle of radius
   * verticies(0,0).  Further, if verticies.Columns>2 the xy
   * displacement of the center of the circle relative to robotPose is
   * taken a verticies(0,1) and (0,2).  If Columns is 2 then verticies(0,1)
   * is taken as x and y=0. 
   * 
   * If SubType is 1 this makes the robot as free space added to the existing
   * free space of this gridData.
   *
   * If SubType is 2 the robot is added as occupied space added to
   * the occupied space of this grid.
   * 
   *
   * @param verticies The list of polygon veticies in the RobotFrame (meters).
   *                  These must go counter-clockwise around the robot.
   * @param robotPose the pose of the robot (frame for verticies)
   */
  void addRobot(Cure::Matrix &verticies, Cure::Pose3D &robotPose);

  /**
   * This will create a grid at a given height (z) based on
   * SubType and  Sonar info.  It assumes a horizontal sonar direction.
   * So if Grid type is 1 its the free space grid implied by this sonar
   * data at this hieght.
   * If gtype is 2 of 4 then it is the ocuppancy grid so the 
   * set of cell that could be the cause of this sonar reading.
   * If gtype is 8 the result is logical not of the gtype 1 case.
   *
   *    
   * This will clear any information in this GridData
   *
   * @param sonar the sonar data
   * @param sensorPose the pose of the sonar (y-axis along the beam center.)
   * @param gtype what type of grid do you want.

   * @param height the hieght of the grid you want in the same frame as
   * sensorPose was given in.
   */
  void sonar(Cure::SonarData &sonar, 
	     Cure::Transformation3D &sensorPose, 
	     unsigned short gtype,double height);

  /**
   * As same as above, but with a sick scan.
   *
   * This will clear any information in this GridData
   *
   * @param sick the sickscan
   * @param sensorPose the pose of the sonar
   * @param gtype what type of grid do you want
   * @param height the height of the grid you want in the same frame as
   * sensorPose was given in.
   */
  void sick(Cure::SICKScan &sick,
	    Cure::Transformation3D &sensorPose,
	    unsigned short gtype, double height);
  
  /**
   * This will create a grid at a given height (z) based on
   * SubType and  Sonar info.  It will calculate the full
   * conic section so that if the sonar is tilting up it can make
   * an ellipse or even a circle on this horizontal grid.
   * So if Grid type is 1 its the free space grid implied by this sonar
   * data at this hieght.
   * If gtype is 2 or 4 then it is the ocuppancy grid so the 
   * set of cell that could be the cause of this sonar reading.
   * If gtype is 8 the result is logical not of the gtype 1 case.
   *    
   * This will clear any information in this GridData
   *
   * @param sonar the sonar data
   * @param sensorPose the pose of the sonar (y-axis along the beam center.)
   * @param gtype what type of grid do you want.

   * @param height the hieght of the grid you want in the same frame as
   * sensorPose was given in.
   */
  void sonar3D(Cure::SonarData &sonar, 
               Cure::Transformation3D &sensorPose, 
               unsigned short gtype, double height);

  /**
   * This will add sonar information to this grid
   *    
   * So if Grid type is 1 its the free space grid implied by this sonar
   * data at this hieght.
   * If gtype is 2 of 4 then it is the ocuppancy grid so the 
   * set of cell that could be the cause of this sonar reading.
   * If gtype is 8 the result is logical not of the gtype 1 case.
   *
   *    
   *
   * @param sonar the sonar data
   * @param sensorPose the pose of the sonar (y-axis along the beam center.)
   */
  void addSonar(Cure::SonarData &sonar, 
		Cure::Transformation3D &sensorPose);

  /**
   * This grows to a size that includes gd and then does and update with gd.
   * @param gd the data to append. 
   */
  void append(GridData &gd);  

  /**
   * This takes a path and generates a control signal by fitting the 
   * path to an arc(or line) and then moving the robot towards that arc.
   * If the robot is pointing more or less along the arc it will
   * move forward while turning to an angle tangent to the arc.
   * If the robot angle is too different than the arc tangent
   * the robot will turn with 0 radius to the tangent.
   * the path is checked for filled cells on the grid as far 
   * as the  command period.  The velocity is adjusted down
   * when a long arc can't be fit.
   *
   *
   * @param velocity the control signal to the robot is returned here
   * as linear velocity, angularvelocity, timeout=2*commandperiod.
   * @param path the path as row column indexes in this grid.
   * @param robotangle the angle the robot is heading in from the x axis.
   * @commandperiod the time between appling these control signals.
   * @param maxdeviation the path will be followed about this closely
   * in meters. 
   * @param maxlinearvelocity (meters/sec) highest allowed value for 
   * velocity(0,0).
   * @param maxangularvelocity (rads/sec) highest allowed value for 
   * velocity(0,1).
   * @return 0 if ok else 1 if can't follow an arc use some other method.
   */
  int  drivePath(Cure::Matrix &velocity,
		 Cure::ShortMatrix &path, 
		 double robotangle,
		 double commandperiod,
		 double maxdeviation=.1,
		 double maxlinearvelocity=1,
		 double maxangularvelocity=1);

  /**
   * Calculates the path between two points on the grid.
   * filled cells are considered obstacles.
   * @param fromx the xy of the start point (meters).
   * @param tox the xy of the destination point (meters).
   * @param path the indexs of the grid cells along the path are returned here
   *        as (row,col) in a nx2 matrix.  Note that Offset must not change
   *        while you use this.  Offset can change if you append for instance.
   * @param searchlimit (meters)the path search will not find paths 
   * longer than this.
   * @return -1 if no path can be  found
   *          0 if ok
   *          1 if a partial path is found, topose is off the grid path is 
   *            to nearest free point on edge of grid.
   */
  int path(double fromx[2],double tox[2],
	   Cure::ShortMatrix &path, double searchlimit);

  /**
   * This will create a navigation grid from this grid by growing 
   * out from all filled cells by an amout radius.  This if radius is the 
   * closest that the center of the robot should come to obstacles then
   * the free space grid gd will be free where the robot can go.
   * @param gd the free space navication grid is returned here.
   * @radius the safety radius of the robot in meters.
   */
  void navSpace(GridData &gd, double radius);

  /**
   * This moves the path one step horizontally or vertically
   * away from any filled cells within maxdist of it.  
   * @param path the path to bend as list (r,c)
   * @param maxdist the futhest filled cells to consider (meters).
   * @param fromind the start index to bend from
   * @param toind the end index to bend to.
   */ 
 void bendPath(ShortMatrix &path, double maxdist, int fromind, int toind);

  /**
   * This will move the path away from filled cells by a number of steps.
   * If the path is beteen two obstacle at the same distance it won't
   * move.  
   * @path the path to bend as list (r,c) 
   * @amount The number of steps to move the path.
   * @maxdist the futhest filled cells to consider, (meters).
   */
  void massagePath(ShortMatrix &path, int amount,double maxdist);

  /**
   * This calculates the moments of the free space around a point
   * (x,y) out to a specified distance form (x,y).  It grows the free
   * space from xy without leaving the disk or radius range around x.
   * It will therefore not include parts of the free space that may be
   * within range but can't be reached from x without leaving the
   * disk.  
   *
   * The moments are in units of  meters.  
   * 
   * The range should be > 4*CellWidth for any kind of good result.
   *
   * @param x the point to calculate the moments around in meters (not cells).
   * @param range the radius of the disk that limits the region.
   * @param n the zero moment (number of cells in free space around x)
   * @param mean a column vector of the mean x (rows) and y (columns)
   * of the free space relative to x,
   * free space, (relative to the row and column of x).
   * @param cov the second moment of the row and columns of the free
   * space around the mean (2x2).
   * @param top if 8 the growth will move diagonally else only to 4 neighbors.
   * @param cubic this should be declared an array of Cure::Matrix cubic[2].
   * It will return with the third moments about the mean (so (2) x 2 x 2).
   * If left out this calculation is skiped.
   */
  void calcMoment(double x[2], double range,
		  int &n, Matrix &mean,Matrix &cov,
		  int top=4,Matrix *cubic=0);

  /**
   *
   * An invariant discriptor is formed from the moments of the free space.
   *
   * the 0 descritor will vary with location as will 1.  The other
   * descriptors will tend to be similar over small changes in
   * location.  1 and 4 can have thier sign reversed relative to a
   * 'close' descriptor due to the ambiguity of defining a direction
   * for the major axis of the second moment.  If the 0 descritor is
   * too small or 1 near 1 or -1 then both +/- values for 1 and 4
   * should be matched to, (ie branch the search into 2 one for the +
   * values and one for -1 times the values).
   *
   * All other descriptors are invariant and smoothly varing.
   *
   * decription(0,0)=magnitude of the mean.  
   *
   * decription(0,1)= cos of the angle between the mean and the major
   * axis of cov.  The sign of the major axis direction v is set by
   * requiring that the cross product mean x v >0.  This then results in
   * 1 and -1 being the same.  If this is close to 1 or -1 then it may
   * be near descriptions with signs reversed for this and descrition(4).
   * That means two branches in the search for a match.  
   * If the mean is too small this will not be very informative so that
   * we multiply it by a sigmoid function based on description(0,0).
   * Also if the condition number of the cov matrix is 1 the major axis 
   * is not determined.  Therefore we multipy by (1-exp(-2(cond-1)).
   *
   *
   * description(0,2)= larger eigenvalue of cov.
   *
   * description(0,3)= smaller eigenvalue of cov.
   *
   * description(0,4)= the cos of the angle between t and the major
   * axis.  If the condition number of the cov amtrix is 1 the major
   * axis is not determined.  Therefore we multipy by
   * (1-exp(-2(cond-1)).  If the mean is too small or description(1,0)
   * is near +/-1 then the sign of this term might be 'wrong' and a
   * branch in the search made.
   *
   * description(0,5)= the magnitude of the vector t=the square
   * distance weighted mean:
   *
   * tx=sum(x*(x*x+y*y))/n 
   *
   * ty=sum(y*(x*x+y*y))/n
   *
   * frame 001
   *
   * description(0,6)= the cubic moment in the direction of t.
   *
   * description(0,7)= the cubic moment in the direction (ty,-tx)
   *
   * @param description returns a row of descriptors length 8 if cubic
   * else 4. 
   * @sigmoidrange if the mean is close to zero then description(0,1)
   * has little meaning.  It is therefore multiplied by a
   * (1-exp-|mean|/sigmoidrange) to minimize its effect in that
   * situation.
   * @mean the mean as returned from GridData::calcMoment input here.
   * @cov the cov as returned from GridData::calcMoment input here.
   * @cubic the cubic as returned from GridData::calcMoment input here.
   * @see GridData::calcMoment.
   */
  void calcDescription(Matrix &description,double sigmoidrange, 
		       Matrix &mean,
		       Matrix &cov,Matrix *cubic=0) ;

  /**
   * This will transform the grid to a new coordinate system so that
   * x=Offset+(r,c)->x'=trans.transform2D(x).
   *
   * The new offset is made to be an interger and the the Columns
   * offset, (Offset[1]), is made a multiple of 32.
   * The size of the grid will change so that no data is lost.
   * @param trans the transformation.
   */
  void transform(Cure::Transformation3D &trans); 

  int count(){return Grid.count();} 

  void read(std::fstream &fs );

  /** Display info on the Object*/
  void print(); 

  double maxX(){
    double d=Grid.Rows;
    d*=CellWidth;
    return (d+Offset[0]);
  }

  double maxY(){
    double d=Grid.Columns;
    d*=CellWidth;
    return (d+Offset[1]);
  }

  double minX(){
    return (Offset[0]);
  }

  double minY(){
    return (Offset[1]);
  }
  /**
   * Returns with the point cooresponding to the center of cell (r,c)
   * in meters with the offset added in.  
   * @param r the grid row.
   * @param c the grid column 
   * @return 1 if r c not on this grid.
   */
  int getPoint(int r, int c, double x[2])
  {
    if (!Grid.inRange(r,c))return 1;
    x[0]=r;
    x[0]*=CellWidth;
    x[0]+=Offset[0];
    x[1]=c;
    x[1]*=CellWidth;
    x[1]+=Offset[1];
    return 0;
  }

  int getRowColumn(int &r, int &c, double x[2])
  {
    double d=(x[0]-Offset[0])/CellWidth+.5;
    r=(int) d;
    d=(x[1]-Offset[1])/CellWidth+.5;
    c=(int)d;
    if (!Grid.inRange(r,c))return 1;
    return 0;
  }
  
protected:
  
private:
};
  /** 
   * Get the transform that takes mean 1 to mean2
   * @param trans the mean2=trans.transform2D(mean1);
   * @param mean1 must be a 2x1 column vector;
   * @param mean2 must be a 2x1 column vector;
   * 
   */ 
 void getTransform(Transformation3D &trans,Matrix &mean1, Matrix &mean2);
}

std::ostream& operator << (std::ostream& os, const Cure::GridData &cmd);

#endif // GridData_hh
