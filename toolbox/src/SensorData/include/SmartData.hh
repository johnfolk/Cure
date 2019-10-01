//    $Id: SmartData.hh,v 1.5 2008/05/01 13:25:33 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_SMARTDATA_HH
#define CURE_SMARTDATA_HH

#include "TimestampedData.hh"
#include "PackedPose.hh"
#include "PackedSICK.hh"
#include "Odometry.hh"
#include "PackedMeasurementSet.hh"
#include "PackedSensorPose.hh"
#include "PackedCommand.hh"
#include "PackedGeneric.hh"
#include "PackedSonar.hh"
#include "PackedGrid.hh"
#include "PackedState.hh"
#include <sstream>  
#include <vector>  

namespace Cure{

  /**
   * Make a specified type of TimestampdedData Object.
   *
   * This function can create a TimestamedData subclass based on the
   * classType.  This is what must be changed to have smarts to your
   * new TimestampData class.
   *
   * @param tpointertpointer the correct type of object will be returned here.
   * @param classType the ClassType of the object you want 
   *               (ie POSE3D_TYPE ect).
   * @param subType the subtype to give the new object.
   * @param packed wether to make a packed version of the object or normal. 
   * @return 0 if ok elase 1.
   */
  unsigned short makeTimestampedData(TimestampedData **tpointerpointer,
				     unsigned char classType, 
				     unsigned short subType=0,
				     bool packed=false);
  /**
   * The is able to make and hold TimestampedData objects of any type.
   * One can set this equal to any such object and it will correctly
   * copy all data to the internal object.  It can then set any other 
   * object equal to the internal object to the extent the Classes match.
   *
   *
   * This class transforms its TPointer into the different subclasses
   * of TimestampeData.  It will delete the old object if it needs to
   * become a new ClassType.
   *
   * So you can use this object to read from an Address that you don't know 
   * the class.
   *
   * @author John Folkesson
   */
  class SmartData:public TimestampedData 
{
protected:
  /**
   * Pointer to an object that this object wraps.
   */
  TimestampedData * TPointer;

public:
  /**Constructor of an empty Object*/
  SmartData() 
  {
    init();  
    TPointer=0;
    setup(TIMESTAMP_TYPE);
  }
  /**
   * This will make a copy of templet and then store the copy in
   * this object.  If templet is a SmartData then it will  
   * copy the object held by templet not templet itself.
   * @param templet Object to be copied into our TPointer
   */
  SmartData(TimestampedData &templet) 
  {
    init();  
    TPointer=0;
    *this=templet;
  }
  /**
   * An object of the specified class will be created and stored in
   * the new SmartData object. 
   *
   *  @param classType the ClassType of the object you want (ie
   *  POSE3D_TYPE ect).
   *
   *  @param subType the subtype to give the new object.
   *
   *  @param packed wether to make a packed version of the object or
   *       normal.
   *
   */
  SmartData(unsigned char classType, 
                unsigned short subType = 0, 
                bool packed = false)
  {
    init();
    TPointer = 0;
    setup(classType, subType, packed);
  }
  /**Called by Constructors*/
  void init()
  {
    m_Packed = false;
    m_ClassType=SMARTDATA_TYPE;
    m_SubType = 0;
    ID = 0;
  }
  /**Will Delete the held object.*/
  ~SmartData(){
    if (TPointer) delete TPointer;
    TPointer=0;
  }
  /**
   * Gets a pointer to the internal object.
   * @return a pointer to the internal object.
   */
  TimestampedData * getTPointer(){return TPointer;}
  /**
   * Gets a pointer to the internal Packed object.
   * @return a pointer to the internal object if it is Packed else 0.
   */
  PackedData * getPackedData(){return TPointer->narrowPackedData();}
  /**fast cast*/
  SmartData * narrowSmartData(){return this;}
  /**
   * This both sets its own Timestamp to the internalobjects Time and
   * returns it.  Normally the two timestamps will be the same.
   * @return the Timestamp of the Internal object.
   **/
  Timestamp getTime(){
    Time=TPointer->Time;
    return Time;
  }
  /**
   * This both sets its own Timestamp and the internal objects Time to t.
   * @paramt  the new Timestamp for the Internal object.
   **/
  void  setTime(const Timestamp &t){
    TPointer->setTime(t);
    Time=t;
  }
  /**
   * Query the subtype.
   * @return the subType of the internal object.
   */
  unsigned short getTPointSubType()const {return TPointer->getSubType();}
  /**
   * Query the classtype.
   * @return the ClassType of the internal object.
   */
  unsigned short getTPointClassType()const {return TPointer->getClassType();}
 
