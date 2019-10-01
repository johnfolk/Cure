// = AUTHOR(S)
//    John Folkesson
//    
// = COPYRIGHT
//    Copyright (c) 2007 John Folkesson
//    
#include <iostream>
#include "LongArray.hh"
Cure::LongArray::LongArray()
{
  m_Dummy=0;
  m_Mask=0;
  m_Array=0;
  m_AllocatedRows=0;
  m_Rows=0;
  grow(1);
  grow(0);
}
Cure::LongArray::LongArray(LongArray &l)
{
  m_Dummy=0;
  m_Mask=0;
  m_Array=0;
  m_AllocatedRows=0;
  m_Rows=0;
  grow(1);
  grow(0);
  (*this)=l;
}


void Cure::LongArray::setHashDepth(const unsigned long hashdepth)
{
  m_Mask=0;
  unsigned long sz=1;
  sz=(sz<<hashdepth);
  m_Mask=sz-1;
  grow(sz);
}
Cure::LongArray::~LongArray(){
  for (unsigned long i=0;i<m_AllocatedRows;i++){
    m_Array[i]-=2;
    delete []m_Array[i];
  }
  delete []m_Array;
}

void Cure::LongArray::print()
{
  std::cerr<<"LongArray "<<m_Rows<<"\n";
  for (long i=0;i<(long)m_Rows;i++){
    printRow(i);
  }
}
void Cure::LongArray::printRow(long i){
  long c=columns(i);
  if (c>0){
    std::cerr<<c<<" row "<<i<<": ";
    for (long j=0;j<c;j++)
      std::cerr<<m_Array[i][j]<<" ";
    std::cerr<<"\n";
  }
  

}
