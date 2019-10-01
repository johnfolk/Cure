// = AUTHOR(S)
//    John Folkesson
//
// = COPYRIGHT
//    Copyright (c) 2007 John Folkesson
//
/*----------------------------------------------------------------------*/

#ifndef CURE_VISIONDATA_hh
#define CURE_VISIONDATA_hh

#include "TimestampedData.hh"
#include "SensorData.hh"
#include "Matrix.hh" 
#include "ShortMatrix.hh" 

#ifndef DEPEND
#include <iostream>  // ostream
#endif

// Forward declaration(s)
class PackedVision;
namespace Cure {
  /**
   * The range of image formats is quite large, (ie unsigned char to
   * double).  For this reason it is not possible to give a specific
   * generic definition of an image.  We choose to define a base class
   * that gives an interface that should allow easy implimentation of
   * subclasses using your favorite vision library's image class in a
   * has a type association to the subclass of CureImage.  So you
   * write a subclass myCureImage that has a member that is a pointer
   * to your favorite type of image structure.  Then you override the
   * simple inline virtual functions in this header.
   * It assumes onlyt that there is some set of numerical arrays stored.
   *
   */
  class CureImage{
  public:
    /**
     * This keeps track of the number of VisionData objects that have
     * pointers to this image.
     */
    int m_PointerCounter;
    /**
     * @return the image height.
     */
    virtual unsigned short getHeight()const{return 0;}
    /**
     * @return the image width.
     */
    virtual unsigned short getWidth()const{return 0;}
    /**
     * @return the number of image.
     */
    virtual unsigned short getNumberOfImages()const{return 0;}
    /**
     * @param h the image height.
     */
    virtual  void setHeight(unsigned short h){return;}
    /**
     * @param w the image width.
     */
    virtual void setWidth(unsigned short w){return;}
    /**
     * @param n the number of image.
     */
    virtual void setNumberOfImages(unsigned short n){return;}
    
    CureImage(const unsigned short width=0, const unsigned short  height=0, 
	      const unsigned short  numofimages=1){
      m_PointerCounter=0;
      setNumberOfImages(numofimages);
      setWidth(width);
      setHeight(height);
    }
    CureImage(const CureImage &src){
      m_PointerCounter=0;
      (*this)=src;
    }
    virtual ~CureImage(){}
    /**
     * Called by VisionData to inccrement the m_PointerCounter
     */
    void increment(){
      m_PointerCounter++;
    }
    /**
     * Called by VisionData to decrement the m_PointerCounter
     * @return true if no pointers left.
     */
    bool decrement(){
      if (m_PointerCounter==0)return true;
      m_PointerCounter--;
      if (m_PointerCounter==0)return true;
      return false;
    }

    /**
     * This converts the pixel numerical value to an unsigned short.
     * @param r the row of th pixel
     * @param c the column of the pixel.
     * @param k the index giving the image arrya to use
     * @return the (r,c) element of the kth image
     */
    virtual unsigned short getUShort(const unsigned short r, 
				     const unsigned short c,
				     const unsigned short k=0) const {
      return 0;
    }
    /**
     * This assigns a pixel to an unsigned short value.
     * param value the value to set the pixel to.
     * @param r the row of th pixel
     * @param c the column of the pixel.
     * @param k the index giving the image arrya to use
     * @return the (r,c) element of the kth image
     */
    virtual  void setUShort(const unsigned short value,
			    const unsigned short r, 
			    const unsigned short c,
			    const unsigned short k=0) {
      return;
    }
    /**
     * Copy operator 
     * 
     *  @param src The object to copy
     */
    virtual CureImage& operator=(const CureImage &src){
      setNumberOfImages(src.getNumberOfImages());
      setWidth(src.getWidth());
      setHeight(src.getHeight());      
      for (unsigned short i=0;i<getNumberOfImages(); i++){
	for (unsigned short j=0; j<getHeight(); j++)
	  for (unsigned short k=0;getWidth(); k++)
	    setUShort(src.getUShort(j,k,i),j,k,i);
      }
      return *this;
    }    
    /**
     * This converts the pixel numerical value to an unsigned char.
     * @param r the row of th pixel
     * @param c the column of the pixel.
     * @param k the index giving the image arrya to use
     * @return the (r,c) element of the kth image
     */
    virtual unsigned char getUChar(const unsigned short r, 
				   const unsigned short c,
				   const unsigned short k=0) const {
      return (unsigned char) getUShort(r,c,k);
    }
    /**
     * This assigns a pixel to an unsigned char value.
     * param value the value to set the pixel to.
     * @param r the row of th pixel
     * @param c the column of the pixel.
     * @param k the index giving the image arrya to use
     * @return the (r,c) element of the kth image
     */
    virtual  void setUChar(const unsigned char value,
			   const unsigned short r, 
			   const unsigned short c,
			   const unsigned short k=0) {
      return setUShort((unsigned short)value,r,c,k);
    }
    
