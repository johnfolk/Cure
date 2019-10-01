//
// = LIBRARY
//
// = FILENAME
//    ConfigFileReader.cc
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

#include "ConfigFileReader.hh"
#include "Address.hh"

#include "CureDebug.hh"

#ifndef DEPEND
#include <sstream>  // istringstream
#include <ctype.h>  // isdigit
#include <algorithm>
#endif

#define SEARCH_INCLUDES_FILES(DEF_RET, FCN, DESCR) CureCERR(75) << "Looking for include files in \"" << m_CfgFileName << "\"\n"; ret = DEF_RET; std::list<std::string> includes; if (getIncludeFiles(includes)==0) { for (std::list<std::string>::iterator fi = includes.begin(); fi != includes.end(); fi++) { ConfigFileReader cfr(m_IncHistory, m_IncPathFromCmdLine, m_IncPathFromFirstFile); if (cfr.init(*fi)) {  CureCERR(70) << "Could not open file \"" << *fi << "\" included from \"" << m_CfgFileName << "\"\n"; continue; } int retttt = cfr.FCN; CureCERR(80) << "retttt=" << retttt << " for " << DESCR << " in file " << m_CfgFileName << "\n"; if (!retttt) { CureCERR(70) << "Found " << DESCR << " in include file \"" << *fi << "\"\n"; ret = NO_ERROR; break; } } }


//===========================================================================

