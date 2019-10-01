//
// = LIBRARY
//
// = FILENAME
//    Measurement.cc
//
// = FUNCTION
//
// = AUTHOR(S)
//    Patric Jensfelt
//
// = DESCRIPTION
//    
// = COPYRIGHT
//    Copyright (c) 2004 Patric Jensfelt
//
/*----------------------------------------------------------------------*/

#include "Measurement.hh"

namespace Cure {

Measurement::Measurement()
  :MeasurementType(0),
   SensorType(0),
   SensorID(0),
   Key(0)
{}

Measurement::Measurement(Measurement &m)
{
  (*this)=m;
}

Measurement::~Measurement()
{}
 

void Measurement::operator=(Measurement &m)
{
  MeasurementType = m.MeasurementType;
  SensorType      = m.SensorType;
  SensorID        = m.SensorID;
  Key             = m.Key;

  Z               = m.Z;
  BoundingBox     = m.BoundingBox;
  W               = m.W;
  SW               = m.SW;
  V               = m.V;
  CovV            = m.CovV;
} 
void Measurement::print(int level)
{
  std::cerr<<"Measurement Type: "<<MeasurementType<<" Sensor: "<<SensorType<<":"<<SensorID<<" Key: "<<Key<<"\n";
  if (level&1){
    Z.print();
  }
  if (level&2){
    BoundingBox.print();
  }
  if (level&4){
    W.print();
    SW.print();
  }
  if (level&8){
    V.print();
  }
  if (level&0x10){
    CovV.print();
  }
}

bool Measurement::getMatrix(const std::string & str, ShortMatrix & index,
		       Cure::Matrix & mat)
{
  if ((index.Rows<1)||(index.Columns<4))
    return false;
  if ((index(0,0)<0)||(index(0,1)<0))
    return false;
  if ((index(0,2)<0)||(index(0,3)<0))
    return false;
  bool rowwise=false;
  if (index(0,4)==-1)
    rowwise=true;
  int n=0;
  Cure::Matrix x;
  if (str=="Z"){
    n=Z.Rows*Z.Columns;
    x=Z;
  } else if ((str=="W")||(str=="w")){
    n=W.Rows*W.Columns;
    x=W;
  } else if ((str=="SW")||(str=="sw")){
    n=SW.Rows*SW.Columns;
    x.reallocate(SW.Rows,SW.Columns);
    for (int i=0;i<SW.Rows;i++)
      for(int j=0;j<SW.Columns;j++)
	x(i,j)=SW(i,j);
  } else if ((str=="V")||(str=="v")){
    n=V.Rows*V.Columns;
    x=V;
  } else if ((str=="CovV")||(str=="covv")){
    n=CovV.Rows*CovV.Columns;
    x=CovV;
  } else if ((str=="BoundingBox")||(str=="boundingBox")){
    n=BoundingBox.Rows*BoundingBox.Columns;
    x=BoundingBox;
  } else if ((str=="Key")||(str=="key")){
    n=1;
    x.reallocate(1);
    x=(double)Key;
  }else if ((str=="MeasurementType")||(str=="mtype")){
    n=1;
    x.reallocate(1);
    x=(double)MeasurementType;
  }
  if (n>0){

    if (rowwise){
      // int c=index(0,3);
      int r=index(0,2);
      if (index(0,3)==0)return false;
      mat.grow(mat.Rows+r,index(0,3));
      r=mat.Rows-r;
      int mi=r;
      int mj=0;
      int k=0;
      for (int i=0;i<x.Rows; i++)
	for (int j=0;j<x.Columns;j++){
	  mat(mi,mj)=x(i,j);
	  mj++;
	  k++;
	  if (k==n)return true;
	  if (mj==mat.Columns){
	    mi++;
	    mj=0;
	    if (mi==mat.Rows)return true;
	  }
	}
      return true;
    }
    mat.reallocate(index(0,2),index(0,3));
    int r=index(0,0)+index(0,2);
    int c=index(0,1)+index(0,3);
    if (!(r*c>n)){
      x.offset(index(0,0),index(0,1),index(0,2),index(0,3));
    }
    mat=x;
    return true;
  }
  return false; 
}

bool Measurement::setMatrix(const std::string & str, ShortMatrix & index,
		       Cure::Matrix & mat)
{
  if ((index.Rows<1)||(index.Columns<4))
    return false;
  if ((index(0,0)<0)||(index(0,1)<0))
    return false;
  if ((index(0,2)<0)||(index(0,3)<0))
    return false;
  bool rowwise=false;
  if (index(0,4)==-1)
    rowwise=true;
  int n=0;
  Cure::Matrix x;
  if (str=="Z"){
    n=Z.Rows*Z.Columns;
    x=Z;
  } else if ((str=="W")||(str=="w")){
    n=W.Rows*W.Columns;
    x=W;
  } else if ((str=="SW")||(str=="sw")){
    n=SW.Rows*SW.Columns;
    x.reallocate(SW.Rows,SW.Columns);
    for (int i=0;i<SW.Rows;i++)
      for(int j=0;j<SW.Columns;j++)
	x(i,j)=SW(i,j);
  } else if ((str=="V")||(str=="v")){
    n=V.Rows*V.Columns;
    x=V;
  } else if ((str=="CovV")||(str=="covv")){
    n=CovV.Rows*CovV.Columns;
    x=CovV;
  } else if ((str=="BoundingBox")||(str=="boundingBox")){
    n=BoundingBox.Rows*BoundingBox.Columns;
    x=BoundingBox;
  } else if ((str=="Key")||(str=="key")){
    n=1;
    x.reallocate(1);
    x=(double)Key;
  }else if ((str=="MeasurementType")||(str=="mtype")){
    n=1;
    x.reallocate(1);
    x=(double)MeasurementType;
  }
  if (n>0){
    int c=index(0,1)+index(0,3);
    int r=index(0,0)+index(0,2);
    int cc=x.Columns;
    int rr=x.Rows;
    if (r*c>n)return false;
    if (rowwise){
      if (index(0,3)==0)return false;
      mat.offset(0,0,1,index(0,3));
      x.offset(index(0,0),index(0,1),1,index(0,3));
      x=mat;
      mat.offset(1,0,0,0);
      x.offset(-index(0,0),-index(0,1),rr,cc);
    }else{
      x.offset(index(0,0),index(0,1),index(0,2),index(0,3));
      x=mat;
      x.offset(-index(0,0),-index(0,1),rr,cc);
    }
    if (str=="Z"){
      Z=x;
    } else if ((str=="W")||(str=="w")){
      W=x;
    } else if ((str=="SW")||(str=="sw")){
      SW=x;
    } else if ((str=="V")||(str=="v")){
      V=x;
    } else if ((str=="CovV")||(str=="covv")){
      CovV=x;
    } else if ((str=="BoundingBox")||(str=="boundingBox")){
      BoundingBox=x;
    } else if ((str=="Key")||(str=="key")){
      Key=(unsigned long)x(0,0);
    }else if ((str=="MeasurementType")||(str=="mtype")){
      MeasurementType=(unsigned short)x(0,0);
    }
    return true;
  }
  return false; 
}


}
