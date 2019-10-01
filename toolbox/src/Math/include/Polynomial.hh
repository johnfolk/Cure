//
// = LIBRARY
//
// = FILENAME
//    Polynomial.hh
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

#ifndef CurePolynomial_hh
#define CurePolynomial_hh

#ifndef DEPEND
#include <iostream>
#endif


// Forward declaration(s)


namespace Cure {
/**
 * Class that represents a polynomial. The coefficents are given in
 * ascending order of power, i.e poly[0] is the constant in the polygon.
 * Based on the Polynomial1 class from  http://www.magic-software.com
 * 
 * @author Patric Jensfelt 
 * @see
 */
class Polynomial {
public:

  /**
   * Constructor
   * @param iDegree degree of the polynomial, -1 do not allocate polynomalial
   */
  Polynomial(int iDegree = -1);

  /**
   * Constructor
   * @param iDegree degree of the polynomial, -1 do not allocate polynomalial
   * @param coeff array of coefficients for the polynomial
   */
  Polynomial(int iDegree, double *coeff);

  /**
   * Copy contructor
   * @param rkPoly polynomial top copy
   */
  Polynomial(const Polynomial &rkPoly);

  /**
   * Destructor
   */
  ~Polynomial();

  /**
   * @param iDegree the new degree for the polynomial
   */
  void setDegree(int iDegree);


  /**
   * @return degree of the polynomial
   */
  int getDegree() const;

  /**
   * @return the array of coefficients
   */
  operator const double*() const;

  /**
   * @return the array of coefficients
   */
  operator double*();

  /**
   * @return the coefficient i
   */
  double operator[] (int i) const;

  /**
   * @return the coefficient i
   */
  double& operator[] (int i);

  /** assignment */
  Polynomial& operator= (const Polynomial& rkPoly);
  
  /** evaluation */
  double operator() (double fT) const;

  //================ arithmetic operations ==================

  /// Add polynomial
  /// @param rkPoly polynomial
  Polynomial operator+ (const Polynomial& rkPoly) const;

  /// Substract polynomaial
  /// @param rkPoly polynomial
  Polynomial operator- (const Polynomial& rkPoly) const;

  /// Multiple with polynomanial
  /// @param rkPoly polynomial
  Polynomial operator* (const Polynomial& rkPoly) const;

  /// Add a scalar to the polynomial
  /// @param fScalar scalar to add (input is degree 0 poly)
  Polynomial operator+ (double fScalar) const;  // 

  /// Add a scalar to the polynomial
  /// @param fScalar scalar to substract (input is degree 0 poly)
  Polynomial operator- (double fScalar) const;

  /// Multiply polynomial by scalar
  /// @param fScalar to multiply with
  Polynomial operator* (double fScalar) const;

  /// Divide polynomial with scalar value
  /// @param fScalar value to divide polynomial with
  Polynomial operator/ (double fScalar) const;

  /// Negate polynomial
  Polynomial operator- () const;
 
  // ================= arithmetic updates =======================

  /// Add polynomaial to this one
  /// @param rkPoly polynomanial to add to this
  Polynomial& operator += (const Polynomial& rkPoly);

  /// Substract polynomaial from this one
  /// @param rkPoly polynomanial to substract from this
  Polynomial& operator -= (const Polynomial& rkPoly);

  /// Multiply polynomaial with this one
  /// @param rkPoly polynomanial to multiply with this
  Polynomial& operator *= (const Polynomial& rkPoly);

  /// Add scalar to this polynomial
  /// @param fScalar value to add to this polynomial
  Polynomial& operator += (double fScalar);

  /// Substract scalar from this polynomial
  /// @param fScalar value to substract from this polynomial
  Polynomial& operator -= (double fScalar);

  /// Multiple this polynomial by a scalar
  /// @param fScalar value to multiple this polynomial by
  Polynomial& operator *= (double fScalar);

  /// Divide this polynomial by a scalar
  /// @param fScalar value to divide this polynomial by
  Polynomial& operator /= (double fScalar);


  /** inversion ( invpoly[i] = poly[degree-i] for 0 <= i <= degree ) */
  Polynomial getInversion () const;
  
  /**
   * @return derivative polynomial
   */
  Polynomial getDerivative() const;

  /**
   * Reduce degree by eliminating all (nearly) zero leading coefficients
   * and by making the leading coefficient one.  
   *
   * @param fEpsilon The input parameter is the threshold for specifying
   * that a coefficient is effectively zero.
   */
  void compress (double fEpsilon);

  /**
   * If 'this' is P(t) and the divisor is D(t) with degree(P) >= degree(D),
   * then P(t) = Q(t)*D(t)+R(t) where Q(t) is the quotient with
   * degree(Q) = degree(P) - degree(D) and R(t) is the remainder with
   * degree(R) < degree(D).  If this routine is called with
   * degree(P) < degree(D), then Q = 0 and R = P are returned.  The value
   * of epsilon is used as a threshold on the coefficients of the remainder
   * polynomial.  If smaller, the coefficient is assumed to be zero.
   */
  void divide (const Polynomial& rkDiv, Polynomial& rkQuot,
               Polynomial& rkRem, double fEpsilon) const;
  
protected:

private:
  /// Degree of the polynomial
  int m_iDegree;

  /// Coefficients for the polynomial
  double *m_afCoeff;
}; // class Polynomial

}  // namespace Cure

/// Operator to print poolynomial on the screen
std::ostream& operator<<(std::ostream &os, const Cure::Polynomial &p);

#endif // CurePolynomial_hh