    /**
     * This converts the pixel numerical value to an  char.
     * @param r the row of th pixel
     * @param c the column of the pixel.
     * @param k the index giving the image arrya to use
     * @return the (r,c) element of the kth image
     */
    virtual  char getChar(const unsigned short r, 
			  const unsigned short c,
			  const unsigned short k=0) const {
      return (char) (getUChar(r,c,k)-0x80);
    }
    /**
     * This assigns a pixel to an  char value.
     * param value the value to set the pixel to.
     * @param r the row of th pixel
     * @param c the column of the pixel.
     * @param k the index giving the image arrya to use
     * @return the (r,c) element of the kth image
     */
    virtual  void setChar(const  char value,
		     const unsigned short r, 
		     const unsigned short c,
		     const unsigned short k=0) {
      return setUChar((unsigned char)((short)value+0x80),r,c,k);
    }

    /**
     * This converts the pixel numerical value to an  short.
     * @param r the row of th pixel
     * @param c the column of the pixel.
     * @param k the index giving the image arrya to use
     * @return the (r,c) element of the kth image
     */
    virtual  short getShort(const unsigned short r, 
			    const unsigned short c,
			    const unsigned short k=0) const {
      return (short) (getUShort(r,c,k)-0x8000);
    }
    /**
     * This assigns a pixel to an  short value.
     * param value the value to set the pixel to.
     * @param r the row of th pixel
     * @param c the column of the pixel.
     * @param k the index giving the image arrya to use
     * @return the (r,c) element of the kth image
     */
    virtual  void setShort(const  short value,
		      const unsigned short r, 
		      const unsigned short c,
		      const unsigned short k=0) {
      return setUShort((unsigned short)((long)value+0x8000),r,c,k);
    }
    
    
    /**
     * This converts the pixel numerical value to an  float.
     * @param r the row of th pixel
     * @param c the column of the pixel.
     * @param k the index giving the image arrya to use
     * @return the (r,c) element of the kth image
     */
    virtual  float getFloat(const unsigned short r, 
			    const unsigned short c,
			    const unsigned short k=0) const {
      return (float) (((float)getUShort(r,c,k))/(float)0xFFFF);
  }
  /**
   * This assigns a pixel to an  float value.
   * param value the value to set the pixel to.
   * @param r the row of th pixel
   * @param c the column of the pixel.
   * @param k the index giving the image arrya to use
   * @return the (r,c) element of the kth image
   */
  virtual  void setFloat(const  float value,
			 const unsigned short r, 
			 const unsigned short c,
			 const unsigned short k=0) {
    return setUShort((unsigned short)(value*(float)0xFFFF),r,c,k);
  }
    
    
  /**
   * This converts the pixel numerical value to an  double.
   * @param r the row of th pixel
   * @param c the column of the pixel.
   * @param k the index giving the image arrya to use
   * @return the (r,c) element of the kth image
   */
  virtual  double getDouble(const unsigned short r, 
			    const unsigned short c,
			    const unsigned short k=0) const {
    return (double)getFloat(r,c,k);;
  }
  /**
   * This assigns a pixel to an  double value.
   * param value the value to set the pixel to.
   * @param r the row of th pixel
   * @param c the column of the pixel.
   * @param k the index giving the image arrya to use
   * @return the (r,c) element of the kth image
   */
  virtual  void setDouble(const  double value,
			  const unsigned short r, 
			  const unsigned short c,
			  const unsigned short k=0) {
    return setFloat((float)value,r,c,k);
  }
  void print();
};

/**
 * This class is just being set up and
 * is not yet fully implemented 
 * All this should be changed by someone else please.
 *
 * SubType= ASSS SSSS SSSS NNNN Where N is the number of images (0-15).  
 * And SSS is an index that gives the format (ImageSize)
 * And A the msb is 1 if this is to allocate memory for coping the images
 * if A is 0 this will point so some shared images instead.
 *
 * The way I set this up the user can chose whether this will copy
 * the image or just point to it.  By using the operator = one
 * copies just the pointer not the array it points to.  You have to
 * be careful using this and calling write with it as the pointer to
 * the CureImage will then be copied and the cure image will have
 * its m_PointerCount incremented.  When the Vision data is deleted
 * or the pointer reassigned the m_PointerCounter is decremented and
 * if it reaches zero the CureImage object is deleted.
 * 
 *
 *
 * @author John Folkesson 
 */
class VisionData   : public SensorData
{  
public:
  friend class PackedVisionData;
  /** Image Data */
  CureImage *m_Image;
  /** The Camera type can be used to look up calibration parameters*/
  unsigned short m_CameraType;
  /** The serial number of this frame*/
  unsigned long m_FrameNumber;

public:
  /** 
   * Sets the pointer to the image and deletes the old one if 
   * nothing left pointing to it.
   * @param img the image to point to.
   */
  void setImage(CureImage *img)
  {
    if (img)img->increment();
    if (m_Image)if (m_Image->decrement())delete m_Image;
    m_Image=img;
  }
  
