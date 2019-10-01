// = RCSID
//    $Id BlockMatrix.cc ,v 1.1 2006/3/1 
//
// = AUTHOR(S)
//    John Folkesson
//    Copyright (c) 2006 John Folkesson
//    

#include "BlockMatrix.hh"

using namespace Cure;
void Cure::BlockMatrix::setAll(Cure::Matrix &m){
  zero();
  if ((Columns==0)||(Rows==0)){
     return;
  }
  if (getTotalHeight()!=(unsigned long)m.Rows)return;
  if (getTotalWidth()!=(unsigned long)m.Columns)return;
  unsigned long pos[Columns+1];
  pos[0]=0;
    for (unsigned long k=0;k<Columns;k++)
      pos[k+1]=pos[k]+getColumnWidth(k);
  unsigned long r=m.Rows;
  for (unsigned long i=0;i<Rows;i++)
    {
      unsigned long rh=getRowHeight(i);
      for (unsigned long n=0;n<Columns;n++){
	m.offset(0,pos[n],rh,pos[n+1]-pos[n]);
	bool test=false;
	for (int j=0;j<m.Rows;j++)
	  for (int k=0;k<m.Columns;k++)
	    if (m(j,k)!=0){
	      test=true;
	      j=m.Rows;
	      k=m.Columns;
	    }
	if (test)
	  equal(m,i,n);
	m.offset(0,-pos[n],0,0);
      }
      m.offset(rh,0,0,0);
    }
  m.offset(-r,0,r,pos[Columns]);
} 
void Cure::BlockMatrix::getAll(Cure::Matrix &m){
  if ((Columns==0)||(Rows==0)){
    m.reallocate(getTotalHeight(),getTotalWidth());
    return;
  }
  if (Rows<Columns)
    {
      unsigned long pos[Columns+1];
      pos[0]=0;
      for (unsigned long k=0;k<Columns;k++)
	pos[k+1]=pos[k]+getColumnWidth(k);
       m.reallocate(getTotalHeight(),pos[Columns]);
       m=0;
       unsigned long r=m.Rows;
       for (unsigned long i=0;i<Rows;i++)
	 {
	   unsigned long rh=getRowHeight(i);
	   for (unsigned long n=0;n<BlockRow[i].Number;n++){
	     BlockMatrix::Element *e=&(BlockRow[i](n));
	     unsigned long col=e->Column;
	     Matrix *pm=&(*e)();
	     m.offset(0,pos[col],rh,pm->Columns);
	     m=(*pm);
	     m.offset(0,-pos[col],0,0);
	   }
	   m.offset(rh,0,0,0);
	 }
       m.offset(-r,0,r,pos[Columns]);
    }else {
      unsigned long pos[Rows+1];
      pos[0]=0;
      for (unsigned long k=0;k<Rows;k++)
	pos[k+1]=pos[k]+getRowHeight(k);
       m.reallocate(pos[Rows],getTotalWidth());
       m=0;
       unsigned long c=m.Columns;
       for (unsigned long i=0;i<Columns;i++)
	 {
	   unsigned long cw=getColumnWidth(i);
	   for (unsigned long n=0;n<BlockColumn[i].Number;n++){
	     BlockMatrix::Element *e=&(BlockColumn[i](n));
	     unsigned long row=e->Row;
	     Matrix *pm=&(*e)();
	     m.offset(pos[row],0,pm->Rows,pm->Columns);
	     m=(*pm);
	     m.offset(-pos[row],0,0,0);
	   }
	   m.offset(0,cw,0,0);
	 }
       m.offset(0,-c,pos[Rows],c);
    }
} 
void Cure::BlockMatrix::addMultTranspose_(BlockMatrix &sm1, 
					  BlockMatrix &sm2,int which)
{
  if (Rows<Columns){
    BlockMatrix::Column *sm1col=sm1.BlockRow;
    BlockMatrix::Column *sm2col=sm2.BlockRow;
      if (which&2)sm2col=sm2.BlockColumn;
      unsigned long top=sm1.Rows;;
      if (sm1.Columns<sm1.Rows){ 
	top=sm1.Columns;
	sm1col=sm1.BlockColumn;
      } 
      for (unsigned long k=0; k<top; k++)
	for (unsigned long i=0;i<sm1col[k].Number;i++){	    
	  BlockMatrix::Element *e=&(sm1col[k](i));
	  unsigned long r=e->Row;
	  unsigned long s=e->Column;
	  if (which&1){
	    r=s;
	    s=e->Row;
	  }
	  addRowProductTrans_(r,sm2col[s],e,which);
	}
  }else{
    BlockMatrix::Column *sm1col=sm1.BlockColumn;
    BlockMatrix::Column *sm2col=sm2.BlockColumn;
    if (which&1)sm1col=sm1.BlockRow;
    unsigned long top=sm2.Columns;;
    if (sm2.Columns>sm2.Rows){ 
	  top=sm2.Columns;
	  sm2col=sm2.BlockRow;
	}
    for (unsigned long k=0; k<top; k++)
      for (unsigned long i=0;i<sm2col[k].Number;i++){	    
	BlockMatrix::Element *e=&(sm2col[k](i));
	unsigned long c=e->Column;
	unsigned long s=e->Row;
	if (which&2){
	  c=s;
	  s=e->Column;
	}
	addColProductTrans_(c,sm1col[s],e,which);
      }
  }
}
void Cure::BlockMatrix::subtractMultTranspose_(BlockMatrix &sm1, 
					  BlockMatrix &sm2,int which)
{
  if (Rows<Columns){
    BlockMatrix::Column *sm1col=sm1.BlockRow;
    BlockMatrix::Column *sm2col=sm2.BlockRow;
    if (which&2)sm2col=sm2.BlockColumn;
    unsigned long top=sm1.Rows;;
    if (sm1.Columns<sm1.Rows){ 
      top=sm1.Columns;
      sm1col=sm1.BlockColumn;
    } 
    for (unsigned long k=0; k<top; k++)
      for (unsigned long i=0;i<sm1col[k].Number;i++){	    
	BlockMatrix::Element *e=&(sm1col[k](i));
	unsigned long r=e->Row;
	unsigned long s=e->Column;
	if (which&1){
	  r=s;
	  s=e->Row;
	}
	subtractRowProductTrans_(r,sm2col[s],e,which);
      }
  }else{
    BlockMatrix::Column *sm1col=sm1.BlockColumn;
    BlockMatrix::Column *sm2col=sm2.BlockColumn;
    if (which&1)sm1col=sm1.BlockRow;
    unsigned long top=sm2.Columns;;
    if (sm2.Columns>sm2.Rows){ 
	  top=sm2.Columns;
	  sm2col=sm2.BlockRow;
	}
    for (unsigned long k=0; k<top; k++)
      for (unsigned long i=0;i<sm2col[k].Number;i++){	    
	BlockMatrix::Element *e=&(sm2col[k](i));
	unsigned long c=e->Column;
	unsigned long s=e->Row;
	if (which&2){
	  c=s;
	  s=e->Column;
	}
	subtractColProductTrans_(c,sm1col[s],e,which);
      }
  }
}
void Cure::BlockMatrix::transpose(){
    if (Rows<Columns){
      for (unsigned long r=0;r<Rows;r++)
	for (unsigned long i=0;i<BlockRow[r].Number;i++)
	  BlockRow[r](i).transpose();
    }else {
      for (unsigned long r=0;r<Columns;r++)
	for (unsigned long i=0;i<BlockColumn[r].Number;i++)
	  BlockColumn[r](i).transpose();
    }
    BlockMatrix::Column *c=BlockColumn;
    BlockColumn=BlockRow;
    BlockRow=c;
    unsigned long r=Columns;
    Columns=Rows;
    Rows=r;
}


