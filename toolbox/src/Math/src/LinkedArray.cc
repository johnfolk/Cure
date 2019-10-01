// = AUTHOR(S)
//    John Folkesson
//    
// = COPYRIGHT
//    Copyright (c) 2004 John Folkesson
//

#include "LinkedArray.hh"

using namespace std;
using namespace Cure;
LinkedArray::LinkedArray()
{
  AllocatedElement=0;
  Length=0;
  Element=0;
  Next=0;
  Prev=0;
}
LinkedArray::LinkedArray(const int len)
{
  AllocatedElement=0;
  Length=0;
  Element=0;
  Next=0;
  Prev=0;
  reallocate(len);
}
LinkedArray::LinkedArray(double *a, int len)
{
  AllocatedElement=0;
  Length=0;
  Element=0;
  Next=0;
  Prev=0;
  if (len>0)
    {
      reallocate(len);
      for (int i=0; i<Length; i++)
	Element[i]=a[i];
    }
}

LinkedArray::~LinkedArray()
{
  clear();
}
void LinkedArray::clear()
{
  if (AllocatedElement)delete [] AllocatedElement;
  AllocatedElement=0;
  Element=0;
  Length=0;
  if (Next)delete Next;
  Next=0;
}

void LinkedArray::reallocate(const int len)
{
  if (len==Length)return;
  if (AllocatedElement)delete [] AllocatedElement;
  AllocatedElement=0;
  Length=len;
  Element=0;
  if (len)
    {
      AllocatedElement=new double[len];
      if (Next==0) 
	{
	  Next=new LinkedArray();
	  Next->Prev=this;
	}
    }
  Element=AllocatedElement;
}
void LinkedArray::setArray( double *a, const int len)
{
  Element=a; 
  Length=len;
  if (Next==0) 
    {
      Next=new LinkedArray();
      Next->Prev=this;
    }
}

void LinkedArray::operator = (const LinkedArray& a) 
{
  if (a.Length==0)
    {
      clear();
      return;
    }
  if (Length!=a.Length) 
    reallocate(a.Length);
  for (int i=0; i<Length; i++)Element[i]=a(i);
  if (a.Next)
    {
      if (!Next)Next=new LinkedArray();
      Next->Prev=this;
      (*Next)=(*(a.Next));
      return;
    }
  if(Next) delete Next;
  Next=0;
}
void LinkedArray::operator = (const double d) 
{
  for (int i=0; i<Length; i++)
    Element[i]=d;
}
void LinkedArray::add(double *a, int len)
{
  if (Next)
    {
      Next->add(a,len);
      return;
    }
  Next=new LinkedArray();
  Next->Prev=this;
  reallocate(len);
  for (int i=0; i<Length; i++)
    Element[i]=a[i];

}
void LinkedArray::add(Matrix &a)
{
if (a.Rows<1)return;
  if (Next)
    {
      Next->add(a);
      return;
    }
  Next=new LinkedArray();
  Next->Prev=this;
  reallocate(a.Columns);
  for (int i=0; i<Length; i++)
    Element[i]=a(0,i);
  a.Element+=a.RowInc;
  a.Rows--;
  Next->add(a);
  a.Element-=a.RowInc;
  a.Rows++;  
}
void LinkedArray::cut()
{
  if (Prev)Prev->Next=Next;
  if (Next)Next->Prev=Prev;
  Next=0;
  Prev=0;
}
void LinkedArray::print()
{
  std::cerr<<"LinkedArray:";
  for (int i=0; i<Length; i++)
    std::cerr<<" "<<Element[i];
  std::cerr<<"\n";
}
