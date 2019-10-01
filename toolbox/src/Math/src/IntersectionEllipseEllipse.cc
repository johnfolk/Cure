//
// = LIBRARY
//
// = FILENAME
//    IntersectionEllipseEllipse.cc
//
// = AUTHOR(S)
//    Patric Jensfelt
//
// = DESCRIPTION 
//    Based on the functions from namespace IntrElp2Elp from
//    http://www.magic-software.com.
// 
// = COPYRIGHT
//    Copyright (c) 2004 Patric Jensfelt
//
/*----------------------------------------------------------------------*/

#include "IntersectionEllipseEllipse.hh"
#include "Polynomial.hh"
#include "PolynomialRoots.hh"

#ifndef DEPEND
#include <cfloat>      // DBL_MAX, DBL_EPSILON
#endif

using namespace Cure;

void 
static getAABB(const Vector2D& rkC, const Vector2D akAxis[2], 
               const double afL[2], 
               double& rfXMin, double& rfXMax, double& rfYMin, double& rfYMax)
{
  double fRx = (afL[0] * fabs(akAxis[0].getX()) + 
                afL[1] * fabs(akAxis[1].getX()));
  rfXMin = rkC.getX() - fRx;
  rfXMax = rkC.getX() + fRx;
  
  double fRy = (afL[0] * fabs(akAxis[0].getY()) +  
                afL[1] * fabs(akAxis[1].getY()));
  rfYMin = rkC.getY() - fRy;
  rfYMax = rkC.getY() + fRy;
}

double 
static evaluateQuadratic (const double afQP[6], double fX, double fY)
{
  return ( (afQP[0] * fX + afQP[1] * fY + afQP[3]) * fX + 
           (afQP[2] * fY + afQP[4]) * fY + afQP[5] );
}

Polynomial 
static getQuartic (const double afP0[6], 
                   const double afP1[6], 
                   double afDet[11])
{
  // polynomial is constructed as a Bezout determinant
  afDet[ 0] = afP0[0]*afP1[1] - afP1[0]*afP0[1];  // AB
  afDet[ 1] = afP0[0]*afP1[2] - afP1[0]*afP0[2];  // AC
  afDet[ 2] = afP0[0]*afP1[3] - afP1[0]*afP0[3];  // AD
  afDet[ 3] = afP0[0]*afP1[4] - afP1[0]*afP0[4];  // AE
  afDet[ 4] = afP0[0]*afP1[5] - afP1[0]*afP0[5];  // AF
  afDet[ 5] = afP0[1]*afP1[2] - afP1[1]*afP0[2];  // BC
  afDet[ 6] = afP0[1]*afP1[4] - afP1[1]*afP0[4];  // BE
  afDet[ 7] = afP0[1]*afP1[5] - afP1[1]*afP0[5];  // BF
  afDet[ 8] = afP0[2]*afP1[3] - afP1[2]*afP0[3];  // CD
  afDet[ 9] = afP0[3]*afP1[4] - afP1[3]*afP0[4];  // DE
  afDet[10] = afP0[3]*afP1[5] - afP1[3]*afP0[5];  // DF
  double fBFpDE = afDet[7] + afDet[9];
  double fBEmCD = afDet[6] - afDet[8];
  
  double afU[5];
  
  // AD*DF - AF*AF
  afU[0] = afDet[2]*afDet[10] - afDet[4]*afDet[4];
  
  // AB*DF + AD*(BF+DE) - 2*AE*AF
  afU[1] = afDet[0]*afDet[10] + afDet[2]*fBFpDE -
    ((double)2.0)*afDet[3]*afDet[4];
  
  // AB*(BF+DE) + AD*(BE - CD) - AE*AE - 2*AC*AF
  afU[2] = afDet[0]*fBFpDE + afDet[2]*fBEmCD - afDet[3]*afDet[3] -
    ((double)2.0)*afDet[1]*afDet[4];
  
  // AB*(BE - CD) + AD*BC - 2*AC*AE
  afU[3] = afDet[0]*fBEmCD + afDet[2]*afDet[5] -
    ((double)2.0)*afDet[1]*afDet[3];
  
  // AB*BC - AC*AC
  afU[4] = afDet[0]*afDet[5]-afDet[1]*afDet[1];
  
  // reduce degree if necessary
  int iDegree = 4;
  while ( iDegree > 0
          &&  fabs(afU[iDegree]) < DBL_EPSILON )
    {
      iDegree--;
    }
  
  Polynomial kPoly(iDegree);
  memcpy(&kPoly[0],afU,(iDegree+1)*sizeof(double));
  return kPoly;
}

