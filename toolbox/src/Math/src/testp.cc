
#ifndef DEPEND
#include <iostream>
#include <stdlib.h>
#include <sys/time.h>
#endif
 
#include "Matrix.hh"
#include "BinaryMatrix.hh"
#include "Vector3D.hh" 
#include "Polynomial.hh"
#include "PolynomialRoots.hh"
#include "IntersectionEllipseEllipse.hh"
#include "Vector3D.hh"

using namespace std;
using namespace Cure;

double getCurrentTime()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (1.0 * tv.tv_sec + 1.0e-6 * tv.tv_usec);
}
int testEigenStuff()
{
  int n=7;
  Matrix m(n), h(n);
  
  Matrix mev(n),mlambda(n);
  m=10;
 for (int j=1; j<n; j++)
   m(j,j)+=10*m(j-1,j-1);
  for (int i=0; i<n; i++)
    for (int j=0; j<n; j++)
      m(i,j)+=(1-(double)rand()/(0.5*(double)RAND_MAX));

 for (int j=0; j<n; j++)
    {
      m(2,j)=0;
      m(3,j)=0;
      m(j,2)=0;
      m(j,3)=0;
    }
  
  m.symetrize(); 
  m.print();

  //  m.eigenFactor(mlambda,mev);
  Matrix t;
  m.symmetricEigen(mlambda,mev);
  mlambda.print();
  mev.print();
  t.multTranspose_(mev,m,1);
  t.multiply(t,mev);
  t.print();
  m.symmetricEigen(mlambda,mev,2);
  mlambda.print();
  mev.print();
  t.multTranspose_(mev,m,1);
  t.multiply(t,mev);
  t.print();

  return 0;
  for (int i=0; i<n; i++)
    mlambda(i,i)=1/mlambda(i,i);
  t.multiply_(mlambda,mev);
  mlambda.multTranspose_(mev,t,1);
  mlambda*=m;
  mlambda.print();

  mlambda=m;
  mlambda.invert();
  mlambda*=m;
  mlambda.print();
  double ev[n*n],lambda[n];
  //  m.pca(ev, lambda);
  mlambda=0;
  for (int i=0; i<n; i++)
    {
      mlambda(i,i)=lambda[i];
      for (int j=0; j<n; j++)
	{
	  mev(i,j)=ev[n*j+i];
	}
    }
  mlambda.print();
  mev.print();
  t.multTranspose_(m,mev,2);
  t.print();
  t.multiply(mev,t);
  t.print();

  for (int i=0; i<n; i++)
    mlambda(i,i)=1/mlambda(i,i);
  t.multiply_(mlambda,mev);
  mlambda.multTranspose_(mev,t,1);
  mlambda*=m;
  mlambda.print();
  return 0;
  /*
 m(0,0)=16;
  m(1,0)=5;
  m(2,0)=9;
  m(3,0)=4;  
  m(0,1)=2;
  m(1,1)=11;
  m(2,1)=7;
  m(3,1)=14;  
  m(0,2)=3;
  m(1,2)=10;
  m(2,2)=6;
  m(3,2)=15;  
  m(0,3)=13;
  m(1,3)=8;
  m(2,3)=12;
  m(3,3)=1;  
  m.print();
  
  //   m.multTranspose(m,m,1);

  

  
  m.eigenFactor(mev,mlambda);
  mlambda.print();
  mev.print();
  m.print();
    return 0;
    
  //m.multTranspose(m,m,1);
   m.eigenFactor(mlambda,mev);
  m.symmetricEigen(mlambda,mev);
  m.print();
  mev.print();
  mlambda.print();
  m.multiply(m,mev);
  m.print();
  
  return 0;
  */
  m.reallocate(n);
  Matrix v(n);
  Matrix a(n);
  m=0;
  for (int j=0; j<n; j++)
    {
      for (int i=0; i<n; i++)
	v(i,j)=rand();
    }
  v.normalize();
  v.orthogonalize();
  v.normalize();
  //  v.print();
  //  a.multTranspose(v,v,1);
  //a.print();
  
  Matrix d(n);
  d=0;
  double dete=1;
  for (int i=0;i<n; i++)
    {
      d(i,i)=(pow(-1.0,i))*pow(10,10*(double)(n-i)/n);
      dete*=d(i,i);
      
    } 
  //d.print();
  a.multTranspose_(d,v,2);
  m.multiply_(v,a);
  //    for (int i=0; i<10000; i++)
    //m.eigenFactor(mlambda,mev);
	//m.symmetricEigen(mlambda,mev,0);
	//m.symmetricEigen(mlambda);
  //m.pca(mev.Element,mlambda.Element);
  //return 0;

  m.eigenFactor(mlambda,mev);
//m.print();
  //mev.print();
  //mlambda.print();
 
  double det=dete;

  for (int i=0; i<n; i++){
    det/=(mlambda(i,i));

  }
  mlambda-=d;
  std::cerr<<"EigenFactor "<<mlambda.trace()<<" "<<det<<" "<<std::endl;

  m.symmetricEigen(mlambda,mev,0);
  d.print();
  mlambda.print();
  //mev.print();
  //mlambda.print();
  // mlambda.print();
  
  //  for (int i=0; i<2;i++)
  //m.eigenize(mev);
  // mev.orthogonalize();
  //m.eigenize(mev,&mlambda);
  a.multTranspose(mev,mev,1);
  std::cerr<<"a "<<std::endl;
   a.print();
  det=dete;
  double ss=0;
  for (int i=0; i<n; i++){
      det/=(mlambda(i,i));
      ss+=(d(i,i)/mlambda(i,i)-1)*(d(i,i)/mlambda(i,i)-1);
      mlambda(i,i)=1/mlambda(i,i);
  }

  //mev.normalize();
  //  mev.orthogonalize();
    //mev.normalize();

  
  //mev.print();
  Matrix minv(n);
  minv.multTranspose_(mlambda,mev,2);
  minv.multiply(mev,minv);
  minv.multiply(m,minv);
  //minv.print();
  ss/=n;
  //  mlambda.print();
  mlambda-=d;
  std::cerr<<"SymmetricEigen "<<mlambda.trace()<<" "<<det<<" "<<ss<<std::endl;
  //mlambda.print();



  // m.pca(mev.Element,mlambda.Element);
  //  mev.print();
  mlambda.Rows=1;
  //mlambda.print();
det=dete;
ss=0;
 for (int i=0; i<n; i++){
  
   det/=(mlambda(0,i));
   ss+=(d(i,i)/mlambda(0,i)-1)*(d(i,i)/mlambda(0,i)-1);
   d(i,i)-=mlambda(0,i);
   mlambda(i,i)=1/mlambda(0,i);
   if (i!=0)mlambda(0,i)=0;
 }
 //  mev.print();
  minv.multTranspose_(mev,mlambda,1);
  minv.multiply(minv,mev);
  minv.multiply(m,minv);
  //minv.print();

 ss/=n;
 std::cerr<<"pca "<<d.trace()<<" "<<det<<" "<<ss<<std::endl;


  return 0;
} 

