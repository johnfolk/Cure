//
// = LIBRARY
//
// = FILENAME
//    PolynomialRoots.hh
//
// = FUNCTION
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

#ifndef CurePolynomialRoots_hh
#define CurePolynomialRoots_hh


#ifndef DEPEND
#include <iostream>
#endif

#include "noncopyable.hh"

namespace Cure {

// Forward declaration(s)
class Polynomial;


/**
 * Class that holds/finds the roots of a polynomial. This class is
 * based on the PolynomialRoots class from http://www.magic-software.com. So
 * far only the biscetion part has been ported.
 *
 * Methods are
 *
 * A: algebraic using closed-form expressions (fast, typically not robust)
 * B: bisection (after root bounding, slow and robust)
 * N: Newton's/bisection hybrid (after root bounding, medium and robust)
 * E: eigenvalues of a companion matrix (fast and robust)
 *
 * Root bounds:
 *
 * For a monic polynomial
 *   x^n + a[n-1]*x^{n-1} + ... + a[1]*x + a[0]
 * the Cauchy bound is
 *   M = 1 + max{|a[0]|,...,|a[n-1]|}.
 * All real-value roots must lie in the interval [-M,M].  For a non-monic
 * polynomial,
 *   b[n]*x^n + b[n-1]*x^{n-1} + ... + b[1]*x + b[0],
 * if b[n] is not zero, divide through by it and calculate Cauchy's
 * bound:
 *  1 + max{|b[0]/b[n]|,...,|b[n-1]/b[n]|}.
 *
 */
class PolynomialRoots : private noncopyable {
public:

  /**
   * Constructor
   *
   * @param fEpsilon threshold for when to stop search for roots
   */
  PolynomialRoots(double fEpsilon);
  
  /**
   * destructor
   */
  ~PolynomialRoots();

  /**
   * 
   */
  int getCount () const;

  /**
   * @return aray with root values
   */
  const double* getRoots () const;

  /**
   * @param i index of root
   * @return value of root i
   */
  double getRoot (int i) const;

  /**
   * @return threshold for when to stop serach for root
   */
  double& getEpsilon ();

  /**
   * @return threshold for when to stop serach for root
   */
  double getEpsilon () const;

  /** fiding root using bisection */
  // general equations: sum_{i=0}^{degree} c(i)*x^i = 0
  bool findB (const Polynomial& rkPoly, int iDigits);

  double getBound (const Polynomial& rkPoly);

  /** find roots on specified intervals using bisection */
  bool findB (const Polynomial& rkPoly, double fXMin, double fXMax, 
              int iDigits);

  /**
   * Count the number of roots on [t0,t1].  Uses Sturm sequences to do the
   * counting.  It is allowed to pass in t0 = -Math<Real>::MAX_REAL or
   * t1 = Math<Real>::MAX_REAL.  The value of m_fEpsilon is used as a
   * threshold on the value of a Sturm polynomial at an end point.  If
   * smaller, that value is assumed to be zero.  The return value is the
   * number of roots.  If there are infinitely many, -1 is returned.
  */
  int getRootCount (const Polynomial& rkPoly, double fT0, double fT1);

protected:

private:
  /** support for findB */
  bool bisection (const Polynomial& rkPoly, double fXMin, double fXMax,
                  int iDigitsAccuracy, double& rfRoot);
  
private:
  double m_fEpsilon;
  int m_iCount, m_iMaxRoot;
  double* m_afRoot;

  static const double THIRD;
  static const double TWENTYSEVENTH;
  static const double SQRT3;
  static const double INVLOG2;
  static const double LOG10;

}; // class PolynomialRoots

}  // namespace Cure

std::ostream& operator<<(std::ostream& os, const Cure::PolynomialRoots& r);

#endif // CurePolynomialRoots_hh
