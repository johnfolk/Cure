// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2005 John Folkesson
//    

#include "PackedCommand.hh"

using namespace std;

namespace Cure {


int PackedCommand::VERSION = 1;

PackedCommand::PackedCommand(){
  init();
}

PackedCommand::PackedCommand(PackedCommand& p){ 
  PackedCommand();
  *this=(p);
}

PackedCommand::PackedCommand(Command& p){ 
  PackedCommand();
  *this=(p);
}

void PackedCommand::init()
{
  m_Packed =true;
  Data = 0;
  DataSize = 0;
  ShortData = 0;
  ShortDataSize = 0;
  m_ClassType=(COMMAND_TYPE);
  setSubType(0);
  setID(0);
  Version = VERSION;
 }

PackedCommand::~PackedCommand()
{
  if (Data)delete []Data;
  Data = 0;
  DataSize = 0;

  if (ShortData)delete []ShortData;
  ShortData = 0;
  ShortDataSize = 0;
}

void
PackedCommand::operator=(TimestampedData& p){
  Command *s=p.narrowCommand();
  if (s) {
    operator=(*s);
  } else {
    PackedData *pp=p.narrowPackedCommand();
    if (pp) {
      // Need to allocate space for the double data before we perform
      // this operation
      setDataSize(pp->getDataSize());

      PackedData::operator=(*pp);
    } else {
      std::cerr << "WARNING: PackedCommand::operator= logic error!!\n";
    }
  }
}


void PackedCommand::operator = (const Command& p)
{
  Time=p.Time;
  unsigned short st=p.CmdFlags.Rows*p.CmdFlags.Columns;
  unsigned short sty=p.getSubType();
  if (sty<st)sty=st;
  setSubType(sty);
  setID(p.getID());
  ShortData[0]=p.CmdFlags.Rows;
  ShortData[1]=p.CmdFlags.Columns;
  ShortData[2]=p.CmdData.Rows;
  ShortData[3]=p.CmdData.Columns;
  ShortData[4]=p.CmdType;
  ShortData[5]=p.Service;
  ShortData[6]=p.Priority;
  ShortData[7]=p.Sender;
  unsigned long t=(unsigned long)p.ReplyAddress;
  ShortData[9]=(0xFFFF&t);
  t=(t>>16);
  ShortData[8]=(0xFFFF&t);
  t=p.CmdID;
  ShortData[11]=(0xFFFF&t);
  t=(t>>16);
  ShortData[10]=(0xFFFF&t);
  t=p.ReplyID;
  ShortData[13]=(0xFFFF&t);
  t=(t>>16);
  ShortData[12]=(0xFFFF&t);
  int k=14;

  for (int i=0; i<p.CmdFlags.Rows; i++)
    for (int j=0; j<p.CmdFlags.Columns; j++)
      {
	ShortData[k]=p.CmdFlags(i,j);
	k++;
      }
  unsigned long dsize=(p.CmdData.Rows*p.CmdData.Columns); 
  if (dsize!=DataSize)  setDataSize(dsize);
  k=0;
  for (int i=0; i<p.CmdData.Rows; i++)
    for (int j=0; j<p.CmdData.Columns; j++)
      {
	Data[k]=p.CmdData(i,j);
	k++;
      }
}

void PackedCommand::unpack(Command& p)
{
  p.setSubType(getSubType());
  p.setID(getID());
  p.Time=Time;
  if (!Data)return;
  if (!ShortData)return;
  p.CmdFlags.reallocate(ShortData[0],ShortData[1]);
  p.CmdData.reallocate(ShortData[2],ShortData[3]);
  p.CmdType=ShortData[4];
  p.Service=ShortData[5];
  p.Priority=ShortData[6];
  p.Sender=ShortData[7];
  unsigned long t=(ShortData[9]);
  t=(t<<16);
  t+=ShortData[8];
  p.ReplyAddress=(long)t;
  t=(ShortData[11]);
  t=(t<<16);
  t+=ShortData[10];
  p.CmdID=t;
  t=(ShortData[13]);
  t=(t<<16);
  t+=ShortData[12];
  p.ReplyID=t;
  int k=14;
  for (int i=0; i<p.CmdFlags.Rows; i++)
    for (int j=0; j<p.CmdFlags.Columns; j++)
      {
	p.CmdFlags(i,j)=ShortData[k];
	k++;
      }
  k=0;
  for (int i=0; i<p.CmdData.Rows; i++)
    for (int j=0; j<p.CmdData.Columns; j++)
      {
	p.CmdData(i,j)=Data[k];
	k++;
      }
  
}

} // namespace Cure
