// = AUTHOR(S)
//    John Folkesson
//    
// = COPYRIGHT
//    Copyright (c) 2007 John Folkesson
//    
#ifndef CURE_CHISQUARE_HH
#define CURE_CHISQUARE_HH
#include "Matrix.hh"

namespace Cure {
  
/**
 * Tabulates limit values for a given alpha as a function of n dof > 1
 *
 * it computes the easy values for integer m=(n/2-1) and interpolates to n
 *
 * n=dof for the chisquare 
 *
 * P(u>limit) < alpha
 *
 * u = (1/2)*sum {i = 1 to n }( x_i^2 ) where x_i are standard normal varibles
 *
 * 
 */
  class ChiSquare{
  public:
    Matrix m_Limit;
    double m_Alpha;
    ChiSquare(){
      m_Alpha=0;
    }
    void operator = (ChiSquare &chi){
      m_Limit=chi.m_Limit;
      m_Alpha=chi.m_Alpha;
    }
    /**
     * One calls this to initialize the table and alpha value
     *
     * @param n the dof>1
     * @param limit the limit on u for this n 
     * ie (P(u>limit | n)=alpha is used to find the alpha
     *
     *
     */
    void setAlpha(int n, double limit);

    /**
     * A more direct way to initialize alpha
     * @param alpha the value for m_Alpha.
     */
    void setAlpha(double alpha);
    
    /**
     * This gets the limits for the set alpha and the n arg
     * @param n the dof>0
     * @return the limit P(u>limit | n)==m_Aplha;
     *
     */
    double operator() (const short n){
      if (n<1)return 0;
      if (m_Limit.Columns<=n)
	m_Limit.grow(1,n);
      if (m_Limit(0,n)>0)return m_Limit(0,n);    
  
      if (n==1){
	double d=(*this)(2);
	m_Limit(0,n)=d*(1+(d-(*this)(4))/(2*d));
	if(m_Limit(0,n)<d/4)
	  m_Limit(0,n)=d/4;
      }else{
	
	if (n%2){
	  double d=(*this)(n-1);
	  m_Limit(0,n)=(d+(*this)(n+1))/2;
	}else{
	  double r=1;
	  int dn=n;
	  for (int i=n-1;i>1;i--){
	    if (m_Limit(0,i)!=0)
	      {
		r=m_Limit(0,i);
		dn=n-i;
		break;
	      }
	  }
	  int m=n/2-1;
	  double dr=r;
	  dr/=2.0;
	  double d=m_Alpha-getAlpha(m,r);
	  long cnt=100;
	  dn/=5;
	  if (dn>10)dn=10;
	  if (dn>0)
	    cnt=(cnt<<dn);
	  while(cnt>0) {
	    if (d>1E-10){
		r-=dr;
		if (r<0){
		  r+=dr;
		  dr/=2.0;
		  break;
		}
	    }else if (d<-1E-10){
	      r+=dr;
	    }else break;
	    d=m_Alpha-getAlpha(m,r);
	    cnt--;
	  }
	  cnt=100;
	  double lim=m_Alpha*1E-2+1E-128;
	  if (lim>1E-4)lim=1E-4;
	  while(cnt>0) {
	      dr/=(2.0);
	      if (d>lim){
		r-=dr;
		if (r<0){
		  r+=dr;
		  dr/=2.0;
		}
	      }else if (d<-lim){
		r+=dr;
		dr*=2.0;
	      }else break;
	      d=m_Alpha-getAlpha(m,r);
	      cnt--;

	  }
	  if (r==0)r=1E-32;
	  m_Limit(0,n)=r;
	}
      }  
      return m_Limit(0,n);    
    }
    
    /**
   * This gets alpha
   * @param m (n/2-1) should be >=0
   * @param r the radius >=0
   * @return P(u>r) for ChiSquare n (n= 2*m+1)
   */
  double getAlpha(int m,double r){
    double d=1;
    double s=1;
    double f=1;
    m++;
    for (int i=1;i<m;i++){
      f*=i;
      d*=r;
      s+=d/f;
    }
    s*=exp(-r);
    return s;
  }
  /**
   *
   *
   */
  double interpolateAlpha(int n,double rlimit){
    int m=n/2-1;
    double d=getAlpha(m,rlimit);
    if (n%2==0)return d;
    d+=getAlpha(m+1,rlimit);
    d/=2.0;
    return d;
  }
  
  };
}
#endif
