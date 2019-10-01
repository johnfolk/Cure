//
// = LIBRARY
//
// = FILENAME
//    FileAddressManager.cc
//
// = FUNCTION
//
// = AUTHOR(S)
//    Patric Jensfelt
//
// = DESCRIPTION
//    
// = COPYRIGHT
//    Copyright (c) 2004 Patric Jensfelt
//
/*----------------------------------------------------------------------*/

#include "FileAddressManager.hh"
#include "MeasurementSet.hh"
#include "ConfigFileReader.hh"

#include "CureDebug.hh"

#ifndef DEPEND
#include <float.h>  // DBL_MAX
#endif

namespace Cure {

FileAddressManager::FileAddressManager()
{
  m_FakeMeasIndex = 0;
}

FileAddressManager::~FileAddressManager()
{
  // We close the files just to be sure but we do not need to free the
  // data that the SmartTPoiners are holding in m_Data it is taken
  // care of by the smart pointer.

  if (m_Odom.m_File.ReadFile.is_open()) {
    m_Odom.m_File.closeReadFile();
  }

  for (std::list<FileData>::iterator fi = m_Files.begin();
       fi != m_Files.end(); fi++) {
    fi->m_File.closeReadFile();
  }
}

int
FileAddressManager::addOdomFile(const std::string &filename, 
                                Cure::Address *addr, double delay)
{
  if (m_Odom.m_File.ReadFile.is_open()) {
    CureCERR(20) << "Odom file already opened, close in old file\n";
    m_Odom.m_File.closeReadFile();
  }

  if (m_Odom.m_File.openReadFile(filename.c_str())) {
    CureCERR(20) << "Could not open odom file \""
                  << filename << "\"\n";
    return RESOURCE_ERROR;
  }

  if (m_Odom.m_Data.setup(POSE3D_TYPE)) {
    CureCERR(20) << "Failed to setup m_Data as Pose3D\n";
    return RESOURCE_ERROR;
  }

  m_Odom.m_PushIndex = addr->Index;
  m_Odom.m_Delay = delay;

  // We always read one element from the file to begin with so that we
  // get a peek of the timestamp
  if (m_Odom.m_File.read(m_Odom.m_Data)) {
    CureCERR(20) << "Could not even read first odom data from file \""
                  << filename << "\", closing it!\n";
    m_Odom.m_File.closeReadFile();
    return RESOURCE_ERROR;
  } else {
    // Compensate for the delay
    m_Odom.m_Data.setTime(m_Odom.m_Data.getDoubleTime() + m_Odom.m_Delay);
  }

  // Write the first odom data to the address, if specified
  if ( m_Odom.m_PushIndex > 0){
    if (AddressBank::theAddressBank().
	write(m_Odom.m_PushIndex,*m_Odom.m_Data.getTPointer())
        &ADDRESS_INVALID) {
      CureCERR(20) << "Could not push first odom data, not trying again!\n";
      m_Odom.m_PushIndex = 0;
    }
    m_OdomTime = m_Odom.m_Data.Time;
  }

  CureDO(60) {
    CureCERR(0) << "First item from file \"" << filename << "\""
                  << " has time " << m_Odom.m_Data.getTime()
                  << std::endl;
    m_Odom.m_Data.getTPointer()->print();
  }

  return 0;
}

int
FileAddressManager::addFile(const std::string &filename, 
                            unsigned char classType,
                            Cure::Address *addr, double delay)
{
  // First we check to make sure that the same filename has not been
  // used to open any other files
  if (m_Odom.m_File.getFilename() == filename) {
    CureCERR(20) << "File \"" << filename 
                  << "\" already used as odom file!!!!\n";
    return RESOURCE_ERROR;
  }

  for (std::list<FileData>::iterator fi = m_Files.begin();
       fi != m_Files.end(); fi++) {
    if (fi->m_File.getFilename() == filename) {
      CureCERR(20) << "File \"" << filename << "\" already opened!\n";
      return RESOURCE_ERROR;
    }
  }

  m_Files.push_back(FileData());
  FileData &fd = m_Files.back();
  if (fd.m_File.openReadFile(filename.c_str())) {
    CureCERR(20) << "Could not open file \"" << filename << "\"\n";
    m_Files.pop_back();
    return RESOURCE_ERROR;
  }

  if (fd.m_Data.setup(classType)) {
    CureCERR(20) << "Could not assign SmartTPointer to classType "
                  << classType << std::endl;
    m_Files.pop_back();
    return RESOURCE_ERROR;
  }
  fd.m_PushIndex = addr->Index; 
  fd.m_Delay = delay;
 
  // Read one element from the file to get a peek at the timestamp
  if (fd.m_File.read(fd.m_Data)) {
    m_Files.pop_back();
    if (fd.m_File.ReadFile.eof()) {
      CureCERR(40) << "File \"" << filename << "\" is empty\n";
      return 0;
    } else {
      CureCERR(30) << "Could not even read the first element in file \""
                    << filename << "\"\n";
      return RESOURCE_ERROR;
    }
  } else {
    // Compensate for delay
    fd.m_Data.setTime(fd.m_Data.getDoubleTime() + fd.m_Delay);
  }

  CureDO(60) {
    CureCERR(0) << "First item from file \"" << filename << "\""
                 << " has time " << fd.m_Data.getTime() 
                 << std::endl;
    fd.m_Data.getTPointer()->print();
  }

  return 0;
}

int
FileAddressManager::addOdomTriggeredEmptyMeas(Cure::Address *addr)
{
  if (m_FakeMeasIndex >0) {
    CureCERR(20) << "WARNING: Changing fake meas addr\n";
  }

  m_FakeMeasIndex = addr->Index;
  
  return 0;
}

int
FileAddressManager::step()
{
  if (!m_Odom.m_File.ReadFile.is_open()) {
    CureCERR(30) << "No odometry file, it is over!\n";
    return RESOURCE_ERROR;
  }

  // First check which file has the oldest data that we should write
  // in this step
  std::list<FileData>::iterator fd = m_Files.end();
  double tMin = DBL_MAX;
  for (std::list<FileData>::iterator fi = m_Files.begin();
       fi != m_Files.end(); fi++) {
    if (fi->m_Data.Time.getDouble() < tMin) {
      tMin = fi->m_Data.Time.getDouble();
      fd = fi;
    }
  }

  if (fd != m_Files.end()) {
    // Make sure that the odometry has been read at least up to the
    // point of the next data
    while (m_Odom.m_Data.Time.getDouble() < tMin) {
      if (m_Odom.m_File.read(m_Odom.m_Data)) {
        CureCERR(20) << "No more odom data in file \""
                      << m_Odom.m_File.getFilename() << "\", closing it!\n";
        m_Odom.m_File.closeReadFile();
        return RESOURCE_ERROR;
      } else {
        // Compensate for the delay
        m_Odom.m_Data.setTime(m_Odom.m_Data.getDoubleTime() + m_Odom.m_Delay);
      }

      if (m_Odom.m_PushIndex > 0) {
        if (AddressBank::theAddressBank().
	    write(m_Odom.m_PushIndex,*m_Odom.m_Data.getTPointer())
            &ADDRESS_INVALID) {
          CureCERR(20) << "Could not push odom data, disconnecting!\n";
          m_Odom.m_PushIndex = 0;
        } else {
          CureCERR(55) << "Wrote odometry with time "
                        << m_Odom.m_Data.getTime()
                        << std::endl;
          m_OdomTime = m_Odom.m_Data.Time;
        }
      }

      if ( m_FakeMeasIndex > 0) {
        Cure::MeasurementSet ms;
        ms.setNumberOfElements(1);
        ms.Measurements[0].MeasurementType = 0;
        ms.Measurements[0].SensorType = 12345;
        ms.Measurements[0].V.reallocate(0);
        ms.Measurements[0].W.reallocate(0);
        ms.Measurements[0].CovV.reallocate(0);
        ms.Measurements[0].Z.reallocate(0);
        ms.Measurements[0].BoundingBox.reallocate(0);
        ms.Time = m_OdomTime;
        CureCERR(55) << "Writing fake measurement set with time "
                      << ms.Time << std::endl;
        if (AddressBank::theAddressBank().write(m_FakeMeasIndex, ms) & ADDRESS_INVALID) {
          CureCERR(20) << "Could not push first odom data diconnect!\n";
          m_Odom.m_PushIndex = 0;
        }
      }
    }

    // Now we can write the other data
    if ( fd->m_PushIndex >0) {
      if (AddressBank::theAddressBank().write(fd->m_PushIndex,*fd->m_Data.getTPointer())
          &ADDRESS_INVALID) {
        CureCERR(20) << "Could not push data ,not trying again!\n";
        fd->m_PushIndex =0;
      } else {
        CureCERR(55) << "Wrote data with time "
                     << fd->m_Data.getTime()
                     << " with class type " 
                     << (int)fd->m_Data.getTPointer()->getClassType()
                     << " and subtype "
                     << fd->m_Data.getTPointer()->getSubType()
                     << std::endl;
        m_DataTime = fd->m_Data.Time;
      }
    }

    // We also need to read one more element from this file
    // Read one element from te file to get a peek at the timestamp
    if (fd->m_File.read(fd->m_Data)) {
      CureCERR(30) << "No more data in file \""
                    << fd->m_File.getFilename() << "\"\n";
      m_Files.erase(fd);
    } else {
      // Compensate for delay
      fd->m_Data.setTime(fd->m_Data.getDoubleTime() + fd->m_Delay);
    }

  } else {
    CureCERR(40) << "There is no more data!!!\n";

    if (m_Odom.m_File.read(m_Odom.m_Data)) {
      CureCERR(20) << "No more odom data in file \""
                    << m_Odom.m_File.getFilename() << "\", closing it!\n";
      m_Odom.m_File.closeReadFile();
      return RESOURCE_ERROR;
    } else {
      // Compensate for the delay
      m_Odom.m_Data.setTime(m_Odom.m_Data.getDoubleTime() + m_Odom.m_Delay);
    }

    if ( m_Odom.m_PushIndex > 0) {
      if (AddressBank::theAddressBank().
	  write(m_Odom.m_PushIndex,*m_Odom.m_Data.getTPointer())
          &ADDRESS_INVALID) {
        CureCERR(20) << "Could not push odom disconnect!\n";
        m_Odom.m_PushIndex = 0;
      }

      m_OdomTime = m_Odom.m_Data.Time;
        
      if (m_FakeMeasIndex >0) {
        Cure::MeasurementSet ms;
        ms.setNumberOfElements(1);
        ms.Measurements[0].MeasurementType = 0;
        ms.Measurements[0].SensorType = 12345;
        ms.Measurements[0].V.reallocate(0);
        ms.Measurements[0].W.reallocate(0);
        ms.Measurements[0].CovV.reallocate(0);
        ms.Measurements[0].Z.reallocate(0);
        ms.Measurements[0].BoundingBox.reallocate(0);
        ms.Time = m_OdomTime;
        CureCERR(55) << "Writing fake measurement set with time "
                      << ms.Time << std::endl;
        if (AddressBank::theAddressBank().write(m_FakeMeasIndex, ms) & ADDRESS_INVALID) {
          CureCERR(20) << "Could not push first odom data diconnect!\n";
          m_Odom.m_PushIndex = 0;
        }
      }
    }
  }

  return 0;
}

int 
FileAddressManager::setupMeasurementFiles(ConfigFileReader &cfg,
                                          Cure::Address *odomAddr,
                                          Cure::Address *dataAddr)
{
  // First we check if we really should be reading from file
  int execMode;
  if (cfg.getExecMode(execMode)) return ConfigFileReader::ERROR_DATA_NOT_FOUND;
  if (execMode != ConfigFileReader::EXECMODE_FILE) {
    CureCERR(20) << "WARNING: Not configed to read from file\n";
    return ConfigFileReader::ERROR_WRONG_EXECMODE;
  }

  std::string filepath;
  if (cfg.getFilePath(filepath)) return ConfigFileReader::ERROR_DATA_NOT_FOUND;

  // First we load the odometry file
  std::string filename;
  if (cfg.getSensorFile(0, filename)) 
    return ConfigFileReader::ERROR_DATA_NOT_FOUND;
  double delay;
  if (cfg.getSensorDelay(0, delay)) delay = 0;  // default no delay
  CureCERR(30) << "Using delay " << delay << "s for odometry\n";
  if (addOdomFile(filepath + "/" + filename, odomAddr, delay)) 
    return ConfigFileReader::ERROR_OPEN_FILE;
  CureCERR(30) << "Using \"" << filepath + "/" + filename << "\"\n";

  // Check which features to use
  std::list<int> typeIDs;
  if (cfg.getFeaturesToUse(typeIDs)) 
    return ConfigFileReader::ERROR_DATA_NOT_FOUND;

  // Go through the features and open the files
  for (std::list<int>::iterator fi = typeIDs.begin();
       fi != typeIDs.end(); fi++) {
    if (cfg.getFeatureFile(*fi, filename)) 
      return ConfigFileReader::ERROR_DATA_NOT_FOUND;
    if (cfg.getFeatureDelay(*fi, delay)) delay = 0;  // default no delay
    CureCERR(30) << "Using delay " << delay << "s for feature type "
                 << *fi << "\n";
    if (addFile(filepath + "/" + filename, MEASUREMENTSET_TYPE, dataAddr,
                delay)) 
      return ConfigFileReader::ERROR_OPEN_FILE;
    CureCERR(30) << "Using \"" << filepath + "/" + filename << "\"\n";
  }

  return 0;
}

int 
FileAddressManager::addOdomFile(ConfigFileReader &cfg, Cure::Address *addr)
{
  std::string filepath;
  if (cfg.getFilePath(filepath)) return ConfigFileReader::ERROR_DATA_NOT_FOUND;

  // First we load the odometry file
  std::string filename;
  if (cfg.getSensorFile(0, filename))  {
    CureCERR(20) << "Could not find odom file\n";
    return ConfigFileReader::ERROR_DATA_NOT_FOUND;
  }
  double delay;
  if (cfg.getSensorDelay(0, delay)) delay = 0;  // default no delay
  CureCERR(30) << "Using delay " << delay << " for odometry\n";
  if (addOdomFile(filepath + "/" + filename, addr, delay)) {
    CureCERR(20) << "Could not find filepath\n";
    return ConfigFileReader::ERROR_OPEN_FILE;
  }

  CureCERR(30) << "Using \"" << filepath + "/" + filename << "\"\n";
  
  return 0;
}

int 
FileAddressManager::addSensorFile(ConfigFileReader &cfg, 
                                  int type, Cure::Address *addr)
{
  // First we map sensor type to classType
  unsigned char classType = 0;
  if (type >= 0 && type < 3) classType = POSE3D_TYPE;
  else if (type == 3) classType = SICKSCAN_TYPE;
  else return ConfigFileReader::ERROR_DATA_NOT_FOUND;

  // Check if we have a filename for this type of sensor
  std::string filename;
  if (cfg.getSensorFile(type, filename)) {
    CureCERR(20) << "Could not find sensor file of type "
                 << type << std::endl;
    return ConfigFileReader::ERROR_DATA_NOT_FOUND;
  }

  // Get the filepath
  std::string filepath;
  if (cfg.getFilePath(filepath)) {
    CureCERR(20) << "Could not find filepath\n";
    return ConfigFileReader::ERROR_DATA_NOT_FOUND;
  }

  double delay;
  if (cfg.getSensorDelay(type, delay)) delay = 0;  // default no delay
  CureCERR(30) << "Using delay " << delay << " for sensor type "
               << type << "\n";

  // Add the file to the manager
  if (addFile(filepath + "/" + filename, classType, addr, delay)) 
    return ConfigFileReader::ERROR_OPEN_FILE;

  CureCERR(30) << "Using \"" << filepath + "/" + filename << "\"\n";
  
  return 0;
}

} // namespace Cure
