//
// = LIBRARY
//
// = FILENAME
//    ConfigFileReader.hh
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

#ifndef CureConfigFileReader_hh
#define CureConfigFileReader_hh

#include "Pose3D.hh"
#include "SensorPose.hh"

#ifndef DEPEND
#include <fstream>
#include <string>
#include <list>
#include <map>
#endif


namespace Cure { 

// Forward declaration(s)
class Address; 

/**
 * This class helps you read though configuration files. One of the
 * main ideas behind the configuration files is to allow one or more
 * other files to be included from a configuration file. This way you
 * can have experiment specific file in a config file and then include
 * common stuff about a robot being used from another file. That file
 * in turn can include configuration stuff for a laser scanner type
 * used by many robot for example.
 *
 * You might wonder why we do not use XML for configuration and that
 * is a valid question. We wanted to make the toolbox as free from
 * dependences as possible and did not want to implement our own
 * XML-parser. 
 * 
 * @author Patric Jensfelt 
 * @see
 */
class ConfigFileReader {
public:

  /**
   * Types of error codes that can be returned 
   */
  enum Errors {
    NO_ERROR = 0,
    ERROR_NO_SUCH_FILE,
    ERROR_CFG_NOT_INITED,
    ERROR_DATA_NOT_FOUND,
    ERROR_EMPTY_LINE,
    ERROR_PARSING,
    ERROR_WRONG_EXECMODE,
    ERROR_OPEN_FILE,
    ERROR_NOTUSED,
    ERROR_CFG_ALREADY_INITED,
    ERROR_CIRCLE_INCLUDE,
  };


  /**
   * Execution mode
   */
  enum ExecModes {
    EXECMODE_FILE = 0,
    EXECMODE_REAL,
  };

public:

  /**
   * Constructor
   * @see init
   */
  ConfigFileReader();

  /**
   * Destructor
   */
  ~ConfigFileReader();

  /**
   * Initialize by opening the file
   *
   * @return NO_ERROR=0 if ok, else error code
   */
  int init(const std::string &cfgFileName);


  //========================================================================
  //       General function to get ints, doubles, etc
  //========================================================================


  /**
   * Gets a single string for a certain KEYWORD
   *
   * @param keyword keyword for this string
   * @param useInclude true if we should descend down include files 
   * @param s string to put result in
   * @param usedCfgFile config file where info was found
   * @return NO_ERROR=0 if ok, else error code
   */
  int getString(const std::string &keyword, bool useIncludes,
                std::string &s,
                std::string &usedCfgFile);

  /**
   * Gets list of arguments for a certain KEYWORD
   *
   * @param keyword keyword for the argument string
   * @param useInclude true if we should descend down include files 
   * @param args string to put result in
   * @param usedCfgFile config file where info was found
   * @return NO_ERROR=0 if ok, else error code
   */
  int getArgs(const std::string &keyword, bool useIncludes,
              std::string &args,
              std::string &usedCfgFile);

  /**
   * Gets a single int for a certain KEYWORD
   *
   * @param keyword keyword for this integer
   * @param useInclude true if we should descend down include files 
   * @param value integer to put result in
   * @param usedCfgFile config file where info was found
   * @return NO_ERROR=0 if ok, else error code
   */
  int getInt(const std::string &keyword, bool useIncludes,
             int &value,
             std::string &usedCfgFile);

  /**
   * Gets a single double for a certain KEYWORD
   *
   * @param keyword keyword for this double
   * @param useInclude true if we should descend down include files 
   * @param value double to put result in
   * @param usedCfgFile config file where info was found
   * @return NO_ERROR=0 if ok, else error code
   */
  int getDouble(const std::string &keyword, bool useIncludes,
                double &value,
                std::string &usedCfgFile);

  /**
   * Get a string for parameters for a certain KEYWORD
   *
   * @param keyword keyword for this parameter string
   * @param useInclude true if we should descend down include files 
   * @param params string to put result in
   * @param usedCfgFile config file where info was found
   * @return NO_ERROR=0 if ok, else error code
   */
  int getParamString(const std::string &keyword, bool useIncludes,
                     std::string &params,
                     std::string &usedCfgFile);

