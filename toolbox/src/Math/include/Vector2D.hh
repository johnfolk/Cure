//
// = LIBRARY
//
// = FILENAME
//    Vector2D.hh
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

#ifndef Cure_Vector2D_hh
#define Cure_Vector2D_hh

#ifndef DEPEND
#include <cassert>   // assert
#include <cstring>   // memcpy
#include <cfloat>    // DBL_MAX, DBL_EPSILON
#include <cmath>     // sqrt
#include <iostream>
#endif

// Forward declaration(s)
  
namespace Cure {
/**
 * 2D vector class
 * Based on the Vector2 class from  http://www.magic-software.com
 * 
 * @author Patric Jensfelt 
 * @see
 */
class Vector2D {
public:
  // construction
  Vector2D();
  Vector2D(double fX, double fY);
  Vector2D(const Vector2D& rkV);
  
  // member access
  double getX() const;
  double& getX();
  double getY() const;
  double& getY();
  
  // coordinate access
  operator const double* () const;
  operator double* ();
  double operator[] (int i) const;
  double& operator[] (int i);
  
  // comparison
  bool operator== (const Vector2D& rkV) const;
  bool operator!= (const Vector2D& rkV) const;
  bool operator<  (const Vector2D& rkV) const;
  bool operator<= (const Vector2D& rkV) const;
  bool operator>  (const Vector2D& rkV) const;
  bool operator>= (const Vector2D& rkV) const;

  // arithmetic operations
  Vector2D operator+ (const Vector2D& rkV) const;
  Vector2D operator- (const Vector2D& rkV) const;
  Vector2D operator* (double fScalar) const;
  Vector2D operator/ (double fScalar) const;
  Vector2D operator- () const;
  
  // arithmetic updates
  Vector2D& operator+= (const Vector2D& rkV);
  Vector2D& operator-= (const Vector2D& rkV);
  Vector2D& operator*= (double fScalar);
  Vector2D& operator/= (double fScalar);
  
  // vector operations
  double length () const;
  double squaredLength () const;
  double dot (const Vector2D& rkV) const;
  double normalize();

  // assignment
  Vector2D& operator= (const Vector2D& rkV);
  
  // returns (y,-x)
  Vector2D perp() const;
  
  // returns (y,-x)/sqrt(x*x+y*y)
  Vector2D unitPerp() const;
  
  // returns cross((x,y,0),(V.x,V.y,0)) = x*V.y - y*V.x
  double kross (const Vector2D& rkV) const;

  /**
   * Gram-Schmidt orthonormalization.  Take linearly independent vectors U
   * and V and compute an orthonormal set (unit length, mutually
   * perpendicular).
   */
  static void orthonormalize (Vector2D& rkU, Vector2D& rkV);
  
  /** Input V must be initialized to a nonzero vector, output is {U,V}, an
   * orthonormal basis.  A hint is provided about whether or not V is
   * already unit length.
   */
  static void generateOrthonormalBasis (Vector2D& rkU, Vector2D& rkV,
                                        bool bUnitLengthV);
  
  // special vectors
  static const Vector2D ZERO;
  static const Vector2D UNIT_X;
  static const Vector2D UNIT_Y;

protected:
  /** support for comparisons */
  int compareArrays (const Vector2D& rkV) const;
  
protected:
  static const int m_iDim = 2;

