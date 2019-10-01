//    $Id: PackedPose.hh,v 1.8 2008/05/05 16:55:30 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_PACKEDPOSE_HH
#define CURE_PACKEDPOSE_HH

#include "Pose3D.hh"
#include "Pose2D.hh"
#include "PackedData.hh"
namespace Cure{

/**
 * A PackedPose is a compressed Pose3D.  It uses the type to store only the 
 * information needed to unpack to a Pose3D.  It does not have the 
 * full functionality of a Pose3D, so if you need that you must unpack it.
 *
 *
 *
 * Subtype=(I,NoTranslate,NoRotate,2D,P5,P4,P3,P2,P1,P0,v5,v4,v3,v2,v1,v0),
 *
 * I indicates that this pose is Incremental rather than cumulative.
 * That is only important for working with cumulated/incremental 
 * pairs of Pose3D objects and is used for type checking.
 *
 * NoTranslate, NoRotate and 2D have to do with 'Packing' the Pose.
 *
 * Not setting these (ie =0) will pack all of xyz and theta Phi Psi,
 * setting the 2D bit will ignore z,phi and psi when packing (assumes they=0).
 * setting the NoRotate will ignore theta, phi, psi.
 * setting the NoTranslate will ignore xyz.
 * (so for 2D translations, just pack xy with (#011############)
 *
 * Fot the Rest of Type:
 * The coordinates that are present in P will have 1 for Pi 
 * and the velocities that are not present will have 0 for vi.
 * So for xy theta pose and xyztheta Velocity we have:
 *
 * Subtype=(####001011001111)=1+2+4+8+64+128+512=719 
 *
 * @author John Folkesson
 */  
class PackedPose: public Cure::PackedData
{
public:
  /**
   * Version number that is used when packing unpacking data to make
   * sure that the same version of the code is used to pack/unpack the
   * data.
   */
  static int VERSION;

private:

public:
  /**Constructor*/
  PackedPose();
  /**
   * Copy Constructor.
   * @param p the data to copy.
   */
  PackedPose(PackedPose& p);
  /**
   * Copy-pack Constructor.
   * @param p the data to copy and pack.
   */
  PackedPose(Pose3D& p);
  /**
   * Copy Constructor.
   * @param p the data to copy and pack.
   */
  PackedPose(Pose2D& p);

  virtual ~PackedPose();
  /** @return pointer to this*/
  PackedPose * narrowPackedPose(){return this;}

  //*******************SET FUNCTIONS***********************************
  /**
   * Copy operator.
   * @param p the data to copy.
   */
  virtual void operator=(TimestampedData& p);

  /**
   * Copy operator.
   * @param p the data to copy and pack.
   */
  void operator=(const Pose3D& p);
  /**
   * Copy operator.
   * @param p the data to copy and pack.
   */
  void operator=(const Pose2D& p);

  /**
   * Unpack the data.
   * @param p the object to unpack into.
   */
  virtual void unpack(TimestampedData& p){
    Pose3D *p3d=p.narrowPose3D();
    if (p3d) {
      unpack(*p3d);
    } else {
      p.Time=Time;
      //std::cerr << "PackedPose::unpack WARNING: Not a Pose3D!!!\n";
    }
  }
  /**
   * Unpack the data.
   * @param p the object to unpack into.
   */
  void unpack(Pose3D& p);

  /**
   * This function will alter the size of the array Data but will not
   * copy the values from the old to the new memory location, i.e. do
   * not call this function unless you no longer need the data
   * @param t the new subtype which inplictly defines the size.
   */
  virtual void setSubType(unsigned short t);
  
  /**This is easy as this has no shortData*/ 
  virtual int convertShortDataSize(int vers){
    if (vers<=version()){
      setShortDataSize(0);
      return 0;
    }
    return 1;
  }
  
  /**
   * Gets the size of the data array.
   * @return the size of the Data array.
   */
  unsigned long getDataSize()const{return getDim(getSubType());}
  /**
   * Convert a subtype to a size.
   * @param subtype the subtype to convert.
   */
  unsigned short getDim(unsigned short subType)const;
  /**
   * print info to the display.
   */
  virtual void print();

protected:
  virtual void init();
  virtual  TimestampedData * makeUnpack(){
    Pose3D *p=new Pose3D();
    unpack(*p);
    return p;
  }

};

} // namespace Cure

#endif // CURE_PACKEDPOSE_HH
