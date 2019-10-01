//
// = LIBRARY
//
// = FILENAME
//    VisionData.cc
//
// = AUTHOR(S)
//    john Folkesson
//
// = COPYRIGHT
//    Copyright (c) 2007 John Folkesson
//
/*----------------------------------------------------------------------*/

#include "VisionData.hh"

namespace Cure {

void VisionData::print()
{
  Time.print();
  std::cerr<< "CameraType=" << m_CameraType<<" ";
  std::cerr<< "FrameNumber=" << m_FrameNumber<< std::endl;
}
} // namespace Cure

std::ostream& 
operator << (std::ostream& os, const Cure::VisionData &cmd)
{
  os << "CameraType=" << cmd.m_CameraType<< std::endl;
  return os;
}
