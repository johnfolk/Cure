// = AUTHOR(S)
//    John Folkesson
//    
//    May 11, 2004
//
//    Copyright (c) 2004 John Folkesson
//    

#define TWO_COLUMN_W

#include "RangeHough.hh"

#include <iostream>
#include <sstream>

using namespace Cure;

RangeHough::RangeHough()
{
  Sick=0;
  Points.reallocate(361,2);
  Accumulator1.reallocate(45,32);
  Accumulator2.reallocate(8,8);
  NumberOfPoints=361;
  BottomIndex=0;
  memset(ScanIndex, -1, 361* sizeof(short)); 
  reset1();
  reset2();
  memset(ScaledScan, 0, 361* sizeof(double)); 
  MaxGamma1=0;
  MaxRho1=0;
  MaxR2=0;
  MaxC2=0;
  VarRange=4E-6;
  SensorVariance=1E-4;
  Cloud.Cloud.reallocate(361,3);
  Cloud.Cloud=0;
  setThresholds(1,       // minimum line length [m]
                10,      // minimum numb points per line
                0.09,    // RhoThreshold [m]
                0.0027,  // VarRhoThreshold [m2]
                0.0075,  // HalfWidth [rad]
                0.017,   // DGamma [rad]
                2.0,     // MaxGap [m]
                60,      // MaxRange [m]
                2);      // Scan resolution in half degs
  EndPointBehind=.05;
  Lines.setNumberOfElements(180);
  Lines.Measurements[0].MeasurementType = 0;
  Lines.Measurements[0].SensorType = 0;
  Lines.Measurements[0].SensorID = 0;
  Lines.Measurements[0].Key = 0;
  Lines.Measurements[0].Z.reallocate(2,1);
  Lines.Measurements[0].Z = 0;
  Lines.Measurements[0].BoundingBox.reallocate(2);
  Lines.Measurements[0].BoundingBox = 0;
  Lines.Measurements[0].W.reallocate(0,2);
  Lines.Measurements[0].W = 0;
  Lines.Measurements[0].V.reallocate(4,1);
  Lines.Measurements[0].V = 0;
  Lines.Measurements[0].CovV.reallocate(4);
  Lines.Measurements[0].CovV = 0;
  Lines.setupMeasurements();
  Lines.setNumberOfElements(0);
  RhoBiasThreshold = -1;
}
void RangeHough::printConfiguration()
{
  std::cerr << "\nSuccessfully configured RangeHough with Scan Setting:\n"
	    << "  HalfWidth=" << HalfWidth
	    << " MaxRange=" << MaxRange
	    << " IndexIncrement=" << IndexIncrement
	    << " VarRange=" << VarRange
	    << " SensorVariance=" << SensorVariance
	    <<"\nLineSettings:\n"
	    << "  LengthThreshold=" << sqrt(LengthSqThreshold)
	    << " CountThreshold=" << CountThreshold
	    << " RhoThreshold=" << RhoThreshold
	    << " VarRhoThreshold=" << VarRhoThreshold
    
	    << " DGamma=" << DGamma
	    << " MaxGap=" << MaxGap
	    << " RhoBiasThreshold=" << RhoBiasThreshold
	    << " EndPointBehind=" << EndPointBehind
	    << std::endl;
  

}
int
RangeHough::config(const std::string &arglist) 
{
  std::istringstream str(arglist);
  int version = -1;

  if ( !(str >> version)) {
    std::cerr << "RangeHough::config() "
              << "Failed to read version number for config params list\n";
    return 1;
  }  
  int ret = 0;
  switch (version) {
  case 1:
    ret = configVer1(arglist);
    break;
  default:
    std::cerr << "RangeHough::config() "
              << "Cannot handle config version " << version << std::endl;
    return 1;
  }

  return ret;  

  return 0;
}

