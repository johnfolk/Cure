//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2005 John Folkesson
//    

#ifndef CURE_MOTIONCOMMAND_HH
#define CURE_MOTIONCOMMAND_HH

#include "CommandExecuter.hh"
namespace Cure{

/**
 * Class that so far only defines errors during motion
 * @author John Folkesson
 */
class MotionCommand
{
public:
   enum CmdFlags {
    COMMAND_OK = 0,
    PERMISSION_DENIED,
    SERVICE_IN_USE,
    GOAL_REACHED,
    GOAL_UNREACHABLE,
    GOAL_OUTOFBOUNDS,
    VELOCITY,
    FOLLOW,
    GOTO,
    RELATIVE_POSE,
    ABSOLUTE_POSE,
    AVOID_ON,
    AVOID_OFF,
    PATH_PLAN,
    SET_PARAMETER,
    GET_PARAMETER,
    NOT_SUPPORTED,
    SET_BRAKE
  };
};

} // namespace Cure

#endif 
