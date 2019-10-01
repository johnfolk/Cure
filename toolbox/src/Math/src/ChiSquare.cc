#include "ChiSquare.hh"

using 
namespace Cure;

void ChiSquare::setAlpha(int n, double rlimit)
{
  if (n<2){
    std::cerr<<"ChiSquare doen't support n<2 yet sorry.\n";
    return;
  }
  if (rlimit<0){
    std::cerr<<"ChiSquare can't use negative rlimit.\n";
    return;
  }
  m_Alpha=interpolateAlpha(n,rlimit);
  m_Limit.reallocateZero(1,n+100);
  double d=(*this)(n)-rlimit;
  double da=0.1;
  if (da>m_Alpha/2.0)da=m_Alpha/2.0;
  long cnt=1000;
  while (cnt>0){
    if (d>.00001){
      m_Alpha+=da;
      if (m_Alpha>=1){
	m_Alpha-=da;;
	break;
      }
    }else if (d<-.00001){
      m_Alpha-=da;
      if(m_Alpha<=0){
	m_Alpha+=da;;
	break;
      }
    }else break;
    if (da>m_Alpha/2.0)da=m_Alpha/2.0;
    m_Limit=0;
    d=(*this)(n)-rlimit;
    cnt--;
  }
  da/=2.0;
  cnt=100;
  for (int i=0;i<20;i++){
    while (cnt>0){
      if (d>.00001){
	m_Alpha+=da;
        if (m_Alpha>=1){
	  m_Alpha-=da;
	}
      }else if (d<-.00001){
	m_Alpha-=da;
	if(m_Alpha<=0){
	  m_Alpha+=da;;
	  da/=2.0;
	}
      }else break;
      if (da>m_Alpha/2.0)da=m_Alpha/2.0;
      m_Limit=0;
      d=(*this)(n)-rlimit;
      cnt--;
    }
    da/=2.0;
    cnt=100;
  }
  for (int i=0;i<n+100;i++){
    (*this)(i);
  }
}
void ChiSquare::setAlpha(double alpha)
{
  m_Alpha=alpha;
  m_Limit.reallocateZero(1,100);
  for (int i=2;i<100;i++){
    (*this)(i);
  }
  
}

