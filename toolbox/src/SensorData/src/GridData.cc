//
// = LIBRARY
//
// = FILENAME
//    GridData.cc
//
// = AUTHOR(S)
//    john Folkesson
//
// = COPYRIGHT
//    Copyright (c) 2005 John Folkesson
//
/*----------------------------------------------------------------------*/

#include "GridData.hh"


namespace Cure {

GridData::GridData(double cellwidth,unsigned short id)
{
  init();
  setID(id);
  CellWidth=cellwidth;
}

GridData::GridData(Timestamp &t, unsigned short id)
{
  Time=t;
  init();
  setID(id);
}

/**
 * Create a copy of a GridData
 */
GridData::GridData(const GridData &src)
{
  init();
  (*this) = src;
}

GridData::~GridData()
{}

void GridData::init()
{
  m_Packed =false;
  m_ClassType=(GRID_TYPE);
  setSubType(1);
  setID(0);
  Birthday=Time;
  CellWidth=1.0/32.0;
  SensorType=SensorData::SENSORTYPE_UNKNOWN;
  Offset[0]=0;
  Offset[1]=0;
  Offset[2]=0;
}

GridData& GridData::operator=(const GridData &src)
{
  Time=src.Time;
  setSubType(src.getSubType());
  setID(src.getID());
  Grid=src.Grid;
  Birthday=src.Birthday;
  CellWidth=src.CellWidth;
  Offset[0]=src.Offset[0];
  Offset[1]=src.Offset[1];
  Offset[2]=src.Offset[2];
  return (*this);
}

void GridData::setupGrid(unsigned short rows, unsigned short columns, 
			 double cellwidth, double offset[3], 
			 Cure::Timestamp t, unsigned short gtype)
{
  setSubType(gtype);
  Grid.reallocate(rows,columns);
  if (m_SubType&1)
    Grid=true;
  else 
    Grid=false;
  Time=t;
  Birthday=t;
  CellWidth=cellwidth;
  Offset[0]=offset[0];
  Offset[1]=offset[1];
  Offset[2]=offset[2];
}


void GridData::equalsNot(GridData & gd)
{
  Time=gd.Time;
  Birthday=gd.Birthday;
  Offset[0]=gd.Offset[0];
  Offset[1]=gd.Offset[1];
  Offset[2]=gd.Offset[2];
  setSubType(gd.getSubType());
  CellWidth=gd.CellWidth;
  Grid.equalsNot(gd.Grid);
}
void GridData::trim(int amount)
{
  trimRows(amount);
  trimColumns(amount);
}
void GridData::trimColumns(int amount)
{
  int t=0;
  int r=Grid.Rows;
  int c=Grid.Columns;
  if ((r==0)||(c==0))return;
  if (m_SubType&1)
    t=r;
  Grid.offset(0,0,r,1);
  int cc=c;
  amount=cc-1;
  for (int i=0; i<cc; i++)
    {
      if ((i<amount)&&(Grid.count()==t))
	{
	  c--;
	  Offset[1]+=CellWidth;
	  Grid.offset(0,1,r,1);
	}
      else {
	if (i>0){
	  c++;
	  Grid.offset(0,-1,r,1);
	}
	i=cc;
      }
    }
  cc=c;
  Grid.offset(0,0,r,c);
  if (c==0)return;
  amount=cc-1;
  
  Grid.offset(0,c-1,r,1);
  for (int i=0; i<cc; i++)
    {
      if ((i<amount)&&(Grid.count()==t))
	{
	  c--;
	  Grid.offset(0,-1,r,1);
	}
      else {
	if (i>0){
	  c++;
	  Grid.offset(0,1,r,1);
	}
	i=cc;
      }
    }
  Grid.offset(0,-(c-1),r,c);
}
void GridData::trimRows(int amount)
{
  int t=0;
  int r=Grid.Rows;
  int c=Grid.Columns;
  if ((r==0)||(c==0))return;
  if (m_SubType&1)
    t=c;
  Grid.offset(0,0,1,c);
  int cc=r;
  amount=cc-1;
  for (int i=0; i<cc; i++)
    {
      if ((i<amount)&&(Grid.count()==t))
	{
	  r--;
	  Offset[0]+=CellWidth;
	  Grid.offset(1,0,1,c);
	}
      else {
	if (i>0){
	  r++;
	  Grid.offset(-1,0,1,c);
	}
	i=cc;
      }
    }
  cc=r;
  Grid.offset(0,0,r,c);
  if (r==0)return;
  amount=cc-1;
  Grid.offset(r-1,0,1,c);
  for (int i=0; i<cc; i++)
    {
      if ((i<amount)&&(Grid.count()==t))
	{
	  r--;
	  Grid.offset(-1,0,1,c);
	}
      else {
	if (i>0){
	  r++;
	  Grid.offset(1,0,1,c);
	}
	i=cc;
      }
    }
  Grid.offset(-(r-1),0,r,c);
}
void GridData::shrink(double shrinkage)
{
  Cure::Matrix box(2);
  box(0,0)=Offset[0]+shrinkage;
  box(0,1)=Offset[1]+shrinkage;
  getPoint(Grid.Rows-1,Grid.Columns-1,&box(1,0));
  box(1,0)-=shrinkage;
  box(1,1)-=shrinkage;
  if ((box(0,0)>box(1,0))||(box(0,1)>box(1,1)))
    {
      Grid.reallocate(0,0);
      return;
    }
  double ox=box(0,0);
  ox/=CellWidth;
  if (ox>0)ox+=.5;
  else ox-=.5;
  int r=(int)(ox);
   ox=r*CellWidth;
  double offx=ox-Offset[0];
  offx/=CellWidth;
  if (offx>0)offx+=.5;
  else offx-=.5;
  int offr=(int)offx;
   ox=box(1,0)-box(0,0);
   ox/=CellWidth;
  if (ox>0)ox+=.5;
  else ox-=.5;
  r=(int)(ox);
   if (r>Grid.Rows-offr)r=Grid.Rows-offr;
  Grid.offset(offr,0,r,Grid.Columns);
  offx=offr*CellWidth;
  Offset[0]+=(offx);
   double oy=box(0,1);
  if (box(0,1)>box(1,1)){
    box(0,1)=box(1,1);
    box(1,1)=oy;
    oy=box(0,1);
  }
  oy/=CellWidth;
  if (oy>0)oy+=.5;
  else oy-=.5;
  int c=(int)(oy);
  oy=c*CellWidth;

  double offy=oy-Offset[1];
  offy/=CellWidth;
  if (offy>0)offy+=.5;
  else offy-=.5;
  int offc=(int)offy;
  oy=box(1,1)-box(0,1);
  oy/=CellWidth;
  if (oy>0)oy+=.5;
  else oy-=.5;
  c=(int)(oy);
  if (c>Grid.Columns-offc)c=Grid.Columns-offc;
  Grid.offset(0,offc,r,c);
  offy=offc*CellWidth;
  Offset[1]+=(offy);
 }

void GridData::shrinkInto(GridData &gd, double shrinkage)
{
  Cure::Matrix box(2);
  box(0,0)=Offset[0]+shrinkage;
  box(0,1)=Offset[1]+shrinkage;
  getPoint(Grid.Rows-1,Grid.Columns-1,&box(1,0));
  box(1,0)-=shrinkage;
  box(1,1)-=shrinkage;
  if ((box(0,0)>box(1,0))||(box(0,1)>box(1,1)))
    {
      gd.Grid.reallocate(0,0);
      return;
    }
  resizeGrid(gd,box);
}
void GridData::resizeGrid(GridData &gd, Cure::Matrix &box )
{
  gd.CellWidth=CellWidth;
  gd.Time=Time;
  gd.Birthday=Birthday;
  gd.setSubType(m_SubType);
  gd.Offset[2]=Offset[2];
  gd.Offset[0]=box(0,0);
  if (box(0,0)>box(1,0)){
    box(0,0)=box(1,0);
    box(1,0)=gd.Offset[0];
    gd.Offset[0]=box(0,0);
  }
  gd.Offset[0]/=CellWidth;
  if (gd.Offset[0]>0)gd.Offset[0]+=.5;
  else gd.Offset[0]-=.5;
  int r=(int)(gd.Offset[0]);
  gd.Offset[0]=r*CellWidth;
  gd.Offset[1]=box(0,1);
  if (box(0,1)>box(1,1)){
    box(0,1)=box(1,1);
    box(1,1)=gd.Offset[1];
    gd.Offset[1]=box(0,1);
  }
  gd.Offset[1]/=CellWidth;
  if (gd.Offset[1]>0)gd.Offset[1]+=.5;
  else gd.Offset[1]-=.5;
  int c=(int)(gd.Offset[1]);
  gd.Offset[1]=c*CellWidth;
  int xc=c%32;
  if (xc<0)xc=32+xc;


  double ox=box(1,0)-box(0,0);
  ox/=CellWidth;
  if (ox>0)ox+=.5;
  else ox-=.5;
  r=(int)(ox);
  double cy=box(0,1);
  cy/=CellWidth;
  if (cy>0)cy+=.5;
  else cy-=.5;
  c=(int)cy;
  double oy=box(1,1)-box(0,1);
  oy/=CellWidth;
  if (oy>0)oy+=.5;
  else oy-=.5;
  c=(int)(oy);
  bool v=false;
  if (m_SubType&1)v=true;
  if ((gd.Grid.Rows!=r)||(gd.Grid.Columns!=c))
    {     
      gd.Grid.grow(0,0);
      gd.Grid.grow(r,c+xc,v);  
      gd.Grid.offset(0,xc,r,c);
    }
  else  gd.Grid=v;
  gd.update(*this);
}
void GridData::accumulate(ShortMatrix &gd,double gdoffset[2])
{
  bool v=false;
  if (m_SubType&1)v=true;
  if ((Grid.Rows==0)||(Grid.Columns==0))return;
  if ((gd.Rows==0)||(gd.Columns==0))
    {
      gd.reallocateZero(Grid.Rows,Grid.Columns);
      if (!v)
	Grid.addTo(gd);
      else 
	Grid.addNotTo(gd);
      gdoffset[0]=Offset[0];
      gdoffset[1]=Offset[1];
      return;
    }
  double xtop[2];
  double mytop[2];
  xtop[0]=gdoffset[0];
  xtop[1]=gdoffset[1];
  getPoint(0,0,mytop);
  xtop[0]-=mytop[0];
  xtop[1]-=mytop[1];
  xtop[0]/=CellWidth;
  xtop[1]/=CellWidth;
  if (xtop[0]>0){
    xtop[0]+=.5;
  }
  else xtop[0]=0;
  if (xtop[1]>0){
    xtop[1]+=.5;
  }
  else xtop[1]=0;
  int xr=(int)xtop[0];  
  int xc=(int)xtop[1];  
  if (xr>0){
    gd.insertRows(0,xr);
    gdoffset[0]-=xr*CellWidth;
  }
  if (xc>0){
    if (xc%32)
      xc=xc+32-(xc%32);
    gd.insertColumns(0,xc);
    gdoffset[1]-=xc*CellWidth;
  }
  xtop[0]=gd.Rows-1;
  xtop[1]=gd.Columns-1;
  xtop[0]/=CellWidth;
  xtop[1]/=CellWidth;
  xtop[0]+=gdoffset[0];
  xtop[1]+=gdoffset[1];
  getPoint(Grid.Rows-1,Grid.Columns-1,mytop);
  mytop[0]-=xtop[0];
  mytop[1]-=xtop[1];
  mytop[0]/=CellWidth;
  mytop[1]/=CellWidth;
  if (mytop[0]>0){
    mytop[0]+=.5;
  }
  else mytop[0]=0;
  if (mytop[1]>0){
    mytop[1]+=.5;
  }
  else mytop[1]=0;
  xr=(int)mytop[0];  
  xc=(int)mytop[1];  
  if ((xr>0)||(xc>0))
    {
      xr+=gd.Rows;
      xc+=gd.Columns;
      gd.grow(xr,xc);
    }
  double dx=gdoffset[0]-Offset[0];
  double dy=gdoffset[1]-Offset[1];
  dx/=CellWidth;
  dy/=CellWidth;
  if (dx>0)dx+=.5;
  else dx-=.5;
  if (dy>0)dy+=.5;
  else dy-=.5;
  int dr=(int)dx;
  int dc=(int)dy;
  int myrowoff=dr;
  int mycoloff=dc;
  int hisrowoff=0;
  int hiscoloff=0;
  if (dr<0)
    {
      myrowoff=0;
      hisrowoff=-dr;
    }
  if (dc<0)
    {
      mycoloff=0;
      hiscoloff=-dc;
    }
  int hisr=gd.Rows-hisrowoff;
  if (hisr<=0)return;
  int hisc=gd.Columns-hiscoloff;
  if (hisc<=0)return;
  int myr=Grid.Rows-myrowoff;
  if (myr<=0)return;
  int myc=Grid.Columns-mycoloff;
  if (myc<=0)return;
  if (hisr>myr)hisr=myr;
  else myr=hisr;
  if (hisc>myc)hisc=myc;
  else myc=hisc;
  int hisrows=gd.Rows;
  int hiscols=gd.Columns;
  int myrows=Grid.Rows;
  int mycols=Grid.Columns;
  Grid.offset(myrowoff,mycoloff,myr,myc);
  gd.offset(hisrowoff,hiscoloff,hisr,hisc);
  gd.reallocateZero(Grid.Rows,Grid.Columns);
  if (!v)
      Grid.addTo(gd);
    else 
      Grid.addNotTo(gd);
  Grid.offset(-myrowoff,-mycoloff,myrows,mycols);
  gd.offset(-hisrowoff,-hiscoloff,hisrows,hiscols);
  return;
}


int GridData::update(GridData &gd,int givereturn)
{
  Time=gd.Time;
  if (CellWidth!=gd.CellWidth)return 0;
  double dx=Offset[0]-gd.Offset[0];
  double dy=Offset[1]-gd.Offset[1];
  dx/=CellWidth;
  dy/=CellWidth;
  if (dx>0)dx+=.5;
  else dx-=.5;
  if (dy>0)dy+=.5;
  else dy-=.5;
  int dr=(int)dx;
  int dc=(int)dy;
  int myrowoff=0;
  int mycoloff=0;
  int hisrowoff=dr;
  int hiscoloff=dc;
  if (dr<0)
    {
      myrowoff=-dr;
      hisrowoff=0;
    }
  if (dc<0)
    {
      mycoloff=-dc;
      hiscoloff=0;
    }
  int myr=Grid.Rows-myrowoff;
  if (myr<=0)return 0;
  int myc=Grid.Columns-mycoloff;
  if (myc<=0)return 0;
  int hisr=gd.Grid.Rows-hisrowoff;
  if (hisr<=0)return 0;
  int hisc=gd.Grid.Columns-hiscoloff;
  if (hisc<=0)return 0;
  if (myr>hisr)myr=hisr;
  else hisr=myr;
  if (myc>hisc)myc=hisc;
  else hisc=myc;
  int myrows=Grid.Rows;
  int mycols=Grid.Columns;
  int hisrows=gd.Grid.Rows;
  int hiscols=gd.Grid.Columns;
  Grid.offset(myrowoff,mycoloff,myr,myc);
  gd.Grid.offset(hisrowoff,hiscoloff,hisr,hisc);
 if ((m_SubType&1)&&(gd.m_SubType&1))
    {
      // This is normal free space estimate.  We are adding the cells
      // known to be free in two grids.
      Grid&=gd.Grid;
    }
  else if ((m_SubType&0x6)&&(gd.m_SubType&0x6))
    {
      // This is occupied space estimate.  We are adding the cells
      // known to be occupied in two grids.
      Grid|=gd.Grid;
    }
  else if ((m_SubType&0x6)&&(gd.m_SubType&1))
    {
    //This is the case of this is a set of cells that could explain the
    //sensor reading (sonar arc) and gd is the cumulative free space grid.
    // Then the result is just those cells that are consistent with both.
      // if none then gd should be updated with an original copy of this. 
      // if small cluster then we know the approx location of something.
      // if many then we just go on.
      Grid&=gd.Grid;
    }
  else if ((m_SubType&1)&&(gd.m_SubType&0x6))
    {
      //This is when we have found that this grid is not consistent with
      //the occupied cells from gd and the free space must be removed
      Grid|=gd.Grid;
    }
  else if ((m_SubType&8)&&(gd.m_SubType&0x8))
    {
      //This is two seen grids being combined 
      //the true cells from gd are added to this
      Grid|=gd.Grid;
    }
  else if ((m_SubType&8)&&(gd.m_SubType&0x1))
    {
      //This is a seen grid having some free space removed 
      //the false cells from gd are removed from this
      Grid&=gd.Grid;
    }
  else 
    {
      //This is the default update
      // exclusive or
      Grid^=gd.Grid;
    }

  int r=0;
  if (givereturn)
    r=Grid.count();
  Grid.offset(-myrowoff,-mycoloff,myrows,mycols);
  gd.Grid.offset(-hisrowoff,-hiscoloff,hisrows,hiscols);
  return r;
}
void GridData::navSpace(GridData &gd, double radius)
{
  gd.Time=Time;
  gd.Birthday=Birthday;
  gd.Offset[0]=Offset[0];
  gd.Offset[1]=Offset[1];
  gd.Offset[2]=Offset[2];
  gd.m_SubType=m_SubType;
  gd.CellWidth=CellWidth;
  radius/=CellWidth;
  if (radius>0)  
    Grid.growInto(gd.Grid, radius,true);
  else gd.Grid=Grid;
}
int GridData::cluster(GridData &gd, double seed[2],double radius)
{
  navSpace(gd,radius);
  int r,c;
  if (getRowColumn(r,c,seed))return -1;
  return gd.Grid.cluster(r,c);
}
int GridData::path(double fromx[2], double tox[2],
		   Cure::ShortMatrix &path, double searchlimit)
{
  searchlimit/=CellWidth;
  int r1;
  int c1;
  int r2;
  int c2;
  if (getRowColumn(r1,c1,fromx))return -1;
  int ret=0;
  if (getRowColumn(r2,c2,tox))
    {
      int rc=-1;
      int cc=-1;
      double dis=1E50;
      for (int i=0; i<Grid.Rows; i++){
	if (!Grid(i,0)){
	  double d=(i-r2)*(i-r2)+c2*c2;
	  if (d<dis){
	    dis=d;
	    rc=i;
	    cc=0;
	  }
	}
	int j=Grid.Columns-1;
	if (!Grid(i,j)){
	  double d=(i-r2)*(i-r2)+(c2-j)*(c2-j);
	  if (d<dis){
	    dis=d;
	    rc=i;
	    cc=j;
	  }
	}
      }
      for (int i=0; i<Grid.Columns; i++){
	if (!Grid(0,i)){
	  double d=(i-c2)*(i-c2)+r2*r2;
	  if (d<dis){
	    dis=d;
	    rc=0;
	    cc=i;
	  }
	}
	int j=Grid.Rows-1;
	if (!Grid(j,i)){
	  double d=(i-c2)*(i-c2)+(r2-j)*(r2-j);
	  if (d<dis){
	    dis=d;
	    rc=j;
	    cc=i;
	  }
	}	
	
      }
      if (dis>1E40)return -1;
      r2=rc;
      c2=cc;
      ret=1;
    }
  double d=Grid.path(r1,c1, r2, c2,path,searchlimit);
  if (d<0)return -1;
  else return ret;
}
void GridData::massagePath(ShortMatrix &path, int amount,double maxdist)
{
  for(int i=0; i<amount; i++)
    {
      bendPath(path,maxdist,i+1,path.Rows-i);
    }
}
void GridData::bendPath(ShortMatrix &path, double maxdist, 
			int fromind, int  toind)
{
  if (fromind<1)fromind=1; 
 if (toind>(path.Rows-1))toind=path.Rows-1;
  maxdist/=CellWidth;
  for (int i=fromind; i<toind; i++){
    ShortMatrix indexes;
    Grid.nearest(path(i,0),path(i,1),indexes, maxdist);
    if (indexes.Rows==1){
      int dx=(path(i,0)-indexes(0,0));
      int dy=(path(i,1)-indexes(0,1));
      if (dx==0){
	if (dy<0){
	  if (Grid.inRange(path(i,0),path(i,1)-1))
	    {
	      path(i,1)--;
	      if (Grid(path(i,0),path(i,1)))path(i,1)++;
	    }
	} else if (dy>0){
	  if (Grid.inRange(path(i,0),path(i,1)+1))
	    {
	      path(i,1)++;
	      if (Grid(path(i,0),path(i,1)))path(i,1)--;
	    }
	}
      } else if (dy==0){
	if (dx<0){
	  if (Grid.inRange(path(i,0)-1,path(i,1))){
	    path(i,0)--;
	    if (Grid(path(i,0),path(i,1)))path(i,0)++;
	  }
	}else if (dx>0){
	  if (Grid.inRange(path(i,0)+1,path(i,1))){
	    path(i,0)++;
	    if (Grid(path(i,0),path(i,1)))path(i,0)--;
	  }
	}
      } else {
	double r=dy/dx;
	if ((r>1)||(r<-1)){
	  if (dy<0){
	    if (Grid.inRange(path(i,0),path(i,1)-1)){
	      path(i,1)--;
	      if (Grid(path(i,0),path(i,1)))path(i,1)++;
	    }
	 } else if (dy>0){
	   if (Grid.inRange(path(i,0),path(i,1)+1)){
	     path(i,1)++;
	     if (Grid(path(i,0),path(i,1)))path(i,1)--;
	   }
	 }
	}else { 
	  if (dx<0){
	    if (Grid.inRange(path(i,0)-1,path(i,1))){
	      path(i,0)--;
	    
	      if (Grid(path(i,0),path(i,1)))path(i,0)++;
	    }
	  } else if (dx>0){
	    if (Grid.inRange(path(i,0)+1,path(i,1))){
	    path(i,0)++;
	    if (Grid(path(i,0),path(i,1)))path(i,0)--;
	    }
	  }
	}
      }      
    }
  }
  
  for (int i=0;i<path.Rows-1; i++)
    {
      if ((path(i,0)==path(i+1,0))&&(path(i,1)==path(i+1,1))){
	  path.deleteRow(i);
	  i--;
      }else {
	int dr=path(i,0)-path(i+1,0);
	int dc=path(i,1)-path(i+1,1);
	if (((dr>1)||(dr<-1))||((dc>1)||(dc<-1))){
	  ShortMatrix ind;
	  Grid.path(path(i,0),path(i,1),path(i+1,0),path(i+1,1),ind,6);
	  if (ind.Rows>2){
	   path.insertRows(i+1,(ind.Rows-2));
	    for (int j=0; j<ind.Rows; j++){
	      path(i+j,0)=ind(j,0);
	      path(i+j,1)=ind(j,1);
	    }
	    i+=(ind.Rows-2);
	  }
	}
      } 
    }


}
int  GridData::drivePath(Cure::Matrix &velocity,
			 Cure::ShortMatrix &path, 
			 double robotangle,
			 double commandperiod,
			 double maxdeviation,
			 double maxlinearvelocity,
			 double maxangularvelocity)
{
  double md=maxdeviation/CellWidth;
  double radius=0;
  int r=path.Rows;
  double center[2];
  double ml=maxlinearvelocity/CellWidth;
  velocity.reallocate(1,3);
  int pathrows=path.Rows;
  velocity(0,2)=commandperiod;
  double targ=velocity(0,2)*ml+.5;
  int target=(int)(targ);
  if (target<40)target=40;
  if (pathrows>target)pathrows=target;
  path.offset(0,0,pathrows,2);
  int f=path.fitToArc(radius,center,robotangle,md);
  if (f==0)return 1;
  if (f==1){
    velocity(0,0)=0;
    velocity(0,1)=0;
    if (r==1)return 1;
    if (r>1){
      double t=atan2(path(1,1)-path(0,1),path(1,0)-path(0,0));
      t-=robotangle;
      while (t>M_PI)t-=2*M_PI;
      while (t<-M_PI)t+=2*M_PI;
      velocity(0,1)=t/(2*velocity(0,2));
      if (velocity(0,1)>maxangularvelocity){
	velocity(0,1)=maxangularvelocity;
      }
      else if (velocity(0,1)<-maxangularvelocity){
	velocity(0,1)=-maxangularvelocity;
      }
      return 0;
    }
  }
  if(f>0){
    if (f<(int)targ){
      ml=(f)/velocity(0,2);
    }
    ml*=CellWidth;
    if (radius<0)
      velocity(0,1)=maxangularvelocity;
    else 
      velocity(0,1)=-maxangularvelocity;
    velocity(0,0)=(-radius*velocity(0,1)*CellWidth); 
    if (velocity(0,0)>ml) {
       velocity(0,1)*=(ml/velocity(0,0));
       velocity(0,0)=ml;
     }
     double x[2];
     double t=robotangle;
     double dt=1;
     if (radius>0) dt=-1;
     if (radius>1)
       dt=-1/radius;
     if (radius<-1)
       dt=-1/radius;
     for (int i=0; i<f; i++)
       {
	 x[0]=center[0]-radius*sin(t);
	 x[1]=center[1]+radius*cos(t);
	 int rw,cl;
	 if (!getRowColumn(rw,cl,x))
	   {
	     if (Grid(rw,cl))
	       {
		 path.offset(0,0,i,2);
		 rw=drivePath(velocity,
			      path, 
			      robotangle,
			      commandperiod,
			      maxdeviation,
			      maxlinearvelocity,
			      maxangularvelocity);
		   path.offset(0,0,r,2);
		   return rw; 
	       }
	   }
	 t+=dt;
       }
     //need to trace path
  }  else if (f<0){
    f=-f;
    if (f<(int)targ){
      ml=(f)/velocity(0,2);
    }
    ml*=CellWidth;
    velocity(0,1)=0;
    velocity(0,0)=ml;
    double n[2];
    n[0]=cos(robotangle);
    n[1]=sin(robotangle);
    for (int i=0; i<f; i++)
      {
	int rw,cl;
	 if (!getRowColumn(rw,cl,center))
	   {
	     if (Grid(rw,cl))
	       {
		 path.offset(0,0,i,2);
		 rw=drivePath(velocity,
			      path, 
			      robotangle,
			      commandperiod,
			      maxdeviation,
			      maxlinearvelocity,
			      maxangularvelocity);
		 path.offset(0,0,r,2);
		 return rw; 
	       }
	   }
	 center[0]+=n[0];
	 center[1]+=n[1];
      }
  }
  path.offset(0,0,r,2);
  
  return 0;
}
void GridData::addRobot(Cure::Matrix &verticies, Cure::Pose3D &robotPose)
{
  GridData gd;
  gd.robot(verticies,robotPose,m_SubType);
  append(gd);
}

void GridData::robot(Cure::Matrix &verticies, Cure::Pose3D &robotPose, 
		       unsigned short gtype)
{
  Time=robotPose.Time;
  Birthday=Time;
  setSubType(gtype);
  if (verticies.Rows<3){
    Pose3D diskpose(robotPose);
    Pose3D p;
    if (verticies.Columns>2)
      p.setXY(&verticies(0,1));
    else if (verticies.Columns>1)
      p.setX(verticies(0,1));
    diskpose+=p;
    disk(verticies(0,0),diskpose,gtype);
    return;
  }
  Cure::Matrix rv(verticies.Rows,verticies.Columns);
  double maxx=robotPose.getX();
  double minx=robotPose.getX();
  double maxy=robotPose.getY();
  double miny=robotPose.getY();
  for (int i=0; i<verticies.Rows; i++)
    {
      robotPose.invTransform2D(&verticies(i,0),&rv(i,0));
      if (rv(i,0)>maxx)maxx=rv(i,0);
      if (rv(i,0)<minx)minx=rv(i,0);
      if (rv(i,1)>maxy)maxy=rv(i,1);
      if (rv(i,1)<miny)miny=rv(i,1);
      
    }
  minx/=CellWidth;
  miny/=CellWidth;
  maxx/=CellWidth;
  maxy/=CellWidth;
  if (minx<0)minx-=1;
  int roff=(int)minx;
  roff-=10;
  Offset[0]=(double)roff;
  Offset[0]*=CellWidth;
  
 if (miny<0)miny-=1;
  int coff=(int)miny;
  coff-=64;
  coff/=32;
  coff*=32;
  Offset[1]=(double)coff;
  Offset[1]*=CellWidth;
  Offset[2]=robotPose.getZ();
  if (maxx>0)maxx+=1;
  roff=(int)maxx-roff;
  roff+=10;
  if (maxy>0)maxy+=1;
  coff=(int)maxy-coff;
  coff+=64;
  coff/=32;
  coff*=32; 
  bool v=false;
  if (m_SubType&1)v=true;
  Grid=v;
  Grid.reallocate(roff,coff,v);
  Cure::ShortMatrix verts(rv.Rows,rv.Columns);
  for (int i=0; i<rv.Rows; i++)
    {
      double d=(rv(i,0)-Offset[0])/CellWidth+.5;
      verts(i,0)=(int)(d);
      d=(rv(i,1)-Offset[1])/CellWidth+.5;
      verts(i,1)=(int)(d);
    }
  if (m_SubType&1)
    {
      Grid.clearPolygon(verts);
    }
  else if (m_SubType&0x14)
    {
      Grid.setPolygon(verts);
    }
  trim(Grid.Rows+Grid.Columns);
}
double GridData::nearest(double center[2], Cure::Matrix &points, 
			 double maxdistance,
			 double mindistance)
{
  int r=0;
  int c=0;
  if (getRowColumn(r,c,center))return -1;
  maxdistance/=CellWidth;
  mindistance/=CellWidth;
  ShortMatrix indexes;
  double d=Grid.nearest(r,c,indexes,maxdistance,mindistance);
  d*=CellWidth;
  points.reallocate(indexes.Rows,indexes.Columns);
  for (int i=0; i<indexes.Rows;i++)
    for (int j=0; j<2;j++)
      {
	points(i,j)=((double)indexes(i,j))*CellWidth+Offset[j];
      }
  return d;
}
double GridData::nearest(double center[2], Cure::Matrix &points, 
			 double direction[2],double maxdistance)
{
  int r=0;
  int c=0;
  if (getRowColumn(r,c,center))return -1;
  maxdistance/=CellWidth;
  ShortMatrix indexes;
  double d=Grid.nearest(r,c,indexes,direction,maxdistance);
  d*=CellWidth;
  points.reallocate(indexes.Rows,indexes.Columns);
  for (int i=0; i<indexes.Rows;i++)
    for (int j=0; j<2;j++)
      {
	points(i,j)=((double)indexes(i,j))*CellWidth+Offset[j];
      }
  return d;
}

void GridData::disk(double radius, Cure::Pose3D &diskPose, 
		       unsigned short gtype)
{
  Time=diskPose.Time;
  Birthday=Time;
  setSubType(gtype);
  double maxx=diskPose.getX()+radius;
  double minx=diskPose.getX()-radius;
  double maxy=diskPose.getY()+radius;
  double miny=diskPose.getY()-radius;
  minx/=CellWidth;
  miny/=CellWidth;
  maxx/=CellWidth;
  maxy/=CellWidth;
  if (minx<0)minx-=1;
  int roff=(int)minx;
  roff-=10;
  Offset[0]=(double)roff;
  Offset[0]*=CellWidth;
  if (miny<0)miny-=1;
  int coff=(int)miny;
  coff-=64;
  coff/=32;
  coff*=32;
  Offset[1]=(double)coff;
  Offset[1]*=CellWidth;
  Offset[2]=diskPose.getZ();
  if (maxx>0)maxx+=1;
  roff=(int)maxx-roff;
  roff+=10;
  if (maxy>0)maxy+=1;
  coff=(int)maxy-coff;
  coff+=64;
  coff/=32;
  coff*=32;
  bool v=false;
  if (m_SubType&1)v=true;
  Grid=v;
  Grid.reallocate(roff,coff,v);
  double center[2];
  diskPose.getXY(center);
  center[0]-=Offset[0];
  center[1]-=Offset[1];
  center[0]/=CellWidth;
  center[1]/=CellWidth;
  center[0]+=.5;
  center[1]+=.5;
  int r=(int)center[0];
  int c=(int)center[1];
  Grid.fillDisk(r,c,(radius/CellWidth),!v);
  trim(Grid.Rows+Grid.Columns);
}
void GridData::circle(double radius, Cure::Pose3D &diskPose, 
		       unsigned short gtype)
{
  Time=diskPose.Time;
  Birthday=Time;
  setSubType(gtype);
  double maxx=diskPose.getX()+radius;
  double minx=diskPose.getX()-radius;
  double maxy=diskPose.getY()+radius;
  double miny=diskPose.getY()-radius;
  minx/=CellWidth;
  miny/=CellWidth;
  maxx/=CellWidth;
  maxy/=CellWidth;
  if (minx<0)minx-=1;
  int roff=(int)minx;
  roff-=10;
  Offset[0]=(double)roff;
  Offset[0]*=CellWidth;
  if (miny<0)miny-=1;
  int coff=(int)miny;
  coff-=64;
  coff/=32;
  coff*=32;
  Offset[1]=(double)coff;
  Offset[1]*=CellWidth;
  Offset[2]=diskPose.getZ();
  if (maxx>0)maxx+=1;
  roff=(int)maxx-roff;
  roff+=10;
  if (maxy>0)maxy+=1;
  coff=(int)maxy-coff;
  coff+=64;
  coff/=32;
  coff*=32;
  bool v=false;
  if (m_SubType&1)v=true;
  Grid=v;
  Grid.reallocate(roff,coff,v);
  double center[2];
  diskPose.getXY(center);
  center[0]-=Offset[0];
  center[1]-=Offset[1];
  center[0]/=CellWidth;
  center[1]/=CellWidth;
  Grid.setArc(center,radius,0,6.5,!v,1.6);
  trim(Grid.Rows+Grid.Columns);
}
void GridData::addSonar(Cure::SonarData &sonar, 
			Cure::Transformation3D &sensorPose)
{

  GridData gd(CellWidth);
  gd.sonar3D(sonar,sensorPose,m_SubType,Offset[2]);
  append(gd);
}

void GridData::sonar3D(Cure::SonarData &sonar, 
		     Cure::Transformation3D &sensorPose, 
		     unsigned short gtype, double height)
{
  Time=sonar.Time;
  Birthday=Time;
  setSubType(gtype);
  Grid.reset(0,0);
  if ((m_SubType&0x6)&&(sonar.Range>=sonar.MaxRange))
    return;
  double x[6];
  sensorPose.getCoordinates(x);
  double jhat[3];
  jhat[0]=0;
  jhat[1]=1;
  jhat[2]=0;
  sensorPose.invRotate(jhat,jhat);
  //We create a 0 pitch equivalent pose
  Cure::Transformation3D trans;
  trans.setXYZ(x);
  trans.setTheta(atan2(-jhat[0],jhat[1]));
  Cure::Transformation3D trans2(trans);
  trans.rotate(jhat,jhat);
  trans.setPhi(0);
  trans.setPsi(atan2(jhat[2],jhat[1]));
  if (jhat[2]==0)
    return this->sonar(sonar,trans,gtype,height);  
  Offset[2]=height;  
  double dz=height-trans2.getZ();
  double a=sonar.BeamWidth/2;
  double c=cos(a);
  double c2=c*c;
  double j12=jhat[1]*jhat[1];
  double rho=sonar.Range*sonar.Range-dz*dz;
  if (rho<1E-3)return;
  if (j12<1E-6){
    if (c2>1E-8){
      double tmp=1/(c2)-1;
      tmp*=dz*dz;
      if (tmp<rho){
	if (!(m_SubType&9))return;
	rho=tmp;
      }
    }
    if (rho<1E-8)return;
    Pose3D pos;
    pos.setX(trans2.getX());
    pos.setY(trans2.getY());
    pos.Time=Time;
    pos.setZ(Offset[2]);
    if (m_SubType&9)
      this->disk(rho,pos,gtype);
    else 
      this->circle(rho,pos,gtype);
    return;
  }
  double tmp=j12/(1-jhat[2]*jhat[2]);
  if (c2>tmp) return;
  double biga=(j12-c2);
  double dz2=dz*dz;
  double bigc=dz2/j12;
  double bigb=jhat[2]*dz*c/j12;
  biga/=j12;
  double ym=c/jhat[1];
  double yb=dz*jhat[2]/jhat[1];
  Matrix points;
  Matrix backpoints;
  if ((biga<1E-8)&&(biga>-1E-8)){
    return;
  }
  //ellipse or parabola
  rho=sqrt(rho);
  double tmp1=-bigb/biga;
  tmp=tmp1*tmp1;
  tmp+=(bigc/biga);
  if (tmp<1E-4)return;//should have been handled already
  tmp=sqrt(tmp);
  double rp=tmp1+tmp;
  double rm=tmp1-tmp;
  if ((biga>0)&&(rp>rho))return;
  if (sonar.Range<rm)return;
  if (rp<0)return;  
  long n=(int)((double)(((rp-rm)+(a*rp*2))/CellWidth)+1.5);
  n*=5;
  long an=n; 
  if (an>3E4) return;
  
  points.reallocate(an,3);
  backpoints.reallocate(an,3);
  double top=rp+CellWidth;
  double dr=CellWidth/4.0;
  double r=rm;
  if (biga>0){
    r=rp;
    top=rho;
  }
  if(top>rho){
    top=rho;
  }else if (!(m_SubType&9))return;
  if (m_SubType&9){
  }else{
    r=rho;
  }
  n=1;
  points(0,0)=0;
  points(0,1)=ym*r+yb;
  backpoints(0,0)=-points(0,0);
  backpoints(0,1)=points(0,1);
  while (r<top){
    bool tryagain=true;
    while (tryagain){
      tryagain=false;
      points(n,1)=ym*r+yb;
      if (points(n,1)-points(n-1,1)>CellWidth/2.0){
	tryagain=true;
	dr/=2.0;
	r-=dr;
      }else{
	points(n,0)=biga*(r-rm)*(r-rp);
	if (points(n,0)<0){
	  tryagain=true;
	  dr/=2.0;
	  r-=dr;
	}else {
	points(n,0)=sqrt(points(n,0));
	double dp=points(n,0)-points(n-1,0);
	if (dp<0)dp=-dp;
	if (dp>CellWidth/2.0){
	  tryagain=true;
	  dr/=2.0;
	  r-=dr;
	}else{
	  if (points(n,1)-points(n-1,1)>dp)dp=points(n,1)-points(n-1,1);
	  if (dp<CellWidth/4.0)dr*=2.0;
	  r+=dr;
	  backpoints(n,0)=-points(n,0);
	  backpoints(n,1)=points(n,1);
	  if (points(n,0)<points(n-1,0))
	    if (points(n,0)<CellWidth/4.0)r=top;
	  n++;
	  if (n==an){
	    an+=100;
	    points.grow(an,3);
	    backpoints.grow(an,3);
	  }
	}
	}
      }
    }
  }
  a=atan2(points(n-1,0),points(n-1,1));
  double da=-CellWidth/(4.0*rho);
  a+=da;
  int mid=n;
  while (points(n-1,0)>CellWidth/4.0){
    points(n,1)=rho*cos(a);
    points(n,0)=rho*sin(a);
    backpoints(n,0)=-points(n,0);
    backpoints(n,1)=points(n,1);
    n++;
    a+=da;
    if (n==an){
      an+=100;
      points.grow(an,3);
      backpoints.grow(an,3);
    }
  }
  if (n==1)return;
  points.Rows=n;
  backpoints.Rows=n;
  Matrix edge(2*n,3);
  int n2=2*n-1;
  for (int i=0;i<n;i++)
    {
      trans2.invTransform(&points(i,0),&edge(i,0));
      trans2.invTransform(&backpoints(i,0),&edge(n2-i,0));
    }
  if (m_SubType&9){
    double inside[3];
    inside[0]=0;
    inside[2]=0;
    n=(int)((double)mid*.7);
    if (n<3)n=3;
    if(n>mid)n=mid;
    if( n>=points.Rows)n=0;
    inside[1]=points(n,1);
    trans2.invTransform(inside,inside);
    fillPoints(edge, inside);
    if (m_SubType&1)
      ~Grid;
  }else setPoints(edge,true);
  trim(Grid.Rows+Grid.Columns);
}
void GridData::setPoints(Cure::Matrix &edge,bool val)
{
  Grid.reset(0,0);
  if (edge.Rows<1)return;
  if (edge.Columns<2)return;
  double maxx=edge(0,0);
  double minx=edge(0,0);
  double maxy=edge(0,1);
  double miny=edge(0,1);
  edge.Columns=2;
  for (long i=1;i<edge.Rows;i++)
    {
      if (edge(i,0)>maxx)maxx=edge(i,0);
      if (edge(i,1)>maxy)maxy=edge(i,1);
      if (edge(i,0)<minx)minx=edge(i,0);
      if (edge(i,1)<miny)miny=edge(i,1);
    }
  maxx++;
  minx--;
  maxy++;
  miny--;
  minx/=CellWidth;
  miny/=CellWidth;
  maxx/=CellWidth;
  maxy/=CellWidth;
  if (minx<0)minx-=1;
  int roff=(int)minx;
  roff-=10;
  Offset[0]=(double)roff;
  Offset[0]*=CellWidth;
  if (miny<0)miny-=1;
  int coff=(int)miny;
  coff-=64;
  coff/=32;
  coff*=32;
  Offset[1]=coff;
  Offset[1]*=CellWidth;
  if (maxx>0)maxx+=1;
  roff=(int)maxx-roff;
  roff+=10;
  if (maxy>0)maxy+=1;
  coff=(int)maxy-coff;
  coff+=64;
  coff/=32;
  coff*=32;
  
  Grid.reallocate(roff,coff,!val);
  Grid=!val;
  if ((roff==0)||(coff==0))return;
  edge(0,0)-=Offset[0];
  edge(0,0)/=CellWidth;
  edge(0,0)+=.5;
  long prevr=(long)edge(0,0);
  edge(0,1)-=Offset[1];
  edge(0,1)/=CellWidth;
  edge(0,1)+=.5;
  long prevc=(long)edge(0,1);
  Grid.setBit(prevr,prevc,val);
  for (long i=1;i<edge.Rows;i++){
    edge(i,0)-=Offset[0];
    edge(i,0)/=CellWidth;
    edge(i,0)+=.5;
    long r=(long)edge(i,0);
    edge(i,1)-=Offset[1];
    edge(i,1)/=CellWidth;
    edge(i,1)+=.5;
    long c=(long)edge(i,1);
    long dr=r-prevr;
    long dc=c-prevc;
    if (((dr>1)||(dc>1)|(dr<-1)||(dc<-1))){
     if (dr>0)dr=1;
     else dr=-1;
     if (dc>0)dc=1;
     else dc=-1;
     for (long ir=prevr;ir!=r;ir+=dr)
	Grid.setBit(ir,prevc,val);
      for (long ic=prevc;ic!=c;ic+=dc)
	Grid.setBit(r,ic,val);
    }    
    Grid.setBit(r,c,val);
    prevr=r;
    prevc=c;
  }
  long r=(long)edge(0,0);
  long c=(long)edge(0,1);
  long dr=r-prevr;
  long dc=c-prevc;
    if (((dr>1)||(dc>1)|(dr<-1)||(dc<-1))){
    if (dr>0)dr=1;
    else dr=-1;
    if (dc>0)dc=1;
    else dc=-1;
      for (long ir=prevr;ir!=r;ir+=dr)
      Grid.setBit(ir,prevc,val);
    for (long ic=prevc;ic!=c;ic+=dc)
      Grid.setBit(r,ic,val);
  }
}
void GridData::fillPoints(Cure::Matrix &edge, double inside[2])
{
  setPoints(edge,true);
  double inx=inside[0]-Offset[0];
  inx/=CellWidth;
  inx+=.5;
  long rin=(long)inx;
  double iny=inside[1]-Offset[1];
  iny/=CellWidth;
  iny+=.5;
  long cin=(long)iny;
  if (rin<1)return;
  if (cin<1)return;
  if (rin>Grid.Rows-2)return;
  if (rin>Grid.Columns-2)return;
  if (Grid(rin,cin))return;
  Grid.cluster(rin,cin);
}
void GridData::sonar(Cure::SonarData &sonar, 
		     Cure::Transformation3D &sensorPose, 
		     unsigned short gtype, double height)
{
  Time=sonar.Time;
  Birthday=Time;
  setSubType(gtype);
  if ((m_SubType&0x6)&&(sonar.Range>=sonar.MaxRange))
    {
      Grid.reset(0,0);
      return;
    }
  double x[6];
  sensorPose.getCoordinates(x);
  Offset[2]=height;
  double r=sonar.Range+10*CellWidth;
  double x2[2],x1[2];
  double a=sonar.BeamWidth/2;
  double b=a+.05;
  if (b<.1)b=.1;
  if (b<M_PI_2)
    x2[0]=-r*tan(b);
  else x2[0]=-r;
  x1[0]=-x2[0];
  x1[1]=r;
  x2[1]=r;
  sensorPose.invTransform2D(x1,x1);
  sensorPose.invTransform2D(x2,x2);
  double minx=x[0];
  if (x1[0]<minx)minx=x1[0];
  if (x2[0]<minx)minx=x2[0];
  double maxx=x[0];
  if (x1[0]>maxx)maxx=x1[0];
  if (x2[0]>maxx)maxx=x2[0];
  double miny=x[1];
  if (x1[1]<miny)miny=x1[1];
  if (x2[1]<miny)miny=x2[1];
  double maxy=x[0];
  if (x1[1]>maxy)maxy=x1[1];
  if (x2[1]>maxy)maxy=x2[1];
  minx/=CellWidth;
  miny/=CellWidth;
  maxx/=CellWidth;
  maxy/=CellWidth;
  if (minx<0)minx-=1;
  int roff=(int)minx;
  roff-=10;
  Offset[0]=(double)roff;
  Offset[0]*=CellWidth;
  if (miny<0)miny-=1;
  int coff=(int)miny;
  coff-=64;
  coff/=32;
  coff*=32;
  Offset[1]=coff;
  Offset[1]*=CellWidth;
  if (maxx>0)maxx+=1;
  roff=(int)maxx-roff;
  roff+=10;
  if (maxy>0)maxy+=1;
  coff=(int)maxy-coff;
  coff+=64;
  coff/=32;
  coff*=32;
  bool v=false;
  if (m_SubType&1)v=true;
  Grid=v;
  Grid.reallocate(roff,coff,v);
  if ((roff==0)||(coff==0))return;
  x[0]-=Offset[0];
  x[0]/=CellWidth;
  x[1]-=Offset[1];
  x[1]/=CellWidth;
  double radius=sonar.Range/CellWidth;
  x[2]=(height-x[2]);
  x[2]/=CellWidth;
  if (m_SubType&1)
    {
      radius-=.5;
      // Grid=1;
      if (x[2]==0)
	Grid.clearSector(x,radius, x[3]+M_PI_2-a,x[3]+M_PI_2+a);      
      else   Grid.clearHsection(x, radius, x[3]+M_PI_2-a,x[3]+M_PI_2+a);
    }
  else if (m_SubType&8)
    {
      radius-=.5;
      //      Grid=0;
      if (x[2]==0)
	Grid.setSector(x,radius, x[3]+M_PI_2-a,x[3]+M_PI_2+a);      
      else   Grid.setHsection(x, radius, x[3]+M_PI_2-a,x[3]+M_PI_2+a);
    }
  else if(m_SubType&0x6)
    {
      radius+=.5;
      //  Grid=0;
      a*=.6;
      if (x[2]==0){
      Grid.setArc(x, radius+1, x[3]+M_PI_2-a, x[3]+M_PI_2+a, true,1.0);
      a*=1.2;
      Grid.setArc(x, radius+.5, x[3]+M_PI_2-a, x[3]+M_PI_2+a, true,1.0);
      a*=1.25;
      Grid.setArc(x, radius+.2, x[3]+M_PI_2-a, x[3]+M_PI_2+a, true,.5);
      a*=1.3;
      Grid.setArc(x, radius, x[3]+M_PI_2-a, x[3]+M_PI_2+a, true,.5);
      }else{

      Grid.setHarc(x, radius+1.5, x[3]+M_PI_2-a, x[3]+M_PI_2+a);
      a*=1.2;
      Grid.setHarc(x, radius+1, x[3]+M_PI_2-a, x[3]+M_PI_2+a);
      a*=1.25;
      Grid.setHarc(x, radius+.5, x[3]+M_PI_2-a, x[3]+M_PI_2+a);
      a*=1.3;  
      Grid.setHarc(x, radius, x[3]+M_PI_2-a, x[3]+M_PI_2+a);
      }
    }
  trim(Grid.Rows+Grid.Columns);
}

void GridData::sick(Cure::SICKScan &sick,Cure::Transformation3D &sensorPose,
		    unsigned short gtype, double height){

  Time = sick.Time;
  Birthday = Time;
  setSubType(gtype);  
  Offset[2] = height;

  // Assumes sensorPose is not rotated on y or x

  // sick.StartAngle is usually 0, which means the measure on the most right
  // hand side, but sensorpose has a -90 degree rotation on z, which means 
  // facing straight. Dont know if this can be written in a more general way.
  // Calculates the sensorpose of the range[0] of the sick scanner.
  double angle; // on z
  angle = sensorPose.getTheta();
  angle -= (M_PI/2.0 - sick.getStartAngle());
  if(angle < -M_PI) angle += M_PI*2.0;
  else if(angle >= M_PI) angle -= M_PI*2.0;

  double minx, maxx, miny, maxy;
  minx = maxx = sensorPose.getX();
  miny = maxy = sensorPose.getY();

  double x[sick.getNPts()], y[sick.getNPts()];
  for(int i = 0; i < sick.getNPts(); i++){
    x[i] = sensorPose.getX() - sin(angle)*sick.getRange(i);
    y[i] = sensorPose.getY() + cos(angle)*sick.getRange(i);

    if(x[i] > maxx) maxx = x[i];
    else if(x[i] < minx) minx = x[i];
    if(y[i] > maxy) maxy = y[i];
    else if(y[i] < miny) miny = y[i];

    angle += sick.getAngleStep();
    if(angle >= M_PI) angle -= M_PI*2.0;
  }

  Offset[0] = minx;
  Offset[1] = miny;

  int r, c; 
  miny -= CellWidth/2.0;
  minx -= CellWidth/2.0;
  r = (int)ceil((maxx-minx)/CellWidth);
  c = (int)ceil((maxy-miny)/CellWidth);
  
  // type 1: 0=free; type 2,4: 1=occupied; type 8: logical not of type 1
  bool value = (gtype&0x9);
  Grid = value;
  Grid.reallocate(r, c, value);
 
  // Finally, fill the grid
  ShortMatrix vs(sick.getNPts(), 2);
  for(int i = 0; i < sick.getNPts(); i++){
    r = int((x[i]-Offset[0])/(CellWidth/2.0));
    r = r/2 + r%2;
  
    c = int((y[i]-Offset[1])/(CellWidth/2.0));
    c = c/2 + c%2;

    vs(i, 0) = r;
    vs(i, 1) = c;
  }

  if(gtype&0x9) Grid.clearPolygon(vs);
  for(int i = 0; i < sick.getNPts(); i++){
    Grid.setBit(vs(i,0), vs(i,1), true);
  }
  if(gtype&0x8) ~Grid;

}
			  
void GridData::calcMoment(double x[2], double range,
			   int &n, Matrix &mean,Matrix &cov,
			  int top, Matrix *cubic) 
{
  if (top!=8)top=4;
  int r=0;
  int c=0;
  getRowColumn(r,c,x);
  mean.reallocate(2,1);
  cov.reallocate(2,2);
  if (cubic){
    cubic[0].reallocate(2,2);
    cubic[1].reallocate(2,2);
  }
  if (!Grid.inRange(r,c)){
    mean=0;
    n=0;
    cov=0;
    if (cubic){
      cubic[0]=0;
      cubic[1]=0;
    }
    return;
  }
  range/=CellWidth;
  range+=.5;  
  
  int lr=r-(int)range;
  lr-=10;
  if (lr<0) lr=0;
  int ur=r+(int)range;
  ur+=10;
  if (ur>Grid.Rows)ur=Grid.Rows;
  int lc=c-(int)range;
  lc-=10;
  if (lc<0) lc=0;
  int uc=c+(int)range;
  uc+=10;
  if (uc>Grid.Columns)uc=Grid.Columns;
  ur-=lr;
  uc-=lc;
  int oldr=Grid.Rows;
  int oldc=Grid.Columns;
   Grid.offset(lr,lc,ur,uc);
  r-=lr;
  c-=lc;
  ShortMatrix cells;
  int lim=10*Grid.Rows;
  cells.reallocate(lim,2);
  cells.offset(0,0,1,2);
  cells(0,0)=r;
  cells(0,1)=c;
  BinaryMatrix visited(Grid.Rows,Grid.Columns);
  double center[2];
  center[0]=(double)r;
  center[1]=(double)c;
  visited.setArc(center,range,-M_PI-.1,M_PI+.1,true,1.6);
  int i=0;
  n=0;
  double temp[9];
  memset(temp,0,9*sizeof(double));
  while (Grid.clusterGrowth(cells, visited,top)){
    if (i==lim){
      int rw=cells.Rows;
      if (rw>lim)lim=cells.Rows+2;
      else lim++;
      cells.grow(lim,2);
      cells.offset(0,0,rw,2);
      i=0;
    }
    temp[0]+=center[0];
    temp[1]+=center[1];
    double cxx=center[0]*center[0];
    double cxy=center[1]*center[0];
    double cyy=center[1]*center[1]; 
    temp[2]+=cxx;
    temp[3]+=cxy;
    temp[4]+=cyy;
    if (cubic){
      temp[5]+=cxx*center[0];
      temp[6]+=center[0]*cxy;
      temp[7]+=center[1]*cxy;
      temp[8]+=center[1]*cyy;
    }
    if (cells.Rows>0){
      center[0]=cells(0,0);
      center[1]=cells(0,1);
    }
    i++;
    n++;
  }
  for (int j=0; j<9; j++)
    temp[j]/=(double)n;

  mean(0,0)=temp[0];
  mean(1,0)=temp[1];
  cov(0,0)=(temp[2]-temp[0]*temp[0]);
  cov(1,0)=(temp[3]-temp[1]*temp[0]);
  cov(0,1)=cov(1,0);
  cov(1,1)=(temp[4]-temp[1]*temp[1]);
  if (cubic){
    cubic[0](0,0)=(temp[5]-mean(0,0)*mean(0,0)*mean(0,0)
		   -3*mean(0,0)*cov(0,0)); 
    cubic[1](1,1)=(temp[8]-mean(1,0)*mean(1,0)*mean(1,0)
		   -3*mean(1,0)*cov(1,1));
    cubic[1](0,0)=(temp[6]-mean(0,0)*mean(0,0)*mean(1,0)
		   -2*mean(0,0)*cov(1,0)
		   -mean(1,0)*cov(0,0));
    cubic[0](1,1)=(temp[7]-mean(1,0)*mean(1,0)*mean(0,0)
		   -2*mean(1,0)*cov(0,1)
		   -mean(0,0)*cov(1,1));
    cubic[0](1,0)=cubic[1](0,0);
    cubic[0](0,1)=cubic[0](1,0);
    cubic[1](0,1)=cubic[0](1,1);
    cubic[1](1,0)=cubic[1](0,1);
    cubic[0]*=(CellWidth*CellWidth*CellWidth);
    cubic[1]*=(CellWidth*CellWidth*CellWidth);
  } 
  Grid.offset(-lr,-lc,oldr,oldc);
  mean(0,0)-=(double)r;
  mean(1,0)-=(double)c; 
  cov*=(CellWidth*CellWidth);
  mean*=(CellWidth);
}	      
void getTransform(Transformation3D &trans,
			   Matrix & mean1, Matrix & mean2)
{
  if (((mean1.Rows!=2)||(mean2.Columns!=1))||
      ((mean2.Rows!=2)||(mean1.Columns!=1)))
    {
      std::cerr<<"GridData::getTransform(...) mean. not 2x1\n";
      return;
    }
  Matrix temp;
  temp.multTranspose_(mean1,mean2,1);
  double c=temp(0,0);
  double s=mean1(0,0)*mean2(1,0)-mean1(1,0)*mean2(0,0);
  trans.setTheta(atan2(s,c));
  temp=mean2;
  trans.invRotate2D(temp.Element,temp.Element);
  temp-=mean1;
  temp*=-1.0;
  trans.setXY(temp.Element);
  return;
}
void GridData::transform(Cure::Transformation3D &trans)
{
  //x=R(x-xbar);
  double x[5];
  x[4]=trans.getTheta();
  x[0]=Offset[0];
  x[1]=Offset[1];
  Offset[2]-=trans.getZ();
  trans.transform2D(x,x);
  x[2]=x[0]/CellWidth;
  x[3]=x[1]/CellWidth;
  bool v=true;
  int extent=2;
  if (m_SubType&14){
    v=false;
  }
  Grid.rotate(x[4],x+2,(!v),extent);
  x[2]*=CellWidth;
  x[3]*=CellWidth;
  Offset[0]=x[0]-x[2];
  Offset[1]=x[1]-x[3];
  
  double c=Offset[1]/CellWidth;
  if (c>0){
    c+=.5;
    int ci=(int)c;
    int cii=32-ci%32;
    if (cii<32){
      Grid.insertColumns(0,ci,v);
    }
    else cii=0;
    c=(double)(cii+ci);
    //  Offset[1]=c*CellWidth;       
    Offset[1]-=ci*CellWidth;       
  }else if (c<0){
    c=-c;
    c+=.5;
    int ci=(int)c;
    int cii=32-ci%32;
    if (cii<32){
      Grid.insertColumns(0,ci,v);
    }
    else cii=0;
    c=(double)(cii+ci);
    c=-c;
    //    Offset[1]=c*CellWidth;       
    Offset[1]-=ci*CellWidth;       
  }
}
void GridData::calcDescription(Matrix &description,double sigmoidrange, 
			       Matrix &mean,
			       Matrix &cov,Matrix *cubic) 
{
  Matrix ev(2);
  int row=description.Rows;
  int col=description.Columns;
  if (cubic){
    if ((row<1)||(col<8)){
      row=1;
      col=8;
      description.reallocate(row,col); 
    } 
  } else {
    if ((row<1)||(col<4)){
      row=1;
      col=4;
      description.reallocate(row,col); 
    } 
  }
  // 2 and 3 are eigenvalues
  MatrixStuff::eigen2X2(&cov(0,0),&ev(0,0),&description(0,2)); 
  double cross=mean(0,0)*ev(0,1)-mean(1,0)*ev(0,0);
  if (cross<0){
    ev(0,0)=-ev(0,0);
    ev(0,1)=-ev(0,1);
    description(0,1)=-description(0,1);
  }
 ev(1,0)=-ev(0,1);
  ev(1,1)=ev(0,0); 
  description.offset(0,0,1,1);
  description.multTranspose(mean,mean,1);
  description.offset(0,0,row,col);
  description(0,0)=sqrt(description(0,0)); //0 is mag of mean
  description.offset(0,1,1,1);
  ev.offset(0,0,1,2);
  description.multiply_(ev,mean);
  ev.offset(0,0,2,2);
  description.offset(0,-1,row,col);
    //1 is cos(theta) angle between mean and major axis
  if (description(0,0)>1E-6)
    description(0,1)/=description(0,0); 
  //1 is meaningless if mean is too small
  description(0,1)*=(1-exp(-(description(0,0)/sigmoidrange))); 
  double cond=(description(0,2)/description(0,3));        
  double sigcondition=(1-exp((-2.0)*(cond-1)));
  //1 is meaningless if we do not know which is major axis
  description(0,1)*=sigcondition;
  if (cubic){
    Matrix t(2,1);
    //t= square distance weighted mean vector.  
    t(0,0)=cubic[0](0,0)+cubic[0](1,1);
    t(1,0)=cubic[1](0,0)+cubic[1](1,1);
    description(0,4)=t(0,0)*ev(0,0)+t(1,0)*ev(0,1);
    description(0,5)=t(0,0)*t(0,0)+t(1,0)*t(1,0);
    //5 is |t| 
    description(0,5)=sqrt(description(0,5));
    if (description(0,5)>1E-6){
      //4 is cos(angle between t and major axis)
      description(0,4)/=description(0,5);
      t/=description(0,5);
    }
    //6 is the cubic moment  in direction of t
   description(0,6)=(t(0,0)*t(0,0)*t(0,0)*cubic[0](0,0)
      +3*t(0,0)*t(0,0)*t(1,0)*cubic[0](0,1)
      +3*t(0,0)*t(1,0)*t(1,0)*cubic[1](0,1)
      +t(1,0)*t(1,0)*t(1,0)*cubic[1](1,1));
    //6 is the cubic moment  in direction perpendicular to t
    description(0,7)=(-t(1,0)*t(1,0)*t(1,0)*cubic[0](0,0)
      +3*t(1,0)*t(1,0)*t(0,0)*cubic[0](0,1)
      -3*t(1,0)*t(0,0)*t(0,0)*cubic[1](0,1)
      +t(0,0)*t(0,0)*t(0,0)*cubic[1](1,1));
    //4 is meaningless if we don't know which axis is major
    description(0,4)*=sigcondition;
   }
}
void GridData::append(GridData &gd)
{
  if (CellWidth!=gd.CellWidth)return;
  Time=gd.Time;
  if ((gd.Grid.Rows==0)||(gd.Grid.Columns==0))return;
  if ((Grid.Rows==0)||(Grid.Columns==0))
    {
      unsigned short st=m_SubType;
      (*this)=gd;
      if (st!=m_SubType)
	{
	  if (st&1)Grid=true;
	  else Grid=false;
	  setSubType(st);
	}
      return;
    }
  bool v=false;
  if (m_SubType&1)v=true;
  double xtop[2];
  double mytop[2];
  gd.getPoint(0,0,xtop);
  getPoint(0,0,mytop);
  mytop[0]-=xtop[0];
  mytop[1]-=xtop[1];
  mytop[0]/=CellWidth;
  mytop[1]/=CellWidth;
  if (mytop[0]>0){
    mytop[0]+=.5;
  }
  else mytop[0]=0;
  if (mytop[1]>0){
    mytop[1]+=.5;
  }
  else mytop[1]=0;
  int myr=(int)mytop[0];  
  int myc=(int)mytop[1];  
  if (myr>0){
    Grid.insertRows(0,myr,v);
    Offset[0]-=myr*CellWidth;
  }
  if (myc>0){
    if (myc%32)
      myc=myc+32-(myc%32);
    Grid.insertColumns(0,myc,v);
    Offset[1]-=myc*CellWidth;
  }
  gd.getPoint(gd.Grid.Rows-1,gd.Grid.Columns-1,xtop);
  getPoint(Grid.Rows-1,Grid.Columns-1,mytop);
  xtop[0]-=mytop[0];
  xtop[1]-=mytop[1];
  xtop[0]/=CellWidth;
  xtop[1]/=CellWidth;
  if (xtop[0]>0){
    xtop[0]+=.5;
  }
  else xtop[0]=0;
  if (xtop[1]>0){
    xtop[1]+=.5;
  }
  else xtop[1]=0;
  myr=(int)xtop[0];  
  myc=(int)xtop[1];  
  if ((myr>0)||(myc>0))
    {
      myr+=Grid.Rows;
      myc+=Grid.Columns;
      Grid.grow(myr,myc,v);
    }
  update(gd);
}


void GridData::setSubType(unsigned short t)
{
  m_SubType = t; 
}

void GridData::print()
{
  Time.print();
  std::cerr<< "SubType=" <<m_SubType<< "Offset=("<<Offset[0]
	   <<", "<<Offset[1]<<", "<<Offset[2]<<std::endl;
  std::cerr<<"Grid= ";
    Grid.print();
 }
 void GridData::read(std::fstream &fs ){
   Matrix m;
   ShortMatrix s;
   while (fs.good()){
     m.read(fs);
     m/=CellWidth;
     s=m;
     Grid.setPolygon(s);     
   }
 }

} // namespace Cure

std::ostream& 
operator << (std::ostream& os, const Cure::GridData &cmd)
{
  os << "SubType=" << cmd.getSubType()<< std::endl;
  return os;
}
