//
// = LIBRARY
//
// = FILENAME
//    IntersectionEllipseEllipse.hh
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

#ifndef Cure_IntersectionEllipseEllipse_hh
#define Cure_IntersectionEllipseEllipse_hh

#include "Vector2D.hh"

#ifndef DEPEND
#endif

// Forward declaration(s)

  
namespace Cure {

/**
 * Functions that can be used to find intersections between
 * ellipses. They are based on the functions from namespace
 * IntrElp2Elp from http://www.magic-software.com.
 *
 * An ellipse has a center C, two unit-length and orthogonal axis directions
 * U[0] and U[1], and axis half-lengths L[0] and L[1].  In parametric form
 * the ellipse is
 *   X(t) = C + cos(t)*L[0]*U[0] + sin(t)*L[1]*U[1]
 * for 0 <= t < 2*pi.
 *
 * In vector-matrix form the ellipse is
 *   (X-C)^T * M * (X-C) = 1
 * where superscript T denotes transpose.  M is the 2-by-2 symmetric matrix
 *   M = U[0]*U[0]^T/L[0]^2 + U[1]*U[1]^T/L[1]^2
 * The vector U[i] is 2-by-1, so U[i]*U[i]^T is a 2-by-2 matrix.
 *
 * As a quadratic equation in two variables, the ellipse is
 *   Q(x,y) = a[0]*x^2 + a[1]*x*y + a[2]*y^2 + a[3]*x + a[4]*y + a[5] = 0
 * where a[1]*a[1] < 4*a[0]*a[2] is required for this to represent an ellipse.
 */
namespace IntersectionEllipseEllipse {

  /** 
   * compute the quadratic equation for the ellipse 
   */
  void convertEllipseToQuadratic (const Vector2D& rkC,
                                  const Vector2D akAxis[2], 
                                  const double afL[2], 
                                  double afQuad[6]);
  
  /** 
   * On return, riQuantity is the number of intersections (in {0,1,2,3,4}) and
   * akP[0],...,akP[riQuantity-1] are the points of intersection.  The return
   * value is 'true' if and only if riQuantity > 0.
   */
  bool findIntersection (const Vector2D& rkC0,
                         const Vector2D akAxis0[2], 
                         const double afL0[2],
                         const Vector2D& rkC1, 
                         const Vector2D akAxis1[2],
                         const double afL1[2], 
                         int& riQuantity, 
                         Vector2D akP[4]);

  /**
   * Return value is 'true' if and only if there is an intersection.  If there
   * is no intersection, the ellipses are either separated by a line or one is
   * properly contained in the other.
   */
  bool testIntersection (const Vector2D& rkC0,
                         const Vector2D akAxis0[2], 
                         const double afL0[2],
                         const Vector2D& rkC1, 
                         const Vector2D akAxis1[2],
                         const double afL1[2]);

  bool testIntersection (const double afQP0[6], const double afQP1[6]);

  /**
   * Small helper matrix
   */
  class SmallMatrix2 {
  public:
    SmallMatrix2();
    SmallMatrix2(const Vector2D &rkU, const Vector2D &rkV);
    SmallMatrix2(const SmallMatrix2 &sm2);
    void operator=(const SmallMatrix2 &sm2);
    double* operator[] (int i);
    const double* operator[] (int i) const;
  private:
    void makeTensorProduct(const Vector2D &rkU, const Vector2D &rkV);
  private:
    double m_afEntry[4];
  };

}; // namespace IntersectionEllipseEllipse

} // namespace Cure

Cure::IntersectionEllipseEllipse::SmallMatrix2 
operator+ (const Cure::IntersectionEllipseEllipse::SmallMatrix2 &m1, 
           const Cure::IntersectionEllipseEllipse::SmallMatrix2 &m2);
Cure::Vector2D 
operator* (const Cure::IntersectionEllipseEllipse::SmallMatrix2 &m, 
           const Cure::Vector2D &rkV);

#endif // Cure_IntersectionEllipseEllipse_hh