  /**
   * Gets a list of strings (like file names) for a certain KEYWORD
   *
   * @param keyword keyword for this parameter string list
   * @param useInclude true if we should descend down include files 
   * @param strings list of string to put result in
   * @param usedCfgFile config file where info was found
   *
   * @return NO_ERROR=0 if ok, else error code
   */
  int getStringList(const std::string &keyword, bool useIncludes,
                    std::list<std::string> &strings,
                    std::string &usedCfgFile);

  /**
   * Gets a list of ints for a certain KEYWORD
   *
   * @param keyword keyword for this list of integers
   * @param useInclude true if we should descend down include files 
   * @param ints list of integers to put result in
   * @param usedCfgFile config file where info was found
   *
   * @return NO_ERROR=0 if ok, else error code
   */
  int getIntList(const std::string &keyword, bool useIncludes,
                 std::list<int> &ints,
                 std::string &usedCfgFile, int minN = -1);

  /**
   * Gets a list of doubles for a certain KEYWORD
   *
   * @param keyword keyword for this list fo doubles
   * @param useInclude true if we should descend down include files 
   * @param doubles list of doubles to put result in
   * @param usedCfgFile config file where info was found
   *
   * @return NO_ERROR=0 if ok, else error code
   */
  int getDoubleList(const std::string &keyword, bool useIncludes,
                    std::list<double> &doubles,
                    std::string &usedCfgFile, int minN = -1);

  /**
   * Gets a string with an int key for a certain KEYWORD. The string
   * could be a list of parameters.
   *
   * @param keyword keyword for this parameter string
   * @param useInclude true if we should descend down include files 
   * @param key identifier key for the string to get
   * @param s string to put result in
   * @param usedCfgFile config file where info was found
   *
   * @return NO_ERROR=0 if ok, else error code
   */
  int getKeyedString(const std::string &keyword, bool useIncludes,
                     int &key, std::string &s,
                     std::string &usedCfgFile);
  
  /**
   * Gets a list of doubles each with a int key for a certain KEYWORD
   *
   * @param keyword keyword for this parameter string
   * @param useInclude true if we should descend down include files 
   * @param v map to store result in
   * @param usedCfgFile config file where info was found
   *
   * @return NO_ERROR=0 if ok, else error code
   */
  int getKeyedDoubles(const std::string &keyword, bool useIncludes,
                      std::map<int, double> &v,
                      std::string &usedCfgFile);

  /**
   * Gets a list of longs each with a int key for a certain KEYWORD
   *
   * @param keyword keyword for this parameter string
   * @param useInclude true if we should descend down include files 
   * @param v map to store result in
   * @param usedCfgFile config file where info was found
   *
   * @return NO_ERROR=0 if ok, else error code
   */
  int getKeyedLongs(const std::string &keyword, bool useIncludes,
                    std::map<int, long> &v,
                    std::string &usedCfgFile);

  /**
   * Gets a list of strings each with a int key for a certain KEYWORD
   *
   * @param keyword keyword for this parameter string
   * @param useInclude true if we should descend down include files 
   * @param strings map to store result in
   * @param usedCfgFile config file where info was found
   *
   * @return NO_ERROR=0 if ok, else error code
   */
  int getKeyedStrings(const std::string &keyword, bool useIncludes,
                      std::map<int, std::string> &strings,
                      std::string &usedCfgFile);

  /**
   * Get a list of strings of parameters for a certain KEYWORD 
   *
   * Could for example be the sensor poses for different types of
   * sensors
   *
   * @param keyword keyword for this parameter string
   * @param useInclude true if we should descend down include files 
   * @param paramsList list to store result in
   * @param usedCfgFile config file where info was found
   *
   * @return NO_ERROR=0 if ok, else error code
   */
  int getParamStrings(const std::string &keyword, bool useIncludes,
                      std::list<std::string> &paramsList,
                      std::string &usedCfgFile);

  /**
   * Get the include path, returns empty path if not found
   * i.e. current directory. Searches for keyword INCLUDEPATH
   *
   * @param path reference to variable to return include path in
   * @return NO_ERROR=0 if ok, else error code
   */
  int getIncludePath(std::string &path);