void  Cure::BlockMatrix::factorQR(Cure::BlockMatrix & h)
{
  unsigned long blkrow=0;
  unsigned short matrow=0; 
  unsigned long blkcol=0;
  unsigned short matcol=0; 
  while (iterationQR(h,blkrow,matrow,blkcol,matcol)){
  }
  return;	 
}

bool  Cure::BlockMatrix::iterationQR(Cure::BlockMatrix & h, 
				      unsigned long & blockrow,
				      unsigned short & matrixrow,
				      unsigned long & blockcolumn,
				      unsigned short & matrixcolumn)

{
  unsigned long nblockrow=blockrow;
  unsigned short nmatrow=matrixrow; 
  if (!incrementRow(nblockrow,nmatrow))return false;
  if (getRowHeight(blockrow)>0){
  double t=getValue(blockrow,matrixrow,
		    blockcolumn,matrixcolumn);	
  double rotate[2];
  double dtemp=getValue(nblockrow,nmatrow,blockcolumn,matrixcolumn);	
  rotate[1]=-dtemp;
  rotate[0]=t*t+rotate[1]*rotate[1];
  if (t>=0){
    if (rotate[0]>1E-16){
      
      rotate[0]=1/sqrt(rotate[0]);
      rotate[1]*=rotate[0];
      rotate[0]*=t;
      if (rotate[0]<0){
	rotate[0]=-rotate[0];
	rotate[1]=-rotate[1];
      }
    }
    else {
      rotate[0]=1;
      rotate[1]=0;
    }
  }else{
    if (rotate[0]>1E-16){
      
      rotate[0]=1/sqrt(rotate[0]);
      rotate[1]*=rotate[0];
      rotate[0]*=t;
      if (rotate[0]>0){
	rotate[0]=-rotate[0];
	rotate[1]=-rotate[1];
      }
    }
    else {
      rotate[0]=-1;
      rotate[1]=0;
    }
  }
  Cure::Matrix *m=0;
  if (nblockrow==blockrow){
    for (unsigned long i=0;i<BlockRow[blockrow].Number;i++){
      m=&BlockRow[blockrow](i)();
      for (int q=0;q<m->Columns;q++){
	double d=rotate[0]*(*m)(matrixrow,q)-
	  rotate[1]*(*m)(nmatrow,q);
	(*m)(nmatrow,q)=rotate[1]*(*m)(matrixrow,q)+
	  rotate[0]*(*m)(nmatrow,q);
	(*m)(matrixrow,q)=d;
      }
    }
    for (unsigned long k=0; k<h.BlockColumn[blockrow].Number;k++)
      {
	m=&h.BlockColumn[blockrow](k)();
	for (int q=0;q<m->Rows;q++){
	  double d=rotate[0]*(*m)(q,matrixrow)-
	    rotate[1]*(*m)(q,nmatrow);
	  (*m)(q,nmatrow)=
	    rotate[1]*(*m)(q,matrixrow)+
	    rotate[0]*(*m)(q,nmatrow);
	  (*m)(q,matrixrow)=d;
	}
      }
  } else {
    bool loop=true;
    unsigned long coli=blockcolumn;
    while (loop){
      Cure::Matrix *m1=0;
      if (getMatrix(m1,blockrow,coli)){
	Cure::Matrix *m2=0;
	if (getMatrix(m2,nblockrow,coli)){
	  for (int q=0;q<m2->Columns;q++){
	    double tempd=rotate[0]*(*m1)(matrixrow,q)-
	      rotate[1]*(*m2)(nmatrow,q);
	    (*m2)(nmatrow,q)=rotate[1]*(*m1)(matrixrow,q)+
	      rotate[0]*(*m2)(nmatrow,q);
	    (*m1)(matrixrow,q)=tempd;
	  }
	} else{
	  for (int q=0;q<m1->Columns;q++){
	    double tempd=rotate[1]*(*m1)(matrixrow,q);
	    setValue(tempd,nblockrow,nmatrow,coli,q);
	    (*m1)(matrixrow,q)*=rotate[0];
	  }
	}
      } else{
	Cure::Matrix *m2=0;
	if (getMatrix(m2,nblockrow,coli)){
	  {
	    for (int q=0;q<m2->Columns;q++){
	      setValue((-rotate[1])*(*m2)(nmatrow,q),
		       blockrow,matrixrow,coli,q);
	      (*m2)(nmatrow,q)*=rotate[0];
	    }
	  }
	}
      }
      coli++;
      unsigned short c1=BlockRow[blockrow].arrayColumnPosition(coli);
      unsigned short c2=BlockRow[nblockrow].arrayColumnPosition(coli);
      if (c2<BlockRow[nblockrow].Number)
	coli=BlockRow[nblockrow](c2).Column;
      else loop=false;
      if (c1<BlockRow[blockrow].Number)
	{
	  loop=true;
	  if (coli>BlockRow[blockrow](c1).Column)
	    coli=BlockRow[blockrow](c1).Column;
	}
    }
    loop=true;
    unsigned long kc=0;
    unsigned long k=h.BlockColumn[blockrow].Number;
    unsigned long kn=0;
    unsigned long k2=h.BlockColumn[nblockrow].Number;
    while (loop){
      if (k>0)
	kc=h.BlockColumn[blockrow](h.BlockColumn[blockrow].Number-k).Row;
      else kc=h.Rows;
      if (k2>0)
	kn=h.BlockColumn[nblockrow]
	  (h.BlockColumn[nblockrow].Number-k2).Row;
      else kn=h.Rows;
      if ((k==0)&&(k2==0))loop=false;
      else{
	if (kn==kc){
	  if (k2)
	    {
	      Cure::Matrix *m1=0,*m2=0;
	      if (h.getMatrix(m1,kc,blockrow))
		if (h.getMatrix(m2,kn,nblockrow)){
		  for (int q=0;q<m1->Rows;q++){
		    double d=rotate[0]*(*m1)(q,matrixrow)-
		      rotate[1]*(*m2)(q,nmatrow);
		    (*m2)(q,nmatrow)=
		      rotate[1]*(*m1)(q,matrixrow)+
		      rotate[0]*(*m2)(q,nmatrow);
		    (*m1)(q,matrixrow)=d;
		  }
		}
	    }
	  k--;
	  k2--;
	}else if (kn<kc){
	  if (k2){
	    Cure::Matrix *m2=0;
	    if (h.getMatrix(m2,kn,nblockrow)){
	      for (int q=0;q<m2->Rows;q++){
		double tempd=(-rotate[1])*(*m2)(q,nmatrow);
		(*m2)(q,nmatrow)*=rotate[0];
		h.setValue(tempd,kn,q,blockrow,matrixrow);
	      }
	    }
	    k2--;
	  }
	}else if (kc<kn){
	  if (k){
	    Cure::Matrix *m1=0;
	    if (h.getMatrix(m1,kc,blockrow)){
	      for (int q=0;q<m1->Rows;q++){
		double tempd=rotate[1]*(*m1)(q,matrixrow);
		(*m1)(q,nmatrow)*=rotate[0];
		h.setValue(tempd,kc,q,nblockrow,nmatrow);
	      }
	    }
	    k--;
	  }
	}
      }
    }
  }

  blockrow=nblockrow;
  matrixrow=nmatrow;  
  setValue(0.0,blockrow,matrixrow,blockcolumn,matrixcolumn);
  }else {
    blockrow=nblockrow;
    matrixrow=nmatrow;  
  }
  if (!incrementColumn(blockcolumn,matrixcolumn))return false;
  return true;
}
/**
 * solves this*x=y, this is also writen over.
 */
