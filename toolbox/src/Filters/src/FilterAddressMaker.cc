// = RCSID
//    $Id: FilterAddressMaker.cc ,v 1.1 2006/09/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2006 John Folkesson
//    

#include "FilterAddressMaker.hh"
#include "Syncronizer.hh"
#include "ArithmaticFilter.hh" 
#include "PoseErrorModel.hh"
#include "FuseFilter.hh"
#include "GyroModel.hh"
#include "PanTiltModel.hh"
#include "SimpleOdoModel.hh" 
#include "PoseCumulator.hh" 
#include "SensorUser.hh"
#include "MeasSequencer.hh"
#include "ScanLineFilter.hh"
#include "SLAMFilter.hh" 
#include "DeadReckoner.hh" 
#include "GyroAdjOdometry.hh"
#include <sstream>  // istringstream

using namespace Cure;

DataFilter * 
FilterAddressMaker::makeFilterSubClass(const std::string classname,
					const std::string constargs)
{
    std::istringstream strdes(constargs);
    std::string cmd;
    //skip the create classname;
    strdes>>cmd>>cmd;
    if (classname=="Syncronizer"){
      int num=1;
      if ((strdes>>num)){
	return new Syncronizer(num);
      }
    }
    if (classname=="PoseErrorModel")
      return new PoseErrorModel();
    if (classname=="FuseFilter")
      return new FuseFilter();
    if (classname=="GyroModel")
      return new GyroModel();
    if (classname=="PanTiltModel")   
      return new PanTiltModel();
    if (classname=="PoseCumulator")
      return new PoseCumulator();
    if (classname=="SimpleOdoModel")
      return new SimpleOdoModel();
    if (classname=="SensorUser")
      return new SensorUser();
    if (classname=="MeasSequencer")
      return new MeasSequencer();
    if (classname=="ScanLineFilter")
      return new ScanLineFilter();
    if (classname=="SLAMFilter")
      return new SLAMFilter();
    if (classname=="DeadReckoner")
      return new DeadReckoner();
    if (classname=="GyroAdjOdometry")
      return new GyroAdjOdometry();
    return this->Cure::AddressMaker::makeFilterSubClass(classname,
							constargs);  
}