  /**
   * Gets a list of include files included from this file i.e. looks
   * for parameter for keyword INCLUDE. The list contains first all
   * include files without any prefix, i.e. assuming that the files
   * are in the local directory, then all files with the same path as
   * the one given on the command line and last all include files with
   * the INCLUDEPATH prepended. The latter two are only added if they
   * are different from the current directory.
   *
   * @param includes list of include files
   * 
   * @return NO_ERROR=0 if ok, else error code
   */
  int getIncludeFiles(std::list<std::string> &includes);


  //========================================================================
  //       Function that are more specific for a certain type of information
  //========================================================================


  /**
   * Get the name of teh robot that we are using
   *
   * @param name reference to string variable to return name of robot in
   * @return NO_ERROR=0 if ok, else error code
   */
  int getRobotName(std::string &name);


  /**
   * Get the robot shape parameters
   * 
   * type 0: circular    => params: radius
   * type 1: rectangular => params: length, width, center_offset
   * type 2: polygon     => params: n_vertices x0 y0 x1 y1 ...
   * type 3: complex     => params: 144 polar ranges from center
   *
   * @param type reference to type variable
   * @param params reference to string to put params in
   *
   * @return NO_ERROR=0 if ok, else error code
   */
  int getRobotShape(int &type, std::string &params);

  /**
   * Get a pose with keyword ROBOTPOSE
   * @param pose reference to pose object to put robot pose in
   * @return NO_ERROR=0 if ok, else error code
   */
  int getRobotPose(Pose3D &pose);

  /**
   * Get the mode of execution
   *
   * 0 - read from file
   * 1 - connect to servers
   *
   * @param mode reference to variable to put exec mode in
   * @return NO_ERROR=0 if ok, else error code
   */
  int getExecMode(int &mode);

  /**
   * Get the display mode
   *
   * 0 - do not display
   * 1 - display using RoboLook
   * 2 - display using X11 display
   *
   * @param reference to variable to return display mode value in
   * @return NO_ERROR=0 if ok, else error code
   */
  int getDisplayMode(int &mode);

  /**
   * Get the host which runs RoboLook
   * @param rlHost reference to variable to retrun RoboLook hostname string in
   * @return NO_ERROR=0 if ok, else error code
   */
  int getRoboLookHost(std::string &rlHost);

  /**
   * Use this function to get the map display offset so that you can
   * make the map you are creating line up nicely with an existing map
   *
y
   * @param x x-coordinate for map display offset (return value)
   * @param y y-coordinate for map display offset (return value)
   * @param theta orienta for map display offset (return value)
   * @return NO_ERROR=0 if ok, else error code
   */
  int getMapDisplayOffset(double &x, double &y, double &theta);

  /**
   * Get info about a certain sensor
   *
   * @param pose reference to a pose object from which the SensorType
   * and ID will be matched and pose part returned.
   *
   * @return NO_ERROR=0 if ok, else error code
   */
  int getSensorPose(Cure::SensorPose &pose);

  /**
   * Get info about a certain sensor
   * @param type the sensor type (first one found ID ignored) 
   * @param pose The SensorType and ID will be matched and pose part returned.
   * @return NO_ERROR=0 if ok, else error code
   */
  int getSensorPose(int type,Cure::SensorPose &pose);

  /**
   * Check if a certain feature should be used or not
   *
   * @param type type of features
   * @param used reference to bool where return value is put
   * @return NO_ERROR=0 if ok, else error code 
   * @see SensorData
   */
  int getUseFeature(int type, bool &used);

  /**
   * Get the odometric model type and the params
   *
   * @param type reference to type variable
   * @param params reference to param string variable
   * @return NO_ERROR=0 if ok, else error code
   */
  int getOdomModel(int &type, std::string &params);

  /**
   * Get the Inertial model type and the params
   *
   * @param type reference to return value for type
   * @param params reference to return value param string
   * @return NO_ERROR=0 if ok, else error code
   */
  int getInertialModel(int &type, std::string &params);

