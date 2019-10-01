//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#include "TimestampedData.hh"

namespace Cure {

TimestampedData::TimestampedData()
{
  init();
}

TimestampedData::~TimestampedData()
{}

void 
TimestampedData::interpolate_(TimestampedData& a, TimestampedData& b, 
			      Timestamp t)
{
  Timestamp dtb=b.Time;
  dtb-=t;
  Timestamp dta=a.Time;
  dta-=t;
  double db=dtb.getDouble();
  if (db<0)db=-db;
  double da=dta.getDouble();
  if (da<0)da=-da;
  if (da<db)(*this)=a;
  else (*this)=b;
  Time=t;
}
void 
TimestampedData::print()
{
  Time.print();
  std::cerr<<" ClassType:"<<(int)getClassType()<< " SubType:"<<(int)getSubType()<<" ID:"<<getID()<< " Packed:"<<(int)isPacked()<<std::endl;
}
bool 
TimestampedData::getMatrix(const std::string & str, ShortMatrix & index,
			   Matrix &mat)
{
  if ((str=="Time")||(str=="time")){  
    mat.reallocate(1);
    mat=Time.getDouble();
  }else if ((str=="ID")||(str=="id")){ 
    mat.reallocate(1);
    mat=getID();
  } else if (((str=="SubType")||(str=="subtype"))||
	     ((str=="m_SubType")||(str=="Subtype"))) { 
      mat.reallocate(1);
      mat=getSubType();
  } else return false;
  return true;
}
bool
TimestampedData::setMatrix(const std::string & str, ShortMatrix & index,
			   Matrix &mat)
{
  if ((str=="Time")||(str=="time")){  
    if ((mat.Rows<1)||(mat.Columns<1))return false;
    setTime(mat(0,0));
  }else if ((str=="ID")||(str=="id")){ 
      if ((mat.Rows<1)||(mat.Columns<1))return false;
      setID((short)mat(0,0));
  } else if (((str=="SubType")||(str=="subtype"))||
	     ((str=="m_SubType")||(str=="Subtype"))) {
    if ((mat.Rows<1)||(mat.Columns<1))return false; 
    mat.reallocate(1);
    setSubType((short)mat(0,0));
  }
  return false;
}

} // namespace Cure

std::ostream& operator<<(std::ostream& os, const Cure::TimestampedData& td)
{
  os << "ClassType:" << (int)td.getClassType() 
     << " SubType:" << (int)td.getSubType()
     << " ID:" << td.getID()
     << " Packed:" << (int)td.isPacked();
  return os;
}