  /**
   * This both sets its own Timestamp to the internalobjects Time and
   * returns it.  Normally the two timestamps will be the same.
   * @return the Time of the Internal object.
   **/
  double getDoubleTime(){
    Time=TPointer->Time;
    return Time.getDouble();
  }
  /**
   * This both sets its own Timestamp to the internalobjects Time and
   * returns it.  Normally the two timestamps will be the same.
   * @param the Timestamp of the Internal object is returned here.
   **/
  void getTime(Timestamp &t){t=getTime();}
  /*
   * Changes Internal object into a new object of the same type and value 
   * as t is holding.
   * @param t the SmartData holding the object to copy.
   */
  void operator=(SmartData & t)
  {
    Time=t.Time;
    setSubType(t.getSubType());
    setID(t.getID());
    operator=(*(t.TPointer));
  }  
  
  /*
   * Changes internal object into a new object of the same type and value 
   * as t.
   *
   * @param t the object to create a copy of and store as the new
   * internal object
   */
  void operator=(TimestampedData & t)
  {
    SmartData *s=t.narrowSmartData();
    if (s) {
      setup(s->TPointer->getClassType(), s->TPointer->getSubType(), 
	    s->TPointer->isPacked()); 
      *this=*s->TPointer;
  } else {
    setup(t.getClassType(), t.getSubType(), t.isPacked());
    *TPointer=t;
  }
    Time=TPointer->Time;
  }  

  virtual  void getDescriptor(Cure::DataDescriptor & d) const{
    TPointer->getDescriptor(d);
  }
  virtual  int  setDescriptor(const Cure::DataDescriptor & d){
    return TPointer->setDescriptor(d);
  }

  /** 
   * The get function will get a pointer to an object of the ClassType
   * if it can.  It will unpack it if it is packed.  The pointer will either
   * be to the same as this or to TPointed.  So its like a 
   * Smart narrow method.
   * 
   *
   * @param classType the class to match (packed classTypes=unpacked
   *                  ones so there are two possible return types.
   * @return a pointer to an object with the class type or 0 if not 
   *        possible.  
   */
  virtual TimestampedData *getClassPointer(unsigned short classType){
    if (m_ClassType==classType)return this;
    TimestampedData  *td=TPointer->getClassPointer(classType);
    if (!td)return 0;
    unpack();
    return TPointer->getClassPointer(classType);
  }
  /** 
   * The set function will set the pointer to an object of the ClassType
   * if it can.  It will unpack it if it is packed.  The pointer will either
   * be to the same as this or to TPointed. 
   * 
   *
   * @param classType the class to match (packed classTypes=unpacked
   *                  ones so there are two possible return types.
   * @return a pointer to an object with the class type or 0 if not 
   *        possible.  
   */
  virtual TimestampedData *setClassPointer(unsigned short classType){
    if (m_ClassType==classType)return this;
    if ((TPointer->getClassType()!=classType)||(TPointer->isPacked()))
      if (setup(classType))return 0;
    return TPointer;
  }