  /**
   * Get the config parameters for the different feature helpers. They
   * are returned in a std::map structure where the key is the feature
   * type
   *
   * @param key key for the helper to get config for
   * @param subconfig index for the subconfiguration to get
   * @param arglist reference to string to put the resulting
   * configuration arguments
   *
   * @return NO_ERROR=0 if ok, else error code
   */
  int getHelperConfig(int key, int subconfig, std::string &arglist);

  /**
   * Get the path to the data files, if not foudn it will be set to
   * the empty string (""), i.e. the current directroy.
   *
   * @param filePath reference to filePath variable to return value in
   * @return always return NO_ERROR=0
   */
  int getFilePath(std::string &filePath);

  /**
   * Use this function to get the path to where the AdaBoost
   * classifier files are
   *
   * @param cpath reference to file path string variable to return into
   * @return NO_ERROR=0 if ok, else error code
   */
  int getAdaBoostHypFilePath(std::string &cpath);

  /**
   * Get the name of the file for a certain sensor data dat
   *
   * @param key key for sensor to return filename for
   * @param file reference to string to return filename in
   * @return NO_ERROR=0 if ok, else error code
   */
  int getSensorFile(int key, std::string &file);

  /**
   * Get the name of the file for a AdaBoost classifier type
   *
   * @param key key for type of adaboost file to return filename for
   * @param file reference to string to return filename in
   * @return NO_ERROR=0 if ok, else error code
   */
  int getAdaBoostHypFile(int key, std::string &file);

  /**
   * Get the name of the file for a certain sensor data dat
   *
   * @param key key for feature to return filename for
   * @param file reference to string to return filename in
   * @return NO_ERROR=0 if ok, else error code
   */
  int getFeatureFile(int key, std::string &file);

  /**
   * Get the delay to apply to the data from a certain sensor
   * type. This can be used to compensate for timing errors.
   *
   * @param key key for sensor to return delay for
   * @param delay reference to double to return delay in
   * @return NO_ERROR=0 if ok, else error code
   */
  int getSensorDelay(int key, double &delay);

  /**
   * Get the delay to apply to the data from a certain feature
   * type. This can be used to compensate for timing errors.
   *
   * @param key key for feature to return delay for
   * @param delay reference to double to return delay in
   * @return NO_ERROR=0 if ok, else error code
   */
  int getFeatureDelay(int key, double &delay);

  /**
   * Get the config parameters for the a certain vision feature extractor
   *
   * @param key key for extractor to return arguments for
   * @param arglist reference to string to return arglist in
   * @return NO_ERROR=0 if ok, else error code
   */
  int getVisionExtractorConfig(int key, std::string &arglist);

  /**
   * Get the configuration for the RangeHough class, it is the same
   * config for the ScanLineFilter.
   *
   * @param subconfig key for Hough subconfig to return param list for 
   * @param arglist reference to string to return arglist in
   * @return NO_ERROR=0 if ok, else error code
   */
  int getRangeHoughConfig(int subconfig, std::string &arglist);

  /**
   * Get the Mahalanobis distance used in the last stateg of matching
   * for a certain feature type
   *
   * @param key key for feature type to return mahalanobis distance for 
   * @param d reference to double to return mahalnobis distance value in
   * @return NO_ERROR=0 if ok, else error code
   */
  int getMahalanobisDist(int key, double &d);

  //========================================================================
  //       Functions that gets all the config about something
  //========================================================================

  /**
   * This function will find the type of features to use in this
   * experiment. A list with the type ids is returned
   *
   * @param typeIDs reference to list to put ids of all used features in
   * @return NO_ERROR=0 if ok, else error code
   */
  int getFeaturesToUse(std::list<int> &typeIDs);

  /**
   * This function will find the sensors to use and their offsets
   *
   * @param sensors list of SensorPose to put all sensor poses in
   * @return NO_ERROR=0 if ok, else error code
   */
  int getSensorPoses(std::list<SensorPose> &sensors);

  /**
   * This function will find the sensors to use 
   *
   * @param chains Returns a list of SensorType, ID, offsettype,
   *               SensorPoseType,RobotSensorPoseType,XtrarotationType for
   *               each sensor to be used
   *
   * @return NO_ERROR=0 if ok, else error code
   */
  int getSensorChains(std::list<unsigned short> &chains);