int
RangeHough::configVer1(const std::string &arglist)
{
  std::istringstream str(arglist);
  int version = -1;
  double minlength;
  short minPointsPerLine;
  double rhoThreshold;
  double varRhoThreshold;
  double halfWidth;
  double dGamma; 
  double maxGap;
  double maxrange;
  short scanResHalfDegrees;
  double sensorVariance;
  double variancePerSquareRange;
  int subcfg=-1;
  if ( !(str >> version>>subcfg)) {
    std::cerr << "RangeHough::config() "
              << "Failed to read subcfg number for config params list\n";
    return 1;
  }  
  switch (subcfg) {
  case 0:
    if (str >> halfWidth >> maxrange >> scanResHalfDegrees 
	>>sensorVariance >> variancePerSquareRange
	>>minlength >> minPointsPerLine
	>> rhoThreshold >> varRhoThreshold >> dGamma
	>> maxGap) {
      setSensorVariance(sensorVariance, variancePerSquareRange);
      setThresholds(minlength, minPointsPerLine, 
		    rhoThreshold, varRhoThreshold,
		    halfWidth, dGamma, maxGap,
		    maxrange, scanResHalfDegrees);
      if (str >> RhoBiasThreshold 
	  >> EndPointBehind) {	
	return 0;
      }      
      return 0;
    }
    std::cerr << "RangeHough::configVer1() "
	      << "Need to specify at least the first 11 config params\n";
    break;
  case 1:
    if (str >>halfWidth >> maxrange >> scanResHalfDegrees 
	>>sensorVariance >> variancePerSquareRange){
      setSensorVariance(sensorVariance, variancePerSquareRange);
      setThresholds(sqrt(LengthSqThreshold),CountThreshold, 
		     RhoThreshold, VarRhoThreshold,
		    halfWidth, DGamma, MaxGap,
		    maxrange, scanResHalfDegrees );
      return 0;
    }
    break;
  case 2:
    if (str >> minlength >> minPointsPerLine
	>> rhoThreshold >> varRhoThreshold >> dGamma
	>> maxGap) {
      setThresholds(minlength, minPointsPerLine, 
		    rhoThreshold, varRhoThreshold,
		    HalfWidth, dGamma, maxGap,
		    MaxRange, IndexIncrement);
      if (str >> RhoBiasThreshold 
	  >> EndPointBehind) {	
	return 0;
      }      
      return 0;
    }
    break;
  default:
    std::cerr << "RangeHough::config() "
	      << "Cannot handle subcfg version " << subcfg << std::endl;
    return 1;
  }
  return 1;
}


void RangeHough::setThresholds(double minlength, short minPointsPerLine, 
                               double rhoThreshold, double varRhoThreshold,
                               double halfWidth, double dGamma, double maxGap,
                               double maxrange,short scanResHalfDegrees)
{  
  IndexIncrement=scanResHalfDegrees;
  if (minPointsPerLine>2)CountThreshold=minPointsPerLine;  
  LengthSqThreshold=minlength*minlength;
  WeightThreshold=(sqrt(LengthSqThreshold)*CountThreshold)/
    (IndexIncrement*.0087*(CountThreshold-1));  
  setMaxRange(maxrange);

  RhoThreshold = rhoThreshold;
  VarRhoThreshold = varRhoThreshold;
  DGamma = dGamma;
  MaxGap = maxGap;
  HalfWidth = halfWidth;
}
void RangeHough::increaseLineQuality(double factor)
{
  RhoThreshold/=factor;
  RhoBiasThreshold/=factor;
  factor*=factor;
  VarRhoThreshold/=factor;    
}

void RangeHough::removeClutter(double factor)
{
  CountThreshold=(int)((double)factor*(double)CountThreshold);
  factor*=factor;
  LengthSqThreshold*=factor;
  WeightThreshold=(sqrt(LengthSqThreshold)*CountThreshold)/
    (IndexIncrement*.0087*(CountThreshold-1));    
}

void RangeHough::increaseMinCovariance(double factor)
{
  factor*=factor;
  VarRange*=factor;
  SensorVariance*=factor;
}

void RangeHough::setBiasThreshold(double rhoBiasTreshold)
{
  RhoBiasThreshold = rhoBiasTreshold;
}