  /**called by consturctors*/
  void init(){
    m_Packed =false;
    m_ClassType=(VISION_TYPE);
    setSubType(0);
    setID(0);
    m_CameraType = 0;
    m_FrameNumber=0;
    m_Image=0;
  }
  /**
   * Copy operator The pointers Image[i] are copied not the arrays
   * they point to. 
   *  @param src The object to copy
   */
  VisionData& operator=(const VisionData &src){
    Time=src.Time;
    setSubType(src.getSubType());
    setID(src.getID());
    SensorID=src.SensorID;
    SensorType=src.SensorType; 
    m_CameraType = src.m_CameraType;
    m_FrameNumber=src.m_FrameNumber;
    setImage(src.m_Image);
    return *this;
  }
  
  /**
   * Create empty VisionData
   * @param id the id 
   */
  VisionData(unsigned short id=0)
    :SensorData(SensorData::SENSORTYPE_CAMERA, id)
  {
    init();
  }
    
  /**
   * Create empty VisionData
   * @param t the time
   * @param id the id 
   */
  VisionData(Timestamp &t, unsigned short id)
    :SensorData(SensorData::SENSORTYPE_CAMERA, id)
  {
    init();
    Time=t;
  }
  /**
   * Create a copy of a VisionData. This copies the m_Image Pointer,
   * not the Object.
   * @param src The object to copy
   */
  VisionData(const VisionData &src)
    :SensorData(SensorData::SENSORTYPE_CAMERA, src.SensorID)
  {
    init();
    (*this) = src;
  }

  /**
   * Destructor
   */
  virtual ~VisionData()
  {
    setImage(0);
  }

  /*
   * This is how we (efficiently) get a VisionData from a TimestampedData.
   */
  VisionData * narrowVisionData(){return this;}
  /**
   * Copy operator
   * @param p The object to copy
   */
  void operator = (TimestampedData& p)
  {
    VisionData *ss=p.narrowVisionData();
    if (ss)operator=(*ss);
    else TimestampedData::operator=(p);
  }
  /**
   * @return the image height.
   */
  virtual unsigned short getHeight()const{
    if (!m_Image)return 0;
    return m_Image->getHeight();
  }
  /**
   * @return the image width.
   */
  virtual unsigned short getWidth()const{
    if (!m_Image)return 0;
    return m_Image->getWidth();
  }
  /**
   * @return the number of image.
   */
  virtual unsigned short getNumberOfImages()const{
    if (!m_Image)return 0;
    return m_Image->getNumberOfImages();
  }
  /**
   * @param h the image height.
   */
  virtual  void setHeight(unsigned short h){
    if (!m_Image)return;
    return m_Image->setHeight(h);
  }
  /**
   * @param w the image width.
   */
  virtual void setWidth(unsigned short w){
    if (!m_Image)return;
    return m_Image->setWidth(w);
  }
  /**
   * @param n the number of image.
   */
  virtual void setNumberOfImages(unsigned short n){
    if (!m_Image)return;
    return m_Image->setNumberOfImages(n);
  }



  /**
   * This converts the pixel numerical value to an unsigned short.
   * @param r the row of th pixel
   * @param c the column of the pixel.
   * @param k the index giving the image arrya to use
   * @return the (r,c) element of the kth image
   */
  virtual unsigned short getUShort(const unsigned short r, 
				   const unsigned short c,
				   const unsigned short k=0) const {
    if (!m_Image)return 0;
    return m_Image->getUShort(r,c,k);
  }
  /**
   * This assigns a pixel to an unsigned short value.
   * param value the value to set the pixel to.
   * @param r the row of th pixel
   * @param c the column of the pixel.
   * @param k the index giving the image arrya to use
   * @return the (r,c) element of the kth image
   */
  virtual  void setUShort(const unsigned short value,
			  const unsigned short r, 
			  const unsigned short c,
			  const unsigned short k=0) {
    if (!m_Image)return;
    return m_Image->setUShort(value,r,c,k);
  }
  /**
   * This converts the pixel numerical value to an unsigned char.
   * @param r the row of th pixel
   * @param c the column of the pixel.
   * @param k the index giving the image arrya to use
   * @return the (r,c) element of the kth image
   */
  virtual unsigned char getUChar(const unsigned short r, 
				 const unsigned short c,
				 const unsigned short k=0) const {
    if (!m_Image)return 0;
    return m_Image->getUChar(r,c,k);
  }
  /**
   * This assigns a pixel to an unsigned char value.
   * param value the value to set the pixel to.
   * @param r the row of th pixel
   * @param c the column of the pixel.
   * @param k the index giving the image arrya to use
   * @return the (r,c) element of the kth image
   */
  virtual  void setUChar(const unsigned char value,
			 const unsigned short r, 
			 const unsigned short c,
			 const unsigned short k=0) {
    if (!m_Image)return;
    return m_Image->setUChar(value,r,c,k);
  }
    
