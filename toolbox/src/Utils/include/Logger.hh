//   
//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2007 John Folkesson
//    

#ifndef CURE_LOGGER_HH
#define CURE_LOGGER_HH

#include "CureDebug.hh"
#include "MutexWrapper.hh"
#include <fstream>
#include <string>

namespace Cure{



   /** Logs a  string l to the log file*/
#define LOG(l)theLogger().log(l);

  /** Logs a  string l and then n doubles d[i] to the log file*/
#define LOGDOUBLE(l,d,n)theLogger().log(l,d,n);

#define LOGFLUSH()theLogger().flush();
  /**
   *
   * @author John Folkesson
   * 
   * This is a simple logger that the entire process can access 
   * forming a single timestamped log file in the true sequence of
   * the execution.  The initialization of this is to call 
   * theLogger(filename,filepath) at the start of the program, 
   * before anyone gets a chance to try to log.  If you don't 
   * bother with this init the logfile will use its default name and 
   * path.  
   *
   * Thats the hardest part.  Then you call these two macros from 
   * your programs to log strings and strings appended with doubles.
   *
   *
   */
  class Logger
  {
  protected:
    MutexWrapper m_Mutex;
    std::fstream m_LogFile;
    bool m_FileOpened;
  public:
    
    Logger(const std::string filename, const std::string filepath)
    {
      std::string stime;
      char temp[200]; 
      time_t t; 
      struct tm st;
      time(&t); 
      gmtime_r(&t, &st);
      strftime(temp, 100, "%Y-%m-%d_%H_%M_%S", &st);
      stime=temp;
      m_FileOpened = false;
      std::string name=filepath+ "/";
      if (filename=="-") name=name+"Curelogfile";
      else name=name=filename;
      name=name+stime+".log";			   
      m_FileOpened = false;
      m_LogFile.open(name.c_str(),std::ios::out);
      if (m_LogFile.good()) {
	m_FileOpened = true;
	m_LogFile<<Cure::getCureCERRTimeString()<< 
	  ": Opened LogFile"<<name<<"\n";	
	std::cerr << "Logger opened WriteFile " << name << " sucess \n";
      }else{
	std::cerr << "Logger failed to open WriteFile \"" << name << "\"\n";
      }
    }
    ~Logger(){
      if (m_FileOpened)m_LogFile.close();
      m_FileOpened = false;
     std::cerr<<"Logger Deleted\n";
  }
    void flush(){
      if (!m_FileOpened)return;
      m_Mutex.lock();
      m_LogFile.flush();
      m_Mutex.unlock();      
    }

    void log(const std::string strg)
    {
      if (!m_FileOpened)return;
      m_Mutex.lock();
      m_LogFile<<Cure::getCureCERRTimeString() <<": "<<strg<<"\n"; 
      m_Mutex.unlock();
    }
    void log(const std::string strg, double *d, const unsigned short n=0)
    {
      if (!m_FileOpened)return;
      m_Mutex.lock();
      m_LogFile<<Cure::getCureCERRTimeString() <<": "<<strg;
      for (int i=0;i<n; i++)
	m_LogFile<<" "<<d[i];
      m_LogFile<<"\n";
      m_Mutex.unlock();
    }
  /**
   * This is the default Logger for the process.  It is set up at 
   * the first call to this static function.
   *
   * This stange way of doing it assure that as long as no one calls
   * this static member the object will not be made.
   * The first time this is called the one and only s_TheLogger
   * object is made.
   *
   * @param filename is the base ename and must be passed at the 
   *        at the first call.  The unique time string will be 
   *        appended as will the .log extension.
   *  @param filepath default is "." .
   */
  static Logger& theLogger(const std::string filename="-", 
			   const std::string filepath="."){
    static Logger s_TheLogger(filename,filepath); 
    return s_TheLogger;
  }
private:
};
} // namespace Cure

#endif // CURE_ADDRESSBANK_HH
