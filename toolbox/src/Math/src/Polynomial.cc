//
// = LIBRARY
//
// = FILENAME
//    Polynomial.cc
//
// = FUNCTION
//
// = AUTHOR(S)
//    Patric Jensfelt
//
// = DESCRIPTION
//  Based on the Polynomial1 class from  http://www.magic-software.com
//   
// = COPYRIGHT
//    Copyright (c) 2004 Patric Jensfelt
//
/*----------------------------------------------------------------------*/

#include "Polynomial.hh"

#ifndef DEPEND
#include <cassert>    // assert
#include <cfloat>     // DBL_MAX
#include <cmath>      // fabs
#include <cstdlib>    // NULL
#include <cstring>    // memset
#endif

using namespace Cure;

Polynomial::Polynomial(int iDegree)
{
  if ( iDegree >= 0 ) {
    m_iDegree = iDegree;
    m_afCoeff = new double[m_iDegree + 1];
  } else {
    // default creation
    m_iDegree = -1;
    m_afCoeff = NULL;
  }
}

//=============================================================================

Polynomial::Polynomial(int iDegree, double *coeff)
{
  if ( iDegree >= 0 ) {
    m_iDegree = iDegree;
    m_afCoeff = new double[m_iDegree + 1];
    memcpy(m_afCoeff, coeff, (m_iDegree + 1) * sizeof(double));
  } else {
    // default creation
    m_iDegree = -1;
    m_afCoeff = NULL;
  }
}

//=============================================================================

Polynomial::Polynomial (const Polynomial& rkPoly)
{
  m_iDegree = rkPoly.m_iDegree;
  m_afCoeff = new double[m_iDegree + 1];
  for (int i = 0; i <= m_iDegree; i++)
    m_afCoeff[i] = rkPoly.m_afCoeff[i];
}

//=============================================================================

Polynomial::~Polynomial()
{
  delete [] m_afCoeff;
}
 
//=============================================================================

void Polynomial::setDegree (int iDegree)
{
  m_iDegree = iDegree;
  delete[] m_afCoeff;
  
  if ( m_iDegree >= 0 )
    m_afCoeff = new double[m_iDegree + 1];
  else
    m_afCoeff = NULL;
}

//=============================================================================

int Polynomial::getDegree () const
{
  return m_iDegree;
}

//=============================================================================

Polynomial::operator const double* () const
{
  return m_afCoeff;
}

//=============================================================================

Polynomial::operator double* ()
{
  return m_afCoeff;
}

//=============================================================================

double 
Polynomial::operator[] (int i) const
{
  assert( 0 <= i && i <= m_iDegree );
  return m_afCoeff[i];
}

//=============================================================================

double& 
Polynomial::operator[] (int i)
{
  assert( 0 <= i && i <= m_iDegree );
  return m_afCoeff[i];
}

//=============================================================================

Polynomial& 
Polynomial::operator= (const Polynomial& rkPoly)
{
  delete[] m_afCoeff;
  m_iDegree = rkPoly.m_iDegree;
  
  if ( m_iDegree >= 0 ) {
    m_afCoeff = new double[m_iDegree+1];
    for (int i = 0; i <= m_iDegree; i++)
      m_afCoeff[i] = rkPoly.m_afCoeff[i];
  }
  
  return *this;
}

//=============================================================================

double 
Polynomial::operator() (double fT) const
{
  assert( m_iDegree >= 0 );
  
  double fResult = m_afCoeff[m_iDegree];
  for (int i = m_iDegree - 1; i >= 0; i--) {
    fResult *= fT;
    fResult += m_afCoeff[i];
  }

  return fResult;
}

//=============================================================================

Polynomial 
Polynomial::operator+ (const Polynomial& rkPoly) const
{
  assert( m_iDegree >= 0 && rkPoly.m_iDegree >= 0 );

  Polynomial kSum;
  int i;

  if ( m_iDegree > rkPoly.m_iDegree ) {
    kSum.setDegree(m_iDegree);
    for (i = 0; i <= rkPoly.m_iDegree; i++)
      kSum.m_afCoeff[i] = m_afCoeff[i] + rkPoly.m_afCoeff[i];
    for (i = rkPoly.m_iDegree+1; i <= m_iDegree; i++)
      kSum.m_afCoeff[i] = m_afCoeff[i];

  } else {
    kSum.setDegree(rkPoly.m_iDegree);
    for (i = 0; i <= m_iDegree; i++)
      kSum.m_afCoeff[i] = m_afCoeff[i] + rkPoly.m_afCoeff[i];
    for (i = m_iDegree+1; i <= rkPoly.m_iDegree; i++)
      kSum.m_afCoeff[i] = rkPoly.m_afCoeff[i];
  }

  return kSum;
}