  /**
   * This function will find the sensor to use 
   * Call successively starting with which=0,1,2.. untill you get
   * ERROR_DATA_NOT_FOUND.  That means no more sensors to be used
   *
   * @param which Specify which sensor you want starting from 0 (0 is the
   *              first Sensor in SENSORCHAINS in ccf file). 
   * @param chain Returns  SensorType, ID, offsettype,
   *               SensorPoseType,RobotSensorPoseType,XtrarotationType
   * @return NO_ERROR=0 if ok, else error code
   */
    int getSensorChain(int which,unsigned short chain[6]);

  /**
   * Get the config parameters for the different feature helpers. The
   * helpers have different subconfigurations. They are returned in a
   * std::map structure where the key is the feature type
   *
   * @param subconfig key of subcnfig to get
   * @param config reference to a map structure to put configuration
   * information in
   * @return NO_ERROR=0 if ok, else error code
   */
  int getHelpersConfig(int subconfig, std::map<int, std::string> &config);

  /**
   * Get files to load sensor data from. The files are returned in a
   * std::map structure where the key is the key of the data type.
   *
   * @param files reference to a map structure to put files and their
   * corresponding keys in.
   * @return NO_ERROR=0 if ok, else error code
   */
  int getSensorFiles(std::map<int, std::string> &files);

  /**
   * Get files to load AdaBoost classifier hypothesis data from. The
   * files are returned in a std::map structure where the key is the
   * key of the classifier.
   *
   * @param files reference to a map structure to put files and their
   * corresponding keys in.
   * @return NO_ERROR=0 if ok, else error code
   */
  int getAdaBoostHypFiles(std::map<int, std::string> &files);

  /**
   * Get files to load feature data from. The files are returned in a
   * std::map structure where the key is the key of the data type.
   *
   *  1 - wall features
   * 64 - lamp features
   * 66 - hline features
   *
   * @param files reference to a map structure to put files and their
   * corresponding keys in.
   * @return NO_ERROR=0 if ok, else error code
   */
  int getFeatureFiles(std::map<int, std::string> &files);

  /**
   * Get delays for the data from the sensors
   *
   * @param delays reference to a map structure to put delays and their
   * corresponding keys in.
   * @return NO_ERROR=0 if ok, else error code
   */
  int getSensorDelays(std::map<int, double> &delays);

  /**
   * Get delays for the feature measurements
   *
   * @param delays reference to a map structure to put delays and their
   * corresponding keys in.
   * @return NO_ERROR=0 if ok, else error code
   */
  int getFeatureDelays(std::map<int, double> &delays);

  /**
   * Get the config parameters for the different vision feature
   * extractors. The configs are returned in a std::map structure
   * where the key is the feature type
   *
   * @param config reference to map structure to put extractor configs in 
   * @return NO_ERROR=0 if ok, else error code
   */
  int getVisionExtractorsConfig(std::map<int, std::string> &config);

  /**
   * Get the mahalanobis distance for all feature types
   * @param config reference to map structure to put mahalnobis
   * distances in for different keys (features types)
   * @return NO_ERROR=0 if ok, else error code
   */
  int getMahalanobisDists(std::map<int, double> &mdists);

  /**
   * @return 1 if not found, else 0.
   * @param des the descriptor's name is found in ccf files and the
   * rest of the des is filled in. 
   */
  int getDescritptor(DataDescriptor &des);
  /*
   * Finds a Global defined vector whos name matches tmp.
   * Returns the define string in tmp and assignes an 
   * unique start key to the vector of strings after the name.
   * So if its a vector of numbers each number can be associated
   * with a unique key by incrementing the start key.
   */
  int getGlobal(std::string &tmp, int &startkey);

  int getSensor(std::string &tmp);

  /**
   *  Finds the class and constructor string for a given objectname.
   */
  int  getConstructorInfo(const std::string objectname,
			    std::string &classname,
			    std::string &configline);
  /*
   * Find the number of filternames in configfile 
   */
  int
  getNumberOfObjects( const std::string &objectbasetype, int &numberOfObjects);

protected:

