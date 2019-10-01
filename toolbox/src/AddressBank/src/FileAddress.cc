// = RCSID
//    $Id: FileAddress.cc ,v 1.1 2004/03/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2004  John Folkesson
//    

#include "FileAddress.hh"
#include "DataSet.hh"
#include "CureDebug.hh"
using namespace std;
#include <sstream>  // istringstream
//#define DEBUG_FILEADDRESS

namespace Cure {

  FileAddress::FileAddress(const unsigned short thread):Address(thread)
{
  m_SignificantFigures=16;
  m_Header=" ";
  m_IncludeTime=true;
  m_TimeOffset=0.0;  
  m_UseDescriptor=false;
  m_FileOpened = false;
  m_NumberOfStrings=0;
  for (short i=0;i<10;i++){
    m_MessageString[i]="";
  }
  m_RowWise=false;
}

FileAddress::~FileAddress()
{
  closeReadFile();
  closeWriteFile();
}
void FileAddress::setDescriptor(DataDescriptor &d){
  m_UseDescriptor=true;
  m_Descriptor=d;
}

int FileAddress::configure(std::string param)
{
   std::istringstream strdes(param);
  std::string cmd;
  if((strdes>>cmd)){
    if (cmd=="m_MessageString"){
      if (m_NumberOfStrings<10){
	if((strdes>>m_MessageString[m_NumberOfStrings])){
	  short i=0;
	  m_MessageIndex[m_NumberOfStrings].grow(1,i+1);	  
	  while((strdes>>m_MessageIndex[m_NumberOfStrings](0,i))){
	    i++;
	    m_MessageIndex[m_NumberOfStrings].grow(1,i+1);
	  }	
	  m_MessageIndex[m_NumberOfStrings].grow(1,i);
	  m_NumberOfStrings++;
	}
      }
    }  else  if (cmd=="m_TimeOffset"){
      long ts=0;
      if((strdes>>ts)){
	m_TimeOffset.Seconds=ts;
	
	if ((strdes>>ts))
	  m_TimeOffset.Microsec=ts;
	// m_TimeOffset.print();
      }
    } else  if (cmd=="m_IncludeTime"){
      bool ts=0;
      if((strdes>>ts)){
	m_IncludeTime=ts;
      }
    } else  if (cmd=="m_RowWise"){
      bool ts=true;
      if((strdes>>ts)){
	m_RowWise=ts;
      }
    } else  if (cmd=="m_Header"){
      m_Header=(param.c_str()+9);
      writeHeader();
    } else  if (cmd=="m_SignificantFigures"){
      strdes>>m_SignificantFigures;
      if (m_SignificantFigures>18)m_SignificantFigures=18;
      if (m_SignificantFigures<1)m_SignificantFigures=1;
    } else  if (cmd=="Print"){
      printConfig();
      if((strdes>>cmd))
	if (cmd=="pause"){
	  std::cout<<"Waiting for enter";
	  getchar();
	}
    }
  } 
  return 0;  
}

void
FileAddress::printConfig()
{
  std::cout<<"FileAddress "<<m_Filename;
  std::cout<<" Header "<<m_Header<<"m_TimeOffset";
  m_TimeOffset.print();
  std::cout<<" m_IncludeTime "<<m_IncludeTime<<"\n";
  for (short i=0; i<m_NumberOfStrings;i++)
    m_MessageIndex[i].print();
    
}

unsigned short 
FileAddress::write(TimestampedData& p)
{
  if (m_UseDescriptor){
    DataDescriptor d;
    p.getDescriptor(d);
    if (!(m_Descriptor.matchClassAndSensor(d)))return TYPE_ERROR;
  }
  if (m_NumberOfStrings>0){
    char timech[32];
    if (m_IncludeTime){
      Timestamp t=p.Time;
      t-=m_TimeOffset;
      double d=t.getDouble();
      gcvt(d,16,timech);
    }
    if (m_RowWise){
      Matrix mat[m_NumberOfStrings];
      short r=0;
      for (short i=0; i<m_NumberOfStrings;i++){
	if (!(p.getMatrix(m_MessageString[i],m_MessageIndex[i],mat[i])))
	  return RESOURCE_ERROR; 
	if (r<mat[i].Rows)r=mat[i].Rows;
      }
      for (short i=0;i<r;i++){
	if (m_IncludeTime)  WriteFile<<timech;
	for (short k=0; k<m_NumberOfStrings;k++){
	  if (mat[k].Rows>=i){
	    for (int j=0;j<mat[k].Columns;j++)
	      {
		char ch[32];
		gcvt(mat[k](i,j),m_SignificantFigures,ch);
		WriteFile<<" "<<ch;
	      }
	  } else {
	    for (int j=0;j<mat[k].Columns;j++)
	      WriteFile<<" "<<0.0;
	  }
	}
	WriteFile << endl;    
      }  
    }else{
      if (m_IncludeTime)  WriteFile<<timech;
      for (short i=0; i<m_NumberOfStrings;i++){
	Matrix mat;
	if (p.getMatrix(m_MessageString[i],m_MessageIndex[i],mat))
	  {
	    
	    for (int i=0;i<mat.Rows;i++)
	      for (int j=0;j<mat.Columns;j++)
		{
		  char ch[32];
		  gcvt(mat(i,j),m_SignificantFigures,ch);
		  WriteFile<<" "<<ch;
		}
	  }
      }
      WriteFile << endl;    
    }
    return 0;
  } 
  if (m_FileOpened && WriteFile.good()){
    std::vector<std::string> vec;
    std::vector<std::string>::iterator it;
    Cure::writeString(p,vec,m_SignificantFigures);
    
    for (it=vec.begin();it!=vec.end();it++)
      WriteFile<<(*it);
    return 0;
  }
  return (RESOURCE_ERROR|ADDRESS_INVALID);
}

unsigned short 
FileAddress::read(TimestampedData& result, const Timestamp t,
		      const int interpolate)
{
  
  unsigned short r=read(t,interpolate);
  if (r&ADDRESS_INVALID)return r;
  TPoint.setEqual(result);
  if (m_UseDescriptor){
    DataDescriptor d;
    result.getDescriptor(d);
    if (!(m_Descriptor.matchClassAndSensor(d)))return read(result,t,interpolate);
  }
  if ((!(t==0))&(interpolate==0))
    if (!(t==result.Time))
      return (r|NO_INTERPOLATE|TIMESTAMP_ERROR);
  return r;
}

unsigned short
FileAddress::read(const Timestamp t, const int interpolate)
{    
  if (m_FileOpened && ReadFile.good()) {
    std::string line;
    getline(ReadFile,line);
    unsigned short ret=readString(TPoint,line);
    if (ret) return (ret|TYPE_ERROR);
    if (TPoint.getTPointClassType()==DATASET_TYPE)
      {
	DataSet *ds=(TPoint.getTPointer()->narrowDataSet());
	if (!ds)return (RESOURCE_ERROR|TYPE_ERROR);
	DataSet result=(*ds);
	for (int i=0;i<(int)result.SetSize;i++)
	  {
	    ret=read(t,interpolate);
	    if (ret) return (ret);
	    result(i)=TPoint;
	  }
	TPoint=result;
      }
    return 0;
  }
  return (RESOURCE_ERROR|ADDRESS_INVALID);
}
  unsigned short 
FileAddress::query()
{
  if (ReadFile.good()||WriteFile.good())
    return 0;
  return (RESOURCE_ERROR|ADDRESS_INVALID);
}

unsigned short 
FileAddress::openReadFile(const std::string &name)
{
  ReadFile.open(name.c_str(),ios::in);
  if (ReadFile.good()) {
    m_FileOpened = true;
    m_Filename = name;
    return 0;      
  }

  CureCERR(10) << "Failed to open ReadFile \"" << name << "\"\n";
  return RESOURCE_ERROR;
}
void FileAddress::writeHeader(){
  if (m_Header==" ")
    return;      
  if (m_FileOpened)
    if (WriteFile.good())
      WriteFile<<"% "<<m_Header<<"\n";
}
unsigned short 
FileAddress::openWriteFile(const std::string &name)
{
  WriteFile.open(name.c_str(),ios::out);
  if (WriteFile.good()) {
    m_FileOpened = true;
    m_Filename = name;
    writeHeader();
    return 0;
  }

  CureCERR(10)<< "Failed to open WriteFile \"" << name << "\"\n";
  return RESOURCE_ERROR;
}

void 
FileAddress::closeReadFile()
{
  ReadFile.close();
  m_FileOpened = false;
  m_Filename = "-";
}

void 
FileAddress::closeWriteFile()
{
  WriteFile.close();
  m_FileOpened = false;
  m_Filename = "-";
}

} // namespace Cure
 