//=============================================================================

Polynomial 
Polynomial::operator- (const Polynomial& rkPoly) const
{
  assert( m_iDegree >= 0 && rkPoly.m_iDegree >= 0 );

  Polynomial kDiff;
  int i;

  if ( m_iDegree > rkPoly.m_iDegree ) {
    kDiff.setDegree(m_iDegree);
    for (i = 0; i <= rkPoly.m_iDegree; i++)
      kDiff.m_afCoeff[i] = m_afCoeff[i] - rkPoly.m_afCoeff[i];
    for (i = rkPoly.m_iDegree+1; i <= m_iDegree; i++)
      kDiff.m_afCoeff[i] = m_afCoeff[i];

  } else {
    kDiff.setDegree(rkPoly.m_iDegree);
    for (i = 0; i <= m_iDegree; i++)
      kDiff.m_afCoeff[i] = m_afCoeff[i] - rkPoly.m_afCoeff[i];
    for (i = m_iDegree+1; i <= rkPoly.m_iDegree; i++)
      kDiff.m_afCoeff[i] = -rkPoly.m_afCoeff[i];
  }

  return kDiff;
}

//=============================================================================

Polynomial 
Polynomial::operator* (const Polynomial& rkPoly) const
{
  assert( m_iDegree >= 0 && rkPoly.m_iDegree >= 0 );

  Polynomial kProd(m_iDegree + rkPoly.m_iDegree);

  memset(kProd.m_afCoeff,0,(kProd.m_iDegree+1)*sizeof(double));
  for (int i0 = 0; i0 <= m_iDegree; i0++) {
    for (int i1 = 0; i1 <= rkPoly.m_iDegree; i1++) {
      int i2 = i0 + i1;
      kProd.m_afCoeff[i2] += m_afCoeff[i0]*rkPoly.m_afCoeff[i1];
    }
  }

  return kProd;
}

//=============================================================================

Polynomial 
Polynomial::operator+ (double fScalar) const
{
  assert( m_iDegree >= 0 );
  Polynomial kSum(m_iDegree);
  kSum.m_afCoeff[0] += fScalar;
  return kSum;
}

//=============================================================================

Polynomial 
Polynomial::operator- (double fScalar) const
{
  assert( m_iDegree >= 0 );
  Polynomial kDiff(m_iDegree);
  kDiff.m_afCoeff[0] -= fScalar;
  return kDiff;
}

//=============================================================================

Polynomial 
Polynomial::operator* (double fScalar) const
{
  assert( m_iDegree >= 0 );

  Polynomial kProd(m_iDegree);
  for (int i = 0; i <= m_iDegree; i++)
    kProd.m_afCoeff[i] = fScalar*m_afCoeff[i];

  return kProd;
}

//=============================================================================

Polynomial 
Polynomial::operator/ (double fScalar) const
{
  assert( m_iDegree >= 0 );

  Polynomial kProd(m_iDegree);
  int i;

  if ( fScalar != (double)0.0 ) {
    double fInvScalar = ((double)1.0)/fScalar;
    for (i = 0; i <= m_iDegree; i++)
      kProd.m_afCoeff[i] = fInvScalar*m_afCoeff[i];
  } else {
    for (i = 0; i <= m_iDegree; i++)
      kProd.m_afCoeff[i] = DBL_MAX;
  }

  return kProd;
}

//=============================================================================

Polynomial 
Polynomial::operator- () const
{
  assert( m_iDegree >= 0 );

  Polynomial kNeg(m_iDegree);
  for (int i = 0; i <= m_iDegree; i++)
    kNeg.m_afCoeff[i] = -m_afCoeff[i];

  return kNeg;
}

//=============================================================================

Polynomial 
operator* (double fScalar, const Polynomial& rkPoly)
{
  assert( rkPoly.getDegree() >= 0 );

  Polynomial kProd(rkPoly.getDegree());
  for (int i = 0; i <= rkPoly.getDegree(); i++)
    kProd[i] = fScalar*rkPoly[i];

  return kProd;
}

//=============================================================================

Polynomial& 
Polynomial::operator += (const Polynomial& rkPoly)
{
  assert( m_iDegree >= 0 );
  *this = *this + rkPoly;
  return *this;
}

//=============================================================================

Polynomial& 
Polynomial::operator -= (const Polynomial& rkPoly)
{
  assert( m_iDegree >= 0 );
  *this = *this - rkPoly;
  return *this;
}

//=============================================================================