bool  Cure::BlockMatrix::solve(BlockMatrix &x,const BlockMatrix &y, 
			       const double relativemin)
{
  Cure::BlockMatrix g;
  g.transpose(y);
  bool ret=true;
  Cure::BlockMatrix u;
  Cure::BlockMatrix a;
  Cure::BlockMatrix b;
  unsigned long blkrow=0;
  unsigned short matrow=0; 
  unsigned long blkcol=0;
  unsigned short matcol=0; 

  if (!getRowIndex(blkrow,matrow,0))return true;
  if (!getColumnIndex(blkcol,matcol,0))return true;
  // ShortMatrix perm(0,2);
  bool loop=true;
  while (loop){ 
    unsigned long oldbc=blkcol;
    unsigned long qrblkrow=blkrow;
    unsigned short qrmatrow=matrow; 
    unsigned long qrblkcol=blkcol;
    unsigned short qrmatcol=matcol; 

    while (oldbc==blkcol){
      if(!iterationHessenberg
	 (g, u,a,b, blkrow, matrow, 
	  blkcol, matcol))loop=false;
      if (!iterationQR(g,qrblkrow,qrmatrow,qrblkcol,qrmatcol))loop=false;
      if(!loop)oldbc=blkcol+1;
    }
  }



  x.reallocate(Columns,1);
  x.setColumnWidth(0,1);
  for (unsigned long i=0; i<Columns;i++)
    x.setRowHeight(i,BlockColumn[i].Width);    
  unsigned long top=getTotalWidth();
  unsigned long current=top;
  if (!getRowIndex(blkrow,matrow,top-1))return false;
  if (!getColumnIndex(blkcol,matcol,top-1))return false;
  while (current>0)
    {
      current--;
      double d=g.getValue(0,0,blkrow,matrow);
      unsigned long k=BlockRow[blkrow].arrayColumnPosition(blkcol);
      int start=matcol+1;
      if (BlockRow[blkrow].Number>0)
	if (BlockRow[blkrow](k).Column!=blkcol)start=0;
      while (k<BlockRow[blkrow].Number)
	{
	  unsigned long kcol=BlockRow[blkrow](k).Column;
	  Cure::Matrix *m=0;
	  if (getMatrix(m,blkrow,kcol))
	    for (int i=start; i<m->Columns; i++)
	      {
		d-=(*m)(matrow,i)*
		  x.getValue(kcol,i,0,0);
	      }
	  start=0;
	  k++;
	}
      if (d!=0){
	double min=d*relativemin;
	if (min<0)min=-min;
	double t=getValue(blkrow,matrow,blkcol,matcol);
	
	if ((t>min)||(t<-min))
	  d/=t;
	else 
	  {
	    //	    if (ret) std::cerr<<" "<<t<<" "<<d<<" BAD DIAGONAL IN SOLVE in block: "<<blkrow<<"\n";
	    d=0;
	    //x.zero();
	    ret=false;
	  }
      }
      x.setValue(d,blkcol,matcol,0,0);
      if (!decrementRow(blkrow,matrow))break;
      if (!decrementColumn(blkcol,matcol))break;
    }
  return ret;
}
void  Cure::BlockMatrix::partialSolvepartly(BlockMatrix &g,
					    const BlockMatrix &y,
					    unsigned long startblock)
{
  g.transpose(y);
  Cure::BlockMatrix u;
  Cure::BlockMatrix a;
  Cure::BlockMatrix b;
  unsigned long blkrow=startblock;
  unsigned short matrow=0; 
  unsigned long blkcol=startblock;
  unsigned short matcol=0;
  //  if (!getRowIndex(blkrow,matrow,0))return;
  //if (!getColumnIndex(blkcol,matcol,0))return;
  // ShortMatrix perm(0,2);
  bool loop=true;
  while (loop){ 
    unsigned long oldbc=blkcol;
    unsigned long qrblkrow=blkrow;
    unsigned short qrmatrow=matrow; 
    unsigned long qrblkcol=blkcol;
    unsigned short qrmatcol=matcol; 
    while (oldbc==blkcol){
      if(!iterationHessenberg
	 (g, u,a,b, blkrow, matrow, 
	  blkcol, matcol))loop=false;
     if (!iterationQR(g,qrblkrow,qrmatrow,qrblkcol,qrmatcol))loop=false;
     if(!loop)oldbc=blkcol+1;
    }
  }
}
void  Cure::BlockMatrix::triangle(BlockMatrix &g,unsigned long startblock)
{
  if (startblock>=Columns)return;
  Cure::BlockMatrix u;
  Cure::BlockMatrix a;
  Cure::BlockMatrix b;
  unsigned long blkrow=startblock;
  unsigned short matrow=0; 
  unsigned long blkcol=startblock;
  unsigned short matcol=0;
  unsigned long startc=0;
  for (unsigned long i=0;i<startblock;i++){
    startc+=getColumnWidth(i);
  }
  if (!getRowIndex(blkrow,matrow,startc))return;
  if (!getColumnIndex(blkcol,matcol,startc))return;
  bool loop=true;
  unsigned long qrblkrow=blkrow;
  unsigned short qrmatrow=matrow; 
  unsigned long qrblkcol=blkcol;
  unsigned short qrmatcol=matcol; 
  while (loop){
    unsigned long oldbc=blkcol;
    qrblkrow=blkrow;
    qrmatrow=matrow; 
    qrblkcol=blkcol;
    qrmatcol=matcol; 
    while (oldbc==blkcol){
      if(!iterationHessenberg
	 (g, u,a,b, blkrow, matrow, 
	  blkcol, matcol))loop=false;
     if (!iterationQR(g,qrblkrow,qrmatrow,qrblkcol,qrmatcol))loop=false;
     if(!loop)oldbc=blkcol+1;
    }
  }
  /*
  double d=getValue(qrblkrow,qrmatrow,qrblkcol,qrmatcol);
  if (d<0){
    setValue(-d,qrblkrow,qrmatrow,qrblkcol,qrmatcol);
    for (unsigned long i=0; i<g.Columns; i++){
      Matrix *m=g.getBlock(qrblkrow,i);
      if (m){
	for (int j=0;j<m->Columns;j++)
	  (*m)(qrmatrow,j)*=-1;
      }
    }
  }
  */
}

