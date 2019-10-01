//
// = LIBRARY
//
// = FILENAME
//    SensorOffset.cc
//
// = AUTHOR(S)
//    John Folkesson
//
// = COPYRIGHT
//    Copyright (c) 2006 john Folkesson
//
/*----------------------------------------------------------------------*/

#include "SensorOffset.hh"
#include "SensorData.hh"
#include "StateData.hh"

#ifndef DEPEND
#endif

namespace Cure {

SensorOffset::SensorOffset()
{
  m_Name="NoName";
  m_Offset=0;
  m_PartialSum=0;
  m_NumberOfOffsets=0;
  m_Jacobian.reallocate(6,12);
}

SensorOffset::SensorOffset(const SensorOffset &src)
{

  (*this) = src;
}
SensorOffset& SensorOffset::operator=(const SensorOffset &src)
{
  m_RobotSensorPose=src.m_RobotSensorPose;
  setNumberOfOffsets(src.m_NumberOfOffsets);
  for (int i=2;i<m_NumberOfOffsets;i++){
    m_Offset[i]=src.m_Offset[i];
    m_PartialSum[i]=src.m_PartialSum[i];
  }
  if (m_NumberOfOffsets==2){
    m_Offset[1]=src.m_Offset[1];
    m_Offset[0]=src.m_Offset[0];
  }  
  m_Jacobian=src.m_Jacobian;
  return (*this);
}

SensorOffset::~SensorOffset()
{
  if (m_Offset)delete[]m_Offset;
  if (m_PartialSum)delete[]m_PartialSum;
}
void SensorOffset::setNumberOfOffsets(unsigned short num)
{
  if (num==m_NumberOfOffsets)return;
  if (num<m_NumberOfOffsets){
    m_NumberOfOffsets=num;
    return;
  }
  Pose3D *off=m_Offset;
  Pose3D *ps=m_PartialSum;
  m_Offset=0;
  m_PartialSum=0;
  if (num>0)m_Offset=new Pose3D[num];  
  if (num>2) m_PartialSum=new Pose3D[num-2];
  for (int i=0;i<m_NumberOfOffsets-2;i++){
    m_PartialSum[i]=ps[i];
    m_Offset[i]=off[i];
  }
  for (int i=m_NumberOfOffsets-2;i<num-2;i++)
    m_PartialSum[i].setCovType(63);
  if (m_NumberOfOffsets>1){
    m_Offset[m_NumberOfOffsets-2]=off[m_NumberOfOffsets-2];
    m_Offset[m_NumberOfOffsets-1]=off[m_NumberOfOffsets-1];
  }
  if (off)delete[]off;
  if (ps)delete[]ps;
  m_NumberOfOffsets=num;
}
/*
int SensorOffset::setupPose(std::istringstream &strdes,
			    bool notoffset, int offset)
{
  if (notoffset){
    m_RobotSensorPose.setupPose(strdes);
    return 0;
  }
  if (offset<0){
    setNumberOfOffsets(m_NumberOfOffsets+1);
    m_Offset[m_NumberOfOffsets-1].setupPose(strdes);
    return 0;
  }
  if (offset>=m_NumberOfOffsets)
    setNumberOfOffsets(offset+1);
  m_Offset[offset].setupPose(strdes);
  return 0;
}
*/
int SensorOffset::setupPose(StateData &p,unsigned short type,bool notoffset, int offset)
{
  if (notoffset){
    p.setPose(m_RobotSensorPose,type);
    return -1;
  }
  if (offset<0){
    setNumberOfOffsets(m_NumberOfOffsets+1);
    offset =m_NumberOfOffsets-1;
  }
  if (offset>=m_NumberOfOffsets)
    setNumberOfOffsets(offset+1);
  p.setPose(m_Offset[offset],type);
  return offset;
}

/*
int SensorOffset::configSensorOffset(const std::string &param, 
				     const std::string &sensordef)
{

  std::string cmd;
  std::istringstream strs(sensordef);
  if((strs>>cmd)){
    m_Name=cmd;
    if((strs>>cmd)){
      unsigned short type,id;
      if (Cure::getSensorType(cmd,type)){
	m_RobotSensorPose.m_SensorType=type;
	if((strs>>id)){
	  m_RobotSensorPose.m_SensorID=id;      
	  m_RobotSensorPose.setupPose(strs);
	}
      }
    }
  }
  std::istringstream strdes(param);
  if((strdes>>cmd))
    if (cmd=="Sensor"){
      if((strdes>>cmd)){
	m_Name=cmd;
	if((strdes>>cmd))
	  if (cmd=="Chain"){
	    unsigned short num;
	    if((strdes>>num)){
	      setNumberOfOffsets(num);
	      int i=0;
	      while ((strdes>>num)){
		if (i<m_NumberOfOffsets-2){
		  m_PartialSum[i].setCovType(num);
		}else if ((m_NumberOfOffsets==1)||(i==m_NumberOfOffsets-1))
		  m_RobotSensorPose.setCovType(num);
		else break;
		i++;
	      }
	    }
	  }else if (cmd=="OffsetNumber"){
	    unsigned short num;
	    if((strdes>>num)){
	      if (m_NumberOfOffsets>num){
		if (m_NumberOfOffsets==1)
		  m_RobotSensorPose.setupPose(strdes);
		else m_Offset[num].setupPose(strdes);
	      }
	    }
	  }
      }
    }
  return 0;
} 
*/
void SensorOffset::calcRobotSensorPose()
{
  if (m_NumberOfOffsets==1){
    m_Jacobian.offset(0,0,m_RobotSensorPose.Covariance.Rows,0);
    return;
  }
  int i=0;
  if (m_NumberOfOffsets==2){
    m_Jacobian.reset(6,12);
    m_RobotSensorPose.add_(m_Offset[i],m_Offset[i+1], &m_Jacobian);
    return;
  }
  Matrix j(6,12);
  m_PartialSum[i].add_(m_Offset[i],m_Offset[i+1], &j);
  Matrix jac[m_NumberOfOffsets-2];
  jac[i]=j;
  j.reset(6,12);
  i++;
  while(i<m_NumberOfOffsets-2)
    {
      m_PartialSum[i].add_(m_PartialSum[i-1],m_Offset[i+1], &j);
      int c=j.Columns;
      c-=jac[i-1].Rows;
      jac[i].reallocate(j.Rows,c+jac[i-1].Columns);
      j.Columns=jac[i-1].Rows;
      jac[i].Columns=jac[i-1].Columns;
      jac[i].multiply_(j,jac[i-1]);
      jac[i].offset(0,jac[i-1].Columns,jac[i].Rows,c);
      j.offset(0,j.Columns,jac[i].Rows,c);
      jac[i]=j;
      jac[i].offset(0,-jac[i-1].Columns,jac[i].Rows,
		    c+jac[i-1].Columns);
      j.reset(6,12);
      i++;
    }
  m_RobotSensorPose.add_(m_PartialSum[i-1],
			     m_Offset[i+1], &j);
  int c=j.Columns;
  c-=jac[i-1].Rows;
  m_Jacobian.reallocate(j.Rows,c+jac[i-1].Columns);
  m_Jacobian.Columns=jac[i-1].Rows;
  m_Jacobian.Columns=jac[i-1].Columns;
  m_Jacobian.multiply_(j,jac[i-1]);
  m_Jacobian.offset(0,jac[i-1].Columns,m_Jacobian.Rows,c);
  j.offset(0,j.Columns,m_Jacobian.Rows,c);
  m_Jacobian=j;
  m_Jacobian.offset(0,-jac[i-1].Columns,m_Jacobian.Rows,
		    c+jac[i-1].Columns); 
}
}
