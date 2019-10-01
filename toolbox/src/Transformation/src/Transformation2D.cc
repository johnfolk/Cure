// = RCSID
//    $Id: Transformation2D.cc ,v 1.1 2004/03/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2004 John Folkesson
//    
#include "Transformation2D.hh"

namespace Cure {

void Transformation2D::transform2D(const double fromx[2],double tox[2])
{  
  double temp_x[2];
  temp_x[0]=fromx[0]-Position(0);
  temp_x[1]=fromx[1]-Position(1);
  rotate2D(temp_x,tox);
}
void Transformation2D::invTransform2D(const double fromx[2],double tox[2])
{  
  invRotate2D(fromx,tox);
  tox[0]+=Position(0);
  tox[1]+=Position(1);
}
void Transformation2D::print()
{
  Position.print();
  Rotation2D::print();
}
Transformation2D Transformation2D::operator +(const Transformation2D& t)const
  {
    Transformation2D res(*this);
    res+=t;
    return(res);
  }
Transformation2D Transformation2D::operator - (const Transformation2D& t)const
{
    Transformation2D res(*this);
    res-=t;
    return(res);
  }

Transformation2D  Transformation2D::inverse()
{
  Transformation2D res;
  res-=*this;
  return(res);    
}

} // namespace Cure