  /**
   * Sets p equal to the internal object this points to,
   * packing/unpacking as needed.  So if p is not packed and the internal
   * object is the right type but packed then it is unpacked into p.
   * 
   * @param  p the object to copy the internal object into.
   */
  void setEqual(TimestampedData &p)
  {  
    if (p.isPacked() | (!TPointer->isPacked())) {
      if (TPointer->narrowSmartData()) {
	std::cerr << "SmartData pointing to SmartData in setEqual\n";
      }
      p = (*TPointer);
    } else {
      unpack(p);
    }
  }
  /**
   * Equality test.
   * @param t the object to compare to the internal object.
   * @return 1 if interna object == t else 0
   */
  int operator==(TimestampedData * t)
  {
    if (TPointer==t) return 1;
    return 0;
  }  

   /*
   * Changes TPointer into a new packed object of the cooresponding to
   * the type and value it has.
   * @return 0 if ok else 1 (Class can't pack).
   */
  unsigned short pack(){
    if (TPointer->isPacked()) return 0;    
    TimestampedData *old = TPointer;
    TPointer = 0;
    if (setup(old->getClassType(), old->getSubType(), true))
      {
	delete TPointer;
	TPointer = old;
	return 1;
      }
    (*TPointer) = (*old);
    delete old;
    return 0;
  }


  
   /*
   * Changes TPointer into a new unpacked object of the cooresponding to
   * the ClassType and values it has.
   * 
   */
  void unpack(){
    if (!TPointer->isPacked())return;
    TimestampedData *old = TPointer;
    TPointer=0;
    setup(old->getClassType(), old->getSubType(), false);
    old->narrowPackedData()->unpack(*TPointer);  
    delete old;
  }
  
  

  /**
   * Set td to an unpacked copy of TPointer;
   * @param t the object to unpack into.
   * 
   */
  void  unpack(TimestampedData & t){
    Time=TPointer->Time;
    PackedData *pkd = TPointer->narrowPackedData();  
    if (pkd) {
      SmartData *sd = t.narrowSmartData();
      if (sd) {        
        sd->setup(TPointer->getClassType());
        pkd->unpack(*sd->getTPointer());
        sd->Time = sd->getTPointer()->Time;
      } else {
        pkd->unpack(t);
      }
    } else {
      t = (*TPointer);
    }
  }



  /**
   * This sets the internal object as specified.
   * @param classType the ClassType of the object you want 
   *               (ie POSE3D_TYPE ect).
   * @param subType the subtype to give the new object.
   * @param pckd wether to make a packed version of the object or normal. 
   *   
   * @return 0 if ok else 1.
   * @see Cure::makeTimestampedData
   */
  unsigned short setup(unsigned char classType, 
                       unsigned short subType = 0,
		       bool pckd = false){
     return makeTimestampedData(&TPointer,classType,subType,pckd);
  }
   /*
    * Changes internal object into a new unpacked object cooresponding to
    * the type and value of t.
    * @param t the object to copy into the internal object
   */
  void setUnpacked(TimestampedData & t){
    *this=t;
    Time=TPointer->Time;
    unpack();
  }


   /*
   * Changes internal object into a new packed object cooresponding to
   * the type and value of t.
   * @param t the object to copy into the internal object
   */
  bool setPacked(TimestampedData & t){
    SmartData *s=t.narrowSmartData();
    if (s) {
      *this=*s->TPointer;
      if (pack()) {
	std::cerr << "SmartData::setPacked, failed to pack!\n";
	return false;
      }
    } else {
      if (setup(t.getClassType(), t.getSubType(), true))return false;
      *TPointer=t;
      Time=TPointer->Time;
    }
    return true;
  }

