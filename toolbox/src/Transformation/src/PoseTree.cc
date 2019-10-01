// = RCSID
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2007 John Folkesson
//    


#include "PoseTree.hh"


using namespace Cure;


void PoseTree::Link::operator=(PoseTree::Link &link){
  m_FromPose=0;
  m_LinkPose=link.m_LinkPose;
  m_ToPose=link.m_ToPose;
  if (m_NumberOfLinks)delete[]m_Links;
  m_NumberOfLinks=link.m_NumberOfLinks;
  m_Links=0;
  if (m_NumberOfLinks>0)
    m_Links=new Link*[m_NumberOfLinks];
  m_JacToLink=link.m_JacToLink;
  m_JacToFrom=link.m_JacToFrom;
  m_AddType=link.m_AddType;
  m_LinkType=link.m_LinkType;
  m_PoseType=link.m_PoseType;
  for (int i=0;i<m_NumberOfLinks;i++){
    m_Links[i]=new PoseTree::Link(*link.m_Links[i]);
    m_Links[i]->m_FromPose=&m_ToPose;
  }
}

PoseTree::Link::Link(unsigned short posetype,
	   unsigned short linktype,  
	   unsigned short fromtype,
	   Cure::Transformation3D *frompose)
{
  m_FromPose=frompose;
  int r=calcRows(posetype);
  int c1=calcRows(linktype);
  int c2=calcRows(fromtype);
  m_JacToFrom.reallocate(r,c2);
  m_JacToLink.reallocate(r,c1);
  m_LinkType=linktype;
  m_AddType=(m_LinkType<<6);
  m_AddType+=fromtype;
  m_PoseType=0;
  m_NumberOfLinks=0;
  m_Links=0; 
}

PoseTree::Link::Link(unsigned short posetype,unsigned short linktype,  
	   double xlink[6], PoseTree::Link *frompose){
  m_LinkPose=xlink;
  m_NumberOfLinks=0;
  m_Links=0; 
  m_FromPose=&(frompose->m_ToPose);
  m_PoseType=posetype;
  int r=calcRows(posetype);
  int c1=calcRows(linktype);
  int c2=calcRows(frompose->m_PoseType);
  m_JacToFrom.reallocate(r,c2);
  m_JacToLink.reallocate(r,c1);
  m_LinkType=linktype;
  m_AddType=(m_LinkType<<6);
  m_AddType+=(unsigned short)frompose->m_PoseType;
}

void PoseTree::Link::setLinkType(unsigned short linktype){
  m_LinkType=linktype;
  unsigned short fromtype=(m_AddType&0x3F);
  m_AddType=(m_LinkType<<6);
  m_AddType+=fromtype;	
  int c1=calcRows(linktype);
  m_JacToLink.reallocate(m_JacToLink.Rows,c1);
}

void PoseTree::Link::setPoseType(unsigned short posetype=63){
  m_PoseType=posetype;
  int r=calcRows(posetype);
  m_JacToFrom.reallocate(r,m_JacToFrom.Columns);
  m_JacToLink.reallocate(r,m_JacToLink.Columns);
}

void PoseTree::Link::setFromType(unsigned short fromtype=63){
  m_AddType=(m_LinkType<<6);
  m_AddType+=fromtype;	
  int c=calcRows(fromtype);
  m_JacToFrom.reallocate(m_JacToFrom.Rows,c);
}

PoseTree::Link * PoseTree::Link::addlink(unsigned short posetype, 
					  unsigned short linktype, 
					  double xlink[6]){
  PoseTree::Link **l=m_Links;
  m_Links=new PoseTree::Link*[m_NumberOfLinks+1];
  memcpy(m_Links, l, m_NumberOfLinks*sizeof(Link*));
  delete []l;
  m_Links[m_NumberOfLinks]=new PoseTree::Link(linktype,posetype,xlink,this);
  m_NumberOfLinks++;
  return m_Links[m_NumberOfLinks-1];
}

void PoseTree::Link::addLinks(int num){
  if (num>0){
    PoseTree::Link **l=m_Links;
    m_Links=new PoseTree::Link*[m_NumberOfLinks+num];
    memcpy(m_Links, l, m_NumberOfLinks*sizeof(Link*));
    delete []l;
    for (int i=0;i<num;i++){
      m_Links[m_NumberOfLinks]=new PoseTree::Link(0,0,m_PoseType,&m_ToPose);
      m_NumberOfLinks++;
    }
  }
}
PoseTree::Link * PoseTree::Link::makeLink(int n){
  int k=n-m_NumberOfLinks+1;
  if (k>0)addLinks(k);
  if (n<0)return 0;
  return m_Links[n];
}

unsigned short PoseTree::Link::count(){
  unsigned short n=1;
  for (int i=0;i<m_NumberOfLinks;i++)
    n+=m_Links[i]->count();
  return n;
}

unsigned short PoseTree::Link::leafDim(short *branches){
  unsigned short dim=m_JacToLink.Columns;
  if ((branches[0]>-1)&&(branches[0]<m_NumberOfLinks))
    dim+=m_Links[branches[0]]->leafDim(branches+1);
  return dim;
}

unsigned short PoseTree::Link::getTypes(short *linktypes,short *posetypes)
{
  linktypes[0]=m_LinkType;
  posetypes[0]=m_PoseType;
  unsigned short dim=1;
  for (int i=0;i<m_NumberOfLinks;i++)
    dim+=m_Links[i]->getTypes(linktypes+dim,posetypes+dim);
  return dim;
} 


