//
// = LIBRARY
//
// = FILENAME
//    PolynomialRoots.cc
//
// = AUTHOR(S)
//    Patric Jensfelt
//
// = DESCRIPTION
//  Based on the PolynomialRoots class from  http://www.magic-software.com
//
// = COPYRIGHT
//    Copyright (c) 2004 Patric Jensfelt
//
/*----------------------------------------------------------------------*/

#include "PolynomialRoots.hh"
#include "Polynomial.hh"


#ifndef DEPEND
#include <cmath>     // log, sqrt, etc
#include <cassert>   // assert
#include <cfloat>    // DML_MAX
#include <vector>
#endif

const double Cure::PolynomialRoots::THIRD = 1.0 / 3.0;
const double Cure::PolynomialRoots::TWENTYSEVENTH = 1.0 / 27.0;
const double Cure::PolynomialRoots::SQRT3 = sqrt(3.0);
const double Cure::PolynomialRoots::INVLOG2 = 1.0 / log(2.0);
const double Cure::PolynomialRoots::LOG10 = log(10.0);

using namespace Cure;

PolynomialRoots::PolynomialRoots (double fEpsilon)
{
  assert( fEpsilon >= (double)0.0 );
  m_fEpsilon = fEpsilon;
  m_iCount = 0;
  m_iMaxRoot = 4;  // default support for degree <= 4
  m_afRoot = new double[m_iMaxRoot];
}

//=============================================================================

PolynomialRoots::~PolynomialRoots ()
{
  delete[] m_afRoot;
}

//=============================================================================

int PolynomialRoots::getCount () const
{
    return m_iCount;
}

//=============================================================================

const double* PolynomialRoots::getRoots () const
{
    return m_afRoot;
}

//=============================================================================

double PolynomialRoots::getRoot (int i) const
{
  assert( 0 <= i && i < m_iCount );
  if ( 0 <= i && i < m_iCount )
    return m_afRoot[i];
  
  return DBL_MAX;
}

//=============================================================================

double& PolynomialRoots::getEpsilon ()
{
  return m_fEpsilon;
}

//=============================================================================

double PolynomialRoots::getEpsilon () const
{
  return m_fEpsilon;
}

//=============================================================================

bool PolynomialRoots::findB (const Polynomial& rkPoly, int iDigits)
{
  double fBound = getBound(rkPoly);
  return findB(rkPoly,-fBound,fBound,iDigits);
}

//=============================================================================

double 
PolynomialRoots::getBound (const Polynomial& rkPoly)
{
  Polynomial kCPoly = rkPoly;
  kCPoly.compress(m_fEpsilon);
  int iDegree = kCPoly.getDegree();
  if ( iDegree < 1 ) {
    // polynomial is constant, return invalid bound
    return -(double)1.0;
  }
  
  double fInvCDeg = ((double)1.0)/kCPoly[iDegree];
  double fMax = (double)0.0;
  for (int i = 0; i < iDegree; i++) {
    double fTmp = fabs(kCPoly[i]) * fInvCDeg;
    if ( fTmp > fMax )
      fMax = fTmp;
  }
  
  return (double)1.0 + fMax;
}

//=============================================================================