void RangeHough::setMaxRange(double maxrange)
{
  if (maxrange)MaxRange=maxrange;
  //This is empirical to optimize for extra long MaxRange and or 
  //low count thresholds.
  //The problem is lots of crap faling into Accumulator1 due
  // to the course resolution.
  if (MaxRange)
    AdjustedWeightThreshold=(MaxRange*MaxRange)/LengthSqThreshold;
  else AdjustedWeightThreshold=0;
  AdjustedWeightThreshold/=3600;
  if (AdjustedWeightThreshold<1)
    AdjustedWeightThreshold=pow(AdjustedWeightThreshold,0.7925);
  else AdjustedWeightThreshold=(AdjustedWeightThreshold-1)*1.7+1;
  AdjustedWeightThreshold*=(CountThreshold*1.7-4.2);
  AdjustedWeightThreshold/=(CountThreshold-2);
  AdjustedWeightThreshold+=1;
  AdjustedWeightThreshold*=WeightThreshold;
  
}
void RangeHough::reset1()
{
  memset(Accumulator1.Element, 0, 1440* sizeof(double));
  Count1=0;
}
void RangeHough::reset2()
{
  memset(Accumulator2.Element, 0, 64* sizeof(double));
  Count2=0;
}
void RangeHough::setScan(SICKScan *s)
{
  NumberOfLines=0;
  Lines.setNumberOfElements(0);
  Lines.Time = s->Time;
  memset(ScanIndex, -1, 361* sizeof(short));  
  //  LinePoints.Element=LinePointsPtr;
  // LinePoints.Rows=0;
  Sick=s;
  if (!MaxRange)setMaxRange(Sick->getMaxRange());
  double adj2=256/MaxRange;
  NumberOfPoints=Sick->getNPts();
  // memset(LineKeys, -1, NumberOfPoints* sizeof(short));
  double as=Sick->getAngleStep()*360/M_PI+.5;
  IndexIncrement=(short)as;
  as=Sick->getStartAngle()*Sick->getAngleStep()*360/M_PI+.5;
  BottomIndex=(short)as;
  short topi=BottomIndex+IndexIncrement*NumberOfPoints;
  int si=0;
  short sk=BottomIndex*23;
  short inck=IndexIncrement*23; 
  for (short i=BottomIndex; i<topi;i+=IndexIncrement,si++,sk+=inck)
    {
      short k=sk;
      Points(i,0)=(*Sick)(si)*Trigger.cosOfHalf(i);
      Points(i,1)=(*Sick)(si)*Trigger.sinOfHalf(i);
      ScaledScan[i]=(*Sick)(si)*adj2;
      double d=ScaledScan[i]/8;
      if (d<31.95)
	{
	  short jmin=(i>>4);
	  short jmax=((i+7)>>4)+22;
	  short a=((jmin<<4)-172-i);
	  ScanIndex[i]=si;
	  for (short top=a+((jmax-jmin)<<4); a<top; a+=16,k++)
	    {
 	      double r=d*Trigger.cosOfHalf(a)+.5;
	      Rho1[k]=((short)r)-1;
	    }
	  if (k<23*(i+1))
	    {
	      Rho1[k]=-8;
	    }
	}
    }
  fillAcc1();  
}
void RangeHough::fillAcc1()
{
  reset1();
  short sk=BottomIndex*23;
  short inck=IndexIncrement*23;
  for (short i=BottomIndex; i<361;i+=IndexIncrement,sk+=inck)
    {
      if (ScanIndex[i]>-1)
	{
	  short jmin=(i>>4);
	  short k=sk;
	  short jmax=jmin+23;
	  for (short j=jmin;j<jmax;j++,k++)
	    {
 	      if ((Rho1[k]>-2)&&(Rho1[k]<32))
		{
		  if (Rho1[k]>-1)
		    Accumulator1(j,Rho1[k])+=(*Sick)(ScanIndex[i]);
		  if (Rho1[k]<31)
		    Accumulator1(j,Rho1[k]+1)+=(*Sick)(ScanIndex[i]);
		}
	    }
	}
    }
  int k=0;
  for (short i=0; i<1440;i++)
    if( Accumulator1.Element[i]>AdjustedWeightThreshold)
      {
	Acc1Indecies[k]=i;
	k++;
      }
  Acc1Count=k;
}
int RangeHough::findMax1()
{
  short maxi=-1;
  double max=0;
  short k=0;
  for (short i=0; i<Acc1Count;i++)
      {
	Acc1Indecies[k]=Acc1Indecies[i];
	k++;
	if( Accumulator1.Element[Acc1Indecies[i]]>max)
	  {
	    max=Accumulator1.Element[Acc1Indecies[i]];
	    maxi=Acc1Indecies[i];
	  }
      }
  Acc1Count=k;
  if (max<WeightThreshold)return 1;
  short maxR1=(maxi>>5);
  short maxC1=maxi-(maxR1<<5);
  MaxGamma1=(maxR1<<4)-172; //center of Acc1 cell units halfdegree  
  MaxRho1=(maxC1<<3)+4;   //center of Acc1 cell units MaxRange/256
  Cloud.Orientation=Trigger.halfToRads(MaxGamma1);
  short mini=(((maxR1-22)<<4));
  if (maxR1<23)mini=0;
  short imax=((maxR1<<4)+15);
  if (imax>360)imax=360;
  k=mini*23;
  short ind=0;
  for (short i=mini; i<imax;i++,k+=23)
    {
      if (ScanIndex[i]>-1)
	{
	  short mk=maxR1-(i>>4); 
	  mk+=k;
	  if ((Rho1[mk]==maxC1)||((Rho1[mk]+1)==maxC1))
	    {
	      Acc1Result[ind]=i;
	      ind++;
	    }
	}    
    }
  Count1=ind;
  if (Count1<CountThreshold)
    {
      Accumulator1.Element[maxi]=0;
      return -1;
    }
  
  return 0;
}
void RangeHough::fillAcc2()
{
  reset2();
  short minj=MaxGamma1-7;
  short minrho=MaxRho1-4;
  for (short i_=0; i_<Count1;i_++)
    {
      short i= Acc1Result[i_];
      short k=(i<<3);
      double d=ScaledScan[i];
      short j=0;
      for (short a=(minj-i); j<8; a+=2,k++,j++)
	{
	  double r=d*Trigger.cosOfHalf(a)+.5;
	  Rho2[k]=((short)r)-minrho;
	  if ((Rho2[k]>-2)&&(Rho2[k]<8))
	    {
	      if (Rho2[k]>-1)
		Accumulator2(j,Rho2[k])+=(*Sick)(ScanIndex[i]);
	      if (Rho2[k]<7)
		Accumulator2(j,Rho2[k]+1)+=(*Sick)(ScanIndex[i]);
	    }
	}
    }
  int k=0;
  for (short i=0; i<64;i++)
    if( Accumulator2.Element[i]>WeightThreshold)
      {
	Acc2Indecies[k]=i;
	k++;
      }
  Acc2Count=k;
}
int RangeHough::findMax2()
{
  short maxi=-1;
  double max=0;
  MaxR2=-1;
  MaxC2=-1;
  for (short i=0; i<Acc2Count;i++)
    if( Accumulator2.Element[Acc2Indecies[i]]>max)
      {
	max=Accumulator2.Element[Acc2Indecies[i]];
	maxi=Acc2Indecies[i];
      }
  if (max<WeightThreshold)
    {
      //      Accumulator1(((MaxGamma1+172)>>4),((MaxRho1-4)>>3))=0;
      return 1;
    }
  MaxR2=(maxi>>3);
  MaxC2=maxi-(MaxR2<<3);
  Gamma=MaxGamma1;
  Gamma+=(MaxR2<<1);
  Gamma-=7;
  Cloud.Orientation=Trigger.halfToRads(Gamma);
  short ind=0;
  for (short i_=0; i_<Count1;i_++)
    {
      short i= Acc1Result[i_];
      short mk=MaxR2+(i<<3);
      if ((Rho2[mk]==MaxC2)||((Rho2[mk]+1)==MaxC2))
	{
	  Acc2Result[ind]=i;
	  ind++;
	}
    }
  Count2=ind;
  if (Count2<CountThreshold)
    {
      Accumulator2.Element[maxi]=0;
      return -1;
    }
  return 0;
}
void RangeHough::emptyAccumulators(short key)
{
  int start=0;
  int top=LineCount;
  
 if (key&1)
    {
      int k=LineResult[0];
       double ang=Trigger.halfToRads(k);
      if (ang>(Cloud.Gamma+HalfWidth))start=1;
    }
  if (key&2)
    {
      int k=LineResult[LineCount-1];
       double ang=Trigger.halfToRads(k);
      if (ang<(Cloud.Gamma-HalfWidth))top--;
    }
    for (short i_=start; i_<top; i_++)
    {
      short i=LineResult[i_];
      short ind=ScanIndex[i];     
      ScanIndex[i]=-1;           
      short k=(i<<3);
      for (short j=0; j<8;k++,j++)
	{
	  if ((Rho2[k]>-2)&&(Rho2[k]<8))
	    {
	      if (Rho2[k]>-1)
		Accumulator2(j,Rho2[k])-=(*Sick)(ind);
	      if (Rho2[k]<7)
		Accumulator2(j,Rho2[k]+1)-=(*Sick)(ind);
	    }
	  Rho2[k]=-8;
	}
      k=i*23;
      short jmin=(i>>4);
      short jmax=jmin+23;
      for (short j=jmin;j<jmax;j++)
	{
	  if ((Rho1[k]>-2)&&(Rho1[k]<32))
	    {
	      if (Rho1[k]>-1)
		Accumulator1(j,Rho1[k])-=(*Sick)(ind);
	      if (Rho1[k]<31)
		Accumulator1(j,Rho1[k]+1)-=(*Sick)(ind);
	    }
	  Rho1[k]=-8;
	  k++;
	}

    }
}
int RangeHough::fitLine()
{
  if (Count2<CountThreshold)
    {
      Accumulator2(MaxR2,MaxC2)=0;
      return 1;
    }
  double n[2];
  n[0]=Trigger.cosOfHalf(Gamma);
  n[1]=Trigger.sinOfHalf(Gamma);
  LineResult[0]=Acc2Result[0];
  short i=Acc2Result[0];
  double d=n[0]*Points(i,0)+n[1]*Points(i,1);
  double t=-n[1]*Points(i,0)+n[0]*Points(i,1);
  short k=1;
  short j=0; 
  for (short i_=1; i_<Count2; i_++)
    {
      i=Acc2Result[i_];
      double d2=n[0]*Points(i,0)+n[1]*Points(i,1);
      double t2=-n[1]*Points(i,0)+n[0]*Points(i,1);
      double drho=RhoThreshold*(1+DGamma*(t2-t));
      if (((d2>d-drho)&&(d2<d+drho))&&((t2-t)<MaxGap))
	{
	  d=d2;
	  t=t2;
	  LineResult[k]=i;
	  k++;
	}
      else
	{
	  Acc2Result[j]=i;
	  j++;
	}
    }
  Count2=j;
  LineCount=k;
  if (LineCount>CountThreshold) {
    Cloud.Cloud.Rows=LineCount;
    for (short i_=0; i_<LineCount; i_++) {
      Cloud(i_,1)=Points(LineResult[i_],0);
      Cloud(i_,2)=Points(LineResult[i_],1);
    }
    Cloud.fitLine();

    k=0;
    for (short i=0; i<LineCount; i++) {
      double d=Cloud.Normal[0]*Cloud(i,1)+Cloud.Normal[1]*Cloud(i,2);
      d-=Cloud.Rho;
      if ((d<RhoThreshold)&&(d>-RhoThreshold)) {
        LineResult[k]=LineResult[i];
        k++;
      }
    }
    
    if (k!=LineCount) {
      LineCount=k;
      if (LineCount>CountThreshold)  {
        Cloud.Cloud.Rows=LineCount;
        for (short i_=0; i_<LineCount; i_++) {
          Cloud.Cloud(i_,1)=Points(LineResult[i_],0);
          Cloud.Cloud(i_,2)=Points(LineResult[i_],1);
        }
        Cloud.fitLine();
      }
      return -1;
    }

    if (RhoBiasThreshold >= 0 && LineCount >= 5) {
      // Test if the points have a too large rho bias indicating that
      // it is not a line structure we have matched against.
      double drho=Cloud.Normal[0]*Cloud(0,1)+Cloud.Normal[1]*Cloud(0,2);
      drho -= Cloud.Rho;        
      double lastDRho = drho;
      double sum = 0;
      for (short i=1; i<LineCount; i++) {
        drho = Cloud.Normal[0]*Cloud(i,1)+Cloud.Normal[1]*Cloud(i,2);
        drho -= Cloud.Rho;
        
        sum += drho * lastDRho;
        
        lastDRho = drho;
      }
      
      if (sum < 0) sum = 0;
      else sum = sqrt(sum / (LineCount - 1) );
      
      if (sum > RhoBiasThreshold) {
        
        //std::cerr << "Removing line with bias sum=" << sum 
        //          << " and LineCount=" << LineCount << std::endl;
        
        return -2;
      } else {
        //std::cerr << "Bias OK sum=" << sum
        //          << " and LineCount=" << LineCount << std::endl;
      }
    }

    if (Cloud.SigmaRho<VarRhoThreshold)	{
      double l= Points(LineResult[0],0)-Points(LineResult[LineCount-1],0);
      l*=l;
      double e= Points(LineResult[0],1)-Points(LineResult[LineCount-1],1);
      e*=e;
      e+=l;
      if (e>LengthSqThreshold) return 0;
    }
  }

  return -1;
}