  /**
   * Constructor used when going through include files so that we can
   * abort if we go through to many
   * 
   * @param incHistory list of filenames included so far
   * @param incPathFromCmdLine include path given by what was given on
   * the command line (../../config.ccf would give ../../ for example)
   * @param incPathFromCmdLine include path specified by the first config 
   * file in the chain
   */
  ConfigFileReader(const std::list<std::string> &incHistory,
                   const std::string &incPathFromCmdLine,
                   const std::string &incPathFromFirstFile);

  /**
   * Looks for a certain keyword that is assumed to be at the
   * beginning of a line in the file. This line will be read and the
   * m_CfgFile will be positioned at the next line after a successfull
   * call to this function.
   *
   * @param keyword keyword to look for in the config file
   * @param lineno line where the keyword was found (return value)
   * @return NO_ERROR=0 if ok, else error code
   */
  int searchKeyword(const std::string &keyword, int &lineno);

  /**
   * Get the config parameters for the different feature helpers. They
   * are returned in a std::map structure where the key is the feature
   * type
   *
   * @param key key for help to get config gor
   * @param subconfig string coding the type of subconfig
   * @param arglist return value for configuration string
   * @return NO_ERROR=0 if ok, else error code
   */
  int getHelperConfig(int key, const std::string &subconfig, 
                      std::string &arglist);

  /**
   * Get the config parameters for the different feature helpers. The
   * helpers have different subconfigurations. They are returned in a
   * std::map structure where the key is the feature type
   *
   * @param subconfig string coding the type of subconfig
   * @param config map structure where the config values for different
   * types of features (first arg in map) are returned
   * @return NO_ERROR=0 if ok, else error code
   */
  int getHelpersConfig(const std::string &subconfig,
                       std::map<int, std::string> &config);

  /**
   * Gets a list of files using getKeyedStrings and adding the
   * constraint that there can only be one file per key and the filename
   * must be without spaces.
   *
   * @param keyword keyword to look for files for
   * @param files map structure where the files with their indices are returned
   * @return NO_ERROR=0 if ok, else error code
   */
  int getFiles(const std::string &keyword, 
               std::map<int, std::string> &files);

  /**
   * Gets a list of delays
   *
   * @param keyword keyword to look for delays for
   * @param files map structure where the delays with their indices
   * are returned 
   * @return NO_ERROR=0 if ok, else error code
   */
  int getDelays(const std::string &keyword, 
                std::map<int, double> &delays);

  /**
   * Get a file with keyword a certain keyword (such as SENSORFILES)
   * with a certain key
   *
   * @param keyword keyword to look for to get a certain file type
   * @param key key to look for file
   * @param filename reference to string to put resulting filename in
   * @return NO_ERROR=0 if ok, else error code
   */
  int getFile(const std::string &keyword, int key, std::string &filename);

  /**
   * Get a delay under a certain keyword for a certain key
   *
   * @param keyword keyword to look for 
   * @param key the key for tjis feature or sensor for exampel
   * @param delay reference to variabel to put resulting delay value in
   * @return NO_ERROR=0 if ok, else error code
   */
  int getDelay(const std::string &keyword, int key, double &delay);


private:

  /**
   * Name of the configuration file that tis object was built upon
   */
  std::string m_CfgFileName;

  /**
   * Pointer to the file stream corresponding the configuration file
   * we are reading from.
   */
  std::fstream *m_CfgFile;

  /** 
   * The files included to get to the current file. This is used to
   * stop circular including files
   */
  std::list<std::string> m_IncHistory;

  /**
   * The include path to the config file given from the command
   * line. That is if you specify ../../../cfg/goofy.ccf this variable
   * will be ../../../cfg. This way it is easier to use the default
   * config files.
   */
  std::string m_IncPathFromCmdLine;

  /**
   * The include path read from the top config file (INCLUDEPATH)
   */
  std::string m_IncPathFromFirstFile;
}; // class ConfigFileReader

} // namespace Cure 

#endif // CureConfigFileReader_hh
