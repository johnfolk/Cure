//    $Id: DataDescriptor.hh,v 1.12 2009/03/23 14:04:25 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2005 John Folkesson
//    

#ifndef CURE_DATADESCRIPTOR_HH
#define CURE_DATADESCRIPTOR_HH

#include <iostream>
#include <stdio.h>
#include <string>
#include <cstdlib>
#include <string.h>
#include "CureDefines.hh"


namespace Cure{


/**
 * This describes a type of TimeStampedData So that inputs and outputs
 * can be labled.  Thus the Cure objects can find the descriptors
 * defined in the ccf files and then find the data they need automatically.
 *
 * So one can have a lines in the ccf (no blanks lines for the real file:
 *
 * DATADESCRIPTORS
 *
 * RawOdometry Pose3D 1 
 * 
 * RawSICK  SICKSCAN  1 1 1 
 * 
 * ...
 *
 *  Then after that the configuration can refer to the string RawOdometry 
 *  and say to input it to some Filter ...
 * 
 *    
 * @see CureDefines.hh  for ClassType codes
 *
 * @author John Folkesson
 */
class DataDescriptor
{
public:
  /** A name for this type of data*/
  std::string Name;
  /**
   * The m_ClassType of the described TimestampedData Objects
   */
  unsigned char  ClassType;
  /**
   * The ID of the described TimestampedData Objects
   */
  unsigned short ID;
  /**
   * If this is SensorData then  the Sensor ID else 0
   */
  unsigned short SensorType;
  /**
   * The SensorID of the described SensorData Objects
   */
  unsigned short SensorID;
  /**
   * The constructor.
   */
  DataDescriptor(std::string n="");
  /**
   *  The constructor with initial values.
   * @param t the object is copied 
   */
  DataDescriptor(const DataDescriptor& t){
    (*this)=t;
  }
  ~DataDescriptor(){}
  /**
   *  The copy operator
   * @param t is copied from here.
   */
  void operator = (const DataDescriptor& t){
    Name=t.Name;
    ClassType=t.ClassType;
    ID=t.ID;
    SensorType=t.SensorType;
    SensorID=t.SensorID;
  }
  /**
   *  The copy operator
   * @param line the ccf file line for this data
   */
  void operator = (const std::string &line);
  /**
   * Test equality (Names not considered).
   * @param t the object to compare. 
   * @return 1 if the objects are the same else 0.
   */
    bool  operator == (const DataDescriptor& t)const {
    if ((t.ClassType!=ClassType) || (t.ID!=ID))
      return false;
    if ((t.SensorType!=SensorType) || (t.SensorID!=SensorID))
      return false;
    return true;
  }
  /**
   * Test not equality (Names not considered).
   * @param t the descriptor to compare. 
   * @return 1 if the objects are not the same else 0.
   */
  bool operator != (const DataDescriptor& t)const {
    if ((t.ClassType!=ClassType) || (t.ID!=ID))
      return true;
    if ((t.SensorType!=SensorType) || (t.SensorID!=SensorID))
      return true;
    return false;
  }


  /**
   * Test equality (Names only considered).
   * @param t the object to compare. 
   * @return 1 if the objects are the same else 0.
   */
  bool sameName(const DataDescriptor& t)const {
    if (Name==t.Name)return true;
    return false;
  }
  /**
   * Conditional equality test if ID is 0 returns true 
   * else does ==.
   * @param t the object to compare. 
   * @return 1 if the objects are the same else 0.
   */
  bool matchWild(const DataDescriptor& t)const {
    if (ID!=0) return ((*this)==t);
    return true;
  }
  /**
   * Test equality (ClassType only considered).
   * @param t the object to compare. 
   * @return 1 if the objects are the same else 0.
   */
  bool sameClass(const DataDescriptor& t)const {
    if (ClassType==t.ClassType)return true;
    return false;
  }
  /**
   * Conditional equality test, if ID is 0 returns sameClass(t)
   *  else does ==.
   * @param t the object to compare. 
   * @return 1 if the objects are the same else 0.
   */
  bool matchClass(const DataDescriptor& t)const {
    if (ID!=0)return ((*this)==t);
    return sameClass(t);
  }
  /**
   * Test equality (ClassType and SensorType only considered).
   * @param t the object to compare. 
   * @return 1 if the objects are the same else 0.
   */
  bool sameClassAndSensor(const DataDescriptor& t)const {
    if (ClassType==t.ClassType)
      if (SensorType==t.SensorType)
	return true;
    return false;
  }
  /**
   * Conditional equality test, if ID is >0 returns ==
   *  else returns sameClassAndSensor(t)
   * @param t the object to compare. 
   * @return 1 if the objects are the same else 0.
   */
  bool matchClassAndSensor(const DataDescriptor& t)const {
    if (ID!=0)return ((*this)==t);
    return sameClassAndSensor(t);
  }
  /*
   * @return 0 if ok 1 if fail -1 if sensor name is in sensor and Class/ID 
   * was found.
   */
  int  setTo(const std::string &line, std::string &sensor);
  /**
   * Print to display.
   */
  void print()const
  {
    std::cerr<<"DataDescriptor: "<< Name << "." << (int)ClassType<<" "<<ID<<" "<<SensorType<<" "<<SensorID<<"\n";;
  }
};

inline std::ostream& operator<< (std::ostream& os, const DataDescriptor &t)
{
  os << t.Name << "." << (int)t.ClassType<<" "<<t.ID<<" "<<t.SensorType<<" "<<t.SensorID<<"\n";
  return os;
}

} // namespace Cure

#endif 
