// = AUTHOR(S)
//    John Folkesson
//    
//    March 11, 2004
//
//    Copyright (c) 2004 John Folkesson
//    
#ifndef CURE_TRIG_H
#define CURE_TRIG_H



namespace Cure {
  /**
   * This class does fast trig calls for half degree increments.
   * @author John Folkesson
   */
class Trig 
{
public:
  /**M_PI/(360);*/
  double HalfToRads;
protected:
  /**An array to store the cos of all half degree increments*/
  double CosAngle[720];
  /**An array to store the tan of some half degree increments*/
  double TanAngle[97];
public:
  /**Constructor*/
  Trig();
  /**
   * @param halfDegrees The angle in units of interger half degrees, 
   * so for instance PI=360.
   * @return the cos of the angle.
   */
  double cosOfHalf(int halfDegrees)
    {
      halfDegrees=halfDegrees%720;
      if (halfDegrees<0)halfDegrees+=720;
      return CosAngle[halfDegrees];
    }
  /**
   * @param halfDegrees The angle in units of interger half degrees, 
   * so for instance PI=360.
   * @return the sin of the angle.
   */
  double sinOfHalf(int halfDegrees)
    {
      halfDegrees-=180;
      halfDegrees=halfDegrees%720;
      if (halfDegrees<0)halfDegrees+=720;
      return CosAngle[halfDegrees];
    }
  /**
   * @param degrees The angle in units of interger degrees, 
   * so for instance PI=180.
   * @return the cos of the angle.
   */ 
  double cosOf(int degrees)
  {
    degrees+=degrees;
    degrees=degrees%720;
    if (degrees<0)degrees+=720;
    return CosAngle[degrees];
  }
  /**
   * @param degrees The angle in units of interger degrees, 
   * so for instance PI=180.
   * @return the sin of the angle.
   */
  double sinOf(int degrees)
  {
    degrees+=degrees;
    degrees-=180;
    degrees=degrees%720;
    if (degrees<0)degrees+=720;
    return CosAngle[degrees];
  }

  /**
   * @return the atan2(y,x) in integer degrees, where x[0] is x and x[1] is y.
   */
  int intAtan(double x[2]);
  /**
   * @return the asin(x) in integer degrees.
   */
  int intAsin(double x);

  /**
   * Convert from degrees to rads.
   * @return radians.
   */
  double toRads(int degrees)
  {
    degrees=(degrees<<1);

    return ((double)degrees)*HalfToRads;
  }
  double halfToRads(int halfdegrees)
  {
    return ((double)halfdegrees)*HalfToRads;
  }

  /**
   * Convert from rads to half degrees.
   * @return halfdegrees.
   */
 int toHalfDegrees(double rads)
  {
    rads/=HalfToRads;
    if (rads>0)rads+=.5;
    else rads-=.5;
    return (int)rads;
  }


  /**
   * Convert from rads to degrees.
   * @return degrees.
   */
 int toDegrees(double rads)  {
    return (toHalfDegrees(rads)>>1);
  }

   /**
   * @param halfDegrees Theta in units of integer half degrees
   * @param cosGamma cos(gamma).
   * @param sinGamma sin(gamma).
   * @return cos(theta-gamma).
   */
 double cosOfHalfMinusGamma(int halfDegrees, double cosGamma, 
			    double sinGamma)
   {
     halfDegrees=halfDegrees%720;
     if (halfDegrees<0)halfDegrees+=720;
     int j=halfDegrees-180;
     if (j<0)j+=720;

     return cosGamma*CosAngle[halfDegrees]+sinGamma*CosAngle[j];  
   }
   /**
   * @param degrees Theta in units of integer  degrees
   * @param cosGamma cos(gamma).
   * @param sinGamma sin(gamma).
   * @return cos(theta-gamma).
   */
 double cosOfThetaMinusGamma(int degrees, double cosGamma, double sinGamma)
   {
      degrees+=degrees;
      return cosOfHalfMinusGamma(degrees, cosGamma, sinGamma);
   }
};
  
}
#endif