bool 
PolynomialRoots::findB (const Polynomial& rkPoly, double fXMin, double fXMax, 
                        int iDigits)
{
  // reallocate root array if necessary
  if ( rkPoly.getDegree() > m_iMaxRoot ) {
    m_iMaxRoot = rkPoly.getDegree();
    delete[] m_afRoot;
    m_afRoot = new double[m_iMaxRoot];
  }

  double fRoot;
  if ( rkPoly.getDegree() == 1 ) {
    if ( bisection(rkPoly,fXMin,fXMax,iDigits,fRoot) )  {
      m_iCount = 1;
      m_afRoot[0] = fRoot;
      return true;
    }
    m_iCount = 0;
    return false;
  }

  // get roots of derivative polynomial
  Polynomial kDeriv = rkPoly.getDerivative();
  findB(kDeriv,fXMin,fXMax,iDigits);

  int i, iNewCount = 0;
  double* afNewRoot = new double[m_iCount+1];
  
  if ( m_iCount > 0 ) {
    // find root on [xmin,root[0]]
    if ( bisection(rkPoly, fXMin, m_afRoot[0], iDigits, fRoot) )
      afNewRoot[iNewCount++] = fRoot;
    
    // find root on [root[i],root[i+1]] for 0 <= i <= count-2
    for (i = 0; i <= m_iCount-2; i++) {
      if ( bisection(rkPoly, m_afRoot[i], m_afRoot[i + 1], iDigits, fRoot) )
        afNewRoot[iNewCount++] = fRoot;
    }
    
    // find root on [root[count-1],xmax]
    if ( bisection(rkPoly, m_afRoot[m_iCount - 1], fXMax, iDigits, fRoot) )
      afNewRoot[iNewCount++] = fRoot;
  } else {
    // polynomial is monotone on [xmin,xmax], has at most one root
    if ( bisection(rkPoly, fXMin, fXMax, iDigits, fRoot) )
      afNewRoot[iNewCount++] = fRoot;
  }
  
  // copy to old buffer
  if ( iNewCount > 0 ) {
    m_iCount = 1;
    m_afRoot[0] = afNewRoot[0];
    for (i = 1; i < iNewCount; i++) {
      double fRootDiff = afNewRoot[i]-afNewRoot[i-1];
      if ( fabs(fRootDiff) > m_fEpsilon )
        m_afRoot[m_iCount++] = afNewRoot[i];
    }
  } else {
    m_iCount = 0;
  }
  
  delete[] afNewRoot;
  return m_iCount > 0;
}

//============================================================================

bool 
PolynomialRoots::bisection (const Polynomial& rkPoly,
                            double fXMin, double fXMax, int iDigits, 
                            double& rfRoot)
{
  double fP0 = rkPoly(fXMin);
  double fP1 = rkPoly(fXMax);
  if ( fP0*fP1 > (double)0.0 )
    return false;
  
  // determine number of iterations to get 'digits' accuracy.
  double fTmp0 = log(fXMax-fXMin);
  double fTmp1 = ((double)iDigits) * LOG10;
  double fArg = (fTmp0 + fTmp1) * INVLOG2;
  int iMaxIter = (int)(fArg + 0.5f);
  
  for (int i = 0; i < iMaxIter; i++) {
    rfRoot = ((double)0.5)*(fXMin + fXMax);
    double fP = rkPoly(rfRoot);
    if ( fP*fP0 < (double)0.0 ) {
      fXMax = rfRoot;
      fP1 = fP;
    } else {
      fXMin = rfRoot;
      fP0 = fP;
    }
  }

  return true;
}

