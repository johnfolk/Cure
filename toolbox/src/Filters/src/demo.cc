#include <iostream>
#include <stdlib.h> 
#include <iomanip>

#include "CureDebug.hh"

#include "DataSlotAddress.hh"
#include "FileAddress.hh" 
#include "DataFilterAddress.hh"
#include "Syncronizer.hh"

#include "SimpleOdoModel.hh"
#include "GyroModel.hh"
#include "GyroAdjOdometry.hh"

using namespace std;
using namespace Cure;

int main(int argc, char * argv[])
{
  cure_debug_level = 30;

  const char *optstring = "hO:I:o:d:";
  char *args = "[-h help] [-O odometry] [-I intertial] [-o outputfile] "
    "[-d debug level]";
  char *odofilename = "notset";
  char *dmufilename = "notset";
  char *outputfilename = "notset";
  char o = getopt(argc, argv, optstring);
  while (o != -1) {
    switch (o) {
    case 'd':
      cure_debug_level = atoi(optarg);
      break;
    case 'O':
      odofilename = optarg;
      break;
    case 'I':
      dmufilename = optarg;
      break;
    case 'o':
      outputfilename = optarg;
      break;
    case 'h':
    case '?':
      std::cerr << "Usage: " << argv[0] << " " << args << std::endl;
      return -1;
      break;
    }
    o = getopt(argc, argv, optstring);
  }

  if (strcmp(odofilename,"notset")==0 || strcmp(dmufilename,"notset")==0) {
    fprintf(stderr, "Too few input args\n");
    std::cerr << "Usage: " << argv[0] << " " << args << std::endl;
    fprintf(stderr, 
            "Ex: ./demo -O testodo.dat -I testdmu.dat -o outputfile.dat\n");
    return -1;
  }

   /**
   * The FileAddress is a port to a file.  
   * Reading and writing pose data is now standardized.
   */
  FileAddress odofile, inertialfile;
  odofile.openReadFile(odofilename);
  inertialfile.openReadFile(dmufilename);

  /**
   * The output file address will be receiving subscription (push) data
   * 
   */
  FileAddress outputfile;
  if (strcmp(outputfilename,"notset")!=0) 
    outputfile.openWriteFile(outputfilename);
  Pose3D podo,pin;

  /**
   * One need not set type for pin (here a Pose3D object) but now I
   * can demonstrate using this type int the constructor below. The
   * type tells what pose information (xy,z,theta,psi, phi, velocities
   * and uncertainies) it contains. (0x4000) means that gyros give
   * only 3D rotation information:
   */
  pin.setSubType(0x4000);

  /**
   * The slots can be made by creating an address to them.  
   * They store a history of data (in this case 100 Poses).
   * The odd number (0x100B) is the subtype (2D with velocity).
   * The Type can specify exactly how to interprete the pose.  That means 
   * there are lots of (10,000's) kinds of pose data one can store.  
   * (ie. just theta or Full 6 dof with covariance and velocity...).
   * These are 'understood' and interpreted properly when adding
   * ect. For example if you add two poses that only deal with xy there is
   * no need to add the z coordinates, the angle or deal with uncertainties.
   * The final true is the compressed flag.  
   * The data is stored in compressed form.
   */
  DataSlotAddress cumulative_odo(100, POSE3D_TYPE, 0x100B, true, false); 
  DataSlotAddress inertial( 100, POSE3D_TYPE, pin.getSubType(), true, false); 
  DataSlotAddress incremental_dead( 100, POSE3D_TYPE, 704, true,false); 
  DataSlotAddress cumulative_dead(100, POSE3D_TYPE, 704, true,false); 

  /* 
   * Lets get some data in the slots to start things off.
   */
  odofile.read(podo);
  CureDO(40) {
    CureCERR(0) << "First odome read: "; // I use 0 here to let the CureDO rule
    podo.print();
  }
  cumulative_odo.write(podo);
  inertialfile.read(pin);
  inertial.write(pin);
 
  /** 
   * Lets set up some cumputation with this data.  We will fuse 
   * the gyro and odometer data.  We model the errors in the
   * odometer and inertial data using simpleModel and GyroModel.
   */
  SimpleOdoModel skid;  
  GyroModel gyro;

  /**
   * One of the parameters of the model is ThetaBias.
   */
  gyro.ThetaBias=.0007;
  GyroAdjOdometry fuser;  

   /** 
    * The Trigger is to regulate the flow of data to the fuser.  We 
    * can't fuse until there is new data on both ports.  The slots will
    * interpolate the poses to the earliest of the two timestamps.
    * We tell the constructor that we need to sync 2 input ports.
    */
  Syncronizer sync(2);

  /**
   * This hook up is the tricky part.  The data can be both pushed and
   * pulled through the filters.  The trigger for the sync will be new
   * odometer data.  This data will be pushed to the sync when we
   * write to the cumulative_odo. We want to push to the first input
   * port, in this case the only input port
   */
  cumulative_odo.push(sync.in(0)); 

  /**
   * Here is another way to connect to a filter port. 
   */
  sync.in(1)->pull(&inertial);


  /**
   * In cases where there is only one input/output you do not need to
   * supply an argument to the in() and out() functions, it will
   * default to 0
   */
  sync.out(0)->push(skid.in());
  skid.out(0)->push(fuser.in());

   /**
   *  The gyro will need to know where to pull its data from:
   */ 
  gyro.in()->pull(sync.out(1));

  fuser.in(1)->pull(skid.out(1)); 
  fuser.in(2)->pull(gyro.out(0)); 
  fuser.in(3)->pull(gyro.out(1)); 
  /**
   * Out(0) gives us pitch and roll info which we ignore now for this demo
   */
  fuser.out(1)->push(&incremental_dead);
  fuser.out(2)->push(&cumulative_dead); 
  cumulative_dead.push(&outputfile);
 
  while (!odofile.read(podo)) {
    CureCERR(10) << "Read podo time "
                 << podo.getTime() << std::endl;
    cumulative_odo.write(podo);
    while(pin.Time<podo.Time) {
      if(inertialfile.read(pin)) {
        cerr<<"done\n";
        return 0;
      } else {
        cerr << ".";
      }
      inertial.write(pin);
    }
    CureCERR(40) << "   Time pin [s]: " << pin.getTime()
                 << std::endl;
  }
   
  return 0;
  
}








