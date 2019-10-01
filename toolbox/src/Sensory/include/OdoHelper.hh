// = AUTHOR(S)
//    John Folkesson
//    
//    March 11, 2004
//
//    Copyright (c) 2004 John Folkesson
//    

#ifndef ODOHELPER_H
#define ODOHELPER_H

#include "Transformation3D.hh"
/**
 * Here we can fit a bias model to a path of odometry data for which we know
 * the true start and end pose.
 * The bias model has three parameters B[3].  The first is the angle drift,
 * The second is the distance scale error, and the third is an angle scale
 * error.
 * so 
 * change in distance=ds.
 *                    ds=ds_odo*B[1].
 * and change in angle=dtheta.
 *                     dtheta=dtheta_odo*B[2]+ds*B[0].
 *                     dtheta=dtheta_odo*B[2]+ds_odo*B[0]*B[1].
 *
 * These shold be small changes.
 *
 *  
 * Dx is the difference between the true and biased odometer
 * coordinates over the entire path.
 * DX=(theta_end-theta_start, x_end-x_start, y_end-y_start)_true - 
 *   (...)_bias_odo
 *
 * A is the jacobian of biased odometry wrt. small changes to B.
 *
 * Minimize:
 *
 * Cost = (A dB-DX)^T  W (A dB-DX) + dB^T P dB.
 *
 * W is the xError weight.
 * P is the SmallBiasWeight.
 *
 * W sets which coordinate, (theta,x,y) is most important to make small.
 * P keeps the change in bias small.
 * 
 * One can set an intial bias, b, to remove most of the errors BEFORE 
 * entering the path.  
 * 
 * So the order of use is this.
 *
 * setInitialBias(b);
 * setTruePose(robotPose);
 * while (path not done)odometerData(odoPose);
 * setTruePose(robotPose);
 *  
 * Now one can change the Weights if one wants and call
 *
 * getBias(b);
 * 
 * as often as one wants (ie for different weights and number of parameters.
 *
 * The path may not be good to fit all three parameters then a simplier 1 or 2
 * parameter model can be specified in the getBias call.
 * The one parameter model uses the intial B[1] and B[2] and only 
 * calculates B[0].
 * The two parameter model constrains dB[2]=dB[1].  (ie. the change to 
 * the theta scale equals the change to the distance scale.)
 *
 * After this one can begin a new path 
 * 
 * setTruePose(robotPose);
 * while (path not done)odometerData(odoPose);
 * setTruePose(robotPose);
 * 
 *
 * The first 'odoPose' cooresponds to the first  'robotPose' 
 * The last 'odoPose' cooresponds to the second  'robotPose' 
 *
 * Note that the bias is not changed except for calls to
 * setInitialBias(b);
 * 
 * So the calculated bias is never saved implictly.  The b returned by
 * getBias(b) is the sum of the initial bias B and the calculated dB.
 *
 *
 */
class OdoHelper
{
public:
  double B[3];
protected:
  Cure::Transformation3D TrueStart;
  Cure::Transformation3D cumOdo;
  Cure::Matrix J;
  Cure::Matrix S;
  Cure::Matrix X;
  Cure::Matrix DX;
  Cure::Matrix  A;
  Cure::Matrix  W;
  Cure::Matrix P;
  Cure::Matrix DB;
  int start;

public:
  OdoHelper();
  OdoHelper(double initialBias[3],Cure::Matrix& xErrorWeight,
	    Cure::Matrix& smallBiasWeight);
  ~OdoHelper(){}
  /**
   * Call before and after entering in the path data.
   * This sets the true change in robot pose over the whole path.
   */
  void setTruePose(Cure::Transformation3D & robotPose);
  /**
   * Call this to set the bias before entering the path.
   * The new bias will be calculated by linearizing around this 
   * bias.
   */
  void setInitialBias(double b[3]){
    B[0]=b[0];
    B[1]=b[1];
    B[2]=b[2];
  }
  /**
   * Call this to enter in the odometer data along the path. 
   *
   * setTruePose(robotPose);
   * while (path not done)odometerData(odoPose);
   * setTruePose(robotPose);
   *
   *
   * The first 'odoPose' cooresponds to the first  'robotPose' 
   * The last 'odoPose' cooresponds to the second  'robotPose' 
   *   
   * @param odoPose the cumulated odometer data along the path..
   */
  void odometerData(Cure::Transformation3D & odoPose);
  /**
   * Set relative importance of errors in the three coordinates 
   * (theta, x,y).
   * @param w the weight matrix first row is theta.
   */
  void setXErrorWeight(Cure::Matrix &w);
  /**
   * Set the weight that holds the bias near the initial values.
   * @param p should be square with 3 or less rows.  The rows coorespond
   *        to the elements of DB.
   */
  void setSmallBiasWeight(Cure::Matrix &p);
  /**
   * Returns the minimum cost bias fit to the model.
   * The path may not be good to fit all three parameters 
   * then a simplier 1 or 2
   * parameter model can be specified in the getBias call.
   * The one parameter model uses the intial B[1] and B[2] and only 
   * calculates B[0].
   * The two parameter model constrains dB[2]=dB[1].  (ie. the change to 
   * the theta scale equals the change to the distance scale.)
   *
   * @param b The fit bias is returned here.
   * @param numberOfBiasParameters one can specify a 1,2 or 3 parameter model.
  */ 
  void getBias(double b[3],int numberOfBiasParameters=3);
  
  /**
   * Call this to add in the bias to the odometer data. 
   *
   * setTruePose(robotPose);
   * while (not done)addBias(odoPose);
   *
   * The function returns with odoPose changed to a the pose of
   * the same path starting at 'robotPose' and calculaated with
   * the bias included.
   */
  void addBias(Cure::Transformation3D & odoPose);
protected:


};

#endif
