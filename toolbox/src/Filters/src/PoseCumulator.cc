// = RCSID
//    $Id: PoseCumulator.cc ,v 1.1 2004/03/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2004 John Folkesson
//    
#include "PoseCumulator.hh"
#include "CureDebug.hh"
namespace Cure {
  PoseCumulator::PoseCumulator( bool allwaysreset,unsigned short thread)
    :PoseFilter(2,2,thread)
  {
    Poses[0].setSubType(0x8000);
    Poses[2].setSubType(0x8000);
    TimeLast=0.0;
    ResetAllways=allwaysreset;
    in(0)->setSequenced(TimeLast);
    FilterName = "PoseCumulator";
  }
  unsigned short PoseCumulator::transformErrorCode(unsigned short &r1, int port)
  {
    if (r1&(RESOURCE_ERROR)){
      CureCERR(10)<<"DataFilter \"" << FilterName << "\""
		  << " got RESOURCE_ERROR on reading a pull. "
		  <<"Port # "<<port<<std::endl;
      if (Ports[port].DataNeeded)return r1;
    }
    else{
      if (r1&(NO_INTERPOLATE)){		
	CureCERR(10)<<"DataFilter \"" << FilterName << "\""
		    << " Needs to be able to interpolate the time "
		    <<"on reading a pull from "
		    <<"Port # "<<port<<std::endl;
	return r1;
      }
      if (r1&(NO_DATA_AVAILABLE))
	{
	  r1=(TIMESTAMP_ERROR|NO_DATA_AVAILABLE);
	  return r1;
	}
      if (r1&TIMESTAMP_TOO_OLD)
	{
	  r1=(TIMESTAMP_ERROR|TIMESTAMP_TOO_OLD);
	  return r1;
	}
    }
    return 0;
  }
  unsigned short PoseCumulator::resetTimeLast(Timestamp t)
  {
    Pose3D p;
    if (t==0)
      {
	TimeLast=Poses[1].Time;
	Ports[0].setSequenced(TimeLast);
	unsigned short r=Ports[0].pullData(Poses[0],TimeLast);
	if (transformErrorCode(r,0))return r;
	if (r){
	  TimeLast=0.0;
	  return (r);
	} 
	else {
	 
	  r=Ports[1].pullData(p,t);
	  if (transformErrorCode(r,1))return r;
	  r=typeCheck(1,&p);
	  if (r)return r;
	 
	t=Poses[1].Time;
	TimeLast=t;
	t-=10000;
	Poses[2]=Poses[0];
	Poses[2].zero();
	Poses[3]=Poses[1];
	Poses[3].Covariance=0;
	Poses[2].Time=Poses[1].Time;
	Poses[0].Time=Poses[1].Time;
	in(0)->setSequenced(t,-3);
	r=Ports[0].pullData(p,TimeLast);
	if ((r)|(p.Time==0.0)){
	    in(0)->setSequenced(t,-1);;
	    r=Ports[0].pullData(p,TimeLast);
	}
	if (p.Time==0)
	  {
	    TimeLast=0.0;
	    r=(TIMESTAMP_ERROR|NO_DATA_AVAILABLE);
	  }
	t+=10000;
	Ports[0].setSequenced(p.Time); 
	if (transformErrorCode(r,0))
	  Ports[0].setSequenced(TimeLast); 
	else
	  {
	    r=typeCheck(0,&p);
	    if (r)return r;
	    //*get time of latest data if any;
	    r=Ports[1].pullData(p,p.Time);
	    if (transformErrorCode(r,1))return r;
	    r=typeCheck(1,&p);
	    if (r)return r;
	    TimeLast=Poses[1].Time;
	  }
	Poses[2]=Poses[0];
	Poses[2].zero();
	Poses[3]=Poses[1];
	Poses[3].Covariance=0;
	Poses[2].Time=Poses[1].Time;
	Poses[0].Time=Poses[1].Time;
	}
	return 0;
      }  
    //    TimeLast=t;
    t-=10000;
    in(0)->setSequenced(t,-1);;
    //get time just before t
    unsigned short r1=Ports[0].pullData(p,t);
    t+=10000;
    if (p.Time==0)
      {
	TimeLast=0.0;
	r1=(TIMESTAMP_ERROR|NO_DATA_AVAILABLE);
      }
    Ports[0].setSequenced(p.Time); 
    if (transformErrorCode(r1,0))
      Ports[0].setSequenced(TimeLast); 
    else
      {
	r1=typeCheck(0,&p);
	if (r1)return r1;
	//*get time of latest data if any;
	r1=Ports[1].pullData(p,p.Time);
	if (transformErrorCode(r1,1))return r1;
	r1=typeCheck(1,&p);
	if (r1)return r1;
	TimeLast=Poses[1].Time;
      }
    Poses[2]=Poses[0];
    Poses[2].zero();
    Poses[3]=Poses[1];
    Poses[3].Covariance=0;
    Poses[2].Time=Poses[1].Time;
    Poses[0].Time=Poses[1].Time;
    p.Time=t;
    //Now move to time t input 1
    r1=getInputs(p,-1);
   if (transformErrorCode(r1,0))
      return r1;
    //Now move to time t input 0 and set Poses[2] and Poses[3] for 
    //this last interval before t
    r1=calc();
    if (r1)return r1;
    return 0;
  }
  