  /**
   * This interpolate works even for PackedData.  It will
   * 'change' into a'a classType if needed. 
   *
   * @param a one object
   * @param b another object
   * @param t the interpolation time
   */
  void interpolate_(TimestampedData& a, TimestampedData& b, 
		    Timestamp t);
  /**
   * This interpolate works even for PackedData.  It will
   * 'change' into a'a classType if needed. 
   *
   * @param a one object
   * @param b another object
   * @param t the interpolation time
   */
  void interpolate_(SmartData& a, SmartData& b, 
		    Timestamp t);
  /**Display info*/
  virtual void print(){
    TimestampedData::print();
    TPointer->print();
  }
  /**
   * This returns data based on the string value and the indcies
   * given in the ShortMatrix.  It just passes the call 
   * to its TPointer object.
   *
   * @param str So Whatever the elements of the set understandd
   * are recongnized strings.
   * 
   * @param index For most classes you set the index as when you would
   * call Cure::Matrix.offset(roff,coff,r,c) with the integers
   * roff,coff,r,c placed in a the top row of index (1x4).  
   * 
   * @param mat the result is returned here in a r x c matrix
   * @return true if ok else false. 
   */
  virtual bool getMatrix(const std::string & str, ShortMatrix & index,
			 Matrix &mat){
    return  getTPointer()->getMatrix(str,index,mat);
  }
  /**
   * This sets data values  based on the string value and the indcies
   * given in the ShortMatrix.  It just passes the call 
   * to its TPointer object.
   *
   * @param str So Whatever the elements of the set understandd
   * are recongnized strings.
   * 
   * @param index For most classes you set the index as when you would
   * call Cure::Matrix.offset(roff,coff,r,c) with the integers
   * roff,coff,r,c placed in a the top row of index (1x4).  
   * 
   * @param mat the value is  here in a r x c matrix
   * @return true if ok else false. 
   */
  virtual bool setMatrix(const std::string & str, ShortMatrix & index,
			 Matrix &mat){
    return  getTPointer()->setMatrix(str,index,mat);
  }

  /*
   *  Converts an ASCII array char of length len into this SmartData
   *  object tPoint.
   * @return 0 if ok else RESOURCE_ERROR;
   */
  unsigned short  readChar(char *chr,int len);
  /**
   * Codes the TimestampedData, p, into ASCII and allocates an array
   * to hold it which is returned in str.  Note you must later 
   * call free(*str) to not leak memory.  The arrays include  
   * line feeds for better print appearance followed by null char for 
   * converting to string tokens for each 'line'.  A line is
   * one simple TimestampData or for DataSet it is one line for
   * the DataSet followed by lines for each set element. 
   * The final char in the array is '\r'=13 to signal the packet end
   *
   * Standard header for all timestamped data is: 
   * StandardHeader = "ID m_SubType Time.Seconds Time.Microsec Version "
   *
   * For most classes this is followed by: "getShortDataSize() getDataSize()". 
   *
   * For DataSets this is changed to:: StandardHeader "SetSize".  This
   * is then followed by the strings for each of the set members.
   *  
   * TimestampedStrings have: StandardHeader "getStringType()" followed
   * by the String.
   *
   * SmartData are not written at all, instead the data that they point to is.
   *
   *
   * @param p the data to convert
   * @param str tring returned here
   * @param headlength the string will get this many null char before
   *        the data starts.
   * @param  significantFigures For doubles: how precise should they be.
   * @return the length of the array in bytes.
   */
  static int writeChar(TimestampedData & p, 
		       char **str,int headlength=0,
		       int significantFigures=16);
    //  static int writeChar(TimestampedData & p, char **str,int headlength=0){
    //writeChar(p, char **str,headlength,16)
    //}

  };



  /**
   *  Used by writeChar to handle recursive calls for DataSet type.
   *
   */
  void writeString(Cure::TimestampedData & p, 
		   std::vector<std::string> &vec, 
		   int sigFigures);

  /**
   *  Used by readChar to handle recursive calls for DataSet type.
   *
   */
  unsigned short readString(Cure::SmartData &tPoint,std::string &str);
  


  
} // namespace Cure

#endif // CURE_SMARTTPOINTER_HH