/**
 * solves this*x=y, this is also writen over.
 */
void  Cure::BlockMatrix::partialSolve(BlockMatrix &g,const BlockMatrix &y)
{
  g.transpose(y);
  Cure::BlockMatrix u;
  Cure::BlockMatrix a;
  Cure::BlockMatrix b;
  unsigned long blkrow=0;
  unsigned short matrow=0; 
  unsigned long blkcol=0;
  unsigned short matcol=0;
  if (!getRowIndex(blkrow,matrow,0))return;
  if (!getColumnIndex(blkcol,matcol,0))return;
  // ShortMatrix perm(0,2);
  bool loop=true;
  while (loop){ 
    unsigned long oldbc=blkcol;
    unsigned long qrblkrow=blkrow;
    unsigned short qrmatrow=matrow; 
    unsigned long qrblkcol=blkcol;
    unsigned short qrmatcol=matcol; 
    while (oldbc==blkcol){
      if(!iterationHessenberg
	 (g, u,a,b, blkrow, matrow, 
	  blkcol, matcol))loop=false;
     if (!iterationQR(g,qrblkrow,qrmatrow,qrblkcol,qrmatcol))loop=false;
     if(!loop)oldbc=blkcol+1;
    }
  }
}
bool
Cure::BlockMatrix::backSubstituteTo(BlockMatrix &x,const BlockMatrix &g,
				    const double maxx, double gzero,
				    unsigned long stopblk)	       
{
  if (maxx<=0)return false;
  if (gzero<0)gzero=0;
  //  x.reallocate(g.Columns,g.Rows);
  x.reallocate(Columns,g.Rows);
  x.copyRowHeightsToColumnWidths(g);
  x.copyColumnWidthsToRowHeights(*this);
  if (g.Columns!=Rows)return false;
  if (g.Rows<1)return true;
  unsigned long top=getTotalWidth();
  unsigned long current=top;
  unsigned long blkrow=0;
  unsigned short matrow=0; 
  unsigned long blkcol=0;
  unsigned short matcol=0; 
  if (!getRowIndex(blkrow,matrow,top-1))return true;
  if (!getColumnIndex(blkcol,matcol,top-1))return false;

  //  while (current>0)
  while (blkcol>=stopblk)
    {
      current--;
      unsigned long xblkcol=0;
      unsigned short xmatcol=0;
      bool xcol=true;
      while (xcol){
	double d=g.getValue(xblkcol,xmatcol,blkrow,matrow);
	unsigned long k=BlockRow[blkrow].arrayColumnPosition(blkcol);
	int start=matcol+1;
	//check for very odd case that there is no block element 
	//at (*this)(blkrow,blkcol)
	if(BlockRow[blkrow].Number>0)
	  if (BlockRow[blkrow](k).Column!=blkcol)start=0;
	while (k<BlockRow[blkrow].Number)
	  {
	    unsigned long kcol=BlockRow[blkrow](k).Column;
	      Cure::Matrix *m=0;
	      if (getMatrix(m,blkrow,kcol))
		for (int i=start; i<m->Columns; i++)
		{
		  d-=(*m)(matrow,i)*
		    x.getValue(kcol,i,xblkcol,xmatcol);
		}
	      start=0;
	      k++;
	  }
	if (d!=0){
	  if (d>gzero)d-=gzero;
	  else if (d<-gzero)d+=gzero;
	  else d=0;
	  double min=d/maxx;
	  if (min<0)min=-min;
	  double t=getValue(blkrow,matrow,blkcol,matcol);
	  if (t>=0){
	    if(t<min){
		d*=(t/min);
		t=min;
	      }
	    }	
	  else if (t>-min){
	    d*=(-t/min);
	    t=-min;
	  }
	  if (d!=0)
	    d/=t;
	}
	x.setValue(d,blkcol,matcol,xblkcol,xmatcol);
	if (!x.incrementColumn(xblkcol,xmatcol))xcol=false;
      }
      if (!decrementRow(blkrow,matrow))break;
      if (!decrementColumn(blkcol,matcol))break;
    }
  return true;
}