Polynomial& 
Polynomial::operator *= (const Polynomial& rkPoly)
{
  assert( m_iDegree >= 0 );
  *this = (*this)*rkPoly;
  return *this;
}

//=============================================================================

Polynomial& 
Polynomial::operator += (double fScalar)
{
  assert( m_iDegree >= 0 );
  m_afCoeff[0] += fScalar;
  return *this;
}

//=============================================================================

Polynomial& 
Polynomial::operator -= (double fScalar)
{
  assert( m_iDegree >= 0 );
  m_afCoeff[0] -= fScalar;
  return *this;
}

//=============================================================================

Polynomial& 
Polynomial::operator *= (double fScalar)
{
  assert( m_iDegree >= 0 );
  *this = (*this)*fScalar;
  return *this;
}

//=============================================================================

Polynomial& 
Polynomial::operator /= (double fScalar)
{
  assert( m_iDegree >= 0 );
  *this = (*this)/fScalar;
  return *this;
}

//=============================================================================

Polynomial 
Polynomial::getDerivative () const
{
  if ( m_iDegree > 0 ) {
    Polynomial kDeriv(m_iDegree-1);
    for (int i0 = 0, i1 = 1; i0 < m_iDegree; i0++, i1++)
      kDeriv.m_afCoeff[i0] = i1*m_afCoeff[i1];
    return kDeriv;
  } else if ( m_iDegree == 0 ) {
    Polynomial kConst(0);
    kConst.m_afCoeff[0] = (double)0.0;
    return kConst;
  }
  return Polynomial();  // invalid in, invalid out
}

//=============================================================================

Polynomial 
Polynomial::getInversion () const
{
  Polynomial kInvPoly(m_iDegree);
  for (int i = 0; i <= m_iDegree; i++)
    kInvPoly.m_afCoeff[i] = m_afCoeff[m_iDegree-i];
  return kInvPoly;
}

//=============================================================================

void 
Polynomial::compress (double fEpsilon)
{
  int i;
  for (i = m_iDegree; i >= 0; i--) {
    if ( fabs(m_afCoeff[i]) <= fEpsilon )
      m_iDegree--;
    else
      break;
  }

  if ( m_iDegree >= 0 ) {
    double fInvLeading = ((double)1.0)/m_afCoeff[m_iDegree];
    m_afCoeff[m_iDegree] = (double)1.0;
    for (i = 0; i < m_iDegree; i++)
      m_afCoeff[i] *= fInvLeading;
  }
}

//=============================================================================

void 
Polynomial::divide (const Polynomial& rkDiv, Polynomial& rkQuot,
                    Polynomial& rkRem, double fEpsilon) const
{
  int iQuotDegree = m_iDegree - rkDiv.m_iDegree;
  if ( iQuotDegree >= 0 ) {
    rkQuot.setDegree(iQuotDegree);

    // temporary storage for the remainder
    Polynomial kTmp = *this;

    // do the division (Euclidean algorithm)
    double fInv = ((double)1.0)/rkDiv[rkDiv.m_iDegree];
    for (int iQ = iQuotDegree; iQ >= 0; iQ--) {
      int iR = rkDiv.m_iDegree + iQ;
      rkQuot[iQ] = fInv*kTmp[iR];
      for (iR--; iR >= iQ; iR--)
        kTmp[iR] -= rkQuot[iQ]*rkDiv[iR-iQ];
    }

    // calculate the correct degree for the remainder
    int iRemDeg = rkDiv.m_iDegree - 1;
    while ( iRemDeg > 0 && fabs(kTmp[iRemDeg]) < fEpsilon )
      iRemDeg--;

    if ( iRemDeg == 0 && fabs(kTmp[0]) < fEpsilon )
      kTmp[0] = (double)0.0;

    rkRem.setDegree(iRemDeg);
    memcpy(rkRem.m_afCoeff,kTmp.m_afCoeff,(iRemDeg+1)*sizeof(double));
  } else {
    rkQuot.setDegree(0);
    rkQuot[0] = (double)0.0;
    rkRem = *this;
  }
}

//===========================================================================

std::ostream& 
operator<<(std::ostream &os, const Cure::Polynomial &p)
{
  os << "poly(deg=" << p.getDegree() << ") = ";
  for (int i = 0; i < p.getDegree(); i++) {
    if ( p[p.getDegree() - i] != (double)0.0)
      os << p[p.getDegree() - i] << "x^" << p.getDegree() - i;
    if (p[p.getDegree() - i - 1] > 0)
      os << " + ";
    else
      os << " ";
  }
  os << p[0];

  return os;

}