void invertAndTransposeTest()
{
  Matrix m(3,3);
  m = 1;
  m(1,1) = 2;
  m(0,2) = 4;
  m.print();
  m.inv().print();
  m.T().print();
}

void testBimaryMatrix()
{
  BinaryMatrix bm(3,50);
  bm.print();
  bm.setBit(2,4,1);
  bm.setBit(2,8,1);
  bm.setBit(3,4,1);
  cerr<<"Test of error printout: Error message is good here:\n";
bm.setBit(1,54,1);
  bm.print();
  BinaryMatrix bm2(bm);
  bm2.print();
  bm&=bm2;
  bm.print();
  bm|=bm2;
  bm.print();
  ~(bm);
  bm.print();
  bm&=bm2;
  bm.print();
  bm|=bm2;
  bm.print();
  ~(bm);
  bm.deleteColumns(3,5);
  bm.print();
  bm.insertColumns(3,2);
  bm.print();
  bm.grow(10,10);
  bm.print();
  bm.swapRows(2,9);
  bm.print();
  bm2=1;
  bm2.print();
  bm2=0;
  bm.transpose(bm2);
  bm.deleteRow(5);
  bm.transpose();
  bm.offset(0,0,2,5);
  bm=1;
  bm.reset();
  bm.swapColumns(4,15);
  bm.print();
  bm2=bm;
  bm.setBit(0,0,0);
  bm.print();
  bm2.print();
  bm^=bm2;
  bm.print();
  ~(bm2);
  bm^=bm2;
  bm.print();
  bm2.print();
  bm2=bm;
  ~(bm2);
  bm^=bm2;
  bm.print();
  bm.offset(0,0,1,bm.Columns);
  bm=0;
  bm.offset(1,0,1,bm.Columns);
  bm=1;
  bm.offset(1,0,1,bm.Columns);
  bm=0;
  bm.reset(3,bm.Columns);
  bm.offset(0,24,3,1);
  bm.print();
  bm=true;
  bm.reset(3,48);
  BinaryMatrix bm4,bm3;
  bm4.reallocate(10,300);
  bm3.reallocate(10,400);
  bm4.offset(3,60,7,80);
  bm4.setBit(6,64,1);
  bm4.setBit(6,0,1);
  bm4.setBit(4,8,1);
  bm4.setBit(0,0,1);
  bm4.setBit(0,64,1);
  bm4.print();
  std::cerr<<bm4(4,4)<<" "<<bm3(4,4)<<" "<<bm4(2,4)<<" "<<bm3(2,4)<<"\n";;
  bm3.offset(2,60+256,7,80);
  bm3=bm4;
  bm3.print();
  std::cerr<<bm4(4,4)<<" "<<bm3(4,4)<<" "<<bm4(2,4)<<" "<<bm3(2,4)<<"\n";;
  getchar();

  bm.print();
  

  
 //  bm.setBit(0,0,0);
  // bm.setBit(1,0,0);
  double origin[2];
  origin[0]=-32.1;
  origin[1]=32.1;
  //~bm;
  bm.print();
  bm.rotate(.75,origin,true,3);
  bm.print();
  cerr<<" "<<origin[0]<<" "<<origin[1]<<"\n";
  getchar();
  bm.rotate(-.75,origin,true,3);
  bm.print(); 
 
  cerr<<bm.count();
  cerr<<" "<<origin[0]<<" "<<origin[1]<<"\n";
  getchar();
   bm2=bm;
   bm.trimRows();
   bm.print();
   bm2.offset(23,0,bm.Rows,bm.Columns);
   getchar();
   bm2.print();
   bm.print();
   cerr<<" "<<bm.match(bm2)<<"\n";
   bm^=(bm2);
   getchar();
   bm.print();
   ShortMatrix ind;
   bm.grow(50,50);
   bm.offset(40,5,1,35);
   bm=1;
   bm.offset(-40,0,35,1);
   bm=1;
   bm.reset(50,50);
   int col=19;
   int row=1;
   bm.setBit(row,col,true);
   bm.print();  
   cerr<<bm(row,col)<<" dis= "<<bm.nearest(row,col,ind,bm.Columns)<<"\n";
   ind.print();
   bm.offset(20,20,10,10);

   bm=1;
   bm.reset(50,50);
   bm.offset(24,30,4,5);
   bm=1;
   bm.reset(50,50);
   bm.offset(20,35,10,2);
   bm=1;
   bm.reset(50,50);

  bm.print();  
   bm.boundary(bm2,6);
   
   bm2.print();

   ShortMatrix vert(11,2);
   vert(0,0)=5;
   vert(0,1)=5;

   vert(1,0)=10;
   vert(1,1)=5;

   vert(2,0)=13;
   vert(2,1)=8;
   
   vert(3,0)=17;
   vert(3,1)=8;
   vert(4,0)=17;
   vert(4,1)=15;
   vert(5,0)=7;
   vert(5,1)=15;
   vert(6,0)=17;
   vert(6,1)=8;
   vert(7,0)=10;
   vert(7,1)=8;
   vert(8,0)=10;
   vert(8,1)=10;
   vert(9,0)=5;
   vert(9,1)=11;
   vert(10,0)=2;
   vert(10,1)=8;
   
   bm.reallocate(20,20);
   bm=0;
   bm.setPolygon(vert);
   bm.print();
   bm=1;
   bm.clearPolygon(vert);
   bm.print();
   bm.reallocate(40,70);

   double center[3];
   center[0]=20;
   center[1]=35;
   center[2]=3;
   bm=0;
   bm.setArc(center,16,-3.14,3.14);
   bm.print();
 
   bm.clearSector(center,28,1,1.6);
   bm.print();
   getchar();
   bm=1;
   bm.clearSector(center,22,1,1.5);
   bm.print();
   getchar();
  bm=1;
   bm.clearSector(center,20,-3.14,3.14);
   bm.print();
   getchar();
   bm=1;
   bm.clearHsection(center,20,-3.14,3.14);
   bm.print(); 
   getchar();

   /*
    for (int i=0; i<32; i++)
     {
       bm=1;
       bm.clearSector(center,30,i*.1,i*.1+.4);
       bm.print();
       cerr<<"\n"<<i;
       getchar();
       bm=1;
       bm.clearHsection(center,30,i*.1,i*.1+.4);
       bm.print();
       cerr<<"\n"<<i;
       getchar();
     }
   */
   bm=0;
   bm.setArc(center,18.5,-3.14,3.14);
   bm.setArc(center,19,-3.14,3.14);
   bm.setArc(center,19.5,-3.14,3.14);
   bm.print();
   getchar();

   ShortMatrix cells(1,2);
   cells(0,0)=0;
   cells(0,1)=0;
   BinaryMatrix neighbors, boundary;

   Matrix celldis;
   bm.clusterGrowth( neighbors,true, 
		     10,15, 20);
/*
   short mindistan[2];
 double dista=bm.BreathFirstSearch(cells,21.0,
				     neighbors,
				     boundary,
       			     mindistan,celldis);
   //cerr<<dista<<"\n";
*/ 
  bm.print();
   //boundary.print();
   neighbors.print();

   return;
   getchar();
   center[2]=5;
   bm=0;
   bm.setHarc(center,25,.5,1);
   bm.print();
   getchar(); 
   bm=0;
   bm.setArc(center,25,.5,1);
   bm.print(); 
}

