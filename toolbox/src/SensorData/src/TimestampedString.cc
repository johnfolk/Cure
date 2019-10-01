//
// = LIBRARY
//
// = FILENAME
//    GenericData.cc
//
// = AUTHOR(S)
//    john Folkesson
//
// = COPYRIGHT
//    Copyright (c) 2007 John Folkesson
//
/*----------------------------------------------------------------------*/

#include "TimestampedString.hh"
#include "PackedTimestampedString.hh"
#include "SmartData.hh"
namespace Cure {

TimestampedString::TimestampedString(unsigned short id)
{
  init();
  setID(id);
}

TimestampedString::TimestampedString(Timestamp &t, unsigned short id)
{
  init();
  Time=t;
  setID(id);
}

/**
 * Create a copy of a TimestampedString
 */
TimestampedString::TimestampedString(const TimestampedString &src)
{
  init();
  (*this) = src;
}

TimestampedString::~TimestampedString()
{
}


void TimestampedString::init()
{
  m_Packed =false;
  m_StringType=0;
  m_ClassType=(STRING_TYPE);
  setSubType(0);
  setID(0);
}
int TimestampedString::compose(TimestampedData &src,
			       std::vector<std::string> messages,
			       std::vector<std::string> delimiters,
			       int significantfigures)
{
  std::ostringstream sstr;  
  char timech[32];
  double d=src.Time.getDouble();
  gcvt(d,16,timech);
  std::vector<std::string>::iterator delim=delimiters.begin();
  if (delim!=delimiters.end()){
    sstr<<(*delim);
    delim++;
  }
  
  
  sstr<<timech;
  for (std::vector<std::string>::iterator it=messages.begin();
       (it!=messages.end());it++){
    Matrix mat;
    std::istringstream strdes(*it);
    std::string s;
    strdes>>s;
    short c=10;
    ShortMatrix ind(1,c);
    short tmp;

    short i=0;
    while(strdes>>tmp){
      if (i==c){
	c+=10;
	ind.grow(1,c);
      }
      ind(0,i)=tmp;
      i++;
    }
    ind.Columns=c;
    if (src.getMatrix(s,ind,mat))
      {
	
	for (int i=0;i<mat.Rows;i++)
	  for (int j=0;j<mat.Columns;j++)
	    {
	      char ch[32];
	      gcvt(mat(i,j),significantfigures,ch);
	      if (delim!=delimiters.end()){
		sstr<<(*delim);
		delim++;
	      }else sstr<<" "; 
	      sstr<<ch;
	    }
      }
  }
  if (delim!=delimiters.end()){
    sstr<<(*delim);
    delim++;
  }
  *this=sstr.str();
  return 0;
}

void TimestampedString::operator=(TimestampedData &src)
{
 
  setTime(src.Time);  
  if (src.getClassType()==STRING_TYPE){
    if (src.isPacked())src.narrowPackedTimestampedString()->unpack(*this);
    else *this=(*src.narrowTimestampedString());
  }else {
    int sigfigures=16;
    std::vector<std::string> vec;
    std::vector<std::string>::iterator it;
    Cure::writeString(src,vec,sigfigures);
    std::ostringstream sstr;
    for (std::vector<std::string>::iterator it=vec.begin();it!=vec.end();it++)
      sstr<<(*it)<<"\n";
    *this=sstr.str();
  }
}
void TimestampedString::print()
  {
    TimestampedData::print();
    std::cerr<<"m_StringType= "<<m_StringType<<" String="<<m_String<<"\n";
  }

} // namespace Cure



std::ostream& 
operator << (std::ostream& os, const Cure::TimestampedString &cmd)
{
  os << "String=" << cmd.getString()<< std::endl;
  return os;
}
