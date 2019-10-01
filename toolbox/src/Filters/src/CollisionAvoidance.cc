// = RCSID
//    $Id: CollisionAvoidance.cc ,v 1.1 2004/03/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2004 John Folkesson
//    
#include "CollisionAvoidance.hh"
#include "DataFilterAddress.hh"
#include "CommandAddress.hh"
#include "CureDebug.hh"
namespace Cure {
  
CollisionAvoidance::CollisionAvoidance()
  :DataFilter(2, 0, true)
{
  FilterName = "CollisionAvoidance"; 
  in(0)->setLatest();
  in(1)->setLatest();
  CommandExecuter::Service=Cure::Command::SERVICE_AVOIDANCE;
  OutputService=Cure::Command::SERVICE_DRIVE;
  DriveDelay=.01;
  SafePeriod=.3;
  InnerSafety=.35;
  Safety=.8;
  SafetyVelocity=1;
  MeanPeriod=.05;
  MinPeriod=.01;
  MaxPeriod=.05;
  PeriodVar=MeanPeriod*MeanPeriod/4;
  Stoped=true;
  AvoidOn=1;
  GoalUnreachable=0;
  GoalReached=0;
  Robot.reallocate(1);
  Robot(0,0)=.3;
  Radius=0;
  TimeOut=0;
  Dx[0]=0;
  Dx[1]=0;
  Dx[2]=0;
  OnArc=0;
  Increment=.04;
  Velocity.reallocate(1,2);
  Status.CmdFlags.reallocate(3,4);
  Status.CmdData.reallocate(3,3);  
  Reply.CmdFlags.reallocate(1,2);
  MinVelocity[0]=0;
  MinVelocity[1]=0;
  MaxChange[0]=.01;
  MaxChange[1]=.01;
  OldVelocity[0]=0;
  OldVelocity[1]=0;
  MaxAvoidHeading=900;
  AvoidHeadingDecrement=30;
  GoalHeading=0;
  resetAvoidHeadings();
}
 