void testSubmatrix()
{
  std::cout << "=========== START testSubmatrix ================\n";
  Matrix a(3,6);
  for (int i = 0; i < a.Rows; i++) {
    for (int j = 0; j < a.Columns; j++) {
      a(i,j) = 10.0 * rand() / (RAND_MAX - 1);
    }
  }

  a.print();
  Matrix b(a,0,0,3,3);
  b.print();
  b.setToSubmatrix(a,0,3,3,3);
  b.print();
  Matrix c(a.getSubmatrix(0,0,3,3));
  c.print();
  a.getSubmatrix(0,0,3,3).print();
  a.print();
  std::cout << "=========== END testSubmatrix ================\n";
}

#include "Math/ChiSquare.hh"

int main(int argc, char * argv[])
{
  BinaryMatrix sig(1,47);
  BinaryMatrix sigs;
  sig.setBit(0,12,true);
  sigs.addOrdered(sig);
  sigs.print();
  sig.setBit(0,13,true);
  sigs.addOrdered(sig);
  sigs.print();
  sig.setBit(0,13,false);
  sigs.addOrdered(sig);
  sigs.print();
  sig.setBit(0,15,true);
  sigs.addOrdered(sig);
  sigs.print();
  sig.setBit(0,15,false);
  sig.setBit(0,12,false);
  sig.setBit(0,0,true);
  sigs.addOrdered(sig);
  sigs.print();
  sig.setBit(0,40,true);
  sigs.addOrdered(sig);
  sigs.print();
  sig.setBit(0,40,false);
  sig.setBit(0,37,true);
  sigs.addOrdered(sig);
  sigs.print();
  getchar();
  ChiSquare chi;
  double chin=8;
  chi.setAlpha(3,chin);
  std::cerr<<"Chi^2 prob(x>"<<chin<<")= "<<chi.m_Alpha<<"\n";
  chi.m_Limit.print();

  invertAndTransposeTest();

  testSubmatrix();

  testBimaryMatrix();

  return 0;
  return testEigenStuff();
  Matrix h(3);
  h(0,0)=5889.36;
  h(0,1)= -37319.7;
  h(0,2)= 139.22 ;
  h(1,0)=-37319.7;
  h(1,1)=237019; 
  h(1,2)=-862.576; 
  h(2,0)=139.22;
  h(2,1)= -862.576;
  h(2,2)= 4.01543;
  Matrix hin(h);
  std::cerr<<hin.invert()<<"\n";
  Matrix id(3);
  id=hin*h;
  h.print();
  hin.print();
  id.print();
  
  return 0;

  double coeff[] = {24, -50, 35, -10, 1};
  Polynomial poly(4, coeff);  // (x-1)(x-2)(x-3)(x-4)
  std::cerr << poly << std::endl;

  PolynomialRoots roots(DBL_EPSILON);
  roots.findB(poly, 10);
  std::cerr << roots << std::endl;

  std::cerr << "\n=======================================================\n\n";

  Vector2D rkC0(-1,0);
  Vector2D akAxis0[2];
  akAxis0[0][0] = 1;
  akAxis0[0][1] = 0;
  akAxis0[0].normalize();
  akAxis0[1] = akAxis0[0].perp();
  double afL0[2] = {1.01, 1};
  Vector2D rkC1(1,0);
  Vector2D akAxis1[2];
  akAxis1[0][0] = 1;
  akAxis1[0][1] = 0;
  akAxis1[0].normalize();
  akAxis1[1] = akAxis1[0].perp();
  double afL1[2] = {1.01, 1};
  int riQuantity; 
  Vector2D akP[4];
  double t1 = getCurrentTime();
  IntersectionEllipseEllipse::findIntersection(rkC0, akAxis0, afL0,
                                               rkC1, akAxis1, afL1,
                                               riQuantity, akP);
  double t2 = getCurrentTime();
  for (int i = 0; i < riQuantity; i++) {
    std::cerr << "Point " << i << ": (" << akP[i][0] 
              << ", " << akP[i][1] << ")" << std::endl;
  }
  std::cerr << "Finding intersection took " << t2-t1 << "s\n";

  std::cerr << "\n=======================================================\n\n";

  int size=5;
  double am[size*size];  
  double ev[size*size],lambda[size],w[size]; 
  for (int i=0; i<size*size; i++)
    {    
      am[i]=sin((double)i);
    }
  Matrix a(am,size,size,size);
  Matrix b(size,size+3);
  b.print();
  Matrix c(size,size);
  a.symetrize();
  c=0;
  c+=1;
  c.print();
  b=a;
  cerr<<"a="<<endl;
  a.print();
  cerr<<"b="<<endl;
  b.print();
  cerr<<"a+b="<<endl;
  c.add(a,b);
  //c=a+b;
  c.print();
  a.print();
  a.transpose(a);
  a.print();
  
  cerr<<a.trace()<<endl;
  cerr<<a.rank()<<endl;
   a.singularValues(w); 
  cerr<<endl;
  cerr<<"EigenValues=";
  for (int i=0; i<size; i++)
    cerr<<w[i]<<" ";
  cerr<<endl;
 
  a+=1;
  b=a;
  a.print();
  cerr<<"inv a="<<endl;
  a.invert();
  a.print();
  c=(a*b);
  cerr<<" a*b="<<endl;
  c.print();
  

  //  cerr<<b.pca(ev, lambda)<<" PCA "<<endl; 
  for (int i=0; i<size; i++)
    cerr<<lambda[i]<<" ";
  cerr<<endl;
  for (int i=0; i<size; i++)
    {
      for (int j=0; j<size;j++)  
	cerr<<ev[i*size+j]<<" ";
      cerr<<endl;
    }
  
  b.singularValues(w);  
  cerr<<endl;
  cerr<<"EigenValues=";
  for (int i=0; i<size; i++)
    cerr<<w[i]<<" ";
  cerr<<endl;
  cerr<<"rank= "<<b.rank();
  cerr<<" trace= "<<b.trace();
  cerr<<" det= "<<b.determinant();
  a.reallocate(6,7);
  for (int i=0; i<6; i++)
    for (int j=0; j<7;j++)
      {    
	a(i,j)=sin((double)i)*cos((double)j);
      }
  a.print();
  a.print();
  b=-1;
  b.print();
  b.singularValues(w);  
  cerr<<endl;
  cerr<<"EigenValues=";
  for (int i=0; i<size; i++)
    cerr<<w[i]<<" ";
  cerr<<endl;
  //  cerr<<b.pca(ev, lambda)<<" PCA "<<endl; 
  for (int i=0; i<size; i++)
    cerr<<lambda[i]<<" ";
  cerr<<endl;
  for (int i=0; i<size; i++)
    {
      for (int j=0; j<size;j++)  
	cerr<<ev[i*size+j]<<" ";
      cerr<<endl;
    }
  b.reallocate(3);
  b=0;
  b(0,0)=2E6;
  b(1,1)=2;
  b(2,1)=10;
  b(1,2)=10;
  b.print();
  
  cerr<<endl;
  cerr<<b.singularValues(w)<<"EigenValues=";
  for (int i=0; i<3; i++)
    cerr<<w[i]<<" ";
  cerr<<endl;
  //  cerr<<b.pca(ev, lambda)<<" PCA "<<endl; 
  for (int i=0; i<3; i++)
    cerr<<lambda[i]<<" ";
  cerr<<endl;
  for (int i=0; i<3; i++)
    {
      for (int j=0; j<3;j++)  
	cerr<<ev[i*3+j]<<" ";
      cerr<<endl;
    }
  a.reallocate(3);
  a=0;
  a=2;
  a(0,2)=1;
  Vector3D v1, v2;
  double x[3];
  x[0]=3;
  x[1]=2;
  x[2]=1;
  v1.print();
  v1=x;
  v1.print();
  v2=v1;
  v2.print();
  double d=v1*v2;
  v1+=v2;
  v1.print();
  v1=v2+v2;
  v1.print();
  v1-=v2;
  v1.print();
  v1/=sqrt(v1*v1);
  v1.print();
  d=v1*v1;
  cerr<<d<<endl;
  a.print();
  v2=v1*a;
  v2.print();
  v2=v1.leftMultBy(a);
  v2.print();
  
  a.print();
  a.multiply(a,a);
  a.print();
  a/=2;
  cerr<<"Not value safe"<<endl;
  a.print();
  a.multiply_(a,a);
  a.print();
  

  return 0;
  for (int i=0; i<100000; i++)
  {
    /*      
      int top=size*size;
      int irow1=0;
      for (int irow=0; irow<top; irow+=size, irow1+=size)
      for (int j=0; j<size; j++)
      {
        am[irow+j]=0;
	int krow=0;
	for (int k=0; k<size; k++,krow+=size)
	am[irow+j]+=bm[irow1+k]*cm[krow+j];
      }
    */
    // mat_mult(am,bm,cm, size,size,size,size,size,size);
    
    //a.Multiply(b,c);
    //a=(b*c);
    //      b.Invert();
    //      sym_invert3x3(b.m, b.m);
    
    //b.eigenize(ev, lambda);
  }
  
  b=a;
  b.print();
  b.invert();
  b.print();
  b*=a;
  b.print();
  b=a;
  b.print();
  b*=a;
  b.print();

  b.print();
  cerr<<b(3,3)<<" ";
  b(3,3)=-100000;
  cerr<<b(3,3)<<endl;
  b.print();

  return 0;
} 