  unsigned short PoseCumulator::getInputs(TimestampedData &inputData,int port)
  {
    if ((port!=1)&&(port!=-1)){
      CureCERR(10) << "\nDataFilter \"" << FilterName <<"\" warning  "
		     << "Trying to get input port number "<<port
		   <<" I only allow write to ports -1 and 1.";
      return (ADDRESS_INVALID|RESOURCE_ERROR);
    }
  if (port==1) {
    if (typeCheck(port,&inputData)){
      CureCERR(10) <<"DataFilter \"" << FilterName << "\""
		   << " got an invalid type written to it. "<<
	  "Port # "<<(port)<<" pulling instead"<<std::endl; 
      port=-1;
    }       
  }
  ErrorCode=0;
  if (inputData.Time==0)
    ErrorCode=TIMESTAMP_ERROR;
  if ((TimeLast>inputData.Time))
    {
      if (!(inputData.Time==0))
	CureCERR(10)<<"WARNING DataFilter \"" << FilterName 
		    << " tried to calc into past"
		    <<" port "<<port<<" current Timestamp\n";
      ErrorCode=(TIMESTAMP_ERROR| TIMESTAMP_TOO_OLD);
    }
  if (ErrorCode) return ErrorCode;
  if(port!=1)
    {
      //if( inputData.Time==0)return reset(inputData.Time);
      Pose3D p;
      unsigned short r1=Ports[1].pullData(p,inputData.Time);
      ErrorCode=(r1);
       if (transformErrorCode(r1,1))return r1;
      //Here we set Poses[1]=p if type matches;
      r1=(r1|typeCheck(1,&p));
      if (r1&(TYPE_ERROR)){
	std::cerr<<"DataFilter \"" << FilterName << "\""
		 << " got a type error on reading a pull. "<<
	  "Port # 1"<<" Disconnecting "<<r1<<std::endl;
	Ports[1].IsPull=false;
      }
      if (inputData.Time<p.Time){
	if (!(inputData.Time==0.0))
	  CureCERR(20)<<"warning Filter "<<FilterName
		      <<" has gotten too far behind the input\n";
	ErrorCode=(ErrorCode|r1|TIMESTAMP_TOO_OLD);
	if (Poses[0].Covariance.Rows){
	  double d=(p.Time.getDouble()-TimeLast.getDouble())*1E-3;
	  Matrix b=Poses[0].Covariance;
	  b=d;
	  Poses[0].Covariance+=d;
	}
	TimeLast=p.Time;
	Transformation3D trans;
	trans-=Poses[3];
	trans+=p;
	Poses[0].Transformation3D::operator=(trans);
	in(0)->setSequenced(TimeLast);
      }
      if ((inputData.Time>Poses[1].Time)) 
	ErrorCode=(ErrorCode|r1|TIMESTAMP_ERROR);	 
    }
  return ErrorCode;
  
  }
  /* 
     Here the Poses[1] timestamp should be the time to go to and the data
     there the cumulated pose for Poses[3].
     The Poses[0] time is at the last data pulled from source interpolted.
     It could be >= the time to go to which means we should pull no more.
  */
  unsigned short PoseCumulator::calc()
  {  
  unsigned short r=ErrorCode;
  ErrorCode=0;
  if (r)
    return r;
  if (Ports[0].IsPull==false){
    std::cerr<<"Warning "<<FilterName<<" must pull on 0\n";
    return RESOURCE_ERROR;
  }
  if (Poses[1].Time==0) return TIMESTAMP_ERROR; 
  if (TimeLast==0)
    {
      return resetTimeLast(TimeLast);
      TimeLast=Poses[1].Time;
      Ports[0].setSequenced(TimeLast);
      unsigned short r1=Ports[0].pullData(Poses[0],TimeLast);
	if (r1){
	  TimeLast=0.0;
	  return (r1);
	} 
	else {
	  Poses[2]=Poses[0];
	  Poses[2].zero();
	  Poses[3]=Poses[1];
	  Poses[3].Covariance=0;
	  Poses[2].Time=Poses[1].Time;
	  Poses[0].Time=Poses[1].Time;
	}
	return 0;
      }  
    Transformation3D trans;
    trans-=(Poses[3]);
    trans+=(Poses[1]);
    Pose3D p;
    p=Poses[0];
    Poses[2].zero();
    Poses[2].Time=Poses[3].Time;
    if (!(p.Time<Poses[1].Time))
      TimeLast=Poses[3].Time;
    while (p.Time<Poses[1].Time)
      {
	TimeLast=p.Time;
	Poses[2].add(Poses[2],p); // here we cumulate the poses
	//here we get the next in the slot
	Poses[2].Time=p.Time;
	unsigned short r1=Ports[0].pullData(p,Poses[1].Time); 
	if (r1&TIMESTAMP_TOO_OLD)
	  {
	    CureCERR(50)<<r1<<"ERROR WARNING\n        "<<FilterName
			<<" got TIMESTAMP_TOO_OLD read form port 0\n";
	    p=trans;
	    p.Time=Poses[1].Time;
	    Ports[0].setSequenced(p.Time);
	    if (p.Covariance.Rows){
	      double d=(p.Time.getDouble()-TimeLast.getDouble())*1E-3;
	      Matrix b=p.Covariance;
	      b=d;
	      p.Covariance+=d;
	    }
	  }
	else if (r1){
	  CureCERR(40)<<r1<<"ERROR WARNING\n        "
		      <<FilterName<<" got error read form port 0\n"
		      <<"        Did you write to input 1 slot before input 0's\n";
	  return RESOURCE_ERROR;
	}
      }
    //Now we have p.Time>=Interpolate time and 
    //Poses[2] is at TimeLast<Interpolate Time
    Poses[3]=Poses[1];
    Poses[2].Time=TimeLast;
    Poses[0].interpolate_(Poses[2],p,Poses[1].Time);
    Poses[0].Time=Poses[1].Time;
    Poses[2].add(Poses[2],Poses[0]);
    Poses[2].Time=Poses[0].Time;
    Poses[0].Transformation3D::operator=(Poses[0].inverse());
    Poses[0].Transformation3D::operator+=(p);
    Poses[0].Time=p.Time;
    for (int i=0; i<Poses[0].Covariance.Rows;i++)
      for (int j=0; j<Poses[0].Covariance.Columns;j++)
	Poses[0](i,j)=p(i,j)-Poses[0](i,j);
    Poses[2].Transformation3D::operator=(trans);
    return (0);
  
  }
  unsigned short PoseCumulator::read(TimestampedData& p,const int port, 
				const Timestamp t,
				const int interpolate)
  {
    unsigned short r=0;
    if ((port==2)||(port==3))
      {
      if (ResetAllways)
	{
	  if (time(port)==t)
	    {
	      if (TimeLast==p.Time)
		{
		  setEqual(port,p);
		  return 0;
		}
	    }
	  r=resetTimeLast(p.Time);
	}
      else
	{
	  if (time(port)==t)
	    {
	      setEqual(port,p);
	      return 0;
	    }
	}
      }
    bool fix=false;
    if ((interpolate==0)&&(p.Time==t))
      fix=true;
    if (r)return r;
    if ((t==0.0)&&(ResetAllways))
      return PoseFilter::read(p,port,p.Time,interpolate);
    r=PoseFilter::read(p,port,t,interpolate);
    if (fix)      
      {
	Matrix x(6,1);
	Poses[2]=x.Element;	
	Poses[2].Covariance=0;
	if (port==2)p=Poses[2];
      }
    return r;
}
  unsigned short PoseCumulator::typeCheck(int port,TimestampedData *inputData)
  {
    unsigned short r=0;
    r=PoseFilter::typeCheck(port,inputData);
    if (r)return r;
    if (!Checked[port])return r;
    Checked[port]--;
    if ((port==0))
      {
	if (((Poses[port].getSubType())&(0x8000))) return 0;
	Poses[port].setSubType(Poses[port].getSubType()|0x8000);
	Poses[port].zero();
      }
    if ((port==1))
      {
	if (!((Poses[port].getSubType())&(0x8000))) return 0;
	Poses[port].setSubType(Poses[port].getSubType()&0x7FFF);
	Poses[port].zero();
      }
    return TYPE_ERROR;
  }

}//Cure