static void scaleEllipsesToQuadratics (const Vector2D& rkC0,
                                       const Vector2D akAxis0[2],
                                       const double afL0[2],
                                       const Vector2D& rkC1, 
                                       const Vector2D akAxis1[2],
                                       const double afL1[2], 
                                       double afQP0[6], 
                                       double afQP1[6], 
                                       double& rfMin,
                                       double& rfHalfRange)
{
  // The ellipse E[i] is contained in the oriented rectangle with vertices
  //   C[i] + s0*L[i][0]*U[i] + s1*L[i][1]*V[i]
  // where |s0| = 1 and |s1| = 1 (four choices).  The implementation for
  // finding intersections first computes the smallest axis-aligned
  // rectangle that contains the oriented rectangles of the ellipses.  The
  // bounding rectangle is mapped to the square [-1,1]^2.  This portion of
  // the algorithm is designed to help keep the floating point calculations
  // robust in the root finding.
  
  // construct axis-aligned bounding rectangle for E0
  double fXMin0, fXMax0, fYMin0, fYMax0;
  getAABB(rkC0,akAxis0,afL0,fXMin0,fXMax0,fYMin0,fYMax0);
  
  // construct axis-aligned bounding rectangle for E1
  double fXMin1, fXMax1, fYMin1, fYMax1;
  getAABB(rkC1,akAxis1,afL1,fXMin1,fXMax1,fYMin1,fYMax1);
  
  // construct axis-aligned bounding square of bounding rectangles
  double fMin = fXMin0, fMax = fXMax0;
  if ( fXMin1 < fMin ) fMin = fXMin1;
  if ( fXMax1 > fMax ) fMax = fXMax1;
  if ( fYMin0 < fMin ) fMin = fYMin0;
  if ( fYMax0 > fMax ) fMax = fYMax0;
  if ( fYMin1 < fMin ) fMin = fYMin1;
  if ( fYMax1 > fMax ) fMax = fYMax1;
  
  double fHalfRange = ((double)0.5)*(fMax - fMin);
  double fInvHalfRange = ((double)1.0)/fHalfRange;
  
  // map ellipses to bounding square [-1,1]^2
  Vector2D kC0Tmp, kC1Tmp;
  double afL0Tmp[2], afL1Tmp[2];
  
  kC0Tmp.getX() = -(double)1.0 + fInvHalfRange*(rkC0.getX() - fMin);
  kC0Tmp.getY() = -(double)1.0 + fInvHalfRange*(rkC0.getY() - fMin);
  kC1Tmp.getX() = -(double)1.0 + fInvHalfRange*(rkC1.getX() - fMin);
  kC1Tmp.getY() = -(double)1.0 + fInvHalfRange*(rkC1.getY() - fMin);
  afL0Tmp[0] = fInvHalfRange*afL0[0];
  afL0Tmp[1] = fInvHalfRange*afL0[1];
  afL1Tmp[0] = fInvHalfRange*afL1[0];
  afL1Tmp[1] = fInvHalfRange*afL1[1];
  
  // convert ellipses to quadratic equations
  IntersectionEllipseEllipse::convertEllipseToQuadratic(kC0Tmp, akAxis0, 
                                                        afL0Tmp, afQP0);
  IntersectionEllipseEllipse::convertEllipseToQuadratic(kC1Tmp, akAxis1, 
                                                        afL1Tmp, afQP1);
  
  rfMin = fMin;
  rfHalfRange = fHalfRange;
}