int RangeHough::findLines(SICKScan *s)
{
  if (s)setScan(s);
  int test1=1;
  while (test1)
    {
      int r1=-1;
      while (r1==-1)r1=findMax1();
      if (r1==0)
	{
	  fillAcc2();
	  int test2=1;
	  while (test2)
	    {
	      int r2=-1;
	      while (r2==-1)r2=findMax2();
	      if (r2==0)
		{
		  int test3=1;
		  while (test3)
		    {
		      int r3=-1;
		      while (r3==-1)r3=fitLine();
		      if (r3==0) {
                        short key=getKey();
                        adjustsLinePoints();
                        Lines(NumberOfLines).MeasurementType=((key<<1)+1);
                        emptyAccumulators(key);	
                        NumberOfLines++;      
                      } else {
                        if (r3 == -2) {
                          emptyAccumulators(getKey());	
                        }
                        test3 = 0;
                      }
		    }
		}
	      else
		{
		  Accumulator1(((MaxGamma1+172)>>4),((MaxRho1-4)>>3))=0;
		  test2=0;
		}
	    }
	}
      else test1=0;
    }
  Lines.setNumberOfElements(NumberOfLines);

  // Go through the measurements and set the SensorType flags
  for (int i = 0; i < NumberOfLines; i++) {
    Lines.Measurements[i].SensorType = s->SensorType;
    Lines.Measurements[i].SensorID = s->SensorID;
  }

  return NumberOfLines;
}
  

