// = RCSID
//    $Id: Vector3D.cc ,v 1.1 2004/03/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2004 John Folkesson
//    
#include "Vector3D.hh"

#ifndef DEPEND
#include <iostream>
#endif

namespace Cure {

Vector3D Vector3D::leftMultBy(const Matrix& m)const 
{
  Vector3D res;
  res.X[0]=m.Element[0]*X[0]+m.Element[1]*X[1]+m.Element[2]*X[2];
  res.X[1]=m.Element[3]*X[0]+m.Element[4]*X[1]+m.Element[5]*X[2];
  res.X[2]=m.Element[6]*X[0]+m.Element[7]*X[1]+m.Element[8]*X[2];
  return (res);
}
Vector3D Vector3D::operator *  (const Matrix& m)const
{
  Vector3D res;
  res.X[0]=m.Element[0]*X[0]+m.Element[3]*X[1]+m.Element[6]*X[2];
  res.X[1]=m.Element[1]*X[0]+m.Element[4]*X[1]+m.Element[7]*X[2];
  res.X[2]=m.Element[2]*X[0]+m.Element[5]*X[1]+m.Element[8]*X[2];
  return (res);
}

void Vector3D::print()
{
  std::cerr<<" Vector3D: "<<X[0]<<" "<<X[1]<<" "<<X[2]<<std::endl;
}

} // namespace Cure

std::ostream& operator<<(std::ostream &os, const Cure::Vector3D &v)
{
  os << "[" << v.X[0] << " " << v.X[1] << " " << v.X[2] << "]";
  return os;
}
