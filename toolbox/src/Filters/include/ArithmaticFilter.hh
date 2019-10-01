//    $Id: ArithmaticFilter.hh,v 1.4 2007/09/14 09:13:50 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_ARITHMATICFILTER_HH
#define CURE_ARITHMATICFILTER_HH

#include "PoseFilter.hh"

namespace Cure{
  
  /**
   * This is a base class for 2 input 1 output type filters.
   * These typically do things like add two poses to get the output pose.
   * @author John Folkesson
   */
  class ArithmaticFilter: public PoseFilter
  {
  protected:
  public:
    /*
     * @param typ the Pose3D subType for the result.
     *
     */
    ArithmaticFilter( unsigned short typ=0);
    virtual ~ArithmaticFilter(){}

    /**
     * This method can be used to specify the subType of the 
     * result (output Pose3D).
     * @param typ the Pose3D subType for the result.
     */
    void setOutputSubType(unsigned short typ){Poses[2].setSubType(typ);}
  };
  
  /**
   * This is a filter that does out1 = in1 + in2.
   */
  class AddFilter: public ArithmaticFilter
  {
  public:
    /*
     * @param typ the Pose3D subType for the result.
     * 
     */
    AddFilter( unsigned short typ=0)
      :ArithmaticFilter(typ){};
  protected:
    unsigned short calc();    
  };
  
  /**
   * This is a filter that does out1 = in1 - in2.
 */
  class SubtractFilter: public ArithmaticFilter
  {
  public:
    /*
     * @param typ the Pose3D subType for the result.
     * 
     */
    SubtractFilter(unsigned short typ=0)
      :ArithmaticFilter(typ){};

  protected:
    unsigned short calc();
  };
  
  /**
   * This is a filter that does out1 = -in1 + in2
 */
  class MinusPlusFilter: public ArithmaticFilter
  {
  public:
    /*
     * @param typ the Pose3D subType for the result.
     * 
     * 
     */
    MinusPlusFilter( unsigned short typ=0)
      :ArithmaticFilter(typ){};

  protected:
    unsigned short calc();
  };
  
  /**
   * This is a filter that does out1 = -in1 - in2
   */
  class MinusMinusFilter: public ArithmaticFilter
  {
  public:
    /*
     * @param typ the Pose3D subType for the result.
     * 
     * 
     */
    MinusMinusFilter( unsigned short typ=0)
      :ArithmaticFilter(typ){};

  protected:
  unsigned short calc();
  };
  
} // namespace Cure

#endif // CURE_ARITHMATICFILTER_HH
