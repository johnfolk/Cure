#include <iostream>
#include <stdlib.h> 

#include "DataSlotAddress.hh"
#include "FileAddress.hh" 
#include "DataFilterAddress.hh"

#include "RangeHough.hh"
#include "ScanLineFilter.hh"

#include <iomanip>

#include "Syncronizer.hh"
#include "SimpleOdoModel.hh"
#include "GyroModel.hh"
#include "GyroAdjOdometry.hh"

using namespace std;
using namespace Cure;
/*
 * @author John Folkesson and Patric Jensfelt
 * Copyright 2005
 * 
 * This demo shows how to take raw odometry, inertial and sick scan data and
 * output fused dead reckoning interpolated to the timestamps of the scans with
 * covariances and line measurements extracted from the scan data. 
 * 
 */
int main(int argc, char * argv[])
{
  const char *optstring = "hO:I:S:i:c:l:";
  char *args = "[-h help] [-O odometry] [-I intertial] [-S sickscans] [-i incrementalout][-c cumulativeout] [-l lineout]";
  char *odofilename = "notset";
  char *dmufilename = "notset";
  char *scanfilename = "notset";
  char *incoutfilename = "notset";
  char *cumoutfilename = "notset";
  char *lineoutfilename = "notset";
  char o = getopt(argc, argv, optstring);
  while (o != -1) {
    switch (o) {
    case 'O':
      odofilename = optarg;
      break;
    case 'I':
      dmufilename = optarg;
      break;
    case 'S':
      scanfilename = optarg;
      break;
    case 'i':
      incoutfilename = optarg;
    case 'c':
      cumoutfilename = optarg;
      break;
    case 'l':
      lineoutfilename = optarg;
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
            "Ex: ./demo2 -O testodo.tdf -I testdmu.tdf -S testlas.tdf -i outputfileinc.tdf -c outputfilecum.tdf -l lineoutputfile.tdf\n");
    return -1;
  }
   /**
   * The FileAddress is a port to a file.  
   * Saving and reading pose data is now standardized.
   */
  FileAddress odofile, inertialfile;
  odofile.openReadFile(odofilename);
  inertialfile.openReadFile(dmufilename);
  FileAddress scanfile;
  scanfile.openReadFile(scanfilename);
  SICKScan sick;

  scanfile.read(sick);
  int n=sick.getNPts();

  DataSlotAddress scans(100, SICKSCAN_TYPE); 
  /**
   * This object does Hough Line extraction from the laser scan data.
   */
  ScanLineFilter scanfilter;
  /**Give it the raw scans*/
  scans.push(scanfilter.in(0));
 
  /**Normally this is in the .ccf but we hard code it here*/
  double minlength=1;//1.5
  short minPointsPerLine=5;
  double rhoThreshold=.09;
  double varRhoThreshold=.0016;//.0004;//.0027;
  double halfWidth=.01;
  double dGamma=.02;
  double maxGap=2;
  double maxrange=60;
  short scanResHalfDegrees=1;//2
  double rhoBiasThreshold=1E-3;
  scanfilter.config( minlength,
		     minPointsPerLine,
		     rhoThreshold,
		     varRhoThreshold,
		     halfWidth,
		     dGamma,
		     maxGap,
		     maxrange,
		     scanResHalfDegrees,
		     rhoBiasThreshold);
  scanfilter.setBehind(.1);
  /**
   * The output file address will be receiving subscription (push) data
   *
   */
  FileAddress incoutfile;
  FileAddress cumoutfile;
  FileAddress lineoutfile;
  if (strcmp(incoutfilename,"notset")!=0) 
    incoutfile.openWriteFile(incoutfilename);
  if (strcmp(cumoutfilename,"notset")!=0) 
    cumoutfile.openWriteFile(cumoutfilename);
  if (strcmp(lineoutfilename,"notset")!=0) 
    lineoutfile.openWriteFile(lineoutfilename);
  scanfilter.out(0)->push(&lineoutfile); 
  /**
   * The slots can be made by creating an address to them.  
   * They store a history of pose data (in this case 100 Poses).
   * The odd numbers 4107 ect. are the Pose3D::Type.  That means there 
   * are lots of (10,000's) kinds of pose data one can store.  
   * (ie. just theta or Full 6 dof with covariance and velocity...).  
   * These are 'understood' and interpreted properly when adding ect. 
   * The final 1 is the compressed flag.  
   * The data is stored in compressed form.
   * The final true says we want type checking on the read and write.
   *  
   * We don't really need to specify all this but we show you how.
   * If we just said cumulative(100) it would work fine but no
   * type checking could be done.
   */
  DataSlotAddress cumulative_odo(100, POSE3D_TYPE, 4107, 1,true); 
  DataSlotAddress inertial( 100, POSE3D_TYPE, 16384, 1,true); 
  DataSlotAddress incremental_dead(100, POSE3D_TYPE, 0x82C0, 1,true); 
  DataSlotAddress cumulative_dead( 100, POSE3D_TYPE, 0x0E00, 1,true); 
   /* 
   * Lets get some data in the slots to start things off.
   */
  Pose3D podo,pin;
  //*Get data from the file*/
  odofile.read(podo);
  /**Put it in the DataSlot*/
  cumulative_odo.write(podo);
  inertialfile.read(pin);
  inertial.write(pin);
 
  /** 
   * Lets set up some computation with this data.  We will fuse 
   * the gyro and odometer data.  We model the errors in the
   * odometer and inertial data using SkidSteerModel and GyroModel.
   */
  SimpleOdoModel skid;  
  GyroModel gyro;
  /**
   * One of the parameters of the model is ThetaBias.
   */
  gyro.ThetaBias=.0005;
  GyroAdjOdometry fuser;  
   /** 
    * The sync is to regulate the flow of data to the fuser.  We 
    * can't fuse until there is new data on both ports.  The slots will
    * interpolate the poses to the earliest of the two timestamps.
    * We tell the constructor that we need to sync 2 input ports.
    */
  Syncronizer sync(2);
  /**
   * This is to insure that the sync saves 20 trigger signals to
   * be tried later when the data is available.
   * Without this everything works but the dead reckoning output
   * timestamps will skip over some scan timestamps
   */
  sync.setMaxNumStoredSignals(20);
  /**
   * This hook up is the tricky part.  The data can be both pushed 
   * and pulled through the filters.
   * The trigger for the sync will be new odometer data.  
   * This data will be pushed to the sync when  we write to 
   * the cumulative_odo. 
   *
   */
  scans.push(sync.trigger());
  sync.in(0)->pull(&cumulative_odo);   //*************
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
  cumulative_dead.push(&cumoutfile);
  incremental_dead.push(&incoutfile);
  cerr<<"ready to start";
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
	  else
	    inertial.write(pin);
	}
      while(sick.Time<podo.Time)
	{
	  if(scanfile.read(sick))
	    {
	      cerr<<"scans done";
	      return 0;
	    }
	  else
	    {
	      scans.write(sick);
	      cerr<<n<<"\n";
	      n++;
	    }
	}
      cerr<<"read ";
    }
   
  return 0;
  
}








