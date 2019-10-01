//    $Id: Plane.hh,v 1.4 2007/09/14 09:13:51 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_PLANE_HH
#define CURE_PLANE_HH

#include "Line2D.hh"

namespace Cure{

/**
 * Implements a 3D plane (finite)
 *
 * @author John Folkesson
 */
class Plane 
{
 public:
  Transformation3D Frame;
  double Length;
  double Width;

 public:
  Plane(){}
  Plane(Plane& p){*this=p;}
  virtual ~Plane(){}

  void operator=(const Plane& p);
  void transform(Transformation3D  *trans, Plane& p);
  void invTransform(Transformation3D  *trans, Plane& p);
};

} // namespace Cure

#endif // CURE_PLANE_HH