  /**
   * This converts the pixel numerical value to an  char.
   * @param r the row of th pixel
   * @param c the column of the pixel.
   * @param k the index giving the image arrya to use
   * @return the (r,c) element of the kth image
   */
  virtual  char getChar(const unsigned short r, 
			const unsigned short c,
			const unsigned short k=0) const {
    if (!m_Image)return 0;
    return m_Image->getChar(r,c,k);
  }
  /**
   * This assigns a pixel to an  char value.
   * param value the value to set the pixel to.
   * @param r the row of th pixel
   * @param c the column of the pixel.
   * @param k the index giving the image arrya to use
   * @return the (r,c) element of the kth image
   */
  virtual  void setChar(const  char value,
		   const unsigned short r, 
		   const unsigned short c,
		   const unsigned short k=0) {
    if (!m_Image)return;
    return m_Image->setChar(value,r,c,k);
  }

  /**
   * This converts the pixel numerical value to an  short.
   * @param r the row of th pixel
   * @param c the column of the pixel.
   * @param k the index giving the image arrya to use
   * @return the (r,c) element of the kth image
   */
  virtual  short getShort(const unsigned short r, 
			  const unsigned short c,
			  const unsigned short k=0) const {
    if (!m_Image)return 0;
    return m_Image->getShort(r,c,k);
  }
  /**
   * This assigns a pixel to an  short value.
   * param value the value to set the pixel to.
   * @param r the row of th pixel
   * @param c the column of the pixel.
   * @param k the index giving the image arrya to use
   * @return the (r,c) element of the kth image
   */
  virtual  void setShort(const  short value,
		    const unsigned short r, 
		    const unsigned short c,
		    const unsigned short k=0) {
    if (!m_Image)return;
    return m_Image->setShort(value,r,c,k);
  }
    
    
  /**
   * This converts the pixel numerical value to an  float.
   * @param r the row of th pixel
   * @param c the column of the pixel.
   * @param k the index giving the image arrya to use
   * @return the (r,c) element of the kth image
   */
  virtual  float getFloat(const unsigned short r, 
			  const unsigned short c,
			  const unsigned short k=0) const {
    if (!m_Image)return 0;
    return m_Image->getFloat(r,c,k);
  }
  /**
   * This assigns a pixel to an  float value.
   * param value the value to set the pixel to.
   * @param r the row of th pixel
   * @param c the column of the pixel.
   * @param k the index giving the image arrya to use
   * @return the (r,c) element of the kth image
   */
  virtual  void setFloat(const  float value,
			 const unsigned short r, 
			 const unsigned short c,
			 const unsigned short k=0) {
    if (!m_Image)return;
    return m_Image->setFloat(value,r,c,k);
  }
    
    
  /**
   * This converts the pixel numerical value to an  double.
   * @param r the row of th pixel
   * @param c the column of the pixel.
   * @param k the index giving the image arrya to use
   * @return the (r,c) element of the kth image
   */
  virtual  double getDouble(const unsigned short r, 
			    const unsigned short c,
			    const unsigned short k=0) const {
    if (!m_Image)return 0;
    return m_Image->getDouble(r,c,k);
  }
  /**
   * This assigns a pixel to an  double value.
   * param value the value to set the pixel to.
   * @param r the row of th pixel
   * @param c the column of the pixel.
   * @param k the index giving the image arrya to use
   * @return the (r,c) element of the kth image
   */
  virtual  void setDouble(const  double value,
			  const unsigned short r, 
			  const unsigned short c,
			  const unsigned short k=0) {
    if (!m_Image)return;
    return m_Image->setDouble(value,r,c,k);
  }


  /** Display info on the Object*/
  void print(); 
protected:
  
private:

};
}
std::ostream& operator << (std::ostream& os, const Cure::VisionData &cmd);

#endif // VisionData_hh