short RangeHough::findLine(short startindex, short endindex, double gamma, 
			   double rho, double dgamma, double drho)
{
  MaxGamma1=Trigger.toHalfDegrees(gamma);
  double d=(rho/MaxRange*256);
  MaxRho1=(short)d;
  Cloud.Orientation=gamma;
  Gamma=Trigger.toHalfDegrees(gamma); 
  short match=-1;
  short boti=startindex*IndexIncrement+BottomIndex;
  short topi=BottomIndex+IndexIncrement*endindex+1;
  Count1=0;
  for (short i=boti;i<topi;i+=IndexIncrement)
    {
      if(ScanIndex[i]>-1)
	{
	  Acc1Result[Count1]=i;
	  Count1++;
	}
    }
  fillAcc2();
  int test2=1;
  double min=1;
  while (test2)
    {
      int r2=-1;
      while (r2==-1)r2=findMax2();
      if (r2==0)
	{
	  int test3=1;
	  while (test3)
	    {
	      int r3=-1;
	      while (r3==-1)r3=fitLine();
	      if (r3==0)
		{
		  short key=getKey();
		  adjustsLinePoints();
		  Lines(NumberOfLines).MeasurementType=((key<<1)+1);
		  double g=(Cloud.Gamma-gamma)/dgamma;
		  double r=(Cloud.Rho-rho)/drho;
		  g*=g;
		  r*=r;
		  g+=r;
		  if (g<min)match=NumberOfLines;
		  NumberOfLines++;
		  Lines.setNumberOfElements(NumberOfLines);      
		  emptyAccumulators(key);	
		}
	      else test3=0;
	    }
	}
      else test2=0;
    }
  return match;
}
  