namespace Cure {

ConfigFileReader::ConfigFileReader()
  :m_CfgFileName(""),
   m_CfgFile(0)
{}

ConfigFileReader::ConfigFileReader(const std::list<std::string> &incHistory,
                                   const std::string &incPathFromCmdLine,
                                   const std::string &incPathFromFirstFile)
  :m_CfgFileName(""),
   m_CfgFile(0),
   m_IncHistory(incHistory),
   m_IncPathFromCmdLine(incPathFromCmdLine),
   m_IncPathFromFirstFile(incPathFromFirstFile)
{}

ConfigFileReader::~ConfigFileReader()
{
  m_IncHistory.clear();
  if (m_CfgFile) delete m_CfgFile;
}
 
int
ConfigFileReader::init(const std::string &cfgFileName)
{
  if (m_CfgFile) {
    CureCERR(20) << "init already called with filename \""
                 << m_CfgFileName << "\"\n";
    return ERROR_CFG_ALREADY_INITED;
  }

  // Check if we are trying to open a file that we have already opened
  // along the include history
  if (m_IncHistory.size() > 0) {
    if (find(m_IncHistory.begin(), m_IncHistory.end(), cfgFileName) !=
        m_IncHistory.end()) {
      CureDO(20) {
        CureCERR(0) << "Include file \"" << cfgFileName 
                    << "\" has already been included:";
        for (std::list<std::string>::iterator hi = m_IncHistory.begin();
             hi != m_IncHistory.end(); hi++) {
          std::cerr << "\"" << *hi << "\" ";
        }
        std::cerr << std::endl;
      }
        
      return ERROR_CIRCLE_INCLUDE;
    }
  }

  m_CfgFileName = cfgFileName;
  if (m_CfgFile) delete m_CfgFile;
  m_CfgFile = new std::fstream;
  m_CfgFile->open(m_CfgFileName.c_str(), std::ios::in);
  if (!m_CfgFile->is_open()) {
    if (m_IncHistory.empty()) {
      CureCERR(30) << "WARNING: could not open config file \"" 
                   << m_CfgFileName << "\"\n";
    } else {
      CureCERR(50) << "WARNING: could not open config file \"" 
                   << m_CfgFileName << "\"\n";
    }
    return ERROR_NO_SUCH_FILE;
  } else {
    CureCERR(70) << "Successfully opened config file \""
                 << cfgFileName << "\"\n";
  }

  m_IncHistory.push_back(m_CfgFileName);

  if (m_IncHistory.size() == 1) {

    // We check if we can extract an include path from the include
    // first include path, typically given at the command line.
    std::string::size_type pos = m_CfgFileName.find_last_of('/');
    if(pos == std::string::npos) { // Not found
      m_IncPathFromCmdLine = "";
    } else {
      m_IncPathFromCmdLine = m_CfgFileName.substr(0,pos+1);
      // Skip current directory
      if (m_IncPathFromCmdLine == "./") m_IncPathFromCmdLine = "";
    }

    // If this is the first include file we read the include path from
    // the file
    getIncludePath(m_IncPathFromFirstFile);

    CureCERR(30) << "m_IncPathFromCmdLine=\"" 
                 << m_IncPathFromCmdLine << "\"  "
                 << "m_IncPathFromFirstFile=\""
                 << m_IncPathFromFirstFile << "\"\n";

    CureCERR(30) << "Successfully opened config file \""
                 << cfgFileName << "\"\n";
  } else {
    CureCERR(75) << "Successfully opened config file \""
                 << cfgFileName << "\" at depth "
                 << m_IncHistory.size() << std::endl;
  }

  return NO_ERROR;
}

int
ConfigFileReader::searchKeyword(const std::string& keyword, int &lineno)
{
  if (!m_CfgFile->is_open()) {
    CureCERR(20) << "You must init cfg file first\n";
    return ERROR_CFG_NOT_INITED;
  }

  if (!m_CfgFile->good()) {
    CureCERR(75) << "File not good, probably reached EOF\n";
    // Is there another way to do this?
    m_CfgFile->close();
    delete m_CfgFile;
    m_CfgFile = new std::fstream;
    m_CfgFile->open(m_CfgFileName.c_str(), std::ios::in);
    if (!m_CfgFile->is_open()) {
      CureCERR(40) << "WARNING: could not open config file \"" 
                   << m_CfgFileName << "\"\n";
      return ERROR_NO_SUCH_FILE;
    }      
  }
   
  // Put the file at the beginning
  m_CfgFile->seekg(0, std::ios::beg);
  lineno = 0;

  // Look for the keyword ROBOT
  std::string tmp;
  std::string line;
  CureCERR(65) << "Searching for keyword \"" << keyword << "\"\n";
  while (getline(*m_CfgFile, line)) {
    lineno++;
    std::istringstream keystr(line);

    if (keystr >> tmp) {
      if (tmp == keyword) {
        return NO_ERROR;
      } else {
        //CureCERR(60) << "tmp=\"" << tmp << "\"\n";
      }
    }
  }

  CureCERR(75) << "Could not find KEYWORD \"" << keyword 
               << "\" in config file \"" << m_CfgFileName << "\"\n";
  return ERROR_DATA_NOT_FOUND;
}

int
ConfigFileReader::getString(const std::string &keyword, bool useIncludes,
                            std::string &s,
                            std::string &usedCfgFile)
{
  int lineno;
  int ret = searchKeyword(keyword, lineno);
  if (ret == ERROR_NO_SUCH_FILE) return ret;

  usedCfgFile = m_CfgFileName;

  if (!ret) {  
    std::string line, tmp;
    while(getline(*m_CfgFile, line)) {
      lineno++;
      std::istringstream str(line);
      CureCERR(70) << "Read line \"" << line << "\"\n";
      if ( !(str >> tmp) ) {
        CureCERR(20) << "WARNING: Empty line near line " << lineno 
                     << " looking for " << keyword
                     << " in file \"" << m_CfgFileName << "\"\n";
        return ERROR_EMPTY_LINE;
      } else if (tmp[0] != '#') {
        s = tmp;
        return NO_ERROR;
      }
    }
  }

  if (useIncludes) {
    // As a last resort we try to look for includes that might contain
    // the necessary information.
    SEARCH_INCLUDES_FILES(ERROR_DATA_NOT_FOUND,
                          getString(keyword, useIncludes, s, usedCfgFile),
                          "string for " << keyword);
  }

  return ret;
}

int 
ConfigFileReader::getInt(const std::string &keyword,
                         bool useIncludes,
                         int &value,
                         std::string &usedCfgFile)
{
  int lineno;
  int ret = searchKeyword(keyword, lineno);
  if (ret == ERROR_NO_SUCH_FILE) return ret;

  usedCfgFile = m_CfgFileName;

  if (!ret) {
    std::string line, tmp;
    while(getline(*m_CfgFile, line)) {
      lineno++;
      std::istringstream str(line);
      CureCERR(70) << "Read line \"" << line << "\"\n";
      if ( !(str >> tmp) ) {
        CureCERR(20) << "WARNING: Empty line near line " << lineno 
                     << " looking for EXECMODE "
                     << " in file \"" << m_CfgFileName << "\"\n";
        return ERROR_EMPTY_LINE;
      } else if (tmp[0] == '#') {
        // Just skip as comment
      } else if (isdigit(tmp[0]) || 
                 tmp[0] == '-' || tmp[0] == '+') {
        value = atoi(tmp.c_str());
        return NO_ERROR;
      } else {
        CureCERR(20) << "WARNING: Error parsing for int "
                     << "near line " << lineno 
                     << " in file \"" << m_CfgFileName << "\"\n";
        return ERROR_PARSING;
      }
    }
  }

  if (useIncludes) {
    // As a last resort we try to look for includes that might contain
    // the necessary information.
    SEARCH_INCLUDES_FILES(ERROR_DATA_NOT_FOUND,
                          getInt(keyword, useIncludes, value,
                                 usedCfgFile),
                          "int for " << keyword);
  }

  return ret;
}

int 
ConfigFileReader::getDouble(const std::string &keyword,
                            bool useIncludes,
                            double &value,
                            std::string &usedCfgFile)
{
  int lineno;
  int ret = searchKeyword(keyword, lineno);
  if (ret == ERROR_NO_SUCH_FILE) return ret;
  
  usedCfgFile = m_CfgFileName;

  if (!ret) {
    std::string line, tmp;
    while(getline(*m_CfgFile, line)) {
      lineno++;
      std::istringstream str(line);
      CureCERR(60) << "Read line \"" << line << "\"\n";
      if ( !(str >> tmp) ) {
        CureCERR(20) << "WARNING: Empty line near line " << lineno 
                     << " looking for EXECMODE "
                     << " in file \"" << m_CfgFileName << "\"\n";
        return ERROR_EMPTY_LINE;
      } else if (tmp[0] == '#') {
        // Just skip as comment
      } else if (isdigit(tmp[0]) || tmp[0] == '.' ||
                 tmp[0] == '+' || tmp[0] == '-') {
        value = atof(tmp.c_str());
        return NO_ERROR;
      } else {
        CureCERR(20) << "WARNING: Error parsing for double "
                     << "near line " << lineno 
                     << " in file \"" << m_CfgFileName << "\"\n";
        return ERROR_PARSING;
      }
    }
  }

  if (useIncludes) {
    // As a last resort we try to look for includes that might contain
    // the necessary information.
    SEARCH_INCLUDES_FILES(ERROR_DATA_NOT_FOUND,
                          getDouble(keyword, useIncludes, value,
                                    usedCfgFile),
                          "double for " << keyword);
  }

  return ret;
}

int
ConfigFileReader::getParamString(const std::string &keyword, bool useIncludes,
                                 std::string &params,
                                 std::string &usedCfgFile)
{
  params = "";
  int lineno;
  int ret = searchKeyword(keyword, lineno);
  if (ret == ERROR_NO_SUCH_FILE) return ret;

  usedCfgFile = m_CfgFileName;

  if (!ret) {
    std::string line, tmp;
    while(getline(*m_CfgFile, line)) {
      lineno++;
      std::istringstream str(line);
      CureCERR(70) << "Read line \"" << line << "\"\n";
      if ( !(str >> tmp) ) {
        CureCERR(20) << "WARNING: Empty line near line " << lineno 
                     << " looking for " << keyword
                     << " in file \"" << m_CfgFileName << "\"\n";
        return ERROR_EMPTY_LINE;
      } else if (tmp[0] != '#') {
        params = line;
        return NO_ERROR;
      }
    }
  }

  if (useIncludes) {
    // As a last resort we try to look for includes that might contain
    // the necessary information.
    SEARCH_INCLUDES_FILES(ERROR_DATA_NOT_FOUND, 
                          getParamString(keyword, useIncludes, params,
                                         usedCfgFile),
                          "param string for " << keyword);
  }

  return ret;
}

int
ConfigFileReader::getStringList(const std::string &keyword,
                                bool useIncludes,
                                std::list<std::string> &strings,
                                std::string &usedCfgFile)
{  
  strings.clear();
  int lineno;
  int ret = searchKeyword(keyword, lineno);
  if (ret == ERROR_NO_SUCH_FILE) return ret;
  
  usedCfgFile = m_CfgFileName;

  if (!ret) {
    std::string line, s;
    while(getline(*m_CfgFile, line)) {
      lineno++;
      std::istringstream str(line);
      CureCERR(70) << "Read line \"" << line << "\"\n";
      if ( !(str >> s) ) {
        if (strings.empty()) {
          CureCERR(20) << "WARNING: Empty line near line " << lineno 
                       << " looking for INCLUDE"
                       << " in file \"" << m_CfgFileName << "\"\n";
          return ERROR_EMPTY_LINE;
        } else {
          break;
        }
      } else {
        if (s[0] != '#') {
          strings.push_back(s);
        }
      }
    }
  }

  if (useIncludes) {
    // As a last resort we try to look for includes that might contain
    // the necessary information.
    SEARCH_INCLUDES_FILES(ERROR_DATA_NOT_FOUND,
                          getStringList(keyword, useIncludes, strings, 
                                        usedCfgFile),
                          "string");
  }

  if (strings.empty()) {
    return ERROR_DATA_NOT_FOUND;
  }

  return ret;
}

int
ConfigFileReader::getIntList(const std::string &keyword, bool useIncludes,
                             std::list<int> &ints,
                             std::string &usedCfgFile, int minN)
{
  ints.clear();
  int lineno;
  int ret = searchKeyword(keyword, lineno);
  if (ret == ERROR_NO_SUCH_FILE) return ret;

  usedCfgFile = m_CfgFileName;

  if (!ret) {
    std::string line, tmp;
    int i;
    while(getline(*m_CfgFile, line)) {
      lineno++;
      std::istringstream str(line);
      CureCERR(70) << "Read line \"" << line << "\"\n";
      if ( !(str >> tmp) ) {
        CureCERR(20) << "WARNING: Empty line near line " << lineno 
                     << " looking for " << keyword
                     << " in file \"" << m_CfgFileName << "\"\n";
        return ERROR_EMPTY_LINE;
      } else if (tmp[0] == '#') {
        // Just skip as comment
      } else if (isdigit(tmp[0]) || 
                 tmp[0] == '-' || tmp[0] == '+') {
        i = atoi(tmp.c_str());
        ints.push_back(i);
        while (str >> i) ints.push_back(i);

        if (minN < 0 || (int)ints.size() >= minN) return NO_ERROR;

        CureCERR(20) << "WARNING: Found only " << ints.size()
                     << " ints for " << keyword << " near line " << lineno 
                     << " in file \"" << m_CfgFileName << "\"\n";
        return ERROR_PARSING;

      } else {
        CureCERR(20) << "WARNING: Error parsing for ints "
                     << "near line " << lineno 
                     << " in file \"" << m_CfgFileName << "\"\n";
        return ERROR_PARSING;
      }
    }
  }

  if (useIncludes) {
    // As a last resort we try to look for includes that might contain
    // the necessary information.
    SEARCH_INCLUDES_FILES(ERROR_DATA_NOT_FOUND, 
                          getIntList(keyword, useIncludes, ints, 
                                     usedCfgFile, minN),
                          "int list for " << keyword);
  }

  return ret;
}

int
ConfigFileReader::getDoubleList(const std::string &keyword, bool useIncludes,
                                std::list<double> &doubles,
                                std::string &usedCfgFile, int minN)
{
  doubles.clear();
  int lineno;
  int ret = searchKeyword(keyword, lineno);
  if (ret == ERROR_NO_SUCH_FILE) return ret;

  usedCfgFile = m_CfgFileName;

  if (!ret) {
    std::string line, tmp;
    double v;
    while(getline(*m_CfgFile, line)) {
      lineno++;
      std::istringstream str(line);
      CureCERR(70) << "Read line \"" << line << "\"\n";
      if ( !(str >> tmp) ) {
        CureCERR(20) << "WARNING: Empty line near line " << lineno 
                     << " looking for " << keyword
                     << " in file \"" << m_CfgFileName << "\"\n";
        return ERROR_EMPTY_LINE;
      } else if (tmp[0] == '#') {
        // Just skip as comment
      } else if (isdigit(tmp[0]) || tmp[0]=='.' || 
		 tmp[0] == '-' || tmp[0] == '+') {
        v = atof(tmp.c_str());
        doubles.push_back(v);
        while (str >> v) doubles.push_back(v);

        if (minN < 0 || (int)doubles.size() >= minN) return NO_ERROR;

        CureCERR(20) << "WARNING: Found only " << doubles.size()
                     << " doubles for " << keyword << " near line " << lineno 
                     << " in file \"" << m_CfgFileName << "\"\n";
        return ERROR_PARSING;
        
      } else {
        CureCERR(20) << "WARNING: Error parsing for doubles "
                     << "near line " << lineno 
                     << " in file \"" << m_CfgFileName << "\"\n";
        return ERROR_PARSING;
      }
    }
  }

  if (useIncludes) {
    // As a last resort we try to look for includes that might contain
    // the necessary information.
    SEARCH_INCLUDES_FILES(ERROR_DATA_NOT_FOUND, 
                          getDoubleList(keyword, useIncludes, doubles,
                                        usedCfgFile, minN),
                          "double list for " << keyword);
  }

  return ret;
}
 
int 
ConfigFileReader::getKeyedString(const std::string &keyword, bool useIncludes,
                                 int &key, std::string &s,
                                 std::string &usedCfgFile)
{
  int lineno;
  int ret = searchKeyword(keyword, lineno);
  if (ret == ERROR_NO_SUCH_FILE) return ret;
  
  usedCfgFile = m_CfgFileName;

  if (!ret) {
    std::string line, tmp;
    while(getline(*m_CfgFile, line)) {
      lineno++;
      std::istringstream str(line);
      CureCERR(70) << "Read line \"" << line << "\"\n";
      if ( !(str >> tmp) ) {
        CureCERR(20) << "WARNING: Empty line near line " << lineno 
                     << " looking for " << keyword
                     << " in file \"" << m_CfgFileName << "\"\n";
        return ERROR_EMPTY_LINE;
      } else if (isdigit(tmp[0])) {
        key = atoi(tmp.c_str());        
        
        // Remove any initial whitespace
        line = line.substr(line.find_first_not_of(" "));
        // Skip to next thing after first number
        line = line.substr(line.find(" "));
        // Remove any initial whitespace
        line = line.substr(line.find_first_not_of(" "));
        
        s = line;
        if (s.length() > 0) return NO_ERROR;

        CureCERR(20) << "WARNING: No string for key " << key
                     << " nead line " << lineno << " in file \""
                     << m_CfgFileName << "\"\n";
      }
    }
  }

  // As a last resort we try to look for includes that might contain
  // the necessary information.
  if (useIncludes) {
    SEARCH_INCLUDES_FILES(ERROR_DATA_NOT_FOUND, 
                          getKeyedString(keyword, useIncludes, key, s,
                                         usedCfgFile),
                          keyword);
    return ret;
  }
  

  return ERROR_DATA_NOT_FOUND;
}

int 
ConfigFileReader::getKeyedDoubles(const std::string &keyword, bool useIncludes,
                                  std::map<int, double> &v,
                                  std::string &usedCfgFile)
{
  v.clear();
  int lineno;
  int ret = searchKeyword(keyword, lineno);
  if (ret == ERROR_NO_SUCH_FILE) return ret;

  usedCfgFile = m_CfgFileName;

  if (!ret) {
    std::string line, tmp;
    int key;
    while(getline(*m_CfgFile, line)) {
      lineno++;
      std::istringstream str(line);
      CureCERR(70) << "Read line \"" << line << "\"\n";
      if ( !(str >> tmp) ) {
        if (v.empty()) {
          CureCERR(20) << "WARNING: Empty line near line " << lineno 
                       << " looking for " << keyword
                       << " in file \"" << m_CfgFileName << "\"\n";
          return ERROR_EMPTY_LINE;
        } else {
          return NO_ERROR;
        }
      } else if (isdigit(tmp[0])) {
        key = atoi(tmp.c_str());        

        double val;
        if (str >> val) {
          v.insert(std::make_pair(key, val));
        } else {
          CureCERR(20) << "WARNING: Failed to get double near line " << lineno 
                       << " looking for " << keyword
                       << " in file \"" << m_CfgFileName << "\"\n";
        }
      }
    }
  }

  if (v.empty()) {
    // As a last resort we try to look for includes that might contain
    // the necessary information.
    if (useIncludes) {
      SEARCH_INCLUDES_FILES(ERROR_DATA_NOT_FOUND, 
                            getKeyedDoubles(keyword, useIncludes, v,
                                            usedCfgFile),
                            keyword);
    }

    return ret;
  }

  return NO_ERROR;
}

int 
ConfigFileReader::getKeyedLongs(const std::string &keyword, bool useIncludes,
                                std::map<int, long> &v,
                                std::string &usedCfgFile)
{
  v.clear();
  int lineno;
  int ret = searchKeyword(keyword, lineno);
  if (ret == ERROR_NO_SUCH_FILE) return ret;

  usedCfgFile = m_CfgFileName;

  if (!ret) {
    std::string line, tmp;
    int key;
    while(getline(*m_CfgFile, line)) {
      lineno++;
      std::istringstream str(line);
      CureCERR(70) << "Read line \"" << line << "\"\n";
      if ( !(str >> tmp) ) {
        if (v.empty()) {
          CureCERR(20) << "WARNING: Empty line near line " << lineno 
                       << " looking for " << keyword
                       << " in file \"" << m_CfgFileName << "\"\n";
          return ERROR_EMPTY_LINE;
        } else {
          return NO_ERROR;
        }
      } else if (isdigit(tmp[0])) {
        key = atoi(tmp.c_str());        

        long val;
        if (str >> val) {
          v.insert(std::make_pair(key, val));
        } else {
          CureCERR(20) << "WARNING: Failed to get double near line " << lineno 
                       << " looking for " << keyword
                       << " in file \"" << m_CfgFileName << "\"\n";
        }
      }
    }
  }

  if (v.empty()) {
    // As a last resort we try to look for includes that might contain
    // the necessary information.
    if (useIncludes) {
      SEARCH_INCLUDES_FILES(ERROR_DATA_NOT_FOUND, 
                            getKeyedLongs(keyword, useIncludes, v,
                                          usedCfgFile),
                            keyword);
    }

    return ret;
  }

  return NO_ERROR;
}

int 
ConfigFileReader::getKeyedStrings(const std::string &keyword, bool useIncludes,
                                  std::map<int, std::string> &strings,
                                  std::string &usedCfgFile)
{
  strings.clear();
  int lineno;
  int ret = searchKeyword(keyword, lineno);
  if (ret == ERROR_NO_SUCH_FILE) return ret;

  usedCfgFile = m_CfgFileName;

  if (!ret) {
    std::string line, tmp;
    int type;
    while(getline(*m_CfgFile, line)) {
      lineno++;
      std::istringstream str(line);
      CureCERR(70) << "Read line \"" << line << "\"\n";
      if ( !(str >> tmp) ) {
        if (strings.empty()) {
          CureCERR(20) << "WARNING: Empty line near line " << lineno 
                       << " looking for " << keyword
                       << " in file \"" << m_CfgFileName << "\"\n";
          return ERROR_EMPTY_LINE;
        } else {
          return NO_ERROR;
        }
      } else if (isdigit(tmp[0])) {
        type = atoi(tmp.c_str());        

        // Remove any initial whitespace
        line = line.substr(line.find_first_not_of(" "));
        // Skip to next thing after first number
        line = line.substr(line.find(" "));
        // Remove any initial whitespace
        line = line.substr(line.find_first_not_of(" "));

        strings.insert(std::make_pair(type, line));
      }
    }
  }

  if (strings.empty()) {
    // As a last resort we try to look for includes that might contain
    // the necessary information.
    if (useIncludes) {
      SEARCH_INCLUDES_FILES(ERROR_DATA_NOT_FOUND, 
                            getKeyedStrings(keyword, useIncludes, strings,
                                            usedCfgFile),
                            keyword);
    }

    return ret;
  }

  return NO_ERROR;
}

int
ConfigFileReader::getParamStrings(const std::string &keyword, bool useIncludes,
                                  std::list<std::string> &paramsList,
                                  std::string &usedCfgFile)
{
  paramsList.clear();
  int lineno;
  int ret = searchKeyword(keyword, lineno);
  if (ret == ERROR_NO_SUCH_FILE) return ret;

  usedCfgFile = m_CfgFileName;

  if (!ret) {
    std::string line, tmp;
    while(getline(*m_CfgFile, line)) {
      lineno++;
      std::istringstream str(line);
      CureCERR(70) << "Read line \"" << line << "\"\n";
      if ( !(str >> tmp) ) {
        if (paramsList.empty()) {
          CureCERR(20) << "WARNING: Empty line near line " << lineno 
                       << " looking for " << keyword
                       << " in file \"" << m_CfgFileName << "\"\n";
          return ERROR_EMPTY_LINE;
        } else {
          return NO_ERROR;
        }
      } else if (tmp[0] != '#') {
        paramsList.push_back(line);
      }
    }
  }

  if (paramsList.empty()) {
    if (useIncludes) {
      // As a last resort we try to look for includes that might contain
      // the necessary information.
      SEARCH_INCLUDES_FILES(ERROR_DATA_NOT_FOUND, 
                            getParamStrings(keyword, useIncludes, paramsList,
                                            usedCfgFile),
                            "params list for " << keyword);
    }
    
    return ret;
  }

  return NO_ERROR;
}

int
ConfigFileReader::getIncludePath(std::string &path)
{
  path = "";
  std::string cfg;
  int ret = getString("INCLUDEPATH", false, path, cfg);
  if (ret) return ret;
  
  // Add an extra slash so that it is OK to write for example . for
  // the path
  path += "/";
  return ret;
}

int
ConfigFileReader::getIncludeFiles(std::list<std::string> &includes)
{  
  includes.clear();

  std::list<std::string> includes1;
  std::string cfg;
  int ret = getStringList("INCLUDE", false, includes1, cfg);

  if (ret) return ret;

  // All include files comes first without the include path
  for (std::list<std::string>::iterator i = includes1.begin();
       i != includes1.end(); i++) {
    includes.push_back(*i);
  }

  if (m_IncPathFromCmdLine != "" && m_IncPathFromCmdLine != "./") {
    // All include files with the same search path as the one
    // specified on the command line
    for (std::list<std::string>::iterator i = includes1.begin();
         i != includes1.end(); i++) {
      includes.push_back(m_IncPathFromCmdLine + *i);
    }
  }

  if (m_IncPathFromFirstFile != "" && m_IncPathFromFirstFile != "./") {
    // All include files with the extra include path specified by the
    // first config file
    for (std::list<std::string>::iterator i = includes1.begin();
         i != includes1.end(); i++) {
      includes.push_back(m_IncPathFromFirstFile + *i);
    }
  }

  return ret;
}

int
ConfigFileReader::getRobotName(std::string &name)
{
  CureCERR(60) << "Looking for robotname\n";
  std::string cfg;
  return getString("ROBOT", true, name, cfg);
}

int
ConfigFileReader::getRobotShape(int &type, std::string &params)
{
  CureCERR(60) << "Looking for robot shape\n";
  std::string cfg;
  return getKeyedString("ROBOTSHAPE", true, type, params, cfg);
}

int
ConfigFileReader::getExecMode(int &mode)
{
  CureCERR(40) << "Looking for exec mode\n";
  std::string cfg;
  return getInt("EXECMODE", true, mode, cfg);
}

int
ConfigFileReader::getDisplayMode(int &mode)
{
  CureCERR(40) << "Looking for display mode\n";
  std::string cfg;
  return getInt("DISPLAYMODE", true, mode, cfg);
}

int
ConfigFileReader::getRoboLookHost(std::string &rlHost)
{
  CureCERR(40) << "Looking for robolook host\n";
  std::string cfg;
  return getString("ROBOLOOK", true, rlHost, cfg);
}

int
ConfigFileReader::getMapDisplayOffset(double &x, double &y, double &theta)
{
  CureCERR(40) << "Looking for mapdisplayoffset\n";
  std::string cfg;
  std::list<double> dlist;
  int ret = getDoubleList("MAPDISPLAYOFFSET", true, dlist, cfg, 3);

  if (ret) return ret;

  std::list<double>::iterator di = dlist.begin();
  x     = *di; di++;
  y     = *di; di++;
  theta = *di; di++;

  return ret;
}

int
ConfigFileReader::getFeaturesToUse(std::list<int> &typeIDs)
{
  CureCERR(40) << "Looking for features to use\n";
  std::string cfg;
  return getIntList("FEATURES", true, typeIDs, cfg);
}

int 

ConfigFileReader::getUseFeature(int type, bool &used)
{
  CureCERR(40) << "Looking if feature " << type << " should be used\n";
  std::string cfg;
  std::list<int> typeIDs;
  int ret = getFeaturesToUse(typeIDs);

  if (ret) return ret;

  if (find(typeIDs.begin(), typeIDs.end(), type) != typeIDs.end()) used = true;
  else used = false;

  return 0;
}

int 
ConfigFileReader::getSensorPoses(std::list<SensorPose> &sensors)
{
  CureCERR(40) << "Looking for sensor poses\n";
  sensors.clear();
  std::string cfg;
  std::list< std::string> strings;
  int ret = getParamStrings("SENSORPOSES", true, strings, cfg);
  if (ret) return ret;
  std::string tmp;
  unsigned short type,id;
  double x[6];
  for (std::list<std::string>::iterator pi = strings.begin();
       pi != strings.end(); pi++) {    
    std::istringstream str(*pi);
    if (str >> type>>id >> x[0] >> x[1] >> x[2] >> x[3] >> x[4] >> x[5]) {
      Cure::SensorPose sp(type, id);
      sp.Pose3D::operator=(x);
      unsigned short covType=0;
      if (str>>covType) {
        sp.setCovType(covType);
	double d=0;
	if (str>>d) sp.Covariance=d;
	else 	sp.Covariance=1E-6;        
      }
      sensors.push_back(sp);
    } else {
      CureCERR(20) << "WARNING: Could not parse SENSORPOSE for sensor "
                   << *pi<< " in file \"" << cfg << "\"\n";

    }
  }

  if (sensors.empty()) {
    return ERROR_DATA_NOT_FOUND;
  }

  return NO_ERROR;

}
int 
ConfigFileReader::getSensorChains(std::list<unsigned short> &sensors)
{
  CureCERR(40) << "Looking for sensor chains\n";
  sensors.clear();
  std::string cfg;
  std::list<std::string> strings;
  int ret = getParamStrings("SENSORCHAINS", true, strings, cfg);
  if (ret) return ret;
  std::string tmp;
  for (std::list<std::string>::iterator pi = strings.begin();
       pi != strings.end(); pi++) {    
    std::istringstream str(*pi);
    //    CureCERR(60) << "Read line \"" << pi->second << "\"\n";
    unsigned short ver,type, id, x[4];
    if (str >>ver>> type>>id ){
      // unsigned short sp=((unsigned short)pi->first);
      // sensors.push_back(key);
      sensors.push_back(ver);
      sensors.push_back(type);
      sensors.push_back(id);
      if (!(str>> x[0] >> x[1] >> x[2] >> x[3])) {
	x[0]=0;
	x[1]=0;
	x[2]=0;
	x[3]=11;
      }
      sensors.push_back(x[0]);
      sensors.push_back(x[1]);
      sensors.push_back(x[2]);
      sensors.push_back(x[3]);
    } else {
      CureCERR(20) << "WARNING: Could not parse SENSORCHAIN for sensor "
                   << *pi << " in file \"" << cfg << "\"\n";
    }
  }
  return 0;
}

int 
ConfigFileReader::getSensorChain(int which,unsigned short chain[6])
{
  CureCERR(40) << "Looking for sensor chain " << which << "\n";
  std::list<unsigned short> chains;
  int ret = getSensorChains(chains);
  if (ret == ERROR_NO_SUCH_FILE) return ret;
  for (std::list<unsigned short>::iterator si = chains.begin();
       si != chains.end(); si++,si++,si++,si++,si++,si++,si++) {
    if (which==0){
      if (*si==1){
	for (int i=0; i<6; i++){
	  si++;
	  chain[i]=*si;
	}
	return NO_ERROR;
      }
      else {
	CureCERR(20) << "WARNING: Could not parse SENSORCHAINS for version "
		     <<*si;
	return ERROR_DATA_NOT_FOUND;
      }
    }
    which--;
  }
  return ERROR_DATA_NOT_FOUND;
}



int
ConfigFileReader::getSensorPose(int type, Cure::SensorPose &pose)
{
  CureCERR(40) << "Looking for sensor pose " << type << "\n";
  std::list<SensorPose> sensors;
  int ret = getSensorPoses(sensors);
  if (ret == ERROR_NO_SUCH_FILE) return ret;

  for (std::list<SensorPose>::iterator si = sensors.begin();
       si != sensors.end(); si++) {
    if (si->m_SensorType == type) {
      pose = *si;
      return NO_ERROR;
    }
  }
  // As a last resort we try to look for includes that might contain
  // the necessary information.
  SEARCH_INCLUDES_FILES(ERROR_DATA_NOT_FOUND, 
                        getSensorPose(type, pose),
                        "sensor pose");
  return ret;
}
int 
ConfigFileReader::getSensorPose(Cure::SensorPose &pose)
{
  CureCERR(40) << "Looking for sensor pose, type=" 
               << pose.m_SensorType << " id="
               << pose.m_SensorID << std::endl;
  std::list<SensorPose> sensors;
  int ret = getSensorPoses(sensors);
  if (ret == ERROR_NO_SUCH_FILE) return ret;

  for (std::list<SensorPose>::iterator si = sensors.begin();
       si != sensors.end(); si++) {
    if (si->m_SensorType == pose.m_SensorType)
      if (si->m_SensorID == pose.m_SensorID)
	{
	  pose = *si;
	  return NO_ERROR;
	}
  }


  // As a last resort we try to look for includes that might contain
  // the necessary information.
  SEARCH_INCLUDES_FILES(ERROR_DATA_NOT_FOUND, 
                        getSensorPose(pose),
                        "sensor pose");
  
  return ret;
}

int 
ConfigFileReader::getOdomModel(int &type, std::string &params)
{
  CureCERR(40) << "Looking for odom model\n";
  std::string cfg;
  return getKeyedString("ODOMMODEL", true, type, params, cfg);
}
int 
ConfigFileReader::getInertialModel(int &type, std::string &params)
{
  CureCERR(40) << "Looking for initerial model\n";
  std::string cfg;
  return getKeyedString("INERTIALMODEL", true, type, params, cfg);
}

int
ConfigFileReader::getFilePath(std::string &filePath)
{
  CureCERR(40) << "Looking for file path\n";
  filePath = "";
  std::string cfg;
  return getString("FILEPATH", true, filePath, cfg);
}

int
ConfigFileReader::getAdaBoostHypFilePath(std::string &cPath)
{
  CureCERR(40) << "Looking for adaboth file path\n";
  cPath = "";
  std::string cfg;
  return getString("ADABOOSTHYP_FILEPATH", true, cPath, cfg);
}

int 
ConfigFileReader::getFiles(const std::string &keyword,
                           std::map<int, std::string> &files)
{
  CureCERR(40) << "Looking for files\n";
  std::map<int, std::string> files1;
  std::string cfg;
  int ret = getKeyedStrings(keyword, true, files1, cfg);
  if (ret) return ret;


  // Do postprocessing of the filename strings
  for (std::map<int, std::string>::iterator i = files1.begin();
       i != files1.end(); i++) {

    // check if this key already has a fileame assigned to it
    bool duplicated = false;
    for (std::map<int, std::string>::iterator j = files.begin();
         j != files.end(); j++) {
      if (i->first == j->first) {
        CureCERR(20) << "WARNING: key=" << i->first 
                     << " already given filename in \"" << cfg << "\"\n";
        duplicated = true;
        break;
      }
    }

    if (!duplicated) {
      // Make sure that there is only a single file in the string and
      // nothing else
      std::istringstream str(i->second);
      std::string filename;
      if (str >> filename) {
        files.insert(std::make_pair(i->first, filename));
      }
    }
  }

  if (files.empty()) {
    return ERROR_DATA_NOT_FOUND;
  }

  CureDO(40) {
    CureCERR(0) << "Files found for keyword \"" << keyword << "\"\n";
    for (std::map<int, std::string>::iterator fi = files.begin();
         fi != files.end(); fi++) {
      CureCERR(0) << "key=" << fi->first << " \""
                  << fi->second << "\"\n";
    }
  }

  return NO_ERROR;
}

int 
ConfigFileReader::getDelays(const std::string &keyword,
                            std::map<int, double> &delays)
{
  CureCERR(40) << "Looking for delays\n";
  std::string cfg;
  return getKeyedDoubles(keyword, true, delays, cfg);
}

int
ConfigFileReader::getFile(const std::string &keyword,
                           int key, std::string &filename)
{
  CureCERR(40) << "Looking for file key=" << key << "\n";
  std::map<int, std::string> files;
  int ret = getFiles(keyword, files);
  if (ret == ERROR_NO_SUCH_FILE) return ret;

  std::map<int, std::string>::iterator fi;
  fi = files.find(key);
  if (ret || fi == files.end()) {
    // As a last resort we try to look for includes that might contain
    // the necessary information.
    SEARCH_INCLUDES_FILES(ERROR_DATA_NOT_FOUND, 
                          getFile(keyword, key, filename),
                          "file");
    return ret;
  }

  filename = fi->second;
  return NO_ERROR;
}

int
ConfigFileReader::getDelay(const std::string &keyword,
                           int key, double &delay)
{
  CureCERR(40) << "Looking for delay key=" << key << "\n";
  std::map<int, double> delays;
  int ret = getDelays(keyword, delays);
  if (ret == ERROR_NO_SUCH_FILE) return ret;

  std::map<int, double>::iterator di;
  di = delays.find(key);
  if (ret || di == delays.end()) {
    // As a last resort we try to look for includes that might contain
    // the necessary information.
    SEARCH_INCLUDES_FILES(ERROR_DATA_NOT_FOUND, 
                          getDelay(keyword, key, delay),
                          "delay");
    return ret;
  }

  delay = di->second;
  return NO_ERROR;
}

int
ConfigFileReader::getSensorFiles(std::map<int, std::string> &files)
{
  CureCERR(40) << "Looking for sensor files\n";
  int ret = getFiles("SENSORFILES", files);

  if (ret) {
    // As a last resort we try to look for includes that might contain
    // the necessary information.
    SEARCH_INCLUDES_FILES(ret, getSensorFiles(files), "sensor files");
  }

  return ret;
}

int
ConfigFileReader::getAdaBoostHypFiles(std::map<int, std::string> &files)
{
  CureCERR(40) << "Looking for adaboost files\n";
  int ret = getFiles("ADABOOSTHYP_FILES", files);

  if (ret) {
    // As a last resort we try to look for includes that might contain
    // the necessary information.
    SEARCH_INCLUDES_FILES(ret, getAdaBoostHypFiles(files), "adaboost files");
  }

  return ret;
}

int
ConfigFileReader::getFeatureFiles(std::map<int, std::string> &files)
{
  CureCERR(40) << "Looking for feature files\n";
  int ret = getFiles("FEATUREFILES", files);

  if (ret) {
    // As a last resort we try to look for includes that might contain
    // the necessary information.
    SEARCH_INCLUDES_FILES(ret, getFeatureFiles(files), "feature files");
  }

  return ret;
}

int
ConfigFileReader::getSensorFile(int key, std::string &filename)
{
  CureCERR(40) << "Looking for sensor file " << key << "\n";
  int ret = getFile("SENSORFILES", key, filename);

  if (ret) {
    // As a last resort we try to look for includes that might contain
    // the necessary information.
    SEARCH_INCLUDES_FILES(ret, getSensorFile(key, filename), "sensor file");
  }

  return ret;
}

int
ConfigFileReader::getFeatureFile(int key, std::string &filename)
{
  CureCERR(40) << "Looking for feature file " << key << "\n";
  int ret = getFile("FEATUREFILES", key, filename);

  if (ret) {
    // As a last resort we try to look for includes that might contain
    // the necessary information.
    SEARCH_INCLUDES_FILES(ret, getFeatureFile(key, filename), "feature file");
  }

  return ret;
}

int
ConfigFileReader::getAdaBoostHypFile(int key, std::string &filename)
{
  CureCERR(40) << "Looking for feature file " << key << "\n";
  int ret = getFile("ADABOOSTHYP_FILES", key, filename);

  if (ret) {
    // As a last resort we try to look for includes that might contain
    // the necessary information.
    SEARCH_INCLUDES_FILES(ret, getFeatureFile(key, filename), "adaboost file");
  }

  return ret;
}

int 
ConfigFileReader::getSensorDelays(std::map<int, double> &delays)
{
  CureCERR(40) << "Looking for sensor delays\n";
  int ret = getDelays("SENSORDELAYS", delays);

  if (ret) {
    // As a last resort we try to look for includes that might contain
    // the necessary information.
    SEARCH_INCLUDES_FILES(ret, getSensorDelays(delays), "sensor delays");
  } 
  
  return ret;
}

int 
ConfigFileReader::getFeatureDelays(std::map<int, double> &delays)
{
  CureCERR(40) << "Looking for feature delays\n";
  int ret = getDelays("FEATUREDELAYS", delays);

  if (ret) {
    // As a last resort we try to look for includes that might contain
    // the necessary information.
    SEARCH_INCLUDES_FILES(ret, getFeatureDelays(delays), "feature delays");
  }

  return ret;
}

int
ConfigFileReader::getSensorDelay(int key, double &delay)
{
  CureCERR(40) << "Looking for sensor delay " << key << "\n";
  int ret = getDelay("SENSORDELAYS", key, delay);

  if (ret) {
    // As a last resort we try to look for includes that might contain
    // the necessary information.
    SEARCH_INCLUDES_FILES(ret, getSensorDelay(key, delay), "sensor delay");
  }

  return ret;
}

int
ConfigFileReader::getFeatureDelay(int key, double &delay)
{
  CureCERR(40) << "Looking for feature delay " << key << "\n";
  int ret = getDelay("FEATUREDELAYS", key, delay);

  if (ret) {
    // As a last resort we try to look for includes that might contain
    // the necessary information.
    SEARCH_INCLUDES_FILES(ret, getFeatureDelay(key, delay), "feature delay");
  }

  return ret;
}

int 
ConfigFileReader::getHelpersConfig(const std::string &subconfig,
                                   std::map<int, std::string> &config)
{
  CureCERR(40) << "Looking for helper config " << subconfig << "\n";
  std::string cfg;
  config.clear();
  return getKeyedStrings(subconfig, true, config, cfg);
}

int
ConfigFileReader::getHelperConfig(int key, const std::string &subconfig,
                                  std::string &arglist)
{
  CureCERR(40) << "Looking for helper config for " << key
               << " in file " << m_CfgFileName
               << std::endl;

  std::map<int, std::string> config;
  int ret = getHelpersConfig(subconfig, config);
  if (ret == ERROR_NO_SUCH_FILE) return ret;
  
  CureCERR(40) << "ret=" << ret << " config.size="
               << config.size() << std::endl;

  std::map<int, std::string>::iterator hi;
  hi = config.find(key);
  if (ret || hi == config.end()) {
    CureCERR(40) << "did not find key " << key << std::endl;
    // As a last resort we try to look for includes that might contain
    // the necessary information.
    SEARCH_INCLUDES_FILES(ERROR_DATA_NOT_FOUND,
                          getHelperConfig(key, subconfig, arglist),
                          "helper config");
    return ret;
  }
  
  CureCERR(40) << "Found subconfig " << subconfig << " for helper " 
               << key << "\n";
  arglist = hi->second;
  return NO_ERROR;
}

int 
ConfigFileReader::getHelpersConfig(int subconfig,
                                   std::map<int, std::string> &config)
{
  CureCERR(40) << "Looking for helper config " << subconfig << std::endl;
  switch (subconfig) {
  case 0:
    return getHelpersConfig("HELPERS", config);
  case 1:
    return getHelpersConfig("FEATUREMATCHING", config);
  case 2:
    return getHelpersConfig("FEATUREINITIALIZATION", config);
  case 3:
    return getHelpersConfig("FEATUREMERGING", config);
  case 4:
    return getHelpersConfig("FEATURESENSORPARAMS", config);
  default:
    CureCERR(20) << "WARNING: Unknown subconfig " << subconfig
                 << " for Helpers\n";
    break;
  }

  // If we get this far something is wrong
  return ERROR_DATA_NOT_FOUND;
}

int
ConfigFileReader::getHelperConfig(int key, int subconfig, 
                                  std::string &arglist)
{
  CureCERR(40) << "Looking for helper config for " << key
               << " in file " << m_CfgFileName
               << std::endl;

  std::map<int, std::string> config;
  int ret = getHelpersConfig(subconfig, config);
  if (ret == ERROR_NO_SUCH_FILE) return ret;
  
  CureCERR(40) << "ret=" << ret << " config.size="
               << config.size() << std::endl;

  std::map<int, std::string>::iterator hi;
  hi = config.find(key);
  if (ret || hi == config.end()) {
    CureCERR(40) << "did not find key " << key << std::endl;
    // As a last resort we try to look for includes that might contain
    // the necessary information.
    SEARCH_INCLUDES_FILES(ERROR_DATA_NOT_FOUND,
                          getHelperConfig(key, subconfig, arglist),
                          "helper config");
    return ret;
  }
  
  CureCERR(40) << "Found subconfig " << subconfig << " for helper "
               << key << "\n";
  arglist = hi->second;
  return NO_ERROR;
}

int 
ConfigFileReader::getVisionExtractorsConfig(std::map<int, std::string> &config)
{
  CureCERR(40) << "Looking for vision extractror config\n";
  config.clear();
  std::string cfg;
  return getKeyedStrings("VISIONEXTRACTORS", true, config, cfg);
}

int
ConfigFileReader::getVisionExtractorConfig(int key, std::string &arglist)
{
  CureCERR(40) << "Looking for vision extractror config " << key << "\n"; 
  std::map<int, std::string> config;
  int ret = getVisionExtractorsConfig(config);
  if (ret == ERROR_NO_SUCH_FILE) return ret;

  std::map<int, std::string>::iterator hi;
  hi = config.find(key);
  if (ret || hi == config.end()) {
    // As a last resort we try to look for includes that might contain
    // the necessary information.
    SEARCH_INCLUDES_FILES(ERROR_DATA_NOT_FOUND,
                          getVisionExtractorConfig(key, arglist),
                          "vision extractor config");
    return ret;
  }

  arglist = hi->second;
  return NO_ERROR;
}

int
ConfigFileReader::getRangeHoughConfig(int subconfig, std::string &arglist)
{
  CureCERR(40) << "Looking for range hough config " << subconfig << "\n";
  arglist = "";
  std::string cfg;
  switch(subconfig) {
  case 0:
    return getParamString("RANGEHOUGH", true, arglist, cfg);
  case 1:
    return getParamString("SCANPARAMETERS", true, arglist, cfg);
  case 2:
    return getParamString("LINEPARAMETERS", true, arglist, cfg);
  default:
    CureCERR(20) << "WARNING: Unknown subconfig " << subconfig
                 << " for RangeHough\n";
    break;
  }

  // If we got ths far we did not find any config
  return ERROR_DATA_NOT_FOUND;
}

int 
ConfigFileReader::getMahalanobisDists(std::map<int, double> &mdists)
{
  CureCERR(40) << "Looking for mahalanobis distances\n";
  mdists.clear();
  std::string cfg;
  return getKeyedDoubles("MAHALANOBIS", true, mdists, cfg);
}

int
ConfigFileReader::getMahalanobisDist(int key, double &d)
{
  CureCERR(40) << "Looking for mahalanobis distance " << key << "\n";
  std::map<int, double> mdists;
  int ret = getMahalanobisDists(mdists);
  if (ret == ERROR_NO_SUCH_FILE) return ret;

  std::map<int, double>::iterator mi;
  mi = mdists.find(key);
  if (ret || mi == mdists.end()) {
    // As a last resort we try to look for includes that might contain
    // the necessary information.
    SEARCH_INCLUDES_FILES(ERROR_DATA_NOT_FOUND, 
                          getMahalanobisDist(key, d),
                          "mahalanobis dist");
    return ret;
  }

  d = mi->second;
  return NO_ERROR;
}

int
ConfigFileReader::getRobotPose(Pose3D &pose)
{
  CureCERR(40) << "Looking for robot pose\n";
  std::string cfg;
  std::list<double> dlist;
  int ret = getDoubleList("ROBOTPOSE", true, dlist, cfg, 6);
  double x[6];
  if (ret) return ret;

  std::list<double>::iterator di = dlist.begin();
  x[0]  = *di; di++;
  x[1]  = *di; di++;
  x[2]  = *di; di++;
  x[3]  = *di; di++;
  x[4]  = *di; di++;
  x[5]  = *di; di++;
  pose=x;
  if (dlist.size()>6)
    pose.setCovType((short)*di);
  else return ret;
  di++;
  if (dlist.size()>7)
    pose.Covariance=(double)*di;
  else return ret;
  di++;
  if (dlist.size()>8)
    pose.setVelType((short)*di);
  return ret;
}
int
ConfigFileReader::getDescritptor(DataDescriptor &des)
{
  std::string desarglist="";
  std::string desparams;
  std::list<std::string> strings;
  if (getParamStrings("DATADESCRIPTORS", true, strings, desarglist))return 1;
  for (std::list<std::string>::iterator pi = strings.begin();
       pi != strings.end(); pi++) {       
    std::istringstream strdes(*pi);
    std::string match;
    if ((strdes>>match)){
      if (match==des.Name){
	int r=des.setTo((*pi),match);
	  if (r>-1)return r;
	if (!getSensor(match)){
	   std::istringstream strde(match);
	   if(strde>>match>>match>>des.SensorID){
	     return getSensorType(match,des.SensorType);
	   }
	   
	}	  
	
      }
    }
  }
  return 1;
}
int
ConfigFileReader::getSensor(std::string &tmp)
{
  std::string desarglist="";
  std::string desparams;
  std::list<std::string> strings;
  if (getParamStrings("SENSORS", true, strings, desarglist))return 1;
  for (std::list<std::string>::iterator pi = strings.begin();
       pi != strings.end(); pi++) {       
    std::istringstream strdes(*pi);
    std::string match;
    if ((strdes>>match)){
      if (match==tmp){
	tmp=(*pi); 
	return 0;
	}
      
    }
  }
  return 1;
}
int
ConfigFileReader::getGlobal(std::string &tmp, int &startkey)
{
  startkey=1;
  std::string desarglist="";
  std::string desparams;
  std::list<std::string> strings;
  if (getParamStrings("GLOBAL", true, strings, desarglist))return 1;
  for (std::list<std::string>::iterator pi = strings.begin();
       pi != strings.end(); pi++) {       
    std::istringstream strdes(*pi);
    std::string match;
    if ((strdes>>match)){
      if (match==tmp){
	  tmp=(*pi); 
	  return 0;
	}
      while ((strdes>>match))startkey++;
    }
  }
  return 1;
}
int
ConfigFileReader::getNumberOfObjects( const std::string &objectbasetype,
				      int &numberOfObjects)
{
  std::string arglist="";
  std::list<std::string> strings;
  if (getParamStrings(objectbasetype, true, strings, arglist))return 1;
  numberOfObjects=strings.size();;
  return 0;
}

int
ConfigFileReader::getConstructorInfo(const std::string objectname,
				     std::string &classname,
				     std::string &configline)
{
  std::string arglist="";
  std::list<std::string> strings;
  std::string cmd;
  if (getParamStrings(objectname, true, strings, arglist))return 1;
  for (std::list<std::string>::iterator pi = strings.begin();
       pi != strings.end(); pi++) {       
    std::istringstream strdes(*pi);
    if (!(strdes>>cmd))return 1;
	if ((cmd=="Create")||(cmd=="create")||
	    (cmd=="Construct")||(cmd=="construct")||
	    (cmd=="Constructor")||(cmd=="constructor"))
	  {
	  configline=(*pi);
	  if (!(strdes>>classname))return 1;
	  return 0;
	}
  }
  return 1;
}

} // namespace Cure
