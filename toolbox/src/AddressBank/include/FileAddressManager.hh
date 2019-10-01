//
// = LIBRARY
//
// = FILENAME
//    FileAddressManager.hh
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

#ifndef FileAddressManager_hh
#define FileAddressManager_hh

#include "FileAddress.hh"

#ifndef DEPEND
#include <list>
#include <string>
#endif


namespace Cure { 
  // Forward declaration(s)
  class Address; 
  class ConfigFileReader;

/**
 * This class helps you managed reading from files try create a
 * scenario that is similar to what you waht get when you run online. 
 *
 * This class will monitor what the next item is in all files and
 * write the one with the oldest timestamp to the corresponding
 * address. Before this is done odometry will be read so that it is at
 * least as new as the data to write. It will keep reading until the
 * odometry file is empty
 *
 * NOTE: assumes that the files are ordered in time with ascending
 * timestamps
 * 
 * @author Patric Jensfelt 
 * @see
 */
class FileAddressManager {
public:
  /**
   * Helper class for FileAddressManager that holds info about one file
   *
   * @author Patric Jensfelt
   * @see
   */
  class FileData {
  public:
    /** Constructor */
    FileData() : m_PushIndex(0), m_Delay(0) {}

    /** WARNING: Does not really copy, just here to be able to put in list */
    FileData(const FileData& src) : m_PushIndex(0), m_Delay(0) {}

    /** File that holds the data we are interested in */
    FileAddress m_File;
    
    /** Pointer to a data object that can hold data from the file */
    SmartData m_Data;

    /** Where to put the data */
    unsigned long m_PushIndex; 

    /** The delay t use for this file */
    double m_Delay;
  };

public:
  /** Timestamp of last written odometry data */
  Timestamp m_OdomTime;

  /** Timestamp of data last written */
  Timestamp m_DataTime;

  /** List of file to read from */
  std::list<FileData> m_Files;

public:
  /** 
   * Creates an empty object. You need to call at least addOdomFile
   * to be able to run */
  FileAddressManager();

  /** 
   * Releases all the allocated memory for this object
   */
  ~FileAddressManager();


  /**
   * Set the odometry source file and the address to write it to
   *
   * @param filename name of file including path
   * @param addr address to write data after reading it
   * @param delay time delay to add to the odometry
   *
   * @return 0 if OK, else error code
   */
  int addOdomFile(const std::string &filename, Address* addr, double delay);

  /**
   * Specify a file with data in of some type. Since you can only read
   * from a file address if you have the right type you need to
   * provide it with a reference to an object of the right type
   *
   * @param filename name of file incluing path
   * @param classType type of data to read from file
   * @param addr address to write data after reading it
   * @param delay time delay to add to the data from this file
   *
   * @return 0 if OK, else error code
   */
  int addFile(const std::string &filename, 
              unsigned char classType,
              Address* addr, double delay);

  /**
   * Sets up the FileAddressMananger for reading measurement data you
   * using the config information. This function assumes that you want
   * to write all the file data to one address except the odometry
   * data that can be written to a separate address.
   *
   * An error will be returned if you have configured to run in real
   * mode and not file execution mode.
   * 
   * @return NO_ERROR=0 if ok, else error code
   */
  int setupMeasurementFiles(ConfigFileReader &cfg,
                            Address *odomAddr,
                            Address *dataAddr);

  /**
   * Adds an odometry file to the FileAddressManager based on the
   * selection made in the config
   * 
   * @return NO_ERROR=0 if ok, else error code
   */
  int addOdomFile(ConfigFileReader &cfg, Address *addr);

  /**
   * Checks to see if any sensor file of a certain type has been
   * listed in the config and adds it
   * 
   * @return NO_ERROR=0 if ok, else error code
   */
  int addSensorFile(ConfigFileReader &cfg, int type, Address *addr);

  /** 
   * If your measurements come in at a low rate your estimation
   * process might run at a too low rate and the predictions become
   * quite rough. This means that you miss valuable information in the
   * odometry that could contribute to better estimation of the
   * pose. You can call this function to let every odometry reading
   * trigger the sending of an empty measurement set to a given
   * address. This empty measurement set can be used to trigger
   * prediction.
   */
  int addOdomTriggeredEmptyMeas(Address* addr);

  /** 
   * Step forward one step in one of the data files, the one which is
   * oldest and thus should be processed first. This will cause a
   * write to the address that was registered together with this
   * files. Before this write is done the odometry source is read so
   * that the odometry is at least as new as the data to write.
   */
  int step();

private:

  /** File data for odometry source */
  FileData m_Odom;

  /** Index of the address write fake meas triggered by odometry */
  unsigned long m_FakeMeasIndex;
}; // class FileAddressManager

} // namespace Cure

#endif // FileAddressManager_hh
