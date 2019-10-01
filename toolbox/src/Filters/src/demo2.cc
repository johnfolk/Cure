#include <iostream>
#include <stdlib.h> 
#include <iomanip>

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
  const char *optstring = "hO:I:i:c:";
  char *args = "[-h help] [-O odometry] [-I intertial] [-i outputincfile][-c outputcumfile]";
  char *odofilename = "notset";
  char *dmufilename = "notset";
  char *outputfilename = "notset";
  char *outputfilename2 = "notset";
  char o = getopt(argc, argv, optstring);
  while (o != -1) {
    switch (o) {
    case 'O':
      odofilename = optarg;
      break;
    case 'I':
      dmufilename = optarg;
      break;
    case 'i':
      outputfilename = optarg;
    case 'c':
      outputfilename2 = optarg;
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
            "Ex: ./demo2 -O testodo.tdf -I testdmu.tdf -i outputfileinc.tdf -c outputfilecum.tdf\n");
    return -1;
  }

   /**
   * The FileAddress is a port to a file.  
   * Saving and reading pose data is now standardized.
   */
  FileAddress odofile, inertialfile;
  odofile.openReadFile(odofilename);
  inertialfile.openReadFile(dmufilename);
  /**
   * The output file address will be receiving subscription (push) data
   */
  FileAddress outputfile;
  FileAddress outputfile2;
  if (strcmp(outputfilename,"notset")!=0) outputfile.openWriteFile(outputfilename);
  outputfile2.openWriteFile(outputfilename2);
  /**
   * The slots can be made by creating an address to them.  
   * They store a history of pose data (in this case 100 Poses).
   * The odd numbers 4107 ect. are the Pose3D::Type.  That means there 
   * are lots of (10,000's) kinds of pose data one can store.  
   * (ie. just theta or Full 6 dof with covariance and velocity...).  
   * These are 'understood' and interpreted properly when adding ect. 
   * The final 1 is the compressed flag.  
   * The data is stored in compressed form if this is true.
   * the final true is the do type checking of data
   */
  DataSlotAddress cumulative_odo(100, POSE3D_TYPE, 4107, 1, true ); 
  DataSlotAddress inertial(100, POSE3D_TYPE, 16384, 1,true); 
  DataSlotAddress incremental_dead(100, POSE3D_TYPE, 704, 1,true); 
  DataSlotAddress cumulative_dead(100, POSE3D_TYPE, 704, 1,true); 
  /* 
   * Lets get some data in the slots to start things off.
   */
  Pose3D podo,pin;
  odofile.read(podo);
  cumulative_odo.write(podo);
  inertialfile.read(pin);
  inertial.write(pin);
 
  /** 
   * Lets set up some cumputation with this data.  We will fuse 
   * the gyro and odometer data.  We model the errors in the
   * odometer and inertial data using SkidSteerModel and GyroModel.
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
   * This hook up is the tricky part.  The data can be both pushed 
   * and pulled through the filters.
   * The trigger for the sync will be new odometer data.  
   * This data will be pushed to the sync when  we write to 
   * the cumulative_odo. 
   *
   */
  cumulative_odo.push(sync.in(0)); 
 
  sync.in(1)->pull(&inertial);   

  sync.out(0)->push(skid.in());
  skid.out(0)->push(fuser.in());

   /**
   *  The gyro will need to know where to pull its data from:
   */ 
  gyro.in()->pull(sync.out(1));

  fuser.in(1)->pull(skid.out(1)); 
  fuser.in(2)->pull(gyro.out(0)); 
  fuser.in(3)->pull(gyro.out(1));
  fuser.out(1)->push(&incremental_dead);
  fuser.out(2)->push(&cumulative_dead); 
  cumulative_dead.push(&outputfile2);
  incremental_dead.push(&outputfile);
  while (!odofile.read(podo))
    {
      cerr << podo.getTime() << std::endl;
      cumulative_odo.write(podo);
      while(pin.Time<podo.Time)
	{
	  if(inertialfile.read(pin))
	    {
	      cerr<<"done";
	      return 0;
	    }
	  inertial.write(pin);
	}
      cerr<<"read ";
    }
   
  return 0;
  
}