bool
Cure::BlockMatrix::backSubstituteTo(BlockMatrix &x, 
				    const BlockMatrix &rhs,
				    const double relativemin,
				    unsigned long stopblk)
{
  x.reallocate(Columns,rhs.Rows);
  x.copyRowHeightsToColumnWidths(rhs);
  x.copyColumnWidthsToRowHeights(*this);
  if (rhs.Columns!=Rows)return false;
  if (rhs.Rows<1)return true;
  unsigned long top=getTotalWidth();
  unsigned long current=top;
  unsigned long blkrow=0;
  unsigned short matrow=0; 
  unsigned long blkcol=0;
  unsigned short matcol=0; 
  if (!getRowIndex(blkrow,matrow,top-1))return true;
  if (!getColumnIndex(blkcol,matcol,top-1))return false;
  while (blkcol>=stopblk)
    {
      current--;
      unsigned long xblkcol=0;
      unsigned short xmatcol=0;
      bool xcol=true;
      while (xcol){
	double d=rhs.getValue(xblkcol,xmatcol,blkrow,matrow);
	
	unsigned long k=BlockRow[blkrow].arrayColumnPosition(blkcol);
	int start=matcol+1;
	//check for very odd case that there is no block element 
	//at (*this)(blkrow,blkcol)
	if (BlockRow[blkrow].Number>0)
	  if (BlockRow[blkrow](k).Column!=blkcol)start=0;
	while (k<BlockRow[blkrow].Number)
	  {
	    unsigned long kcol=BlockRow[blkrow](k).Column;
	      Cure::Matrix *m=0;
	      if (getMatrix(m,blkrow,kcol))
		for (int i=start; i<m->Columns; i++)
		  {
		    d-=(*m)(matrow,i)*
		      x.getValue(kcol,i,xblkcol,xmatcol);
		  }
	      start=0;
	      k++;
	  }
	if (d!=0){
	  double min=d*relativemin;
	  if (min<0)min=-min;
	  double t=getValue(blkrow,matrow,blkcol,matcol);
	  
	  if ((t>min)||(t<-min))
	    d/=t;
	  else 
	    {
	      d=0;
	      return false;
	    }
	  
	}
	x.setValue(d,blkcol,matcol,xblkcol,xmatcol);
	if (!x.incrementColumn(xblkcol,xmatcol))xcol=false;
      }
      if (!decrementRow(blkrow,matrow))break;
      if (!decrementColumn(blkcol,matcol))break;
    }
  return true;
}
bool
Cure::BlockMatrix::partialSolvepartly(BlockMatrix &x,const BlockMatrix &g,
				      const double relativemin,
				      unsigned long length)
{
  bool ret=true;
  x.reallocate(Columns,1);
  x.setColumnWidth(0,1);
  for (unsigned long i=0; i<Columns;i++)
    x.setRowHeight(i,BlockColumn[i].Width);    
  unsigned long top=getTotalWidth();
  unsigned long current=top;
  unsigned long blkrow=0;
  unsigned short matrow=0; 
  unsigned long blkcol=0;
  unsigned short matcol=0;
  length=Columns-length;
  if (!getRowIndex(blkrow,matrow,top-1))return Columns-1;
  if (!getColumnIndex(blkcol,matcol,top-1))return Columns-1;
  while (current>0)
    {
      if (blkrow<length)break;
      current--;
      double d=g.getValue(0,0,blkrow,matrow);
      
      unsigned long k=BlockRow[blkrow].arrayColumnPosition(blkcol);
      int start=matcol+1;
      if (BlockRow[blkrow].Number>0)
	if (BlockRow[blkrow](k).Column!=blkcol)start=0;
      while (k<BlockRow[blkrow].Number)
	{
	  unsigned long kcol=BlockRow[blkrow](k).Column;
	  Cure::Matrix *m=0;
	  if (getMatrix(m,blkrow,kcol))
	    for (int i=start; i<m->Columns; i++)
	      {
		d-=(*m)(matrow,i)*
		  x.getValue(kcol,i,0,0);
	      }
	  start=0;
	  k++;
	}
      
	
      if (d!=0){
	double min=d*relativemin;
	if (min<0)min=-min;
	double t=getValue(blkrow,matrow,blkcol,matcol);
	
	if ((t>min)||(t<-min))
	  d/=t;
	else 
	  {
	    //	    if (ret) std::cerr<<" "<<t<<" "<<d<<" BAD DIAGONAL IN SOLVE in block: "<<blkrow<<"\n";
	    d=0;
	    ret=false;
	  }
      }
      
      x.setValue(d,blkcol,matcol,0,0);
      if (!decrementRow(blkrow,matrow))break;
      if (!decrementColumn(blkcol,matcol))break;
    }
  return ret;
}
bool
Cure::BlockMatrix::partialSolve(BlockMatrix &x,const BlockMatrix &g,
				const double relativemin )
{
  bool ret=true;
  x.reallocate(Columns,1);
  x.setColumnWidth(0,1);
  for (unsigned long i=0; i<Columns;i++)
    x.setRowHeight(i,BlockColumn[i].Width);    
  unsigned long top=getTotalWidth();
  unsigned long current=top;
  unsigned long blkrow=0;
  unsigned short matrow=0; 
  unsigned long blkcol=0;
  unsigned short matcol=0; 
  if (!getRowIndex(blkrow,matrow,top-1))return Columns-1;
  if (!getColumnIndex(blkcol,matcol,top-1))return Columns-1;
  while (current>0)
    {
      current--;
      double d=g.getValue(0,0,blkrow,matrow);
      unsigned long k=BlockRow[blkrow].arrayColumnPosition(blkcol);
      int start=matcol+1;
      if(BlockRow[blkrow].Number>0)
	if (BlockRow[blkrow](k).Column!=blkcol)start=0;
      while (k<BlockRow[blkrow].Number)
	{
	  unsigned long kcol=BlockRow[blkrow](k).Column;
	  Cure::Matrix *m=0;
	  if (getMatrix(m,blkrow,kcol))
	    for (int i=start; i<m->Columns; i++)
	      {
		d-=(*m)(matrow,i)*
		  x.getValue(kcol,i,0,0);
	      }
	  start=0;
	  k++;
	}


      if (d!=0){
	double min=d*relativemin;
	if (min<0)min=-min;
	double t=getValue(blkrow,matrow,blkcol,matcol);
	
	if ((t>min)||(t<-min))
	  d/=t;
	else 
	  {
	    //	    if (ret) std::cerr<<" "<<t<<" "<<d<<" BAD DIAGONAL IN SOLVE in block: "<<blkrow<<"\n";
	    d=0;
	    ret=false;
	  }
      }
      x.setValue(d,blkcol,matcol,0,0);
      if (!decrementRow(blkrow,matrow))break;
      if (!decrementColumn(blkcol,matcol))break;
    }
  return ret;
}

