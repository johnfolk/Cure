// = AUTHOR(S)
//    John Folkesson
//
// = COPYRIGHT
//    Copyright (c) 2007 John Folkesson
//
/*----------------------------------------------------------------------*/

#ifndef CURE_TIMESTAMPEDSTRING_hh
#define CURE_TIMESTAMPEDSTRING_hh

#include "TimestampedData.hh"
#include "Matrix.hh" 

#ifndef DEPEND
#include <iostream>  // ostream
#endif
#include <vector>  

namespace Cure {

class PackedTimestampedString;
/**
 * This class is used to send generic string data over a cure address
 *
 *
 *
 *
 * @author John Folkesson 
 */
class TimestampedString :  public TimestampedData 
{  
public:

public:
  
  friend class PackedTimestampedString;
  /**
   * Create empty TimestampedString
   * @param id the id of this
   */
  TimestampedString(unsigned short id = 0);
  /**
   * Create empty TimestampedString
   * @param t the time
   * @param id the id of this
   */
  TimestampedString(Timestamp &t,unsigned short id = 0);

  /**
   * Create a copy of a TimestampedString
   * @param s The object to copy
   */
  TimestampedString(const TimestampedString &s);
  /**called by constructors*/
  void init();
  /**
   * Destructor
   */
  virtual ~TimestampedString();
  /*
   * This is how we (efficiently) get a TimestampedString from a TimestampedData.
   */
  TimestampedString * narrowTimestampedString(){return this;}
  /**
   * Copy operator
   * @param src The object to copy
   */
  TimestampedString& operator=(const TimestampedString &src)
  {
    Time=src.Time;
    setID(src.getID());
    m_String = src.m_String;
    m_StringType=src.m_StringType;
    setSubType(m_String.length());
    return (*this);
  }
  /**
   * Store a string in this.
   * @param tree The object to copy
   */
  void operator=(std::string &str){
    m_String=str;
    setSubType(m_String.length());
  } 
  /**
   * @param tree the PoseTree stored in this is returned here
   * @return 1 if fails esle 0
   */
  int getString(std::string &str){
    str=m_String;
    return 0;
  }
  
  /**
   * Copy operator
   * @param p The object to copy
   */
  void operator = (TimestampedData& p);
  TimestampedString& operator=(const std::string &str)
  {
    m_String = str;
    setSubType(m_String.length());
    return (*this);
  }
  /** This sets the subtype to the length of the string.*/
  void setSubType(unsigned short){
    m_SubType=m_String.length();
  }
  /** This sets the subtype to the length of the string.*/
  void setStringType(unsigned short t){
    m_StringType=t;
  }
  /** This sets the subtype to the length of the string.*/
  void setString(std::string &str){
    *this=str;
  }

  /** This sets the subtype to the length of the string.*/
  unsigned short getStringType(){
    return m_StringType;
  }
  /** This sets the subtype to the length of the string.*/
  const std::string getString() const{
    return m_String.c_str();
  }

  /**
   * This method codes a timestame data into a string according to an
   * arbitrary formate specified by a pair of string vectors.  the first
   * vector says what data to encode and the second tells what string to
   * insert between each number.  All strings start with the time
   * converted to a double.  The strin is stored in this object's
   * m_String but no other change to this object is made (ie m_ID,
   * m_StringType and Time are unchanged.
   *
   *
   * @param src the timestamped data to code into the string
   *
   * @param mesages these are format strings used to get Matrix form
   * the timestamped data object.  They say which data from src will
   * be in the string and in what order.  These strings all start with
   * a single word followd by a space then a series of short int.  The
   * word gives the data such as 'Covariance' then the the index might
   * be 0 0 2 2 which says offset the cov matix by 0 rows and 0
   * columns then return the 2x2 submatix matrix from that offset.
   * Each Data type has its own set of messages @see getMatrix(...).
   *
   *
   * @param delimiters The first element will start the string, followed
   * by the timestamp, then the next element of delimiters, and so on
   * until there are no more numbers to write, then the next deltimiter is
   * used if there is one.  If the delimters run out spaces are used
   * except for the start and end which are then empty strings.
   *
   * @param significantfigures the number of significant figures for
   * the doubles.
   * 
   * @return 0 if ok
   */
  int compose(TimestampedData &src,
	      std::vector<std::string> messages,
	      std::vector<std::string> delimiters,
	      int significantfigures=16);
  /** coming soon*/
  int parse(TimestampedData &src,
	    std::vector<std::string> messages,
	    std::vector<std::string> delimiters,
	    int significantfigures=16);
  
  /**
   * Version number that can be used reading from file to
   * make sure that the reader of the data has the same version as the
   * writer.
   */
  virtual int version(){return 1;}

  /** 
   *Return the indexed char in the string.  Be sure not to exceed the
   *string length=m_SubType.
   */
  char operator() (int index){
    return m_String[index];
  }
  void get(std::string &str){
    str=m_String;
  }
  /** Display info on the Object*/
  void print(); 
public:
  
protected:
  /**
   * This is the string
   */
  std::string m_String;
  /** This is a free member to store any classification of the string*/
  unsigned long m_StringType;
  
private:
};
}

std::ostream& operator << (std::ostream& os, const Cure::TimestampedString &cmd);

#endif // TimestampedString_hh
