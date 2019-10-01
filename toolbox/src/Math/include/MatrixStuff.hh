// = RCSID
//    $Id: MatrixStuff.hh,v 1.7 2010/05/25 08:51:35 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
// = COPYRIGHT
//    Copyright (c) 2003 John Folkesson
//    

#ifndef CURE_MATRIXSTUFF_HH
#define CURE_MATRIXSTUFF_HH
#include <math.h>
#include <iostream>	
#include <stdlib.h>
#include <string.h>

namespace Cure {

/**
 * These are helper functions for the Matrix class and should not be 
 * needed to be examined by most users.
 * Functions for dealing with matrices defined as double arrays.
 * 
 * A matrix here is a double array Mij= Element[i*n+j]. 
 * Where n is the number of columns in the matrix. 
 *
 * @author John Folkesson
 */
namespace MatrixStuff {
  /**
   * These static inline functions are the key to making 
   * the Matrix Class fast.  Even faster than writing out these 
   * for loops in your code.  
   */
  static inline void matEqual( double *ans, const double *m1, 
                               const int r, const int c, 
                               const int offset_a, const int offset_1) 
  {
    long top=r*offset_a;
    if ((offset_a==offset_1)&&(offset_a==c))
      memcpy(ans,m1,sizeof(double)*(top));
    else{
      long irow1=0;
      for (long irow=0; irow<top; irow+=offset_a, irow1+=offset_1)
	memcpy(ans+irow,m1+irow1,sizeof(double)*(c));
    }
  }
  static inline void matAdd( double *ans, const double *m1,const double *m2, 
                             const int r,const int c, const int offset_a,
                             const int offset_1, const int offset_2) 
  {
    long top=r*offset_a;
    long irow1=0;
    long irow2=0;
    for (long irow=0; irow<top; irow+=offset_a, irow1+=offset_1, irow2+=offset_2)
      for (int j=0; j<c; j++)
        ans[irow+j]=m1[irow1+j]+m2[irow2+j];
  }

  static inline void matSubtract(double *ans,const double *m1,const double *m2, 
                                 const int r, const int c, const int offset_a,
                                 const int offset_1,const int offset_2) 
  {
    long top=r*offset_a;
    long irow1=0;
    long irow2=0;
    for (long irow=0; irow<top; irow+=offset_a, irow1+=offset_1, irow2+=offset_2)
      for (int j=0; j<c; j++)
        ans[irow+j]=m1[irow1+j]-m2[irow2+j];
  }
  static inline void matMult(double *ans,const double *m1,const double *m2, 
                             const int r1,const int c1_r2, const int c2, 
                             const int offset_a,const int offset_1,
                             const int offset_2) 
  {
    long top=r1*offset_a;
    long irow1=0;
    for (long irow=0; irow<top; irow+=offset_a, irow1+=offset_1)
      for (int j=0; j<c2; j++)
	{
	  ans[irow+j]=0;
	  long krow=0;
	  for (int k=0; k<c1_r2; k++,krow+=offset_2)
	    ans[irow+j]+=m1[irow1+k]*m2[krow+j];
	}
  }
    
  static inline void offsetCopy( double *ans, const double *m1, 
                               const int r, const int c, 
                                 const int offset_a, const int offset_1) 
  {
    long top=r*offset_a;
    long irow=0;
    for (long i=0; irow<top; i+=offset_1,irow+=offset_a)
      for (int j=0; j<c; j++)
        ans[irow+j]=m1[i+j];  
  }

  static inline void matPlusEqual( double *ans, const double *m1, 
                                   const int r, const int c, 
                                   const int offset_a, const int offset_1) 
  {
    long top=r*offset_a;
    long irow1=0;
    for (long irow=0; irow<top; irow+=offset_a, irow1+=offset_1)
      for (int j=0; j<c; j++)
        ans[irow+j]+=m1[irow1+j];
  }


  static inline void matMinusEqual(double *ans, const double *m1,
                                   const int r, const int c, 
                                   const int offset_a, const int offset_1) 
  {
    long top=r*offset_a;
    long irow1=0;
    for (long irow=0; irow<top; irow+=offset_a, irow1+=offset_1)
      for (int j=0; j<c; j++)
        ans[irow+j]-=m1[irow1+j];
  }


  static inline void matAddProd(double *ans,const double *m1,const double *m2, 
                                const int r1,const int c1_r2,const int c2, 
                                const int offset_a,const int offset_1,
                                const int offset_2) 
  {
    long top=r1*offset_a;
    long irow1=0;
    for (long irow=0; irow<top; irow+=offset_a, irow1+=offset_1)
      for (int j=0; j<c2; j++)
        {
          long krow=0;
          for (int k=0; k<c1_r2; k++,krow+=offset_2)
            ans[irow+j]+=m1[irow1+k]*m2[krow+j];
        }
  }

  static inline void matSubProd(double *ans,const double *m1,const double *m2, 
                                const int r1,const int c1_r2,const int c2, 
                                const int offset_a,const int offset_1,
                                const int offset_2) 
  {
    long top=r1*offset_a;
    long irow1=0;
    for (long irow=0; irow<top; irow+=offset_a, irow1+=offset_1)
      for (int j=0; j<c2; j++)
        {
          long krow=0;
          for (int k=0; k<c1_r2; k++,krow+=offset_2)
            ans[irow+j]-=m1[irow1+k]*m2[krow+j];
        }
  }

