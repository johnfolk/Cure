// = RCSID
//    $Id: ArithmaticFilter.cc ,v 1.1 2004/03/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2004 John Folkesson
//    
#include "ArithmaticFilter.hh"

namespace Cure {

ArithmaticFilter::ArithmaticFilter( unsigned short typ)
      :PoseFilter( 2, 1)
{
  setOutputSubType(typ);
  FilterName = "ArithmaticFilter";
}

unsigned short AddFilter::calc()
{
  Poses[2].add_(Poses[0],Poses[1]);
  if (Poses[0].Time==0)Poses[2].Time=Poses[1].Time;
  else Poses[2].Time=Poses[0].Time;			 
  return 0;
}
unsigned short SubtractFilter::calc()
{
  Poses[2].subtract_(Poses[0],Poses[1]);
  if (Poses[0].Time==0)Poses[2].Time=Poses[1].Time;
  else Poses[2].Time=Poses[0].Time;			 
  return 0;
}
unsigned short MinusPlusFilter::calc()
{
  Poses[2].minusPlus_(Poses[0],Poses[1]);
  if (Poses[0].Time==0)Poses[2].Time=Poses[1].Time;
  else Poses[2].Time=Poses[0].Time;			 
  return 0;
}
unsigned short MinusMinusFilter::calc()
{
  Poses[2].minusMinus_(Poses[0],Poses[1]);
  if (Poses[0].Time==0)Poses[2].Time=Poses[1].Time;
  else Poses[2].Time=Poses[0].Time;			 
  return 0;
}

} // namespace Cure