unsigned short PoseTree::Link::setTypes(short *linktypes,short *posetypes){
  m_LinkType=linktypes[0];
  m_PoseType=posetypes[0];
  unsigned short dim=1;
  for (int i=0;i<m_NumberOfLinks;i++)
    dim+=m_Links[i]->setTypes(linktypes+dim,posetypes+dim);
  return dim;
} 





void PoseTree::addBranch(short num,
			 short *branches, 
			 short *posetypes,
			 short *linktypes, 
			 double *initlinkposes)
{
  if (num<0)return;
  int c=m_Branches.Columns;
  int cold=c;
  if ((num+1)>c)c=num+1;
  int r=m_Branches.Rows;
  m_Branches.grow(r+1,c);
  for (int i=0; i<num;i++)
    m_Branches(r,i)=branches[i];
  for (int i=0;i<r;i++)
    for (int j=cold;j<c;j++)
      m_Branches(i,j)=-1;
  for (int i=num;i<c;i++)
    m_Branches(r,i)=-1;
  
  if (m_Branches(r,0)>-1){
    c=0;
    PoseTree::Link *link=makeLink(m_Branches(r,c));
    while(link){
      link->setLinkPose(&initlinkposes[6*c]);
      link->setLinkType( linktypes[c]);
      link->setPoseType(posetypes[c]);
      c++;
      if (c>=num)break;
      link=link->makeLink(m_Branches(r,c));
    }
  }
  reallocate();
}




void PoseTree::addBranch(short num,
		   short *branches)
{ 
  if (num<0)return;
  int c=m_Branches.Columns;
  int cold=c;
  if ((num+1)>c)c=num+1;
  int r=m_Branches.Rows;
  m_Branches.grow(r+1,c);
  for (int i=0; i<num;i++)
    m_Branches(r,i)=branches[i];
  for (int i=0;i<r;i++)
    for (int j=cold;j<c;j++)
      m_Branches(i,j)=-1;
  for (int i=num;i<c;i++)
    m_Branches(r,i)=-1;
  if (m_Branches(r,0)>-1){
    c=0;
    PoseTree::Link *link=makeLink(m_Branches(r,c));
    while(link){
      c++;
      if (c>=num)break;
      link=link->makeLink(m_Branches(r,c));
    }
  }
  reallocate();
}


void PoseTree::addLinks(int num){
  PoseTree::Link **l=m_Links;
  m_Links=new Link*[m_NumberOfLinks+num];
  memcpy(m_Links, l, m_NumberOfLinks*sizeof(PoseTree::Link*));
  delete []l;
  for (int i=0;i<num;i++){
    m_Links[m_NumberOfLinks]=new PoseTree::Link(0,0,m_CovType,&m_RootPose);
    m_NumberOfLinks++;
  }
  reallocate();
}
PoseTree::Link * PoseTree::makeLink(int n){
  int k=n-m_NumberOfLinks+1;
  if (k>0)addLinks(k);
  if (n<0)return 0;
  return m_Links[n];  
}



unsigned short PoseTree::count(){
  unsigned short n=1;
  for (int i=0;i<m_NumberOfLinks;i++)
    n+=m_Links[i]->count();
  return n;
}



void PoseTree::save(ShortMatrix &treeinfo, Matrix &poseinfo){
  calc();
  poseinfo.reallocateZero(dim(),1);
  getCoordinates(&poseinfo(0,0));
  if (m_Branches.Columns==0){
    m_Branches.reallocate(1);
    m_Branches(0,0)=-1;
  }
  int c=m_Branches.Columns;
  int cnt=count();
  int r=(2*cnt)/c+1;
  treeinfo.Columns=0;
  treeinfo.grow(m_Branches.Rows+r,c);
  getTypes(treeinfo.Element,treeinfo.Element+cnt);
  treeinfo.offset(r,0,m_Branches.Rows,m_Branches.Columns);
  treeinfo=m_Branches;
  treeinfo.offset((-r),0,r+m_Branches.Rows,c);
}
int  PoseTree::restore(ShortMatrix &treeinfo, Matrix &poseinfo,
		       Timestamp &time){
  clean();
  m_Time=time;
  if (poseinfo.Columns!=1)return 1;
  int dm=poseinfo.Rows;
  int c=treeinfo.Columns;
  int cnt=dm/6;
  int r=(2*cnt)/c+1;
  if (r>=treeinfo.Rows)return 1;
  for (int i=r;i<treeinfo.Rows;i++)
    addBranch(treeinfo.Columns,&treeinfo(i,0)); 
  setTypes(treeinfo.Element,treeinfo.Element+cnt-1);
  setCoordinates(&poseinfo(0,0));
  reallocate();
  if (dm!=dim())return 1;
  return 0;
}
void PoseTree::print(){
  calc();
  m_Time.print();
  std::cerr<<" RootCovType "<<m_CovType<<"\n";
  m_RootPose.print();
  m_Branches.print();
  for (int i=0;i<m_NumberOfLinks;i++){
    m_Links[i]->print();
    std::cerr<<"LEAF \n";
  }
  
}		
void PoseTree::Link::print(){
  std::cerr<<"LinkPose CovType "<<m_LinkType
	   <<" ToPose CovType "<<m_PoseType<<"\n";
  m_LinkPose.print();
  m_ToPose.print();
  for (int i=0;i<m_NumberOfLinks;i++){
    m_Links[i]->print();
  }
}		