void 
IntersectionEllipseEllipse::convertEllipseToQuadratic(const Vector2D& rkC,
						      const Vector2D akAxis[2],
						      const double afL[2], 
						      double afQuad[6])
{
  Vector2D akScaledAxis[2] =  
    {
      akAxis[0]/afL[0],
      akAxis[1]/afL[1]
    };
  
  SmallMatrix2 kTensor0(akScaledAxis[0],akScaledAxis[0]);
  SmallMatrix2 kTensor1(akScaledAxis[1],akScaledAxis[1]);
  SmallMatrix2 kA = kTensor0 + kTensor1;
  Vector2D kPrd;
  kPrd[0] = kA[0][0] * rkC[0] + kA[0][1] * rkC[1];
  kPrd[1] = kA[1][0] * rkC[0] + kA[1][1] * rkC[1];
  Vector2D kB = -((double)2.0)*kPrd;
  double fC = rkC.dot(kPrd) - (double)1.0;
  
  afQuad[0] = kA[0][0];
  afQuad[1] = ((double)2.0)*kA[0][1];
  afQuad[2] = kA[1][1];
  afQuad[3] = kB[0];
  afQuad[4] = kB[1];
  afQuad[5] = fC;
  
  double fMax = fabs(afQuad[0]);
  int i;
  for (i = 1; i < 6; i++) {
    double fTest = fabs(afQuad[i]);
    if ( fTest > fMax )
      fMax = fTest;
  }
  
  double fInvMax = ((double)1.0)/fMax;
  for (i = 0; i < 6; i++)
    afQuad[i] *= fInvMax;
}

bool 
IntersectionEllipseEllipse::findIntersection (const Vector2D& rkC0,
					      const Vector2D akAxis0[2], 
					      const double afL0[2],
					      const Vector2D& rkC1, 
					      const Vector2D akAxis1[2],
					      const double afL1[2], 
					      int& riQuantity, 
					      Vector2D akP[4])
{
  double afQP0[6], afQP1[6], fMin, fHalfRange;
  scaleEllipsesToQuadratics(rkC0,akAxis0,afL0,rkC1,akAxis1,afL1,afQP0,afQP1,
                            fMin,fHalfRange);
  
  // Get the 4th-degree polynomial whose roots lead to intersections of the
  // ellipses.  The y-values are in the interval [-1,1], so we only need
  // to locate roots there.  The increase to [-1.5,1.5] is to catch roots
  // that are nearly 1 or -1 without having to worry about floating point
  // round-off errors.
  double afDet[11];
  Polynomial kPoly = getQuartic(afQP0,afQP1,afDet);
  PolynomialRoots kPR(DBL_EPSILON);
  kPR.findB(kPoly,-(double)1.5,(double)1.5,6);
  int iYCount = kPR.getCount();
  const double* afY = kPR.getRoots();
  if ( iYCount == 0 ) {
    riQuantity = 0;
    return false;
  }
  
  // Adjustment for quadratics to allow for relative error testing when
  // eliminating extraneous roots.
  double fNorm0 = afQP0[0]*afQP0[0] + ((double)2.0)*afQP0[1]*afQP0[1] +
    afQP0[2]*afQP0[2];
  double fNorm1 = afQP1[0]*afQP1[0] + ((double)2.0)*afQP1[1]*afQP1[1] +
    afQP1[2]*afQP1[2];
  
  // test roots to eliminate extraneous ones that occurred due to "squaring"
  riQuantity = 0;
  for (int iY = 0; iY < iYCount; iY++) {
    double fY = afY[iY];
    double fXNumer = -((afDet[1]*fY+afDet[3])*fY+afDet[4]);
    double fXDenom = afDet[0]*fY + afDet[2];
    double fX = fXNumer/fXDenom;
    
    const double fEpsilon = (double)1e-03;
    double fTest = evaluateQuadratic(afQP0,fX,fY);
    if ( fabs(fTest) < fEpsilon*fNorm0 ) {
      fTest = evaluateQuadratic(afQP1,fX,fY);
      if ( fabs(fTest) < fEpsilon*fNorm1 ) {
        akP[riQuantity].getX() = fX;
        akP[riQuantity].getY() = fY;
        riQuantity++;
      }
    }
  }
  
  // map intersections back to original space
  for (int i = 0; i < riQuantity; i++) {
    akP[i].getX() = fHalfRange*(akP[i].getX()+(double)1.0) + fMin;
    akP[i].getY() = fHalfRange*(akP[i].getY()+(double)1.0) + fMin;
  }
  
  return riQuantity > 0;
}

