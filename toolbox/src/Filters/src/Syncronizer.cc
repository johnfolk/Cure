// = RCSID
//    $Id: Syncronizer.cc ,v 1.1 2004/03/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2004 John Folkesson
//    


#include "Syncronizer.hh"
#include "CureDebug.hh"
#include "DataFilterAddress.hh"
#include "SmartData.hh"


namespace Cure {

Syncronizer::Syncronizer( int num)
  :DataFilter(num, num, true) 
{
  N = num;
  Data = new SmartData[2 * N];
  LastTime = 0.0;
  FilterName = "Syncronizer";

  m_StoreUnsuccTriggerSignals = true;
  m_MaxNumPendingData = -1;
}

Syncronizer::~Syncronizer()
{
  if (N){
    delete [] Data;
    Data = 0;
    N=0;
  }
}

unsigned short Syncronizer::write(TimestampedData& td,const int port)
{
  if ((!(port<Ins))||(port<-1)){
    std::cerr << "\nSyncronizer \"" << FilterName <<"\" warning  "
              << "Trying to write input port number "<<port
	      <<" I only have "<<(Ins)<<" input ports.\n";
    return (RESOURCE_ERROR|ADDRESS_INVALID);
  }

  CureDO(60) { 
    CureCERR(0) << "Syncronizer \"" << FilterName << "\" being "
                << "written to on port " << port << " @ "
                << Timestamp::getCurrentTime() << " with t="
                << td.getTime() << std::endl;
  }

  if(LastTime>td.Time) return (TIMESTAMP_ERROR| TIMESTAMP_TOO_OLD);
  if (m_StoreUnsuccTriggerSignals) {
    PendingDataList.add(td,port);
   
    // If so desired make sure that list of pending data does not
    // become too long
    if (m_MaxNumPendingData >= 0 && 
        PendingDataList.getLength() > m_MaxNumPendingData) {
      // Remove the first item in the queue
      PendingDataList.remove(0);      
    }
  }


  LastTime=td.Time;
  unsigned short r=0;
  bool test=true;
  while (test) {
    int index=-1;
    
    if (m_StoreUnsuccTriggerSignals) {
      if (PendingDataList.get(0,Trigger,&index)) {
        PendingDataList.remove(0);
        CureDO(60) {
          std::cerr << "Syncronizer::write Nothing more to read @ "
                    << Timestamp::getCurrentTime() << "\n";
        }
        return r; 
      }
    } else {
      Trigger = td;
    }
    
    r=(r|getInputs(*Trigger.getTPointer(),index));
    
    // If we are trying to read data that is too old we simply skip
    // this data and go for the next item in the list hoping that we
    // will eventually catch up
    if (r & TIMESTAMP_TOO_OLD) {
      if (m_StoreUnsuccTriggerSignals) {
        // Remove this item from the list
        PendingDataList.remove(0);      
        
        // Remove the error flags 
        //r &= ( !(TIMESTAMP_ERROR | TIMESTAMP_TOO_OLD) );
        r = 0;
        
        // Start with the next signal in the list, go back to the
        // start of the while(test) loop
        continue;
      } else {
        // We do not store any signals and thus have no other signal
        // to attend to
        std::cerr << "Syncronizer::write  too old data, leaving @ "
                  << Timestamp::getCurrentTime() << "\n";
        return r;
      }
    }
    
    if (r&((TYPE_ERROR|ADDRESS_INVALID))) {
      std::cerr << "Syncronizer::write error="
                << r << " getInputs TYPE_ERROR or ADDRESS_INVALID\n";        
      return RESOURCE_ERROR;        
    }

    if (r&NO_INTERPOLATE){
      std::cerr << "\nSyncronizer \"" << FilterName <<"\" warning  "
                << "must have interpolating pull inputs\n";
      if (m_StoreUnsuccTriggerSignals) PendingDataList.remove(0);
      return r;
    }
    
    if(r&(TIMESTAMP_ERROR|RESOURCE_ERROR)) {
      if (r&RESOURCE_ERROR) {
        std::cerr << "Syncronizer::write error=" << r
                  << " RESOURCE_ERROR\n";
      }
      return (r&(!(TIMESTAMP_ERROR|RESOURCE_ERROR)));
    }

    for (int i=0; i<N;i++) Data[N+i]=Data[i];

    if (m_StoreUnsuccTriggerSignals) PendingDataList.remove(0);

    r=(r|setOutputs());
    
    // If we do not store signals we have no more data to process
    // and thus go out of the while loop
    if (!m_StoreUnsuccTriggerSignals) break;
  }

  CureDO(60) {
    std::cerr << "Syncronizer::write done @ "
              << Timestamp::getCurrentTime() << "\n";
  }

  return r;
}

unsigned short Syncronizer::read(TimestampedData &p,const int port, 
				const Timestamp t,
				const int interpolate)
{
  if (!(port<Ins+Outs)){
    std::cerr << "\nSyncronizer \"" << FilterName <<"\" warning  "
              << "Trying to read port number "<<port
	      <<" I only have "<<(Ins+Outs)<<"  ports.";
    return (RESOURCE_ERROR|ADDRESS_INVALID);
  }
  if (port<Ins){
    std::cerr << "\nSyncronizer \"" << FilterName <<"\" warning  "
              << "Trying to read and input port number "<<port
	      <<" I have "<<(Ins)<<" input ports.";
    return (RESOURCE_ERROR|ADDRESS_INVALID);
  }
  Data[port].setEqual(p);
  if (t==0) return 0;
  if(p.Time==t)
    return 0;
  return (TIMESTAMP_ERROR|NO_INTERPOLATE);
}

}//Cure
