//
// = LIBRARY
//
// = FILENAME
//    Vector2D.cc
//
// = AUTHOR(S)
//    Patric Jensfelt
//
// = DESCRIPTION
//  Based on the Vector2 class from  http://www.magic-software.com
//
// = COPYRIGHT
//    Copyright (c) 2004 Patric Jensfelt
//
/*----------------------------------------------------------------------*/

#include "Vector2D.hh"

#ifndef DEPEND
#endif

const Cure::Vector2D Cure::Vector2D::ZERO(0.0,0.0);
const Cure::Vector2D Cure::Vector2D::UNIT_X(1.0,0.0);
const Cure::Vector2D Cure::Vector2D::UNIT_Y(0.0,1.0);

using namespace Cure;

Vector2D::Vector2D()
{
  // the vector is uninitialized
}


Vector2D::Vector2D(double fX, double fY)
{
  m_afTuple[0] = fX;
  m_afTuple[1] = fY;
}

Vector2D::Vector2D(const Vector2D &rkV)
{
  m_afTuple[0] = rkV[0];
  m_afTuple[1] = rkV[1];
}

double 
Vector2D::getX() const
{
    return m_afTuple[0];
}

double& Vector2D::getX()
{
  return m_afTuple[0];
}


double Vector2D::getY() const
{
  return m_afTuple[1];
}


double& Vector2D::getY()
{
  return m_afTuple[1];
}


Vector2D Vector2D::perp () const
{
  return Vector2D(m_afTuple[1], -m_afTuple[0]);
}


Vector2D Vector2D::unitPerp () const
{
  Vector2D kPerp(m_afTuple[1], -m_afTuple[0]);
  kPerp.normalize();
  return kPerp;
}


double Vector2D::kross (const Vector2D& rkV) const
{
    return m_afTuple[0] * rkV.m_afTuple[1] - m_afTuple[1] * rkV.m_afTuple[0];
}

void Vector2D::orthonormalize (Vector2D& rkU, Vector2D& rkV)
{
  // If the input vectors are v0 and v1, then the Gram-Schmidt
  // orthonormalization produces vectors u0 and u1 as follows,
  //
  //   u0 = v0/|v0|
  //   u1 = (v1-(u0*v1)u0)/|v1-(u0*v1)u0|
  //
  // where |A| indicates length of vector A and A*B indicates dot
  // product of vectors A and B.

  // compute u0
  rkU.normalize();

  // compute u1
  double fDot0 = rkU.dot(rkV); 
  rkV -= fDot0*rkU;
  rkV.normalize();
}


void Vector2D::generateOrthonormalBasis (Vector2D& rkU, Vector2D& rkV,
                                        bool bUnitLengthV)
{
    if ( !bUnitLengthV )
        rkV.normalize();

    rkU = rkV.perp();
}

std::ostream&
operator<<(std::ostream& os, const Vector2D &v)
{
  os << "[" << v.getX() << ", " << v.getY() << "]";
  return os;
}
