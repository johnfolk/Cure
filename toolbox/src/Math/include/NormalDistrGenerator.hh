//
// = FILENAME
//    NormalDistrGenerator.hh
//
// = FUNCTION
//
// = AUTHOR(S)
//    Patric Jensfelt
//
// = COPYRIGHT
//    Copyright (c) 2006 Patric Jensfelt
//
/*----------------------------------------------------------------------*/

#ifndef Cure_NormalDistrGenerator_hh
#define Cure_NormalDistrGenerator_hh

namespace Cure {

/**
 * This class helps you generate random variables from a Normal
 * distribution. This class is based on the algorithm from Numerical
 * Recipes in C and uses the Box-Muller formula.
 *
 * @author Patric Jensfelt
 */
class NormalDistrGenerator {
public:
  /**
   * Constructor
   *
   * @param mean desired mean value
   * @param std desired standard deviation
   * @param setSeed if true the seed for the random generator will be
   * set using the current time.
   */
  NormalDistrGenerator(double mean = 0.0, double std = 1.0,
                       bool setSeed = true);

  void setMean(double mean) { m_Mean = mean; }
  double getMean() const { return m_Mean; }
  
  void setStdDev(double std) { m_StdDev = std; }
  double getStdDev() const { return m_StdDev; }

  /**
   * Call this function to generate a new value from a normal
   * distribution using the parameters within the class
   *
   * @return value from N(getMean(), getStdDev())
   */
  double getNewValue();

  /**
   * Call this function to generate a new value from a normal
   * distribution using the parameters mean and std passed in to the
   * function
   *
   * @param mean mean value
   * @param std stabdard deviation
   *
   * @return value from N(mean, std)   
   */
  double getNewValue(double mean, double std);

protected:
  double m_Mean;
  double m_StdDev;

  bool m_HaveSaved;
  double m_SavedValue;
};

inline 
NormalDistrGenerator::NormalDistrGenerator(double mean, double std,
                                           bool setSeed)
  :m_Mean(mean),
   m_StdDev(std),
   m_HaveSaved(false),
   m_SavedValue(0)
{
  if (setSeed) srand(time(0));
}

inline
double 
NormalDistrGenerator::getNewValue()
{
  return getNewValue(m_Mean, m_StdDev);
}

inline
double 
NormalDistrGenerator::getNewValue(double mean, double std)
{
  if (m_HaveSaved) {
    m_HaveSaved = false;
    return (mean + std * m_SavedValue);
  }
          
  // Get two numbers, -1 to +1 and make sure that they are in the unit
  // circle
  double v1, v2, rsq;
  do {
    v1 = 2.0 * rand() / (1.0 + RAND_MAX) - 1.0;
    v2 = 2.0 * rand() / (1.0 + RAND_MAX) - 1.0;
    rsq = v1*v1+v2*v2;
  } while (rsq >= 1.0 || rsq == 0);
  
  double fac = sqrt(-2.0*log(rsq)/rsq);

  m_SavedValue = v1 * fac;
  m_HaveSaved = true;

  return (mean + std * v2 * fac);
}

};

#endif // Cure_NormalDistrGenerator_hh
