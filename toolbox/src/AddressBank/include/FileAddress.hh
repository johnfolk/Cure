//    $Id: FileAddress.hh,v 1.15 2007/09/16 09:04:42 johnf Exp $
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2004 John Folkesson
//    

#ifndef CURE_FILEADDRESS_HH
#define CURE_FILEADDRESS_HH


#include "AddressBank.hh"
#include "Address.hh"
#include "SmartData.hh"
#include <fstream>
#include <string>

namespace Cure{

/**
 * A FileAddress is a Address that reads and writes from/to a file.
 * 
 *
 * The idea is that one creates a FileAddress, calls openRead or OpenWrite
 * Then calls Read or Write.
 * The File is closed on deleting the FileAddress.
 * The File is read sequentially from the start to the end. 
 * Upon reaching eof the ReadFile is closed and
 * reading returns ADDRESS_INVALID on further reads.  
 * @author John Folkesson
 */
  class FileAddress: public Address
{
public:
  std::fstream ReadFile;
  std::fstream WriteFile;
   SmartData TPoint;
  /**  
   * If set this will only write data that matches the Descriptor and
   * read until it finds matching data or eof.
   */
  bool m_UseDescriptor;
  bool m_RowWise;
  std::string m_Header;
  short m_NumberOfStrings;
  ShortMatrix m_MessageIndex[10];
  std::string m_MessageString[10];
  bool m_IncludeTime;
  Timestamp m_TimeOffset;  
  short m_SignificantFigures;
public:

  /**
   * Constructor that does nto really do any thing. You wil lhave to
   * call OpenReadFile or OpenWriteFile to open a file for reading or
   * writing.
   */
  FileAddress(const unsigned short thread=0);

  /**
   * Destructor
   */
  ~FileAddress();

  /** 
   * @return name of file that is opened for reading/writing if any
   */
  const std::string& getFilename() const;

  /**
   * This will write a TimestampedData 'p' to the file.
   *
   * If the FileAddress is somehow not pointing to anything writable
   * return (RESOURCE_ERROR|ADDRESS_INVALID);
   *
   * @param p  to be copied
   * @return 0 if ok, else (RESOURCE_ERROR|ADDRESS_INVALID);
   */
  virtual unsigned short write(TimestampedData& p);
  
  virtual int configure(std::string param);
  /**
   * if t==0 return the next  from file
   * else return TIMESTAMP_ERROR.
   * 
   * If the FileAddress is somehow not pointing to anything readable
   * return (RESOURCE_ERROR|ADDRESS_INVALID).
   *
   * Note that one could make this do interpolate ... but we have no 
   * implemented that yet. 
   *
   * @param result object  to be copied into
   * @param t requested time must be 0 (the default)
   * @param interpolate flag is ignored
   * @return 0 if ok, else (RESOURCE_ERROR|ADDRESS_INVALID)
   *                  NO_INTERRPOLATE, TIMESTAMP_ERROR.
   */
  virtual unsigned short read(TimestampedData& result, const Timestamp t=0,
	   const int interpolate=0);

  /*
   * Returns 0 if any file is open either for read or write else error code.
   * @return 0 if ok, else (RESOURCE_ERROR|ADDRESS_INVALID);
   */
  unsigned short query();  
  /**
   * Set how many digits to save when writing doubles.
   * @param sigfig the number of digits in the doubles written to files.
   */
  void setSignificantFigures(short sigfig){
    if (sigfig>18)sigfig=18;
    if (sigfig<1)sigfig=1;
    m_SignificantFigures=sigfig;
  }
  /** 
   * Open a file for readings. The file obviously have to exist to do this
   * @return 0 if ok, else RESOURCE_ERROR
   */
  virtual unsigned short openReadFile(const std::string &name);

  /** 
   * Open a file for writing. If a file already exists it will be
   * overwritten.
   * @return 0 if ok, else RESOURCE_ERROR
   */
  virtual unsigned short openWriteFile(const std::string &name);

  /**
   * Close a file opened for reading
   */
  virtual void closeReadFile();

  /**
   * Close a file opened for writing
   */
  virtual void closeWriteFile();
  /** 
   * This will also set UseDescriptor to true.  That will
   * then copy the Descriptor to all data read from or writen to file.
   */
  void setDescriptor(DataDescriptor &d);

  bool operator == (const std::string name){return (m_Descriptor.Name==name);}

  void printConfig();
  void writeHeader();
protected:
  
  /**
   * Function to read from a file not available for users
   */
  virtual unsigned short read(const Timestamp t=0, const int interpolate=0);

  /** True if a file is open */
  bool m_FileOpened;

  /** stores the name of the last file opened */
  std::string m_Filename; 
};

inline const std::string& 
FileAddress::getFilename() const { return m_Filename; }

} // namespace Cure

#endif 