//=============================================================================
int 
PolynomialRoots::getRootCount (const Polynomial& rkPoly, 
                               double fT0, double fT1)
{
  int iDegree = rkPoly.getDegree();
  const double* afCoeff = (const double*)rkPoly;
  
  if ( iDegree == 0 ) {
    // polynomial is constant on the interval
    if ( afCoeff[0] != (double)0.0 )
      return 0;
    else
      return -1;  // to indicate "infinitely many"
  }
  
  // generate the Sturm sequence
  std::vector<Polynomial*> kSturm;
  Polynomial* pkF0 = new Polynomial(rkPoly);
  Polynomial* pkF1 = new Polynomial(pkF0->getDerivative());
  kSturm.push_back(pkF0);
  kSturm.push_back(pkF1);
  
  while ( pkF1->getDegree() > 0 ) {
    Polynomial* pkF2 = new Polynomial(-1);
    Polynomial kQuot;
    pkF0->divide(*pkF1,kQuot,*pkF2, DBL_EPSILON);
    *pkF2 = -(*pkF2);
    kSturm.push_back(pkF2);
    pkF0 = pkF1;
    pkF1 = pkF2;
  }
  
  int i;
  double fValue0, fValue1;
  
  // count the sign changes at t0
  int iSignChanges0 = 0;
  if ( fT0 == -DBL_MAX ) {
    pkF0 = kSturm[0];
    if ( pkF0->getDegree() & 1 )
      fValue0 = -(*pkF0)[pkF0->getDegree()];
    else
      fValue0 = (*pkF0)[pkF0->getDegree()];
    
    if ( fabs(fValue0) < m_fEpsilon )
      fValue0 = (double)0.0;
    
    for (i = 1; i < (int)kSturm.size(); i++) {
      pkF1 = kSturm[i];
      
      if ( pkF1->getDegree() & 1 )
        fValue1 = -(*pkF1)[pkF1->getDegree()];
      else
        fValue1 = (*pkF1)[pkF1->getDegree()];
      
      if ( fabs(fValue1) < m_fEpsilon )
        fValue1 = (double)0.0;
      
      if ( fValue0*fValue1 < (double)0.0 || fValue0 == (double)0.0 )
        iSignChanges0++;
      
      fValue0 = fValue1;
      pkF0 = pkF1;
    }
  } else {
    pkF0 = kSturm[0];
    fValue0 = (*pkF0)(fT0);
    if ( fabs(fValue0) < m_fEpsilon )
      fValue0 = (double)0.0;
    
    for (i = 1; i < (int)kSturm.size(); i++) {
      pkF1 = kSturm[i];
      fValue1 = (*pkF1)(fT0);
      if ( fabs(fValue1) < m_fEpsilon )
        fValue1 = (double)0.0;
      
      if ( fValue0*fValue1 < (double)0.0 || fValue0 == (double)0.0 )
        iSignChanges0++;
      
      fValue0 = fValue1;
      pkF0 = pkF1;
    }
  }
  
  // count the sign changes at t1
  int iSignChanges1 = 0;
  if ( fT1 == DBL_MAX) {
    pkF0 = kSturm[0];
    fValue0 = (*pkF0)[pkF0->getDegree()];
    if ( fabs(fValue0) < m_fEpsilon )
      fValue0 = (double)0.0;
    
    for (i = 1; i < (int)kSturm.size(); i++) {
      pkF1 = kSturm[i];
      fValue1 = (*pkF1)[pkF1->getDegree()];
      if ( fabs(fValue1) < m_fEpsilon )
        fValue1 = (double)0.0;
      
      if ( fValue0*fValue1 < (double)0.0 || fValue0 == (double)0.0 )
        iSignChanges1++;
      
      fValue0 = fValue1;
      pkF0 = pkF1;
    }
  } else {
    pkF0 = kSturm[0];
    fValue0 = (*pkF0)(fT1);
    if ( fabs(fValue0) < m_fEpsilon )
      fValue0 = (double)0.0;
    
    for (i = 1; i < (int)kSturm.size(); i++) {
      pkF1 = kSturm[i];
      fValue1 = (*pkF1)(fT1);
      if ( fabs(fValue1) < m_fEpsilon )
        fValue1 = (double)0.0;
      
      if ( fValue0*fValue1 < (double)0.0 || fValue0 == (double)0.0 )
        iSignChanges1++;
      
      fValue0 = fValue1;
      pkF0 = pkF1;
    }
  }
  
  // clean up
  for (i = 0; i < (int)kSturm.size(); i++)
    delete kSturm[i];
  
  if ( iSignChanges0 >= iSignChanges1 )
    return iSignChanges0 - iSignChanges1;
  
  // theoretically we should not get here
  assert( false );
  return 0;
}

//===========================================================================

std::ostream&
operator<<(std::ostream& os, const Cure::PolynomialRoots& r)
{
  os << "roots = {";
  for (int i = 0; i < r.getCount(); i++) {
    os << r.getRoots()[i];
    if (i < (r.getCount() - 1) )
        os << ", ";
  }
  os << "}";

  return os;
}
