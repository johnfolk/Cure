// = RCSID
//    $Id: Plane.cc ,v 1.1 2004/03/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2004 John Folkesson
//    
#include "Plane.hh"

namespace Cure {

void Plane::operator = (const Plane& l)
{
  Frame=l.Frame;
  Length=l.Length;  
  Width=l.Width;
}
void Plane::transform(Transformation3D  *trans, Plane& p)
{
  Frame=p.Frame-*trans;
}
void Plane::invTransform(Transformation3D  *trans, Plane& p)
{
  Frame=p.Frame +(*trans);
}

} // namespace Cure