bool
Cure::BlockMatrix::partialSolve(BlockMatrix &x,const BlockMatrix &g,
				const double maxx, double gzero )
{
  if (maxx<=0)return false;
  if (gzero<0)gzero=0;
  x.reallocate(Columns,1);
  x.setColumnWidth(0,1);
  for (unsigned long i=0; i<Columns;i++)
    x.setRowHeight(i,BlockColumn[i].Width);    
  unsigned long top=getTotalWidth();
  unsigned long current=top;
  unsigned long blkrow=0;
  unsigned short matrow=0; 
  unsigned long blkcol=0;
  unsigned short matcol=0; 
  if (!getRowIndex(blkrow,matrow,top-1))return Columns-1;
  if (!getColumnIndex(blkcol,matcol,top-1))return Columns-1;
  while (current>0)
    {
      current--;
      double d=g.getValue(0,0,blkrow,matrow);
      unsigned long k=BlockRow[blkrow].arrayColumnPosition(blkcol);
      int start=matcol+1;
      if (BlockRow[blkrow].Number>0)
	if (BlockRow[blkrow](k).Column!=blkcol)start=0;
      while (k<BlockRow[blkrow].Number)
	{
	  unsigned long kcol=BlockRow[blkrow](k).Column;
	  Cure::Matrix *m=0;
	  if (getMatrix(m,blkrow,kcol))
	    for (int i=start; i<m->Columns; i++)
	      {
		d-=(*m)(matrow,i)*
		  x.getValue(kcol,i,0,0);
	      }
	  start=0;
	  k++;
	}
      if (d!=0){
	if (d>gzero)d-=gzero;
	else if (d<-gzero)d+=gzero;
	else d=0;
	double min=d/maxx;
	if (min<0)min=-min;
	double t=getValue(blkrow,matrow,blkcol,matcol);
	if (t>=0){
	  if(t<min){
	    d*=(t/min);
	    t=min;
	  }
	}	
	else if (t>-min){
	  d*=(-t/min);
	  t=-min;
	}
	if (d!=0)
	  d/=t;
      }
	
      x.setValue(d,blkcol,matcol,0,0);
      if (!decrementRow(blkrow,matrow))break;
      if (!decrementColumn(blkcol,matcol))break;
    }
  return true;
}

