//
// = LIBRARY
//
// = FILENAME
//    Command.cc
//
// = AUTHOR(S)
//    john Folkesson
//
// = COPYRIGHT
//    Copyright (c) 2005 John Folkesson
//
/*----------------------------------------------------------------------*/

#include "Command.hh"

#ifndef DEPEND
#include <string.h>  // memcpy
#include <time.h>    // time
#include <stdio.h>   // fprintf
#endif

namespace Cure {

Command::Command(unsigned short id)
{
  init();
  setID(id);
}

Command::Command(Timestamp &t, unsigned short id)
{
  init();
  Time=t;
  setID(id);
}

/**
 * Create a copy of a Command
 */
Command::Command(const Command &src)
{
  init();
  (*this) = src;
}

Command::~Command()
{}

void Command::init()
{
  m_Packed =false;
  m_ClassType=(COMMAND_TYPE);
  setSubType(0);
  setID(0);
  CmdType = 0;
  Service = 0;
  Sender = 0;
  Priority=0;
  ReplyAddress=0;
  CmdID=0;
  ReplyID=0;
}

Command& Command::operator=(const Command &src)
{
  Time=src.Time;
  setSubType(src.getSubType());
  setID(src.getID());
  CmdType = src.CmdType;
  Service = src.Service;
  Priority = src.Priority;
  Sender=src.Sender;
  CmdData=src.CmdData;
  CmdFlags=src.CmdFlags;
  ReplyAddress=src.ReplyAddress;
  CmdID=src.CmdID;
  ReplyID=src.ReplyID;
 return (*this);
}


void Command::setSubType(unsigned short t)
{
  m_SubType = t; 
  if (t!=(CmdFlags.Rows*CmdFlags.Columns))
      CmdFlags.reallocate(1,t);
}

void Command::print()
{
  Time.print();
  std::cerr<< "cmdType=" << getCmdType()<< std::endl
     << "Service=" << getService() << std::endl
     << "Priority=" << Priority << std::endl
     << "Sender=" << Sender << std::endl
     << "CmdId=" << CmdID << std::endl
     << "ReplyID=" << ReplyID << std::endl;
  std::cerr<<"CmdData= ";
    CmdData.print();
    std::cerr<<"CmdFlags= ";
    CmdFlags.print();
}

} // namespace Cure

std::ostream& 
operator << (std::ostream& os, const Cure::Command &cmd)
{
  os << "cmdType=" << cmd.getCmdType()<< std::endl
     << "Service=" << cmd.getService() << std::endl
     << "Priority=" << cmd.Priority << std::endl
     << "Sender=" << cmd.Sender << std::endl
     << "CmdId=" << cmd.CmdID << std::endl
     << "ReplyID=" << cmd.ReplyID << std::endl;
  return os;
}
