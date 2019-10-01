//
// = AUTHOR(S)
//    John Folkesson
//    
//    Copyright (c) 2005 John Folkesson
//    
#include "CommandAddress.hh"
#include "CommandExecuter.hh"
#include "AddressBank.hh"

using namespace Cure;

CommandAddress::CommandAddress(const unsigned short thread):Address(thread)
{
  SizeOfReplies=3;
  Replies=new Command*[SizeOfReplies];  
  NumberOfReplies=0;
  FirstReply=0;
  NextReply=0;

  PushClients = 0;
  NumPushClientsAlloc = 0;
  NumPushClients = 0;
  PullClient=0;
}

void CommandAddress::disconnectAddress()  
{
  Address::disconnectAddress();
  if (Replies){
    short i=FirstReply;
    while (i!=NextReply){
      delete Replies[i];
      i++;
      if (i==SizeOfReplies)i=0;
    }
    delete []Replies;
    FirstReply=0;
    NextReply=0;
    Replies=0;
    SizeOfReplies=0;
  }
  if ((NumPushClientsAlloc)&&(PushClients)) delete [] PushClients;
  PushClients=0;
  NumPushClientsAlloc=0;
  NumPushClients = 0;
  PullClient=0;
}

void CommandAddress::init(CommandExecuter *c)  
{
  Executer = c;
}


void CommandAddress::add(Command & p){
  if (NumberOfReplies==SizeOfReplies)
    {
      Command **r=Replies;
      SizeOfReplies+=3;
      Replies=new Command*[SizeOfReplies];
      short j=0;
      short i=FirstReply;
      while (i!=NextReply){
	Replies[j]=r[i];
	i++;
	j++;
	if (i==SizeOfReplies)i=0;
      }
      NextReply=j+1;
      delete []r;
    }
  Replies[NextReply]=new Command(p);
  NextReply++;
  NumberOfReplies++;
  if (NextReply==SizeOfReplies)NextReply=0;
}
void CommandAddress::remove(short ind){
   delete Replies[ind];
  NumberOfReplies--;
  Command **r=Replies+ind;
  ind++;
  if (ind==SizeOfReplies)ind=0;
  while (ind!=NextReply)
    {
      r[0]=Replies[ind];
      r=Replies+ind;
      ind++;
      if (ind==SizeOfReplies)ind=0;
    }
  NextReply--;
  if (NextReply==-1)NextReply=SizeOfReplies;
}
int CommandAddress::getReply(Command &p){
  short i=FirstReply;
  while (i!=NextReply){
    if (Replies[i]->ReplyID==p.CmdID)
      {
	p=(*Replies[i]);
	remove(i);
	return 0;
      }
    i++;
    if (i==SizeOfReplies)i=0;
  }
  return 1;
}
unsigned short CommandAddress::read(TimestampedData & result, 
				    const Timestamp t, 
				    const int interpolate){
  if (t==0)
    result=Executer->OutputCommand;
  else {
    return (TIMESTAMP_ERROR|RESOURCE_ERROR);
  }
  return 0;
}
  
unsigned short CommandAddress::write(TimestampedData & p)
{
  if (p.getClassType()==COMMAND_TYPE)
    {
      if (p.isPacked())
	{
	  p.narrowPackedData()->unpack(Executer->InputCommand);
	}
      else Executer->InputCommand=p;   
    }
  else if (p.narrowSmartData())
    {
      SmartData *sd=p.narrowSmartData();
      if (sd->getTPointer()->getClassType()==COMMAND_TYPE)
	{
	  sd->setEqual(Executer->InputCommand);
	}
    }
  else{
    return TYPE_ERROR;
  }
 return Executer->processCommand();
}
unsigned short CommandAddress::pushData(bool setReplyAdd)
{  
  unsigned short r=0;
  if (setReplyAdd)
    Executer->OutputCommand.ReplyAddress=Index;
  else Executer->OutputCommand.ReplyAddress=0;
  Executer->OutputCommand.Service=Executer->OutputService;
  for (short i = 0; i < NumPushClients; i++) {
    Executer->OutputCommand.CmdID=CmdID;
    CmdID++;
    r=(r|writeTo(PushClients[i], Executer->OutputCommand)); 
    if (r== ADDRESS_INVALID) {
      if (Executer)
	std::cerr <<"Command Executer \"" 
		  <<"\" warning  "
		  << ", failed to pushData, disconnecting!!!\n";
      disconnect(PushClients[i]);
      i--;
      r=(r&(~ADDRESS_INVALID));
    }
  }
  return r;
}
void  CommandAddress::sendReply(Command & c, bool setReplyAdd)
{  
  if (c.ReplyAddress==0)return;
  Executer->Reply.CmdID=CmdID;
  Executer->Reply.Service=c.Sender;
  Executer->Reply.ReplyID=c.CmdID;
  CmdID++;
  if (setReplyAdd)
    Executer->Reply.ReplyAddress=Index;
  else Executer->Reply.ReplyAddress=0;
  writeTo(c.ReplyAddress, Executer->Reply);
}

unsigned short CommandAddress::pullData()
{
  Timestamp t=0;
  unsigned short r=0;
  if (IsPull) {
      r=readFrom(PullClient,Executer->InputCommand,t, 0);
    if (r){
      if (r&ADDRESS_INVALID)
	{
	  std::cerr << "Command Address \"" 
		      <<"\" warning  "
		      << "Failed to pullData, disconnecting!!!\n";
	  disconnect(PullClient);	  
	  return (RESOURCE_ERROR);
	}
      else if (r&TYPE_ERROR)
	{
	  std::cerr << "Command Address\"" 
		    <<"\" warning  "
		    << "Got TYPE_ERROR on pull\n";
	  return (RESOURCE_ERROR);
	}
      else if (r&RESOURCE_ERROR){
	std::cerr <<"Command  \"" 
		  <<"\" warning  "
		  << "Got RESOURCE_ERROR on pull\n";
	return RESOURCE_ERROR;
      }
      return r;
    }
  }
  return 0;
}

void CommandAddress::disconnect(const unsigned long index)
{
  short i = 0; 
  while(i < NumPushClients) {
    if (index == PushClients[i]) {
      for(short j = i + 1; j< NumPushClients; j++) {
	PushClients[j-1] = PushClients[j];
      }
      NumPushClients--;
    }
    else
      i++;
  }
  if (index==PullClient)
    IsPull=false;
}



unsigned short CommandAddress::push(Address * pa)
{
  if (NumPushClients == NumPushClientsAlloc) {
    // Need to allocate more space for more push connections
    unsigned long *temp = new unsigned long[NumPushClientsAlloc + 1];
    for (short i = 0; i < NumPushClientsAlloc; i++) temp[i] = PushClients[i];
    temp[NumPushClientsAlloc] = pa->Index;
    if (NumPushClientsAlloc) delete [] PushClients;
    PushClients = temp;
    NumPushClientsAlloc++;
    NumPushClients++;
    return 0;
  }
  // If we got here there is already allocated space for more connections
  PushClients[NumPushClients] = pa->Index;
  NumPushClients++;
  return 0;
}
 unsigned short CommandAddress::pull(Address * pa)
{
  if (IsPull){
    std::cerr << "\nCommandAddresss  Command "
	      <<" warning "<<" I am replacing the Pull Address "
	      <<"\n";
  } 
  PullClient = pa->Index;
  IsPull=true;
  return 0;

}