void  Cure::BlockMatrix::qR
(Cure::BlockMatrix & q){
  hessenberg(q);
  factorQR(q);
}

void  Cure::BlockMatrix::hessenberg
(Cure::BlockMatrix & h)
{
  //  h.reallocate(Rows,Rows);
  //for (unsigned long  i=0;i<Rows;i++)
  //h.setDiagonal(1.0,i,getRowHeight(i));  
  Cure::BlockMatrix u;
  Cure::BlockMatrix a;
  Cure::BlockMatrix b;
  unsigned long  blockrow=0;
  unsigned short  matrixrow=0;
  unsigned long blockcolumn=0;
  unsigned short  matrixcolumn=0;
  while (iterationHessenberg
	   (h, u, a,b,blockrow, matrixrow, 
	    blockcolumn, matrixcolumn))
    {
    }
}


bool  Cure::BlockMatrix::iterationHessenberg
(Cure::BlockMatrix & h, Cure::BlockMatrix &u,
 Cure::BlockMatrix & a, Cure::BlockMatrix &b,
 unsigned long & blockrow,
 unsigned short & matrixrow,
 unsigned long & blockcolumn,
 unsigned short & matrixcolumn)
{ 
  if ((Columns==0)||(Rows==0))return false;
  int hgt=getRowHeight(blockrow);
  int wdt=(int)BlockColumn[blockcolumn].Width;  
  if ((hgt==0)||(wdt==0))
    {
      if (hgt==0)
	if (!incrementRow(blockrow,matrixrow))return false;     
      if (wdt==0)
	if (!incrementColumn(blockcolumn,matrixcolumn))return false;  
      return true;
    }
  unsigned short nri=matrixrow;
  unsigned long nr=blockrow;
  if (!incrementRow(nr,nri))return false;
  unsigned short cnri=nri;
  unsigned long cnr=nr;
  if (!incrementRow(cnr,cnri))return false;
  Cure::Matrix temp(1);
  if (((blockrow==0)&&(matrixrow==0))||(u.Columns==0)){
    a.reallocate(Rows,1);
    b.reallocate(1,Rows);
    u.reallocate(Rows,1);
    u.setColumnWidth(0,1);
    for (unsigned long i=0; i<Rows;i++){
      u.setRowHeight(i,BlockRow[i].Width);
    }
  }
  u.zero();
  double d=0;
  Cure::Matrix *t=0;
  for (unsigned long i=0;i<BlockColumn[blockcolumn].Number;i++) 
    {
      unsigned long row=BlockColumn[blockcolumn](i).Row;
      if (row==nr)t=&BlockColumn[blockcolumn](i)();
      else if (row>nr){
	Cure::Matrix *pt=&BlockColumn[blockcolumn](i)();
	pt->offset(0,matrixcolumn,pt->Rows,1);
	u.equal(*pt,row,0);
	temp.multTranspose_(*pt,*pt,1);
	pt->offset(0,-matrixcolumn,pt->Rows,wdt);
	d+=temp(0,0);
      }
    }
  double dt=0;
  if (t){
    dt=(*t)(nri,matrixcolumn);
    t->offset(0,matrixcolumn,t->Rows,1);
    u.equal(*t,nr,0);

    t->offset(0,-matrixcolumn,t->Rows,wdt);
    t=&(*u(nr,0))();
    for (int i=0;i<nri+1;i++)
      (*t)(i,0)=0;
    temp.multTranspose_(*t,*t,1);
    d+=temp(0,0);
  }
  double sq=dt*dt+d;
  sq=sqrt(sq);
  sq+=dt;
  u.setValue(sq,nr,nri,0,0); 
  sq*=sq;
  sq+=d;
  if (sq>1E-32)
    {
      sq=sqrt(sq);
      sq=-M_SQRT2/sq;
      u*=sq;
    }else{
      u.zero();
    }
  b.multTranspose_(u,(*this),1);
  subtractProduct_(u,b);
  for (unsigned long i=0;i<BlockColumn[blockcolumn].Number;i++) 
    {
      unsigned long k=BlockColumn[blockcolumn](i).Row;
      Cure::Matrix *pt=&BlockColumn[blockcolumn](i)();
      if (k==nr){
	for (int i=nri+1; i<pt->Rows;i++)
	(*pt)(i,matrixcolumn)=0;
      }
      else if (k>nr){
	for (int i=0; i<pt->Rows;i++)
	  (*pt)(i,matrixcolumn)=0;
      }
    }
  a.multiply_(h,u);
  h.subtractMultTranspose_(a,u,2);  
  cnr=nr;
  blockrow=nr;
  matrixrow=nri;
  nr=blockcolumn;
  if (!incrementColumn(blockcolumn,matrixcolumn))return false;
  if (nr==0)return true;
  if (nr==blockcolumn)nr--;
  if(nr==0)return true;
  nr--;
  for (unsigned long j=0;j<BlockColumn[nr].Number;j++) 
    {
      unsigned long row=BlockColumn[nr](j).Row;
	if (row>=cnr)
	  {
	    zero(row,nr);
	    j--;
	  }
    }
  return true;
}