short RangeHough::getKey()
{
  short key=0;
  int k=LineResult[0];
  double t=Points(k,1)*Cloud.Normal[0]-
    Points(k,0)*Cloud.Normal[1];
  k-=IndexIncrement;
  int j=1;
  double d=t/Cloud.Rho;
  if (d<0)d=-d;
  if ((d<.5)&&(k>BottomIndex+5))
    {
      double rk=Points(k,0)*Cloud.Normal[0]+
	Points(k,1)*Cloud.Normal[1]-Cloud.Rho;
      if (rk>EndPointBehind)
	{
	  key++;
	  int tst=1;
	  while (tst)
	    {
	      k-=IndexIncrement;
	      j+=IndexIncrement;
	      if (k<BottomIndex)tst=0;
	      else
		{
		  double tk=Points(k,1)*Cloud.Normal[0]-
		    Points(k,0)*Cloud.Normal[1]-t;
		  if ((tk<-.5)&&(j>2))tst=0;
		  else if (tk>0) 
		    {
		      key--;
		      tst=0;
		    }
		  else
		    {
		      rk=Points(k,0)*Cloud.Normal[0]+
			Points(k,1)*Cloud.Normal[1]-Cloud.Rho;
		      double temp=(-tk/.05);
		      if (temp<j)temp=j;
		      if (rk<EndPointBehind*temp)
			{
			  key--;
			  tst=0;
			}
		    }
		}
	    }
	  
	}
    }
  j=1;
  k=LineResult[LineCount-1];
  t=Points(k,1)*Cloud.Normal[0]-
    Points(k,0)*Cloud.Normal[1];
  d=t/Cloud.Rho;
  if (d<0)d=-d;
   k+=IndexIncrement;
  short topi=BottomIndex+IndexIncrement*NumberOfPoints;
  if ((d<.5)&&(k<topi-6))
    {
      double rk=Points(k,0)*Cloud.Normal[0]+
	Points(k,1)*Cloud.Normal[1]-Cloud.Rho;
      if (rk>EndPointBehind)
	{
	  key+=2;
	  int tst=1;
	  while (tst)
	    {
	      k+=IndexIncrement;
	      j+=IndexIncrement;	
	      if (k>=topi)tst=0;
	      else
		{
		  double tk=Points(k,1)*Cloud.Normal[0]-
		    Points(k,0)*Cloud.Normal[1]-t;
		  if ((tk>.5)&&(j>2))tst=0;
		  else if (tk<0) 
		    {
		      key-=2;
		      tst=0;
		    }
		  else
		    {

		      rk=Points(k,0)*Cloud.Normal[0]+
			Points(k,1)*Cloud.Normal[1]-Cloud.Rho;
		      double temp=(-tk/.05);
		      if (temp<j)temp=j;
		      if (rk<EndPointBehind*temp)
			{
			  key-=2;
			  tst=0;
			}
		    }
		}
	    }
	}
    }
  return key;
}
 