bool 
IntersectionEllipseEllipse::testIntersection (const Vector2D& rkC0,
					      const Vector2D akAxis0[2], 
					      const double afL0[2],
					      const Vector2D& rkC1, 
					      const Vector2D akAxis1[2],
					      const double afL1[2])
{
  double afQP0[6], afQP1[6], fMin, fHalfRange;
  scaleEllipsesToQuadratics(rkC0,akAxis0,afL0,rkC1,akAxis1,afL1,afQP0,afQP1,
                            fMin,fHalfRange);
  
  double afDet[11];
  Polynomial kPoly = getQuartic(afQP0,afQP1,afDet);
  
  PolynomialRoots kPR(DBL_EPSILON);
  int iCount = kPR.getRootCount(kPoly,-DBL_MAX, DBL_MAX);
  
  return iCount > 0;
}

bool 
IntersectionEllipseEllipse::testIntersection (const double afQP0[6], 
					      const double afQP1[6])
{
  double afDet[11];
  Polynomial kPoly = getQuartic(afQP0,afQP1,afDet);
  
  PolynomialRoots kPR(DBL_EPSILON);
  int iCount = kPR.getRootCount(kPoly,-DBL_MAX, DBL_MAX);
  
  return iCount > 0;
}

IntersectionEllipseEllipse::SmallMatrix2::SmallMatrix2()
{
  for (int i=0; i<4; i++)
    m_afEntry[i]=0;
}
IntersectionEllipseEllipse::SmallMatrix2::SmallMatrix2(const Vector2D &rkU, 
						       const Vector2D &rkV)
{
  makeTensorProduct(rkU,rkV);
}
IntersectionEllipseEllipse::SmallMatrix2::SmallMatrix2(const SmallMatrix2 &sm2)
{
  for (int i=0; i<4; i++)
    m_afEntry[i] = sm2.m_afEntry[i];
}

void
IntersectionEllipseEllipse::SmallMatrix2::operator=(const SmallMatrix2 &sm2)
{
  for (int i=0; i<4; i++)
    m_afEntry[i] = sm2.m_afEntry[i];
}

double*
IntersectionEllipseEllipse::SmallMatrix2::operator[] (int i)
{
  assert( 0 <= i && i <= 1 );
  return m_afEntry + i * 2;
}

const double*
IntersectionEllipseEllipse::SmallMatrix2::operator[] (int i) const
{
  assert( 0 <= i && i <= 1 );
  return m_afEntry + i * 2;
}

void 
IntersectionEllipseEllipse::SmallMatrix2::makeTensorProduct(const Vector2D &rkU,
							    const Vector2D &rkV)
{
  m_afEntry[0] = rkU[0]*rkV[0];
  m_afEntry[1] = rkU[0]*rkV[1];
  m_afEntry[2] = rkU[1]*rkV[0];
  m_afEntry[3] = rkU[1]*rkV[1];
}

IntersectionEllipseEllipse::SmallMatrix2 
operator+ (const IntersectionEllipseEllipse::SmallMatrix2 &m1, 
           const IntersectionEllipseEllipse::SmallMatrix2 &m2)
{
  IntersectionEllipseEllipse::SmallMatrix2 res;
  res[0][0] =  m1[0][0] + m2[0][0];
  res[0][1] =  m1[0][1] + m2[0][1];
  res[1][0] =  m1[1][0] + m2[1][0];
  res[1][1] =  m1[1][1] + m2[1][1];
  
  return res;
}

Vector2D 
operator* (const IntersectionEllipseEllipse::SmallMatrix2 &m, 
           const Vector2D &rkV)
{
  Vector2D res;
  res[0] = m[0][0] * rkV[0] + m[0][1] * rkV[1];
  res[1] = m[1][0] * rkV[0] + m[1][1] * rkV[1];

  return res;
}

