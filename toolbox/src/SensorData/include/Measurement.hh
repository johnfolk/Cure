// = AUTHOR(S)
//    John Folkesson
//    
//    August, 2004
 //
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_MEASUREMENT_HH
#define CURE_MEASUREMENT_HH


#include "Matrix.hh" 
#include "ShortMatrix.hh" 

namespace Cure {
  /**
   * The Measurement Object is a rather generic holder of information
   * on measurments.  The combination of MeasurementType and
   * SensorType tell about how a particular measurement should be
   * interpreted.  These are set by the object that creates a
   * measurement such as a feature extraction object.  The Measurments
   * are used then by fro example a Localization or SLAM Object.  The
   * important information is how to match the measurement to the
   * world model (Map) and then what does this measurement add to our
   * knowledge about the feature and or robot pose.  It is this information
   * that is stored in the various matricies and integers of this object.
   * 
   *  @author John Folkesson
   */
  class Measurement
  {

  public:
    /**
     * Type of measurement
     */
    unsigned short MeasurementType;  

    /**
     * Tells what type of sensor was used to create this measurement
     * @see SensorData.hh for a list of SensorTypes
     */  
    unsigned short SensorType;

    /**
     * There might be more than one sensor of a certain type and
     * therefore we need a way to tell them apart.
     */
    unsigned short SensorID;

    /**
     * Key that will be the same for measurements that are matched over
     * time. That is, if the extraction mechanism has some way of
     * telling if a certain measurement comes from the same structure
     * in the environment it should try to use the same Key over time.
     * The Key is essentially a track id.
     */
    unsigned long Key;

    /**
     * The column vector Z gives some information on the measurment that
     * can be helpful when matching the maesurement to the world model.
     */
    Cure::Matrix Z;

    /**
     * Used for rough matching where the bounding box is used to see
     * if the measurement comes in the right ball park of a feature.
     */
    Cure::Matrix BoundingBox;

    /**
     * Dense information can be used to add Information/Initialization
     * of features or for any other general data about this measurement.
     */
    Cure::Matrix W;
    Cure::ShortMatrix SW;

    /**
     * Used for Update of Features.  This is normally a column vector
     * that describs the measurement.  The V vector can be an
     * over representation of the information in the measurement.  The
     * MeasurementType tells the MapFeature how to use V and CovV and
     * which directions in the V space contain useful information.
     */
    Cure::Matrix V;
    /**The Covariance estimate of V*/
    Cure::Matrix CovV;

  protected:
  public:
    /**
     * The constructor.
     */
    Measurement();
  public:
    /**
     * The copy constructor.
     * @param m the measurment to copy.
     */
    Measurement(Measurement &m);
    ~Measurement();
    /**
     * The copy operator.
     * @param m the measurment to copy.
     */
    void operator = (Measurement &m);
    /**Display info on the screen.*/
    void print(int level=0);
  /**
   * This returns data based on the string value and the indcies
   * given in the ShortMatrix.  
   *
   * @param str So Key, Z,W,SW, V, CovV and BoundingBox 
   * are recongnized strings.
   * 
   * @param index Set the index as when you would call
   * Cure::Matrix.offset(roff,coff,r,c) with the integers
   * roff,coff,r,c placed in a the top row of index (1x5).
   * if (index(0,4) is -1 then index(0,i)i<2 are ignored and
   * a matrix to hold all of the requested data from str is 
   * appended to rows of mat with mat.Columns=index(0,3).
   * Number of rows appended is given by index(0,2).
   * Extra space is filled with 0.  
   * 
   * @param mat the result is returned here in a r x c matrix
   * @return true if ok else false. 
   */
    bool getMatrix(const std::string & str, ShortMatrix & index,
		   Matrix &mat);
    bool setMatrix(const std::string & str, ShortMatrix & index,
		   Matrix &mat);

  };
}
#endif