  /**
   * ans = -m1 *m1.
   */
  static inline void matMinusMult(double *ans,const double *m1,
                                  const double *m2,const int r1,
                                  const int c1_r2, const int c2, 
                                  const int offset_a,const int offset_1,
                                  const int offset_2) 
  {
    long top=r1*offset_a;
    long irow1=0;
    for (long irow=0; irow<top; irow+=offset_a, irow1+=offset_1)
      for (int j=0; j<c2; j++)
        {
          ans[irow+j]=0;
          long krow=0;
          for (int k=0; k<c1_r2; k++,krow+=offset_2)
            ans[irow+j]-=m1[irow1+k]*m2[krow+j];
        }
  }
  /**
   * This uses kramer's rule to calc 2x2 inverse.
   * @param m double array with the matrix elements.
   * @param ans the result is returned here.
   * @return 1 if singular.
   * @return 0 if ok.
   */
  static inline int invert2x2(double m[4], double ans[4]){
    double d=m[0]*m[3]-m[2]*m[1];
    if (d==0)return 1;
    double temp=m[3]/d;
    ans[3]=m[0]/d; 
    ans[0]=temp;
    temp=-m[2]/d;
    ans[1]=-m[1]/d;
    ans[2]=temp;
    return 0;
  }
  
  
  /**
   * This takes a 2x2 matrix and finds the 2
   * (eigen values, eigen vectors)=(lambda[0],ev[0..1]),
   * (lambda[1],ev[2..3]).
   * @param m double array with the matrix elements.
   * @param ev the result is returned here.
   * @param lambda the eigenvalues are returned here.
   * @return 1 if there are no real roots.
   * @return 2 if one root is negative.
   * @return 0 if ok.
   */
  static inline int eigen2X2(double m[4], double ev[4],double lam[2]){
    double tr,diff,v[2];
    tr=m[0]+m[3];
    tr/=2;
    diff=m[0]-m[3];
    diff/=2;
    lam[0]=diff*diff+m[2]*m[1];
    if (lam[0]<0) return 1;
    lam[0]=sqrt(lam[0]);
    lam[1]=tr-lam[0];
    lam[0]+=tr;
    v[0]=(m[0]-lam[0]);
    if (v[0]!=0)
      {
	v[0]=-m[1]/v[0];
	v[1]=1/sqrt(v[0]*v[0]+1);
	v[0]*=v[1];
      }
    else 
      {
	v[0]=(m[0]-lam[1]);
	if (v[0]!=0)
	  {
	  v[0]=-m[1]/v[0];
	  v[1]=1/sqrt(v[0]*v[0]+1);
	  v[0]*=v[1];
	  v[0]=v[1];
	  v[1]=-v[0];
	}
	else  
	  {
	    v[0]=1;
	    v[1]=0;
	  }
      }
    ev[0]=v[0];
    ev[1]=v[1];
    ev[2]=v[1];
    ev[3]=-v[0];
    if (lam[1]<0) return 2;
    if (lam[0]<0) return 2;
    return 0;
  }

  static inline void mat3x3Mult(double *ans,const double *m1,const double *m2) 
  {
    ans[0]=m1[0]*m2[0]+m1[1]*m2[3]+m1[2]*m2[6];
    ans[1]=m1[0]*m2[1]+m1[1]*m2[4]+m1[2]*m2[7];
    ans[2]=m1[0]*m2[2]+m1[1]*m2[5]+m1[2]*m2[8];
    ans[3]=m1[3]*m2[0]+m1[4]*m2[3]+m1[5]*m2[6];
    ans[4]=m1[3]*m2[1]+m1[4]*m2[4]+m1[5]*m2[7];
    ans[5]=m1[3]*m2[2]+m1[4]*m2[5]+m1[5]*m2[8];
    ans[6]=m1[6]*m2[0]+m1[7]*m2[3]+m1[8]*m2[6];
    ans[7]=m1[6]*m2[1]+m1[7]*m2[4]+m1[8]*m2[7];
    ans[8]=m1[6]*m2[2]+m1[7]*m2[5]+m1[8]*m2[8];
  }

  static inline void mat3x3TrnMult(double *ans,const double *m1,const double *m2) 
  {
    ans[0]=m1[0]*m2[0]+m1[3]*m2[3]+m1[6]*m2[6];
    ans[1]=m1[0]*m2[1]+m1[3]*m2[4]+m1[6]*m2[7];
    ans[2]=m1[0]*m2[2]+m1[3]*m2[5]+m1[6]*m2[8];
    ans[3]=m1[1]*m2[0]+m1[4]*m2[3]+m1[7]*m2[6];
    ans[4]=m1[1]*m2[1]+m1[4]*m2[4]+m1[7]*m2[7];
    ans[5]=m1[1]*m2[2]+m1[4]*m2[5]+m1[7]*m2[8];
    ans[6]=m1[2]*m2[0]+m1[5]*m2[3]+m1[8]*m2[6];
    ans[7]=m1[2]*m2[1]+m1[5]*m2[4]+m1[8]*m2[7];
    ans[8]=m1[2]*m2[2]+m1[5]*m2[5]+m1[8]*m2[8];
  }

  static inline void mat3x3Trn(double ans[9],const double m[9]) 
  {
    ans[0]=m[0];
    double d=m[1];
    ans[1]=m[3];
    ans[3]=d;
    d=m[2];
    ans[2]=m[6];
    ans[6]=d;
    ans[4]=m[4];
    d=m[5];
    ans[5]=m[7];
    ans[7]=d;
    ans[8]=m[8];
  }

} // namespace Cure

} // namespace MatrixStuff

#endif // CURE_MATRIXSTUFF_HH
