//    $Id: PoseCumulator.hh,v 1.9 2008/05/13 20:29:01 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_POSECUMULATOR_HH
#define CURE_POSECUMULATOR_HH

#include "PoseFilter.hh"

namespace Cure{
  
/**
 * Setting an object to pull from this with the inputs set to
 * pull from slots will cause this to look like interpolated pose data 
 * at the timestamps pulled at.   

 * This class has the function of cumulating incremetal output with
 * uncertianty.
 * 
 * It has 2 inputs and 2 outputs.
 * When input 1  is writen to 0 is read. 
 *
 * port 0, Input 0 is the Incremental change in the cumulated frame. 
 * Its SubType must be (1### #### #### ####).  It must be set to pull data.
 * The independent incremental uncertainty in that change is also included.
 *
 * port 1, Input 1 is the Cumulated Poses. 
 * Its SubType must be (0### #### #### ####).
 *
 * port 2, Output 0 is the Incremental change in the cumulated frame at 
 * the pose of the last Calc().  
 * Its SubType must be (1### #### #### ####).  
 * 
 * port 3, Output 1 is the last value of input 1 for cumulated Pose.  
 * Its SubType must be (0### #### #### ####).
 *
 * Reading with with a Timestamp will cause the calculation 
 * to be attempted.
 * Reading with a Timestamp=Last calc time will only return the data 
 * stored there and not cause any new calculation.
 *
 * There must be a pull client on Port 0 for this calculation to work
 * as the individual increments need to be read.
 * If Port 1 is writen to then there is no need the pull there and the 
 * Timestamp of the data writen is cumulated to.
 * Normally Port 1 will also be set to pull then the Read output
 * or trigger timestamp will be cumulated to.
 *
 * A read on the output causes the calculation and a TIMESTAMP_ERROR
 * if the data is not available (yet) on the inputs. 
 * A read at the time of the last calc will just return the data.
 * A read before the time of the last calc will return TIMESTAMP_ERROR.
 *
 * 
 * @author John Folkesson
 */
class PoseCumulator: public PoseFilter
{
public:
  /**
   * If this is true then reads to output 0 
   * will give the incremental change from the p.Time to t
   * if false it give the change since the last calculation.
   */  
  bool ResetAllways;
protected:
  Timestamp TimeLast;
public:
  /**
   * @param allwaysreset if this is true then reads to output 0 
   * will give the incremental change from the p.Time to t
   * if false it give the change since the last calculation.
   * 
   * 
   */
  PoseCumulator( bool allwaysreset=false,unsigned short thread=0);
  virtual ~PoseCumulator(){}
  unsigned short  resetTimeLast(Timestamp t);
  virtual unsigned short getInputs(TimestampedData &inputData,int port=-1);
  virtual unsigned short calc();
  /**
   * if Reset Allways is true then reads to output 0 (port(2))
   * will give the incremental change from the p.Time to t
   * if false it give the change since the last calculation.
   * 
   * 
   */
  virtual unsigned short read(TimestampedData& p,const int port, 
			      const Timestamp t,
			      const int interpolate);

  virtual unsigned short typeCheck(int port,TimestampedData * inputData=0);  
private:
  unsigned short transformErrorCode(unsigned short &r1, int port);

};


} // namespace Cure

#endif // CURE_ANDPOSETRIGGER_HH