double BlockMatrix::determinant(){
    BlockMatrix h;
    BlockMatrix a(*this);
    a.hessenberg(h);
    a.factorQR(h);
    unsigned long row=0;
    double d=0;
    while(row<Rows){
      Matrix *m=&(*a(row,row))();
      for (int i=0;i<m->Rows;i++)
	d*=(*m)(i,i);
      row++;
    }
    return d;
  }

double BlockMatrix::trace(){
    unsigned long row=0;
    double d=0;
    while(row<Rows){
      Matrix *m=&((*(*this)(row,row)))();
	for (int i=0;i<m->Rows;i++)
	  d+=(*m)(i,i);
      row++;
    }
    return d;
  }

void Cure::invertPositiveDef(Cure::Matrix &m,double minEigen)  
{
  int pdim=m.Rows;
  if (pdim<2)
    {
      if (pdim==1){
	if (m(0,0)>0){
	  m(0,0)=1/m(0,0);
	} else{
	  m(0,0)=0;
	}
      }
      return;
    }
  Matrix lambda(pdim);
  Matrix ev(pdim);
  m.symmetricEigen(lambda,ev,2);
  int test=0;  
  double d=lambda(0,0);
  double e=lambda(pdim-1,pdim-1);
  if (d<0)d=-d;
  if (e<0)e=-e;
  if (d>1E7*e)test=1; 
 if (minEigen<d*1E-8)minEigen=d*1E-8;
  for (int i=0; i<pdim; i++)
    if (lambda(i,i)<minEigen)test=1;
  if (!test)test=m.invert();
  if (test)
    {
      m=0;
      for (int i=0; i<pdim; i++)
	{
	  if (lambda(i,i)>=minEigen)
	    {
	      for (int j=0; j<pdim; j++)
		for (int k=0; k<pdim; k++)
		  m(k,j)+=(ev(j,i)*ev(k,i)/lambda(i,i));
	    }
	}
      test=0;
    }
}