  CollisionAvoidance::~CollisionAvoidance(){
}
CommandAddress * CollisionAvoidance::command(){
  return &CmdAdd;
}
void CollisionAvoidance::resetAvoidHeadings(){
  for (int i=0; i<360; i++)
    AvoidHeadings[i]=0;
  Mode=0;
} 
unsigned short CollisionAvoidance::comRelease()
{
  stop();
  CurrentCommand=InputCommand;  
  CurrentCommand.Priority=0;
  CurrentCommand.Time=CurrentTime;
  Reply.CmdFlags.reallocate(1,2);
  Reply.CmdFlags(0,0)=Cure::MotionCommand::COMMAND_OK;
  Reply.CmdFlags(0,1)=Cure::MotionCommand::COMMAND_OK;
  CmdAdd.sendReply(CurrentCommand);  
  OutputCommand.CmdType=Cure::Command::RELEASE;
  OutputCommand.Time=CurrentTime;
  CmdAdd.pushData();
  return 0;
}
unsigned short CollisionAvoidance::comSet()
{
  Reply.CmdFlags.reallocate(1,2); 
  Reply.CmdFlags(0,0)=Cure::MotionCommand::COMMAND_OK;
  if (InputCommand.CmdFlags(0,0)==Cure::MotionCommand::AVOID_ON){
    AvoidOn=1;
    Reply.CmdFlags(0,1)=Cure::MotionCommand::AVOID_ON;
  }
  else  if (InputCommand.CmdFlags(0,0)==Cure::MotionCommand::AVOID_OFF)
    {
      stop();
      CurrentCommand=InputCommand;
      AvoidOn=0;
      Reply.CmdFlags(0,1)=Cure::MotionCommand::AVOID_OFF;
    }
  else  if (InputCommand.CmdFlags(0,0)==Cure::MotionCommand::SET_PARAMETER)
    {
      Reply.CmdFlags(0,1)=Cure::MotionCommand::SET_PARAMETER;
      if (InputCommand.CmdFlags(0,1)==DRIVE_DELAY)
	DriveDelay=InputCommand(0,0);
      else if (InputCommand.CmdFlags(0,1)==SAFE_PERIOD)
	SafePeriod=InputCommand(0,0);
      else if (InputCommand.CmdFlags(0,0)==INNER_SAFETY)
	InnerSafety=InputCommand(0,0);
      else if (InputCommand.CmdFlags(0,0)==SAFETY)
	Safety=InputCommand(0,0);
      else if (InputCommand.CmdFlags(0,0)==SAFETY_VELOCITY)
	SafetyVelocity=InputCommand(0,0);
      else if (InputCommand.CmdFlags(0,0)==MEAN_PERIOD)
	MeanPeriod=InputCommand(0,0);
      else if (InputCommand.CmdFlags(0,0)==MIN_PERIOD)
	MinPeriod=InputCommand(0,0);
      else if (InputCommand.CmdFlags(0,0)==MAX_PERIOD)
	MaxPeriod=InputCommand(0,0);
      else if (InputCommand.CmdFlags(0,0)==PERIOD_VAR)
	PeriodVar=InputCommand(0,0);
    }
  CmdAdd.sendReply(InputCommand);  
  return 0;
}
unsigned short CollisionAvoidance::comStop()
{

  stop();
  GoalHeading=0;
  resetAvoidHeadings();
  OutputCommand.Priority=CurrentCommand.Priority;
  stop();
  CurrentCommand=InputCommand;
  CurrentCommand.Time=CurrentTime;
  Reply.CmdFlags.reallocate(1,2);
  Reply.CmdFlags(0,0)=Cure::MotionCommand::COMMAND_OK;
  Reply.CmdFlags(0,1)=Cure::MotionCommand::COMMAND_OK;
  CmdAdd.sendReply(CurrentCommand);  
  return 0;
}
unsigned short CollisionAvoidance::comSpeed()
{
  Reply.CmdFlags.reallocate(1,2);
  Reply.CmdFlags(0,0)=Cure::MotionCommand::COMMAND_OK;
  Reply.CmdFlags(0,1)=Cure::MotionCommand::COMMAND_OK;
  if (InputCommand.CmdFlags(0,0)!=MotionCommand::VELOCITY){
    Reply.CmdFlags(0,1)=Cure::MotionCommand::NOT_SUPPORTED;
    CmdAdd.sendReply(InputCommand);
    return (TYPE_ERROR);
  }
  CurrentCommand=InputCommand;
  setPath();  
  CmdAdd.sendReply(CurrentCommand);  
  return 0;
}
unsigned short CollisionAvoidance::calc()
{
  learnTime();  
  if (Stoped)return 0;
  if (setVelocity())      
    CmdAdd.sendReply(CurrentCommand);  
  else speed();
  return 0;
}
void CollisionAvoidance::speed()
{
  for (int i=0;i<2;i++){
    double dv=Velocity(0,i)-OldVelocity[i];
    
    if (dv>MaxChange[i]){
      if ((Velocity(0,i)>MinVelocity[i])||(Velocity(0,i)<-MinVelocity[i]))
	{
	  if (OldVelocity[i]==-MinVelocity[i])
	    if (Velocity(0,i)>0)OldVelocity[i]=0;
	  Velocity(0,i)=OldVelocity[i]+MaxChange[i];
	}
    }
    else if (dv<-MaxChange[i]){
      if ((Velocity(0,i)>MinVelocity[i])||(Velocity(0,i)<-MinVelocity[i]))
	{
	  if (OldVelocity[i]==MinVelocity[i])
	    if (Velocity(0,i)<0)OldVelocity[i]=0;
	  Velocity(0,i)=OldVelocity[i]-MaxChange[i];
	}

    }
  }
  if (MinVelocity[0]>0)
    if ((Velocity(0,0)< MinVelocity[0])&&(Velocity(0,0)>-MinVelocity[0])){
      if (Velocity(0,0)>0)
	Velocity(0,0)=MinVelocity[0];
      else if (Velocity(0,0)<0)
	Velocity(0,0)=-MinVelocity[0];
    }

  if (MinVelocity[1]>0)
    if ((Velocity(0,1)< MinVelocity[1])&&(Velocity(0,1)>-MinVelocity[1])){
      if (Velocity(0,1)>0)
	Velocity(0,1)=MinVelocity[1];
      else if (Velocity(0,1)<0)
	Velocity(0,1)=-MinVelocity[1];
    }
  OutputCommand.Priority=CurrentCommand.Priority;
  OutputCommand.CmdType=Cure::Command::SPEED;
  OutputCommand.Time=CurrentTime;
  OutputCommand.CmdData.reallocate(1,2);
  OutputCommand.CmdData(0,0)=Velocity(0,0);
  OutputCommand.CmdData(0,1)=Velocity(0,1);
  OutputCommand.CmdFlags.reallocate(1);
  OutputCommand.CmdFlags(0,0)=MotionCommand::VELOCITY;
  OldVelocity[0]=Velocity(0,0);
  OldVelocity[1]=Velocity(0,1);
  CmdAdd.pushData(true);
  decrementAvoidHeadings();
}
void CollisionAvoidance::setStatus()
{
  Status.Time=CurrentTime;
  Status.Sender=Service;
  Status.Priority=0;
  Status.CmdFlags.reallocate(2,4);
  Status.CmdFlags(0,0)=CmdProtocol;
  Status.CmdFlags(0,1)=CurrentCommand.Priority;
  Status.CmdFlags(0,2)=CurrentCommand.CmdType;
  Status.CmdFlags(0,3)=CurrentCommand.CmdID;

  Status.CmdFlags(1,0)=AvoidOn;
  /**Stoped and why*/
  Status.CmdFlags(1,1)=Stoped;
  Status.CmdFlags(1,2)=GoalReached;
  Status.CmdFlags(1,3)=GoalUnreachable;
  
  
  Status.CmdData.reallocate(3,1);
  Status(0,0)=Velocity(0,0);
  Status(0,1)=Velocity(0,1);
  Status(0,2)=TimeOut;
}

void CollisionAvoidance::stop()
{
  CurrentTime.setToCurrentTime();
  OutputCommand.CmdType=Cure::Command::STOP;
  OutputCommand.Time=CurrentTime;
  OutputCommand.CmdData=0;
  CmdAdd.pushData();
  Velocity=0;
  OldVelocity[0]=Velocity(0,0);
  OldVelocity[1]=Velocity(0,1);
  Stoped=true;
}
unsigned short CollisionAvoidance::processReply()
{
  if (InputCommand.CmdFlags.Columns<2)return 0;
  if (InputCommand.CmdFlags(0,0)==Cure::MotionCommand::COMMAND_OK)
    if (InputCommand.CmdFlags(0,1)==Cure::MotionCommand::GOAL_UNREACHABLE){
      Reply.CmdFlags(0,1)=Cure::MotionCommand::GOAL_UNREACHABLE;
      GoalUnreachable=2;      
      CmdAdd.sendReply(CurrentCommand);
      stop();
    }
  return 0;
}
void CollisionAvoidance::learnTime()
{
  CurrentTime.setToCurrentTime();
  if (LastTime!=0)
    {
      Timestamp dtts=CurrentTime;
      dtts-=LastTime;
      double dt=dtts.getDouble();
      if (dt<1.0){
	double d=MeanPeriod*(.999);
	d+=(dt*(.001));
	MeanPeriod=d;
	if (!Stoped){
	  double sd=sqrt(PeriodVar);
	  d=MaxPeriod-sd;
	  if (d<(MeanPeriod+sd))d=MeanPeriod+sd;
	  if (d<dt)
	    {
	      d=MaxPeriod*(.99);
	      d+=(dt*(.01));	
	      MaxPeriod=d;
	      if (MaxPeriod<SafePeriod)MaxPeriod=SafePeriod;
	    }
	}
	d=(dt);
	d-=MeanPeriod;
	d*=d;
	d*=.001;
	d+=PeriodVar*(.999);	
	PeriodVar=d;
      }
      else {
	if (!Stoped)
	  CureCERR(40)<<FilterName<<" Got very long delay "<<dt<<"seconds\n";
      }
    }
  LastTime=CurrentTime;
}
void CollisionAvoidance::setPath()
{
  GoalReached=0;
  GoalUnreachable=0;
  Stoped=0;
  TimeOut=1.0;
  LastSendTime=0.0;
  if (CurrentCommand.CmdData.Columns>2)
    TimeOut=CurrentCommand(0,2);
  Velocity(0,0)=CurrentCommand(0,0);
  Velocity(0,1)=CurrentCommand(0,1);
  if ((Velocity(0,0)==0)&&(Velocity(0,1)==0))
    return stop();
  
  Radius=1E6; 
  Dx[0]=0;
  Dx[1]=0;
  if (Velocity(0,1)>1E-8)
    {
      Dx[2]=1;
      Radius=Velocity(0,0)/Velocity(0,1);
   }
  else if (Velocity(0,1)<-1E-8)
    {
      Dx[2]=-1;
      Radius=Velocity(0,0)/Velocity(0,1);
    } 
  if (Radius>1E3)OnArc=0;
  else OnArc=1;
  double th=CurrentPose.getTheta();
  if (!OnArc){
    if (Velocity(0,0)>0){
      Dx[0]=cos(th);
      Dx[1]=sin(th);
    }else {
      Dx[0]=-cos(th);
      Dx[1]=-sin(th);
    }
    Dx[2]=0;
    Radius=0;
  } else {
    double dt=TimeOut/2;
    if (dt>(2*MeanPeriod))dt=2*MeanPeriod;
    if (Velocity(0,1)>0)
      th+=M_PI_2;
    else th-=M_PI_2;
  }
  th*=180/M_PI;
  while (th>=359.5)th-=360;
  while (th<-.5)th+=360;
  th+=.5;
  GoalHeading=(unsigned short)th;
  if (GoalHeading>=360)GoalHeading=0;
  if ((Safety+Radius)>1)
    Increment=CurrentGrid.CellWidth/(Safety+Radius);
  else Increment=CurrentGrid.CellWidth;
 }

int CollisionAvoidance::setVelocity()
{
  GoalReached=0;
  Velocity(0,0)=CurrentCommand(0,0);
  Velocity(0,1)=CurrentCommand(0,1);
  Timestamp t=CurrentTime;
  if (LastSendTime!=0){
    t-=LastSendTime;
  }
  else t=0.0;
  LastSendTime=CurrentTime;
  TimeOut-=t.getDouble();
  if (TimeOut<1E-3){
    Velocity=0;
    Reply.CmdFlags(0,1)=Cure::MotionCommand::GOAL_REACHED;
    GoalReached=1;
    stop();
    std::cerr<<"Timeout";
    LastSendTime=0.0;
    return 1;
  }
  //This checks if we will not be able to stop in time out 
  //due to the duty cycle  of the triggering 
  double mt=((MeanPeriod+sqrt(PeriodVar))/TimeOut);
  if (mt>1){    //We slow down so we don't go to far even if too long time.
    Velocity(0,0)/=mt;
    Velocity(0,1)/=mt;
  }

  if (AvoidOn)
    return trace();
  return 0;
}
double  CollisionAvoidance::maxVelRatio(double d)
{
  double r=0;
  if (d>0){
    double t=Safety-d; 
    if (t<1E-3)return 1;
    d-=InnerSafety;
    d/=(Safety-InnerSafety);
    double s=SafetyVelocity;
    if (d<1E-1)
      s*=.1;
    else s*=d;
    if ((!OnArc)||((Radius>.5)||(Radius<-.5))){
      if (Velocity(0,0)>0){
	if (s>Velocity(0,0))s=1;
	else s/=Velocity(0,0);
      }
      else if (Velocity(0,0)<0){
	if (s>-Velocity(0,0))s=1;
	else s/=(-Velocity(0,0));
      }
    } else {
      s*=2;
      if (Velocity(0,1)>0){
	if (s>Velocity(0,1))s=1;
	else s/=Velocity(0,1);
      }
      else if (Velocity(0,1)<0){
	if (s>-Velocity(0,1))s=1;
	else s/=(-Velocity(0,1));
      }
      
    }
    r=s;
  }
  if (r==0)return 0; 
  double t=r*Velocity(0,0);
 
  if ((Velocity(0,0)>MinVelocity[0])&&(t<MinVelocity[0]))
    {
      r=MinVelocity[0]/Velocity(0,0);
    }
  else if ((Velocity(0,0)<-MinVelocity[0])&&(t>-MinVelocity[0]))
    {
      r=-MinVelocity[0]/Velocity(0,0);
    }
  t=r*Velocity(0,1);
  if ((Velocity(0,1)>MinVelocity[1])&&(t<MinVelocity[1]))
    {
      r=MinVelocity[1]/Velocity(0,1);
    }
  else if ((Velocity(0,1)<-MinVelocity[1])&&(t>-MinVelocity[1]))
    {
      r=-MinVelocity[1]/Velocity(0,1);
    }
  return r;
}
void CollisionAvoidance::decrementAvoidHeadings()
{
  if (Mode==0)return;
  int max=0;
  for (int i=0; i<360; i++){
    if (AvoidHeadings[i]>AvoidHeadingDecrement)
      AvoidHeadings[i]-=AvoidHeadingDecrement;
    else AvoidHeadings[i]=0;
    if (AvoidHeadings[i]>max)max=AvoidHeadings[i];
  }
  if (max==0)Mode=0;
}
void CollisionAvoidance::setAvoidHeadings( double x[2])
{
  Matrix points;
  double d=CurrentGrid.nearest(x,points, InnerSafety);
  if (d<0)return;
  int top=90;
  if (d<InnerSafety/2)top=100;
  if (d<InnerSafety/4)top=110;
  if (d<InnerSafety/8)top=120;
  for (int j=0; j<points.Rows; j++)
    {
      Mode=1;
      double th=atan2(points(j,1)-x[1],points(j,0)-x[0]);
      th*=180/M_PI;
      while (th>=359.5)th-=360;
      while (th<-.5)th+=360;
      th+=.5;
      unsigned short heading=(unsigned short)th;
      if (heading>=360)heading=0;
      AvoidHeadings[heading]+=top;
      if (AvoidHeadings[heading]>MaxAvoidHeading)
	AvoidHeadings[heading]=MaxAvoidHeading;
      for (int k=1; k<top; k++)
	{
	  int i=heading+k;
	  while (i>359)i-=360;
	  AvoidHeadings[i]+=(top-k);
	  if (AvoidHeadings[i]>MaxAvoidHeading)
	    AvoidHeadings[i]=MaxAvoidHeading;
	  i=heading-k;
	  while (i<0)i+=360;
	  AvoidHeadings[i]+=(top-k);
	  if (AvoidHeadings[i]>MaxAvoidHeading)
	    AvoidHeadings[i]=MaxAvoidHeading;
	}
    }
}
//NeverGiveUp
int CollisionAvoidance::setAvoidVelocity(double th)
{

  std::cerr<<"\nsetAvoidVelocity"<<th;
  unsigned short min=MaxAvoidHeading+1;
  th*=180/M_PI;
  while (th>=359.5)th-=360;
  while (th<-.5)th+=360;
  th+=.5;
  unsigned short heading=(unsigned short)th;
  if (heading>=360)heading=0;
  std::cerr<<" "<<heading<<" Goal "<<GoalHeading;
  int h=0;
  if (AvoidHeadings[GoalHeading]<min){
    min=AvoidHeadings[GoalHeading];
    h=GoalHeading;
  }
  for (int k=1; k<180; k++)
    {
      int i=GoalHeading+k;
      while (i>359)i-=360;
      if (AvoidHeadings[i]<min){
	min=AvoidHeadings[i];
	h=i;
      }
      i=heading-k;
      while (i<0)i+=360;
      if (AvoidHeadings[i]<min){
	min=AvoidHeadings[i];
	h=i;
      }
    }
  if (min==MaxAvoidHeading)
    {
      Velocity(0,0)=0;
      Velocity(0,1)=0;
      resetAvoidHeadings();
      return 0;
    }
  std::cerr<<"final h "<<h<<"\n";
  Velocity(0,0)=0;
  int dt=h-heading;
  while (dt>180)dt-=360;
  while (dt<-180)dt+=360;
  if (dt>0){
    Velocity(0,1)=MinVelocity[1]+.1;
      if ((min<10)&&(dt<30))
	Velocity(0,0)=MinVelocity[0]+.05;
  }
  else if (dt<0)
    {
      Velocity(0,1)=-MinVelocity[1]-.1;
      if ((min<10)&&(dt>-30))
	Velocity(0,0)=MinVelocity[0]+.05;
    }
  else if (dt==0)
    {
      Velocity(0,1)=0;
       if (min<20)
	Velocity(0,0)=MinVelocity[0]+.1;
    }
  return 0;
}  
int CollisionAvoidance::trace()
{ 
  double start[3],center[2];
  CurrentPose.getXYTheta(start);
  double n[2];
  //Vector pointing to the left
  n[0]=-sin(start[2]);
  n[1]=cos(start[2]);
  center[0]=start[0]+n[0]*Radius;
  center[1]=start[1]+n[1]*Radius;    
  Rotation2D r;
  r.setTheta(Dx[2]*Increment);
  GridData rob(CurrentGrid.CellWidth);
  Pose3D p=CurrentPose;
  p.setZ(CurrentGrid.Offset[2]);
  rob=0;
  rob.robot(Robot,p,2);
  unsigned short st= CurrentGrid.getSubType();
  CurrentGrid.setSubType(1);
  rob.update(CurrentGrid);
  CurrentGrid.setSubType(st);
  if (rob.count()>0)
    setAvoidHeadings(start);
  if (Mode==1)return setAvoidVelocity(start[2]);
  double x[3];
  x[0]=start[0];
  x[1]=start[1];
  x[2]=start[2];
  Matrix points;
  double mn[2];
  mn[0]=-n[0];
  mn[1]=-n[1];
  double d=CurrentGrid.nearest(x,points,Safety,0);
  if (d<0)return 1;
  if (d<Safety){
    removeBehind();
    d=CurrentGrid.nearest(x,points,Safety,0);
    if (d<Safety){
      d=maxVelRatio(d);
      Velocity(0,0)*=d;
      Velocity(0,1)*=d;  
    }
  }
  double maxt=(MaxPeriod+DriveDelay+sqrt(PeriodVar));
  bool pos=true;
  double maxTravel=maxt;
  if (Velocity(0,OnArc)<0)pos=false;
  if (pos)
    maxTravel*=Velocity(0,OnArc);  
  else
   maxTravel*=(-Velocity(0,OnArc));  
  double travel=0;
  while(true){
    travel+=Increment;
    r.rotate2D(n,n);
    mn[0]=-n[0];
    mn[1]=-n[1];
    x[0]=center[0]+travel*Dx[0]-Radius*n[0];
    x[1]=center[1]+travel*Dx[1]-Radius*n[1];
    x[2]=start[2]+travel*Dx[2];
    p.setXYTheta(x);
    rob=0;
    rob.robot(Robot,p,2);
    unsigned short st= CurrentGrid.getSubType();
    CurrentGrid.setSubType(1);
    rob.update(CurrentGrid);
    CurrentGrid.setSubType(st);
    if (rob.count()>0) {
      double ratio=(travel/((maxt+.01)*(Velocity(0,OnArc))));
      if (ratio<0)ratio=-ratio;
      if (ratio>1)
	ratio=1;	
      Velocity*=ratio;
      std::cerr<<ratio<<" Traced collision "<<travel<<"\n";
      return 0;
    }
    Matrix points;
    d=CurrentGrid.nearest(x,points,n,Safety);
    d=CurrentGrid.nearest(x,points,mn,d);
    if (d<0)return 1;
    if (d<Safety){
      if (limitVelocity(d, maxt, travel)){
	if (pos)
	  maxTravel=Velocity(0,OnArc)*maxt;  
	else
	  maxTravel=(-Velocity(0,OnArc)*maxt);  
      }
    }
    if (travel>maxTravel){
      d=CurrentGrid.nearest(x,points,Safety,0);
      if (d<0)d=0;;
      if (d<Safety)limitVelocity(d, maxt, travel);  
      return 0;
    }
  }
  return 0;
}
void CollisionAvoidance::removeBehind()
{
  GridData rb(CurrentGrid.CellWidth);
  rb.robot(Robot, CurrentPose,1);   
  double y[3],x[3],v[2];
  CurrentPose.getXYTheta(x);
  v[0]=cos(x[2])*rb.CellWidth;;
  v[1]=sin(x[2])*rb.CellWidth;;
  if (Velocity(0,0)<0){
    v[0]=-v[0];
    v[1]=-v[1];
  }
  Pose3D p;
  y[0]=x[0]-v[0];
  y[1]=x[1]-v[1];
  y[2]=x[2];
  p.setXYTheta(y);
 
  rb.addRobot(Robot,p);
  int top=(int)(.25/rb.CellWidth)+1;

  for (int i=0; i<top; i++)
    {
      y[0]-=v[0];
      y[1]-=v[1];
      p.setXYTheta(y);
      rb.addRobot(Robot,p);
    }
  unsigned short st= CurrentGrid.getSubType();
  CurrentGrid.setSubType(1);
  CurrentGrid.append(rb);
  CurrentGrid.setSubType(st);
  if ((Robot.Rows==1)&&(Robot.Columns==1))
    {
      int r,c;
      CurrentGrid.getRowColumn(r,c,x);
      if ((Velocity(0,0)<1E-2)&&(Velocity(0,0)>-1E-2))
	{
	  CurrentGrid.Grid.fillDisk(r,c,(Safety/rb.CellWidth),false);
	}
    }
}
bool CollisionAvoidance::limitVelocity(double d,double maxt, double travel)
{
  double m=maxVelRatio(d);
  if (m<1)
    {
      travel/=maxt;
     if (Velocity(0,OnArc)>0)travel/=(Velocity(0,OnArc));
     else travel/=(-Velocity(0,OnArc));
     if (travel>m)
       if (travel<1){
	 m=travel;
       }
     if (m<1E-3)m=1E-3;
     Velocity(0,0)*=m;
     Velocity(0,1)*=m;
     return true;
    }
  return 0;
}
Cure::TimestampedData * CollisionAvoidance::data(int port){
  if (port < 0 || port >= (Ins+Outs)) {
    std::cerr << "WARNING: PoseFilter::data port " << port 
	      << " out of bounds must be >0 and <"
                << Ins+Outs << std::endl;
  }
  if (port==1)
    return &CurrentGrid;
  if (port==0)
    return &CurrentPose;      
  return 0;
}

unsigned short CollisionAvoidance::typeCheck(int port,TimestampedData * inputData)
{
  unsigned short r=0;
  if (inputData)
    {
      if (port==1)
   	if (inputData->getClassType()==GRID_TYPE)
	  {
	    if (inputData->isPacked())
	      {
	      inputData->narrowPackedData()->unpack(*data(port));
	      }
	    else *data(port)=*inputData;   
	  }
	else if (inputData->narrowSmartData())
	  {
	    SmartData *sd=inputData->narrowSmartData();
	    if (sd->getTPointer()->getClassType()==GRID_TYPE)
	      {
		sd->setEqual(*data(port));
	      }
	  }    
	else {  
	std::cerr << "\nDataFilter \"" << FilterName <<"\" warning  "
		  << "Wrong type of input to  "<<port<<":\n";
	return TYPE_ERROR;
	}
      if (port==0)
   	if (inputData->getClassType()==POSE3D_TYPE)
	  {
	    if (inputData->isPacked())
	      {
	      inputData->narrowPackedData()->unpack(*data(port));
	      }
	    else *data(port)=*inputData;   
	  }
	else if (inputData->narrowSmartData())
	  {
	    SmartData *sd=inputData->narrowSmartData();
	    if (sd->getTPointer()->getClassType()==POSE3D_TYPE)
	      {
		sd->setEqual(*data(port));
	      }
	  }    
	else {  
	std::cerr << "\nDataFilter \"" << FilterName <<"\" warning  "
		  << "Wrong type of input to  "<<port<<":\n";
	return TYPE_ERROR;
	}
 }
  if (!Checked[port])return r;
  Checked[port]--;
  return r;
}

} // namespace Cure