void RangeHough::adjustsLinePoints()
{
  double gam=Cloud.Gamma;
  for (short i=0; i<LineCount;i++)
    { 
      int k=LineResult[i];
      int si=ScanIndex[k];     
      double tempx=(*Sick)(si);
      double ang=Trigger.halfToRads(k);
      if (ang>(gam+HalfWidth)) ang-=HalfWidth;
      else if (ang<(gam-HalfWidth))ang+=HalfWidth;
      Cloud(i,1)=tempx*cos(ang);
      Cloud(i,2)=tempx*sin(ang);
    }
  Cloud.fitLine();
  //double rho=Cloud.Rho;
  double *n=Cloud.Normal;
  double var2=0;  
  for (short i=0; i<LineCount;i++)
    { 
      int k=LineResult[i];
      int si=ScanIndex[k];
      double tempx=(*Sick)(si);
      double ns=Points(k,0)*n[0]+Points(k,1)*n[1];
	
	
	
	/*
	double d=(ns*ns);
	var2+=(d*VarRange); 
	d/=(tempx*tempx);
	var2+=(1-d)*SensorVariance;
	*/
	double d=(ns/tempx);
	var2+=(d*SensorVariance);
	d*=d;
	var2+=((1-d)*VarRange*tempx*tempx);
	/*	
	  if (tempx>rho)
	  {
	    double cs=Points(k,1)*n[0]-Points(k,0)*n[1];
	    if (cs>1E-9){
	      double dtheta=(ns-rho)/cs;
	      if (dtheta>HalfWidth)dtheta=HalfWidth;
	      else if (dtheta<-HalfWidth)dtheta=-HalfWidth;
	      //  double c=1-(dtheta*dtheta/2);
	      Cloud(i,1)=Points(k,0)-(Points(k,1)*dtheta);
	      Cloud(i,2)=Points(k,1)+(Points(k,0)*dtheta);
	    }
	*/
		/*
	  if (tempx>rho)
	    if (ns>1E-9)
	      {
	      double b=cs/ns;
	      double c=2*(rho-ns)/ns;
	      c=(b*b-c);
	      if (c>0)
		{
		  c=sqrt(c);
		  if (b>0)b-=c;
		  else b+=c;
		  if (b>HalfWidth)b=HalfWidth;
		  else if (b<-HalfWidth)b=-HalfWidth;
		  b/=tempx;
		  c=(b*Points(k,1));
		  b*=Points(k,0);
		  Cloud(i,1)=Points(k,0)-c;
		  Cloud(i,2)=Points(k,1)+b;
		}

	    }
	
	    }
		*/
    }
  if (LineCount>1)
    var2/=LineCount;
  Cloud.SensorVar=SensorVariance;
  Cloud.fitLine();
  if (Cloud.SigmaRho<var2)Cloud.SigmaRho=var2;
  Measurement & m=Lines.Measurements[NumberOfLines];

  // Set the W to be the scan points stored in Cloud.Cloud. Take out
  // the first column of the Cloud.Cloud matrix that holds the scan
  // points with an extra first column with distance that are
  // currently all zero. We do this with a little trick by stepping
  // the element ointer forward one step placing it at the second
  // column on the first row. Then we say that the Cloud only has two
  // column which allows us to copy it as a two column matrix. This is
  // possible snce the internal structure of the matrix still knowns
  // that there are three column internally in the matrix. The
  // resulting matrix will only have two columns though.
#ifdef TWO_COLUMN_W
  Cloud.Cloud.Element++;
  Cloud.Cloud.Columns = 2;
  m.W = Cloud.Cloud;
  Cloud.Cloud.Columns = 3;
  Cloud.Cloud.Element--;
#else
  m.W = Cloud.Cloud;
#endif

  // The measurement
  m.Z(0,0) = Cloud.Gamma;
  m.Z(1,0) = Cloud.Rho;

  // If (nx,ny) is the normal to the Cloud (-ny,nx) is a tangent
  // vector.
  double tx = -n[1];
  double ty = n[0];

  double d;

  // Project first point onto the line. 
  // Note hat this assumes that (tx, ty) is normalized.
  d = (tx * (Cloud.Cloud(0,1)-Cloud.Center[0])+
       ty * (Cloud.Cloud(0,2)-Cloud.Center[1]));
  m.V(0,0) = Cloud.Center[0] + tx * d;
  m.V(1,0) = Cloud.Center[1] + ty * d;

  // Project last point in the Cloud onto the line
  d = (tx * (Cloud.Cloud(LineCount-1,1)-Cloud.Center[0])+
       ty * (Cloud.Cloud(LineCount-1,2)-Cloud.Center[1]));
  m.V(2,0)=Cloud.Center[0]-Cloud.Normal[1]*d;
  m.V(3,0)=Cloud.Center[1]+Cloud.Normal[0]*d;

  if (m.W.Rows<2) {
    m.CovV=Cloud.SigmaRho;
  } else {
    double a;

    // NOTE: n is a pointer to the Cloud.Normal vector

    // We approximate the covariance matrix as the sum of two parts,
    // one from teh distance uncertainty and the other from the end
    // point along the line uncertainty

    // The first part is the uncertainty that comes from the distance
    // uncertainty, i.e. a=CloudSigmaRho
    // 
    //     ( nx )
    // C = (    ) a ( nx ny )
    //     ( ny )
    a=Cloud.SigmaRho;
    m.CovV(0,0)=a*n[0]*n[0];
    m.CovV(0,1)=a*n[0]*n[1];
    m.CovV(1,1)=a*n[1]*n[1];
    m.CovV(2,2)=m.CovV(0,0);
    m.CovV(2,3)=m.CovV(0,1);
    m.CovV(3,3)=m.CovV(1,1);

    // The second contribution is from the uncertainty along the
    // line. We approximate this as the bigger of two factors. The
    // distance between the two point closest to an end point and the
    // size of the beam.

    // Start point
    double dx = m.W(0,1)-m.W(1,1);
    double dy = m.W(0,2)-m.W(1,2);    
    dx*=dx;
    dy*=dy;
    a=dx+dy;   // Squared distance between fist and second point along line

    int si = ScanIndex[LineResult[0]]; //Index of first point
    d = (*Sick)(si);  // distance to first point
    d *= 2.0 * HalfWidth;   // Size of beam at that distance
    d *= d;  // Squared size of beam size
    d *= 2;  // Double it to add some margin

    // Pick bigger of two different uncertainties (end point dist and
    // beam size)
    if (d>a) a=d;  

    // Add uncertainty along line
    // 
    //     ( tx )
    // C = (    ) a ( tx ty )
    //     ( ty )
    m.CovV(0,0)+=a*tx*tx;
    m.CovV(0,1)+=a*tx*ty;
    m.CovV(1,1)+=a*ty*ty;
    m.CovV(1,0)=m.CovV(0,1);  // Make sure matrix is symmetric

    // Repeat for end point
    int k=m.W.Rows-1;
#ifdef TWO_COLUMN_W
    dx=m.W(k,0)-m.W(k-1,0);
    dy=m.W(k,1)-m.W(k-1,1);    
#else
    dx=m.W(k,1)-m.W(k-1,1);
    dy=m.W(k,2)-m.W(k-1,2);    
#endif
    dx*=dx;
    dy*=dy;
    a=dx+dy;

    si=ScanIndex[LineResult[LineCount-1]];
    d=(*Sick)(si);
    d*=2.0*HalfWidth;
    d*=d;
    d*=2;
    if (d>a)a=d;

    m.CovV(2,2)+=a*tx*tx;
    m.CovV(2,3)+=a*tx*ty;
    m.CovV(3,3)+=a*ty*ty;
    m.CovV(3,2)=m.CovV(2,3);
  }

  m.BoundingBox(0,0)=Cloud(0,1);
  m.BoundingBox(0,1)=Cloud(0,2);
  m.BoundingBox(1,0)=Cloud(LineCount-1,1);
  m.BoundingBox(1,1)=Cloud(LineCount-1,2);
  if (m.BoundingBox(0,0)>m.BoundingBox(1,0))
    {
      d=m.BoundingBox(0,0);
      m.BoundingBox(0,0)=m.BoundingBox(1,0);
      m.BoundingBox(1,0)=d;
    }
  if (m.BoundingBox(0,1)>m.BoundingBox(1,1))
    {
      d=m.BoundingBox(0,1);
      m.BoundingBox(0,1)=m.BoundingBox(1,1);
      m.BoundingBox(1,1)=d;
    }

}