  double m_afTuple[m_iDim];
}; // class Vector2D
  
inline Vector2D::operator const double*() const
{
  return m_afTuple;
}

//=============================================================================

inline Vector2D::operator double*()
{
  return m_afTuple;
}

//=============================================================================

inline double 
Vector2D::operator[] (int i) const
{
  const int dim = 2;
  assert( 0 <= i && i < dim );
  return m_afTuple[i];
}

//=============================================================================

inline double& 
Vector2D::operator[] (int i)
{
  const int dim = 2;
  assert( 0 <= i && i < dim );
  return m_afTuple[i];
}

//=============================================================================

inline Vector2D& 
Vector2D::operator= (const Vector2D& rkV)
{
  const int dim = 2;
  memcpy(m_afTuple,rkV.m_afTuple,dim*sizeof(double));
  return *this;
}

//=============================================================================

inline bool 
Vector2D::operator== (const Vector2D& rkV) const
{
  const int dim = 2;
  return memcmp(m_afTuple,rkV.m_afTuple,dim*sizeof(double)) == 0;
}

//=============================================================================

inline bool 
Vector2D::operator!= (const Vector2D& rkV) const
{
  const int dim = 2;
  return memcmp(m_afTuple,rkV.m_afTuple,dim*sizeof(double)) != 0;
}

//=============================================================================

inline int 
Vector2D::compareArrays (const Vector2D& rkV) const
{
  const int dim = 2;
  for (int i = 0; i < dim; i++) {
    unsigned int uiTest0 = *(unsigned int*)&m_afTuple[i];
    unsigned int uiTest1 = *(unsigned int*)&rkV.m_afTuple[i];
    if ( uiTest0 < uiTest1 )
      return -1;
    if ( uiTest0 > uiTest1 )
      return +1;
  }
  return 0;
}

//=============================================================================

inline bool 
Vector2D::operator< (const Vector2D& rkV) const
{
  return compareArrays(rkV) < 0;
}

//=============================================================================

inline bool 
Vector2D::operator<= (const Vector2D& rkV) const
{
  return compareArrays(rkV) <= 0;
}

//=============================================================================

inline bool 
Vector2D::operator> (const Vector2D& rkV) const
{
  return compareArrays(rkV) > 0;
}

//=============================================================================

inline bool 
Vector2D::operator>= (const Vector2D& rkV) const
{
  return compareArrays(rkV) >= 0;
}

//=============================================================================

inline Vector2D 
Vector2D::operator+ (const Vector2D& rkV) const
{
  Vector2D kSum;
  const int dim = 2;
  for (int i = 0; i < dim; i++)
    kSum.m_afTuple[i] = m_afTuple[i] + rkV.m_afTuple[i];
  return kSum;
}

//=============================================================================

inline Vector2D 
Vector2D::operator- (const Vector2D& rkV) const
{
  Vector2D kDiff;
  const int dim = 2;
  for (int i = 0; i < dim; i++)
    kDiff.m_afTuple[i] = m_afTuple[i] - rkV.m_afTuple[i];
  return kDiff;
}

//=============================================================================

inline Vector2D 
Vector2D::operator* (double fScalar) const
{
  Vector2D kProd;
  const int dim = 2;
  for (int i = 0; i < dim; i++)
    kProd.m_afTuple[i] = fScalar*m_afTuple[i];
  return kProd;
}

//=============================================================================

inline Vector2D 
Vector2D::operator/ (double fScalar) const
{
  Vector2D kQuot;
  const int dim = 2;
  int i;
  
  if ( fScalar != (double)0.0 ) {
    double fInvScalar = ((double)1.0)/fScalar;
    for (i = 0; i < dim; i++)
      kQuot.m_afTuple[i] = fInvScalar*m_afTuple[i];
  } else {
    for (i = 0; i < dim; i++)
      kQuot.m_afTuple[i] = DBL_MAX;
  }

  return kQuot;
}

//=============================================================================

inline Vector2D 
Vector2D::operator- () const
{
  Vector2D kNeg;
  const int dim = 2;
  for (int i = 0; i < dim; i++)
    kNeg.m_afTuple[i] = -m_afTuple[i];
  return kNeg;
}

//=============================================================================

inline Vector2D 
operator* (double fScalar, const Vector2D& rkV)
{
  Vector2D kProd;
  const int dim = 2;
  for (int i = 0; i < dim; i++)
    kProd[i] = fScalar*rkV[i];
  return kProd;
}

//=============================================================================

inline Vector2D& 
Vector2D::operator+= (const Vector2D& rkV)
{
  const int dim = 2;
  for (int i = 0; i < dim; i++)
    m_afTuple[i] += rkV.m_afTuple[i];
  return *this;
}

//=============================================================================

inline Vector2D& 
Vector2D::operator-= (const Vector2D& rkV)
{
  const int dim = 2;
  for (int i = 0; i < dim; i++)
    m_afTuple[i] -= rkV.m_afTuple[i];
  return *this;
}

//=============================================================================

inline Vector2D& 
Vector2D::operator*= (double fScalar)
{
  const int dim = 2;
  for (int i = 0; i < dim; i++)
    m_afTuple[i] *= fScalar;
  return *this;
}

//=============================================================================

inline Vector2D& 
Vector2D::operator/= (double fScalar)
{
  const int dim = 2;
  int i;
  
  if ( fScalar != (double)0.0 ) {
    double fInvScalar = ((double)1.0)/fScalar;
    for (i = 0; i < dim; i++)
      m_afTuple[i] *= fInvScalar;
  } else {
    for (i = 0; i < dim; i++)
      m_afTuple[i] = DBL_MAX;
  }

  return *this;
}

//=============================================================================

inline double 
Vector2D::length () const
{
  const int dim = 2;
  double fSqrLen = (double)0.0;
  for (int i = 0; i < dim; i++)
    fSqrLen += m_afTuple[i]*m_afTuple[i];
  return sqrt(fSqrLen);
}

//=============================================================================

inline double 
Vector2D::squaredLength () const
{
  const int dim = 2;
  double fSqrLen = (double)0.0;
  for (int i = 0; i < dim; i++)
    fSqrLen += m_afTuple[i]*m_afTuple[i];
  return fSqrLen;
}

//=============================================================================

inline double 
Vector2D::dot (const Vector2D& rkV) const
{
  const int dim = 2;
  double fDot = (double)0.0;
  for (int i = 0; i < dim; i++)
    fDot += m_afTuple[i]*rkV.m_afTuple[i];
  return fDot;
}

//=============================================================================

inline double 
Vector2D::normalize ()
{
  const int dim = 2;
  double fLength = length();
  int i;
  
  if ( fLength > DBL_EPSILON ) {
    double fInvLength = ((double)1.0)/fLength;
    for (i = 0; i < dim; i++)
      m_afTuple[i] *= fInvLength;
  } else {
    fLength = (double)0.0;
    for (i = 0; i < dim; i++)
      m_afTuple[i] = (double)0.0;
  }
  
  return fLength;
}

} // namespace Cure

std::ostream& operator<<(std::ostream &os, const Cure::Vector2D& v);


#endif // Cure_Vector2D_hh
