// = AUTHOR(S)
//    John Folkesson
//    
//    March 1, 2006
//
//    Copyright (c) 2006 John Folkesson
//    
#ifndef BLOCKMATRIX_H
#define BLOCKMATRIX_H

#include "Matrix.hh"
#include "ShortMatrix.hh"
#include "CureDebug.hh"

namespace Cure{  

  /**   
   * This class has existed for some time but is poorly documented so
   * I have been keeping it out of cure toolbox until now.  It still is
   * poorly doc but in the toolbox.  
   *
   * The BlockMatrix class is for handling sparse blocks inside of a
   * matrix efficiently.  So if one has a band of N 3x3 matricies for
   * example one can solve a linear equation in time that is linear
   * in N.  The BlockMatrix has ordinary CURE::Matrix as the elements
   * of its array.  This makes it more complicated to work with as one
   * needs to set up things a bit more carefully (ie each column/Row
   * has its own 'width'.  If you don't specify these widths funny
   * stuff might happen.  One might skip down in this to the
   * BlockMatrix class as the Element and Column are only used
   * internally and need not be dealt with by most users.
   *
   * 
   */
class BlockMatrix
{
public:
  class Column;
  protected:
  /**
     * This is an element of the block matrix array.  It has a matrix
     * and knows its own row/column number.  This is allocated as
     * needed to the block Matrix.  It can be necessary to explictly
     * delete these (by calling zero(r,c) on the BlockMatrix) if you
     * know the element is zero.  That is to say the multiplication
     * may have been set up to eliminate this but the element will
     * hang around with like 1E-18 in the values.  This will then lead
     * to fill ...
     */
  class Element{
    friend class Cure::BlockMatrix;
    friend class Cure::BlockMatrix::Column;
  public:
    /** Each element of a BlockMatrix knows its Row counted in blocks*/
    unsigned long Row;
    /** Each element of a BlockMatrix knows its Column counted in blocks*/
    unsigned long Column;
    /** Here are where the numbers actually are kept.*/
    Cure::Matrix Block;
    /** Constructor is empty */
    Element(){}
    /**
     * You can specify to set the new element's matrix to m or its transpose.
     */
    Element(const Cure::Matrix &m,const unsigned long r,
	   const unsigned long c,bool transpose=false){
      if (transpose) setMatrixTranspose(m,r,c);
      else setMatrix(m,r,c);
    }
    ~Element(){}
  protected:
   /**
     * This sets up the size and location of this element.
     * It is called from the BlockMatrix class methods. 
     * @param r the row index of the block element.
     * @param height the hieght (number of rows) of the block element.
     * @param c the column index of the block element.
     * @param width the width (number of columns) of the block element.
    */
    void setMatrix(const unsigned long r,short height,
		   const unsigned long c,short width){
      Block.reallocate(height,width);
      Row=r;
      Column=c;
    }
    /**
     * This sets thsi element equal to a given matrix. 
     * It is called from the BlockMatrix class methods. 
     * @param m the matrix to set this equal to.
     * @param r the row index of the block element.
     * @param c the column index of the block element.
     */
    void setMatrix(const Cure::Matrix &m,const unsigned long r,
		const unsigned long c){
      Block=m;
      Row=r;
      Column=c;
    }
    /**
     * This sets the element equal to the transpose of a given matrix.
     * It is called from the BlockMatrix class methods.  
     * @param m the matrix to set this equal to its transpose.
     * @param r the row index of the block element.
     * @param c the column index of the block element.
     */
    void setMatrixTranspose(const Cure::Matrix &m,const unsigned long r,
			 const unsigned long c){
      Block.transpose(m);
      Row=r;
      Column=c;
    }
    /*
     * This is the way to access the Matrix of the block.
     * It is called from the BlockMatrix class methods. 
     */
    Cure::Matrix & operator()(){return Block;}
    /**
     * Copy an element.
     * @param e the element to copy. 
     */
    void operator =(const Cure::BlockMatrix::Element &e){
      setMatrix(e.Block,e.Row,e.Column);	
    }
    /**
     * Copy a matrix to this.
     * 
     * @param m the matrix to set this equal to.
     */
    void operator =(const Cure::Matrix &m){Block=m;}
    /*
     * Tranpose both the matrix and the location.
     */
    void transpose(){
      Block.transpose();
      unsigned long r=Row;
      Row=Column;
      Column=r;
    }
  };
public:
    /**
     * This is a class for both rows and columns (blockwise).  It does the
     * bookeeping on the elements.  So each element has a pointer on
     * the BlockColumn and BlockRow. arrays.  Thus it is just as
     * efficient to go down a column as across a row.  Generally one
     * should not need to look at this as the BlockMatrix class calls
     * these for you.
    */
  class Column{
    friend class Cure::BlockMatrix;
  public:
    unsigned long  getNumber(){return Number;}
  protected:
    /**
     * Here are the pointers to the elements in ascending either row or
     * column order.
     */
    BlockMatrix::Element **Index;
    /**The number of Elements in the column/row*/
    unsigned long  Number;
    /**The width of the column's element's Block */
    unsigned short Width;
    /**This keeps track of how much memory the object has allocated*/
    unsigned long AllocatedNumber;
    Column(){
      Index=0;
      AllocatedNumber=0;
      Number=0;
      Width=0;
    }
    Column(Cure::BlockMatrix::Column &c){
      AllocatedNumber=0;
      Number=0;
      Width=0;
      Index=0;
      (*this)=c;
    }
    ~Column(){
      if (AllocatedNumber==0)return;
      if (Index)delete []Index;
      AllocatedNumber=0;
    }
    void operator =(Cure::BlockMatrix::Column &c){
      zero();
      Width=c.Width;
      for (unsigned long i=0;i<c.Number;i++)
	add(&c(i));
    }
    /**
     * Get an element of the column.  This assumes the caller has
     * determined there is some element at this location
     * (ie. i<Number). 
     * @param i the location of the element in the Index array. 
     */
    BlockMatrix::Element & operator()(unsigned long i)const
    {return *Index[i];}
    /*
     * Adds and element,(used only for coping when the order is known).
     * use addToRow or addToColumn otherwise to keep the order right.
     */
    void add(BlockMatrix::Element *p, short w=-1);
    /**
     * Adds to a row in ascending column order.
     * @param w sets the width (actually height) of the row if not negative.
     */
    void addToRow(BlockMatrix::Element *p, short w=-1);
    /**
     * Adds to a column in ascending row order.
     * @param w sets the width  of the column if not negative.
     */
    void addToColumn(BlockMatrix::Element *p, short w=-1);
    /*
     * Forgets the element pointers.
     */
    void zero(){ Number=0;}
    /**
     * Take a pointer off the index array.
     * @param p will be removed if found on Index.
     */
    void remove(const BlockMatrix::Element *p);
    /**
     * Check if an element is on this column
     * @param p will looked for.
     * @retrun true if p found else false.
     */
    bool find(const BlockMatrix::Element*p){
      for (unsigned long i=0;i<Number;i++)
	if (Index[i]==p)return true;
      return false;
    }  
      /**
       * This is used to find a particular column element in a BlockRow.
       * @c the column to search for.
       * @return the array postition of the first index equal to or
       * greater than c.
       */
    unsigned short arrayColumnPosition(unsigned long c){
      for(unsigned long i=0;i<Number;i++)
	  if (Index[i]->Column>=c)return i;
      return Number;
    }
      /**
       * This is used to find a particular row element in a BlockColumn.
       * @r the row to search for.
       * @return the array postition of the first Row equal to or
       * greater than r.
       */
    unsigned short arrayRowPosition(unsigned long r){
      for(unsigned long i=0;i<Number;i++)
	  if (Index[i]->Row>=r)return i;
      return Number;
    }
    /**
     * Returns the element in colum c if any and changes startindex to
     * point to first equal to c or just below c, for starting search
     * for next nonzero.  This is used to increment though a BlockRow.
     * @param c the BlockColumn Element in this BlockRow to try and return.
     * @startindex the search will start here and this will be set for
     * searching for the next element in the blockrow.
     * @return 0 if not found else the Element at c.
     */
    BlockMatrix::Element * getColumn(unsigned long c,unsigned long &startindex);
    /**
     * Returns the element in row r if any and changes startindex to
     * point to first equal to r or just below r, for starting search
     * for next nonzero.  This is used to increment though a Blockcolumn.
     * @param r the BlockRow Element in this BlockColumn to try and return.
     * @startindex the search will start here and this will be set for
     * searching for the next element in the blockcolumn.
     * @return 0 if not found else the Element at r.
     */
    BlockMatrix::Element * getRow(unsigned long r,unsigned long &startindex);
    
    
    /**
     * Returns the element in column c if any.
     *
     * @param c the Blockcolumn Element in this BlockRow to try and return.
     * @return 0 if not found else the Element at r.
     */
    BlockMatrix::Element* getColumn(const unsigned long c){
      for(unsigned long i=0;i<Number;i++)
	if (Index[i]->Column>=c){
	  if(Index[i]->Column==c)return Index[i];
	  else return 0;
	}
      return 0;
    }
    /**
     * Returns the element in row r if any.
     *
     * @param r the BlockRow Element in this BlockColumn to try and return.
     * @return 0 if not found else the Element at r.
     */
    BlockMatrix::Element* getRow(const unsigned long r){
      for(unsigned long i=0;i<Number;i++)
	if (Index[i]->Row>=r){
	  if(Index[i]->Row==r)return Index[i];
	  else return 0;
	}
      return 0;
    }
  };
public:
  /** The number of BlockRows in this matrix*/
  unsigned long Rows;
  /** The number of BlockColumns in this matrix*/
  unsigned long Columns;
  /** 
   * The array of Columns.  the BlockMatrix class has two pointers to
   * every BlockMatrix::Element, one on BlockColumn and one on
   * BlockRow.  Thus the elements can be acessed either by looking
   * down the Column or across the row which ever is easier.  It is
   * important to access the elements though the functions of the
   * BlockMatrix class in order to avoid messing things up.  One
   * possible problem might be geting the rows out of order on the
   * Column Index array (for example).
   */
  BlockMatrix::Column *BlockColumn;
  /** The array of rows */ 
  BlockMatrix::Column *BlockRow;

protected:
  /** The Number of rows allocated*/
  unsigned long AllocatedRows;
  /** The Number of columns allocated*/
  unsigned long AllocatedColumns;
  
public:
  /**
   * Constructor fo the BlockMatrix Class
   * @param r the number of block rows to allocate (that is rows of Matrix).
   * @param c the number of block columns to allocate (that is cols of Matrix).
   *
   */
  BlockMatrix(unsigned long r=0,
	      unsigned long c=0){
    AllocatedRows=0;
    AllocatedColumns=0;
    BlockColumn=0;
    BlockRow=0;
    reallocate(r,c);
  }
  /**Copy constructor */
  BlockMatrix(BlockMatrix &bm){
    AllocatedRows=0;
    AllocatedColumns=0;
    BlockColumn=0;
    BlockRow=0;
    (*this)=bm;
  }
  ~BlockMatrix(){
    zero();
    if (AllocatedRows)
      if (BlockRow)
	delete[]BlockRow;
    if (AllocatedColumns)
      if (BlockColumn)
	delete[]BlockColumn;
    BlockRow=0;
    BlockColumn=0;
    AllocatedRows=0;
    AllocatedColumns=0;
  }
  /**
   * Allocates the number of Blocks.
   * @param r the number of block rows.
   * @param c the number of block columns.
   */
  void reallocate(unsigned long r=0,unsigned long c=0);
  /**
   * Copy the values in one  blockrow  to another
   * different blockrow in this or another BlockMatrix.
   * The old values in the target are overwritten. 
   * Trying to copy a row to itself will result in zeroing the values.
   * @param fromrow the row of matricies to copy from this BlockMatrix.
   * @param torow the row of matricies to copy to on bm.
   * @param bm the BlockMatrix to copy to.
   * @param setw if false the column widths of bm are assumed to be correct
   *             else if true they are copied from this.
   */
  void copyRow(unsigned long fromrow,unsigned long torow, BlockMatrix &bm, 
	       bool setw=true)
  {
    bm.zeroRow(torow);
    bm.setRowHeight(torow,getRowHeight(fromrow));
    if (setw){
      bm.copyColumnWidths(*this);
    }
    for (unsigned long j=0;j<BlockRow[fromrow].getNumber();j++)
      {
	BlockMatrix::Element *e=&BlockRow[fromrow](j);
	bm.add((*e)(),torow,e->Column);
      }
  }
  /**
   * Same as copyRow but for a column
   * @see copyRow.
   * 
   *
   */
  void copyColumn(unsigned long fromcol,unsigned long tocol, BlockMatrix &bm,
		  bool seth=true)
  {
    bm.zeroColumn(tocol);
    bm.setColumnWidth(tocol,getColumnWidth(fromcol));
    if (seth){
      bm.copyRowHeights(*this);
    }
    for (unsigned long j=0;j<BlockColumn[fromcol].getNumber();j++)
      {
	BlockMatrix::Element *e=&BlockColumn[fromcol](j);
	bm.add((*e)(),e->Row,tocol);
      }
  }
  /**
   * moves blockrow to a new blockrow shifting the rest of the 
   * blockMatrix rows up or down to make room.  Nothing is overwritten
   * just shifted around.
   *
   */
  void moveRow(unsigned long fromrow,unsigned long torow)
  {
    if (fromrow==torow)return;
    BlockMatrix bm(1,Columns);
    copyRow(fromrow,0,bm);
    if (fromrow<torow){
    for (unsigned long j=fromrow;j!=torow;j++)
	copyRow(j+1,j,*this,false);
    }else {
    for (unsigned long j=fromrow;j!=torow;j--)
      copyRow(j-1,j,*this,false);
    }
    bm.copyRow(0,torow,(*this),false);
  }
  
  /**
   * Same as moveRow but for column
   * 
   * 
   *
   */
  void moveColumn(unsigned long fromcol,unsigned long tocol)
  {
    if (fromcol==tocol)return;
    BlockMatrix bm(Columns,1);
    copyColumn(fromcol,0,bm);
    if (fromcol<tocol){
    for (unsigned long j=fromcol;j!=tocol;j++)
	copyColumn(j+1,j,*this,false);
    }else {
    for (unsigned long j=fromcol;j!=tocol;j--)
      copyColumn(j-1,j,*this,false);
    }
    bm.copyColumn(0,tocol,(*this),false);
  }
  
  /**
   * moves a number of blockrows from row to a new blockrow shifting
   * the rest of the blockMatrix rows up or down to make room.
   * Nothing is overwritten just shifted around.
   *
   */
  void moveRows(unsigned short num,unsigned long fromrow,unsigned long torow)
  {
    if (fromrow==torow)return;
    BlockMatrix bm(num,Columns);
    for (unsigned short i=0; i<num;i++){  
      copyRow(fromrow+i,i,bm);
    }
    if (fromrow<torow){
      for (unsigned long j=fromrow;j!=torow;j++)
	copyRow(j+num,j,*this,false);
    }else {
      for (unsigned long j=fromrow;j!=torow;j--)
	copyRow(j-1,j+num-1,*this,false);
    }
    for (unsigned short i=0; i<num;i++){  
      bm.copyRow(i,torow+i,(*this),false);
    }
  }
  /**
   * moves a number of blockColumns from Column to a new blockColumn shifting
   * the rest of the blockMatrix Columns up or down to make room.
   * Nothing is overwritten just shifted around.
   *
   */
  void moveColumns(unsigned short num,unsigned long fromColumn,
		   unsigned long toColumn)
  {
    if (fromColumn==toColumn)return;
    BlockMatrix bm(num,Columns);
    for (unsigned short i=0; i<num;i++){  
      copyColumn(fromColumn+i,i,bm);
    }
    if (fromColumn<toColumn){
      for (unsigned long j=fromColumn;j!=toColumn;j++)
	copyColumn(j+num,j,*this,false);
    }else {
      for (unsigned long j=fromColumn;j!=toColumn;j--)
	copyColumn(j-1,j+num-1,*this,false); 
    }
    for (unsigned short i=0; i<num;i++){  
      bm.copyColumn(i,toColumn+i,(*this),false);
    }
  }
  
  /**
   * Copy a Block Matrix's rows heights.
   * @param bm the matrix to copy.
   */
  void copyRowHeights(const BlockMatrix &bm){
    reallocate(bm.Rows,Columns);
    for (unsigned long i=0; i<Rows;i++)
      setRowHeight(i,bm.getRowHeight(i));
  }
  /**
   * Copy a Block Matrix's column widths.
   * @param bm the matrix to copy.
   */
  void copyColumnWidths(const BlockMatrix &bm){
    reallocate(Rows,bm.Columns);
    for (unsigned long i=0; i<Columns;i++)
      setColumnWidth(i,bm.getColumnWidth(i));
  }
  void copyColumnWidthsToRowHeights(const BlockMatrix &bm){
    reallocate(bm.Columns,Columns);
    for (unsigned long i=0; i<Rows;i++)
      setRowHeight(i,bm.getColumnWidth(i));
  }
  void copyRowHeightsToColumnWidths(const BlockMatrix &bm){
    reallocate(Rows,bm.Rows);
    for (unsigned long i=0; i<Columns;i++)
      setColumnWidth(i,bm.getRowHeight(i));
  }
  /**
   * Copy a Block Matrix.
   * @param bm the matrix to copy.
   */
  void operator =(Cure::BlockMatrix &bm);
  /**
   * Become the transpos of another blockmatrix.
   * @param bm the matrix to copy transpose to this.
   */
  void transpose(const BlockMatrix &bm);
  /**
   * @param m this ordinary matrix gets set to equal this entire block matrix
   */
  void getAll(Cure::Matrix &m);
 
  /**
   * You have to set up the Block heights and Widths correctly ahead of time.
   * @param m  this entire block matrix gets set to equal to m.
   */
  void setAll(Cure::Matrix &m);
  /*
   * Make this smaller by deleting the last rows or columns.
   */
  bool shrink(unsigned long r, unsigned long c)
  {
    if ((r>Rows)||(c>Columns))return false;
    for (unsigned long i=r;i<Rows;i++)
      zeroRow(i);
    for (unsigned long i=c;i<Columns;i++)
      zeroColumn(i);
    Rows=r;
    Columns=c;
    return true;
  }
  /*
   * Make this largerer by adding to the last rows
   */
  void grow(unsigned long r, unsigned long c)
  {
    if (r>AllocatedRows)
    {
      BlockMatrix::Column *temp=BlockRow;
      BlockRow=new BlockMatrix::Column[r];
      for (unsigned long i=0; i<Rows; i++)
	BlockRow[i]=temp[i];
      if (temp)delete []temp;
      AllocatedRows=r;
    }else if (r<Rows){
      for (unsigned long i=r; i<Rows; i++)
	zeroRow(i);
    }
    Rows=r;
    if (c>AllocatedColumns)
    {
      BlockMatrix::Column *temp=BlockColumn;
      BlockColumn=new BlockMatrix::Column[c];
      for (unsigned long i=0; i<Columns; i++)
	BlockColumn[i]=temp[i];
      if (temp) delete []temp;
      AllocatedColumns=c;
      
    }else if (c<Columns){
      for (unsigned long i=c;i<Columns;i++)
	zeroColumn(i);
    }
    Columns=c;
  }

  /**
   * DEPRECIATED
   * Set a particular value in a particular block Matrix creating a
   * new block if needed.  This is a convenient way to be sure a matrix exists
   * before accessing it to set individul values.
   * @param blkr the row index of the block.
   * @param blkc the column index of the block.
   * @param d the value to set along the diagonal of the block (rest is
   * set to 0).  If 0 it just creates a matrix of zeros in the block location.
   * @return the pointer to the newly set matrix.
   */
  Matrix * setBlockMatrix( const unsigned long blkr,
		      const unsigned long blkc,double d=0){
    return setBlock(blkr,blkc,d);
  }
  /**
   * Set a particular value in a particular block Matrix creating a
   * new block if needed.  This is a convenient way to be sure a matrix exists
   * before accessing it to set individul values.
   * @param blkr the row index of the block.
   * @param blkc the column index of the block.
   * @param d the value to set along the diagonal of the block (rest is
   * set to 0).  If 0 it just creates a matrix of zeros in the block location.
   * @return the pointer to the newly set matrix.
   */
  Matrix * setBlock( const unsigned long blkr,
		      const unsigned long blkc,double d=0){
    if ((blkr>=Rows)||(blkc>=Columns)){
      std::cerr<<"BlockMatrix::setBlock out of range\n";
      return 0;
    }
    BlockMatrix::Element * e=(*this)(blkr,blkc);
    if (!e)
      e=setMatrix(blkr,BlockRow[blkr].Width,blkc,BlockColumn[blkc].Width);
    else (*e)().reallocate(BlockRow[blkr].Width,BlockColumn[blkc].Width);
    (*e)()=d;
    return &(*e)();
  }
  /**
   * Sets a block of this to equal a matrix.
   * @param m the matrix to set the block element equal to.
   * @param blkr the block row.
   * @param blkc the block column.
   */
  Matrix *  setBlock(const unsigned long blkr,
		const unsigned long blkc,
		const Cure::Matrix &m){
    setRowHeight(blkr,m.Rows);
    setColumnWidth(blkc,m.Columns);
    BlockMatrix::Element * e=(*this)(blkr,blkc);
    if (e)e->setMatrix(m,blkr,blkc);
    else e=setElement(m,blkr,blkc);
    return &(*e)();
  }
  /**
   * DEPRECIATED
   * Sets a block of this to equal a matrix.
   * @param m the matrix to set the block element equal to.
   * @param r the block row.
   * @param c the block column.
   */
  void equal(const Cure::Matrix &m,const unsigned long r,
	     const unsigned long c){
    setBlock(r,c,m);
  }
  /**
   * adds a matrix to a block of this creating the block if needed.
   * @param m the matrix to add to the block element.
   * @param r the block row.
   * @param c the block column.  
   */
  void add(const Cure::Matrix &m,const unsigned long r,
	   const unsigned long c){
    BlockMatrix::Element * e=(*this)(r,c);
    if (e)(*e)()+=(m);
    else setElement(m,r,c);
  }
  /**
   * subtracts a matrix to a block of this, creating if needed.
   * @param m the matrix to subtract from the block element.
   * @param r the block row.
   * @param c the block column.  
   */
  void subtract(const Cure::Matrix &m,const unsigned long r,
		const unsigned long c){
    BlockMatrix::Element * e=(*this)(r,c);
    if (e)(*e)()-=(m);
    else {
      e=setElement(m,r,c);
      (*e)().minus();
    }
  }  
  /**
   * delete a block.
   * @param r the block row.
   * @param c the block column.  
   */
  void zero(const unsigned long r,
	    const unsigned long c){
    BlockMatrix::Element *ind=0;
    if (getIndex(ind,r,c)){
      BlockRow[r].remove(ind);
      BlockColumn[c].remove(ind);
      delete ind;
    }
  }
  /**
   * delete a row of blocks.
   * @param r the block row.
   */
  void zeroRow(const unsigned long r){
    BlockMatrix::Element *ind=0;
    while (BlockRow[r].Number>0){
      ind=&BlockRow[r](0);
      BlockRow[r].remove(ind);
      BlockColumn[ind->Column].remove(ind);
      delete ind;
    }
  }
  /**
   * delete a column of blocks.
   * @param c the block column.  
   */
  void zeroColumn(const unsigned long c){
    BlockMatrix::Element *ind=0;
    while (0<BlockColumn[c].Number){
      ind=&BlockColumn[c](0);
      BlockColumn[c].remove(ind);
      BlockRow[ind->Row].remove(ind);
      delete ind;
    }
  }
  /**
   * delete all blocks.
   */
  void zero(){
    if (AllocatedRows<AllocatedColumns){
      for (unsigned long r=0;r<AllocatedRows;r++){
	BlockMatrix::Element *ind=0;
	for (unsigned long i=0;i<BlockRow[r].Number;i++){
	  ind=&BlockRow[r](i);
	  BlockColumn[ind->Column].zero();
	  delete ind;
	}
	BlockRow[r].zero();
      }
    }else {
      for (unsigned long r=0;r<AllocatedColumns;r++){
	BlockMatrix::Element *ind=0;
	for (unsigned long i=0;i<BlockColumn[r].Number;i++){
	  ind=&BlockColumn[r](i);
	  BlockRow[ind->Row].zero();
	  delete ind;
	}
	BlockColumn[r].zero();
      }
    }
  }
  /**
   * Set every value to its negative.
   */
  void minus(){
    if (Rows<Columns){
      for (unsigned long r=0;r<Rows;r++)
	for (unsigned long i=0;i<BlockRow[r].Number;i++)
	  BlockRow[r](i)().minus();
    } else {
      for (unsigned long r=0;r<Columns;r++)
	for (unsigned long i=0;i<BlockColumn[r].Number;i++)
	  BlockColumn[r](i)().minus();      
    }
  }
  /**
   * transpose this matrix (all elements)
   */
  void transpose();
  /**
   * Set a particular value in a particular block Matrix element.
   * @param d the value be assigned to the block's matrix element.
   * @param blkr the row index  of the block.
   * @param matr the row of the matrix inside the block.
   * @param blkc the column index of the block.
   * @param matc the column of the matrix inside the block. 
  */
  void setValue(double d,  
		const unsigned long blkr,
		const short matr,
		const unsigned long blkc,
		const short matc){
    /*
    Matrix *m=getBlock(blkr,blkc);
    if (!m)m=setBlock(blkr,blkc);
    (*m)(matr,matc)=d;
    */
    BlockMatrix::Element * e=(*this)(blkr,blkc);
    if (!e)
      e=setMatrix(blkr,BlockRow[blkr].Width,blkc,BlockColumn[blkc].Width);     
    (*e)()(matr,matc)=d;
  }
  void operator = (double d)  
  {
    unsigned long top=Rows;
    if (Columns<Rows)top=Columns;
    for (unsigned long i=0;i<top;i++){
      BlockMatrix::Element * e=(*this)(i,i);
      if (!e)
	e=setMatrix(i,BlockRow[i].Width,i,BlockColumn[i].Width);     
      (*e)()=d;
    }
  }
  /**
   * Get a particular value in a particular block Matrix element.
   * @param blkr the row index  of the block.
   * @param matr the row of the matrix inside the block.
   * @param blkc the column index of the block.
   * @param matc the column of the matrix inside the block. 
   * @return the value at that location. 
   */
  double getValue(const unsigned long blkr,const short matr,
		  const unsigned long blkc,
		  const short matc)const{
    Matrix *m=getBlock(blkr,blkc);
    if(m)return(*m)(matr,matc);
    else return 0;
  }
  

  /**
   * Get a particular value in the entire matrix.  This is accutally
   * calcualted by adding up all the row and column widths so it is a
   * very inefficient way to get a value.  Far better to know the
   * block and matrix positions.
   * @param r the actual factual row of the value.
   * @param c the actual factual column of the value.
   * @return the value at that location. 
   */
  double getValue(unsigned long r,
		unsigned long c)const;
  /**
   * Set a particular value in the entire matrix.  This is accutally
   * calcualted by adding up all the row and column widths so it is a
   * very inefficient way to set a value.  Far better to know the
   * block and matrix positions.
   * @param d the value to give the matrix element.
   * @param r the actual factual row of the value.
   * @param c the actual factual column of the value.
   */
   void setValue(const double d,unsigned long r,
		 unsigned long c);
  /**
   * Sets the hieght of a block row. That is the number of rows
   * in all the block matrix elements in the row.
   * @param r the block row to set the hieght of.
   * @param height the value of the hieght (number of rows)
   */
  void setRowHeight(unsigned long r,
		    unsigned short height){
    if (r>=Rows)
      std::cerr<<"BlockMatrix::setRowHeight out of range\n";
    else BlockRow[r].Width=height;
  }
  /**
   * Sets the width of a block column. That is the number of Columns
   * in all the block matrix elements in the column.
   * @param c the block column to set the width of.
   * @param width the value of the width (number of columns)
   */
  void setColumnWidth(unsigned long c,
		      unsigned short width){
    if (c>=Columns)
      std::cerr<<"BlockMatrix::setColumnWidth out of range\n";
    else BlockColumn[c].Width=width;
  }
  /**
   * DEPRECIATED
   * Get the matrix at a location. 
   * @param r the block row.
   * @param c the block column.  
   * @return 0 if block does not exist else the matrix at this location.
   */
  Matrix * getBlockMatrix(unsigned long r, unsigned long c){
    return getBlock(r,c);
  }
  /**
   * Get the matrix at a location. 
   * @param blkr the block row.
   * @param blkc the block column.  
   * @return 0 if block does not exist else the matrix at this location.
   */
  Matrix * getBlock(const unsigned long blkr, const unsigned long blkc)const{
    if ((blkr>=Rows)||(blkc>=Columns)){
      std::cerr<<"BlockMatrix::getBlock out of range\n";
      return 0;
    }
    BlockMatrix::Element *e=(*this)(blkr,blkc);
    if (e)return &(*e)();
    return 0;
  }
  bool getColumn( BlockMatrix::Element *&ind, 
		 unsigned long r, 
		 unsigned long c)const{
    ind=getColumn(r,c);
    if (ind)return true;
    return false;
  }
  bool  getRow(BlockMatrix::Element *&ind, 
	       unsigned long r, 
	       unsigned long c)const{
    ind=getRow(r,c);
    if (ind)return true;
    return false;
  }
 void  multiply_(BlockMatrix &bm1, BlockMatrix &bm2){
    zero();
    reallocate(bm1.Rows,bm2.Columns); 
    copyRowHeights(bm1);
    copyColumnWidths(bm2);
    addProduct_(bm1,bm2);
  }
  /**
   * Multiplies m1 and this and put the product in ans.
   * @param ans ans=m1*this
   * @param m1 
   * @return true if matrix right size
   */
  bool leftMultiply_(Matrix &ans, Matrix &m1){ 
    long c=getTotalHeight();
    if (c!=m1.Columns)return false;
   long w=getTotalWidth();
    long r=m1.Rows;
    ans.reallocateZero(m1.Rows,w);
    unsigned long top=Columns;
    BlockMatrix::Column *col=BlockColumn;
    long temp[Rows];
    long d=0;
    for (int i=0;i<(int)Rows;i++){
      temp[i]=d;
      d+=getRowHeight(i);
    }
    int cc=0;
    for (unsigned long k=0; k<top; k++){
      for (unsigned long i=0;i<col[k].Number;i++){
	{
	  BlockMatrix::Element *e=&col[k](i);
	  long cw=temp[e->Row];
	  ans.offset(0,cc,r,e->Block.Columns);
	  m1.offset(0,cw,r,e->Block.Rows);  
	  ans.addProduct(m1,e->Block);
	  ans.offset(0,-cc,r,w);
	  m1.offset(0,-cw,r,c);
	}
      }
      cc+=getColumnWidth(k);
    }
    return true;
  }
  /**
   * subtracts this from m1 and put the answer in m1.
   * @param m1 =m1-this
   * @return true if matrix right size
   */
  bool minusFrom_(Matrix &m1){ 
    long r=getTotalHeight();
    if (r!=m1.Rows)return false;
    long w=getTotalWidth();
    if (w!=m1.Columns)return false;
    unsigned long top=Columns;
    BlockMatrix::Column *col=BlockColumn;
    long temp[Rows];
    long d=0;
    for (int i=0;i<(int)Rows;i++){
      temp[i]=d;
      d+=getRowHeight(i);
    }
    int cc=0;
    for (unsigned long k=0; k<top; k++){
      for (unsigned long i=0;i<col[k].Number;i++){
	{
	  BlockMatrix::Element *e=&col[k](i);
	  long cw=temp[e->Row];
	  m1.offset(cw,cc,e->Block.Rows,e->Block.Columns);  
	  m1-=e->Block;
	  m1.offset(-cw,-cc,r,w);
	}
      }
      cc+=getColumnWidth(k);
    }
    return true;
  }
  bool addProduct_(BlockMatrix &sm1, BlockMatrix &sm2){ 
    if ((sm1.Rows!=Rows)||(sm2.Columns!=Columns))
      return false;
    if (sm2.Rows!=sm1.Columns)
      return false;
    if (Rows<Columns){
      unsigned long top=sm1.Rows;
      BlockMatrix::Column *row=sm1.BlockRow;
      if (sm1.Rows>sm1.Columns){
	top=sm1.Columns;
	row=sm1.BlockColumn;
      }
      for (unsigned long k=0; k<top; k++)
        for (unsigned long i=0;i<row[k].Number;i++){
	  {
	    BlockMatrix::Element *e=&row[k](i);
	    unsigned long r=e->Row;
	    unsigned long s=e->Column;
	    addRowProduct_(r,sm2.BlockRow[s],e);
	  }
	}
    }else{
     unsigned long top=sm2.Columns;
     BlockMatrix::Column *row=sm2.BlockColumn;
     if (sm2.Rows<sm2.Columns){
	top=sm2.Rows;
	row=sm2.BlockRow;
      }
      for (unsigned long k=0; k<top; k++)
        for (unsigned long i=0;i<row[k].Number;i++){
	  {
	    BlockMatrix::Element *e=&row[k](i);
	    unsigned long s=e->Row;
	    unsigned long c=e->Column;
	    addColumnProduct_(c,sm1.BlockColumn[s],e);
	  }
	}
    }  
    return true;
  }

  bool subtractProduct_(BlockMatrix &sm1, BlockMatrix &sm2){
    if ((sm1.Rows!=Rows)||(sm2.Columns!=Columns))
      return false;
    if (sm2.Rows!=sm1.Columns)
      return false;
    if (Rows<Columns){
      unsigned long top=sm1.Rows;
      BlockMatrix::Column *row=sm1.BlockRow;
      if (sm1.Rows>sm1.Columns){
	top=sm1.Columns;
	row=sm1.BlockColumn;
      }
      for (unsigned long k=0; k<top; k++)
        for (unsigned long i=0;i<row[k].Number;i++){
	  {
	    BlockMatrix::Element *e=&row[k](i);
	    unsigned long r=e->Row;
	    unsigned long s=e->Column;
	    subtractRowProduct_(r,sm2.BlockRow[s],e);
	  }
	}
    }else{
     unsigned long top=sm2.Columns;
     BlockMatrix::Column *row=sm2.BlockColumn;
     if (sm2.Rows<sm2.Columns){
	top=sm2.Rows;
	row=sm2.BlockRow;
      }
      for (unsigned long k=0; k<top; k++)
        for (unsigned long i=0;i<row[k].Number;i++){
	  {
	    BlockMatrix::Element *e=&row[k](i);
	    unsigned long s=e->Row;
	    unsigned long c=e->Column;
	    subtractColumnProduct_(c,sm1.BlockColumn[s],e);
	  }
	}
    }  
    return true;
  }
  void multTranspose_(BlockMatrix &sm1, BlockMatrix &sm2,int which){
    zero();
    if (which==0){
      reallocate(sm1.Rows,sm2.Columns);
      copyRowHeights(sm1);
      copyColumnWidths(sm2);
    } 
    else if (which==1){
      reallocate(sm1.Columns,sm2.Columns);
      copyColumnWidthsToRowHeights(sm1);
      copyColumnWidths(sm2);
    } 
    else if (which==2){
      reallocate(sm1.Rows,sm2.Rows);
      copyRowHeights(sm1);
      copyRowHeightsToColumnWidths(sm2);
    }
    if (which==3){
      reallocate(sm1.Columns,sm2.Rows);
      copyColumnWidthsToRowHeights(sm1);
      copyRowHeightsToColumnWidths(sm2);
    }  
    addMultTranspose_(sm1, sm2,which);
  }
  void addMultTranspose_(BlockMatrix &sm1, BlockMatrix &sm2,int which);
      
  void subtractMultTranspose_(BlockMatrix &sm1, 
			      BlockMatrix &sm2,int which);
      
  /**
   * This subtracts the product of transpose(a) and b.  It will 
   * allocate the rows and columns and set the row heights and widths.
   * It can calulate only part of the result (all rows but ounly the columns 
   * after and includeing startcol.
   * Result is that this =this - a^Tb for all columns startcol and higher
   *
   * 
   */
  void subtractTransposeAB_(BlockMatrix &a, 
			    BlockMatrix &b,
			    const unsigned long startcol){
    
    reallocate(a.Columns,b.Columns);
    //    subtractMultTranspose_(a,b,1);
    // return;
    if (a.Rows!=b.Rows)return;
    copyColumnWidths(b);
    //    for (unsigned long i=startcol;i<Columns;i++)
    //setColumnWidth(i,b.getColumnWidth(i));
    copyColumnWidthsToRowHeights(a);
    BlockMatrix::Column *cols=b.BlockColumn;
    for (unsigned long k=0; k<a.Columns; k++){
      for(unsigned long c=startcol; c<Columns; c++){
	for (unsigned long i=0;i<cols[c].Number;i++){	    
	  BlockMatrix::Element *e=&(cols[c](i));
	  Matrix *m1=a.getBlock(e->Row,k);
	  if (m1){
	    Cure::Matrix m;
	    m.multTranspose_(*m1,(*e)(),1);
	    Matrix *m3=getBlock(k,c);
	    if (m3)(*m3)-=m;
	    else {
	      m.minus();
	      setBlock(k,c,m);		
	    }
	  }
	}
      }
    }
  }

  bool incrementRow(unsigned long &row,unsigned short &rindex){
    rindex++;
    while (row<Rows){
      if (rindex<BlockRow[row].Width)return true;
      rindex-=BlockRow[row].Width;
      row++;
    }
    return false;
  }

  bool incrementColumn(unsigned long &col, unsigned short &cindex){
    cindex++;
    while (col<Columns){
      if (cindex<BlockColumn[col].Width)return true;
      cindex-=BlockColumn[col].Width;
      col++;
    }
    return false;
  }
  bool decrementRow(unsigned long &row,unsigned short &rindex){
    while (rindex>BlockRow[row].Width)
      rindex--;
    while (rindex==0)
      if (row>0)
	{
	  row--;
	  rindex+=BlockRow[row].Width;
	}else return false;
    rindex--;
    return true;
  }
  bool decrementColumn(unsigned long &col,unsigned short &cindex){
    while (cindex>BlockColumn[col].Width)
      cindex--;
    while (cindex==0)
      if (col>0)
	{
	  col--;
	  cindex+=BlockColumn[col].Width;
	}else return false;
    cindex--;
    return true;
  }
  unsigned short getRowHeight(unsigned long row)const{
    if (row<Rows)
      return BlockRow[row].Width;
    std::cerr<<" "<<row<<" BlockMatrix::getRowHeight out of bounds\n";
    return 0;
  }
  unsigned short getColumnWidth(unsigned long col)const{
    if (col<Columns)
      return BlockColumn[col].Width;
    std::cerr<<" "<<col<<" BlockMatrix::getColumnWidth out of bounds\n";
    return 0;
  }
  unsigned long getTotalHeight(){
    unsigned long high=0;
    for (unsigned long i=0;i<Rows;i++)
      high+=getRowHeight(i);
    return high;
  }
  unsigned long getTotalWidth(){
    unsigned long high=0;
    for (unsigned long i=0;i<Columns;i++)
      high+=getColumnWidth(i);
    return high;
  }
  double determinant();
  double trace();

  bool getMatrix(Cure::Matrix* &m,unsigned long blkrow,
		 unsigned long blkcol)
    const{
    BlockMatrix::Element *e=(*this)(blkrow,blkcol);
    if (e){
      m=&(*e)();
      return true;
    }
    return false;
  }
  /**
   * (this)x=g^T
   * Does the back substitution back to and including the block 'stopblk'.
   * Assums this to be upper triangle form
   *
   * @param x answer is returned here with dim g^T
   *
   * @param g can be a row vector (rhs^T U) or just U for getting a
   * partial inverse @param stopblk the backsubstituion stops after
   * solving this block.  
   *
   *  @param maxx the largest x element that you
   *  want to allow.  So the x will be maxx if answer of the equation
   * equals maxx and then maxx*(maxx/ansx) if ansx>maxx. (Avoid a
   * discontinuity)
   *
   * @param gzero If the backsub magnituded before divideng by the
   * diagonal is less than this it is set to zero. (It is the level of
   * the noise in the calculation.
   *   
   * @return false if fail else true (only fails if the matricies are
   * wrong size or the maxx<0)
   */
  bool
  backSubstituteTo(BlockMatrix &x,const BlockMatrix &g,
		   const double maxx, double gzero,
		   unsigned long stopblk);
  /**
   * (this)x=rhs^T
   * Does the back substitution back to and including the 'stoprow'.
   * Assumes this to be upper triangle form with columns = rhs columns.
   *
   * @param x answer is returned here with dim rhs^T
   *
   * @param rhs can be a row vector (rhs^T U) or just U for getting a
   * partial inverse 
   * @param stoprow the backsubstituion stops after
   * solving this row of x.  
   *
   *  @param relativemin will fail if |x|>1/relativemin
   *
   * @return false if fail else true (fails if any |x| element ends up
   *                   bigger than 1/relativemin.
   *
   *
   *
   */
  bool backSubstituteTo(BlockMatrix &x,const BlockMatrix &rhs, 
			const double relativemin,
			unsigned long stopblock);
  
  /** 
   * this after becomes R=Q^T U^T (this before)
   * Q is orthogonal U is a reflection (also orthogonal
   * g = g_before U Q  
   *
   * Here we start part way down the matrix so the assumption is
   * the matrix blocks above startblock are already upper triangular.
   */
  void  triangle(BlockMatrix &g,
		 unsigned long  startblock);
  /** 
   * this after becomes R=Q^T U^T (this before)
   * Q is orthogonal U is reflection/Projection UU=U=U^T,
   * g = y^T U Q   or g^T= Q^T U^T y
   */
  void  partialSolve(BlockMatrix &g,const BlockMatrix &y);
  /** 
   * this after becomes R=Q^T U^T (this before)
   * Q is orthogonal U is reflection/Projection UU=U=U^T,
   * g = y^T U Q   or g^T= Q^T U^T y
   *
   * Here we start part way down the matrix so the assumption is
   * the matrix blocks above startblock are already upper triangular.
   */
  void  partialSolvepartly(BlockMatrix &g,const BlockMatrix &y, 
			   unsigned long  startblock);
  bool  partialSolve(BlockMatrix &x,const BlockMatrix &g,
		     const double relativemin);
  bool  partialSolvepartly(BlockMatrix &x,const BlockMatrix &g,
			   const double relativemin, 
			   unsigned long length);
  bool
  partialSolve(BlockMatrix &x,const BlockMatrix &g,
				  const double maxx, double gzero );
  
  /**
   * solves this*x=y, this is also writen over.
   * @param relativemin If an element o
   * 
   * @return true if successful.
   */
  bool solve(BlockMatrix &x,const BlockMatrix &y,
	     const double relativemin=1E-9);
  /**
   * Not really hessenberg as I leave the upper part alone.
   *  It is trivail to get the true hessenberg where 
   * only three diagonals remain.  This is a waste of time 
   * for solve which is what I want to use this for.
   *
   *  A=UT 
   *  this starts as A and becomes T
   *   U=(U^T) and U^T*U=I and T is nearly upper triangular.
   *  T has Tij=0 for all i>j+1
   *    
   * If you want U then set h to the identity matrix at the start.
   * h can be a vector for solving a linear equation or just
   * an empty Rows=Columns=0 Matrix for 'low' cost no return.
   * @param h The matrix will be transposed then be multiplied from the right
   * by the reflection matrix U 
   */
  void  hessenberg(Cure::BlockMatrix & h);
  /**
   * Takes a Matrix i the form after hessenberg and makes it upper
   * triangular.  T=QR where Q is orthogonal and R is upper triangle.
   * this starts as T (see hessenberg) and ends as R.
   * If you want Q then set q to the identity matrix at the start.
   * q can be a vector for solving a linear equation or just
   * an empty Rows=Columns=0 Matrix for 'low' cost no return.
   * @param q This matrix will be multiplied from the right
   * by the orthogonal matrix Q 
   */
  void qR(Cure::BlockMatrix & q);
  /**
   * Does one iteration of the QR rotations.
   * @param blockrow current row to have zero set below the diagonal
   * by a rotation 
   * @matrixrow the row inside the block (ie if the blocks are 3x3
   * then this is 0,1 or 2).
   * 
   * @param h This matrix will be multiplied from the right
   * by the orthogonal matrix U 
   * @return true until finished then false.
   */
  bool  iterationQR(Cure::BlockMatrix & h, 
		    unsigned long & blockrow,
		    unsigned short & matrixrow,
		    unsigned long & blockcolumn,
		    unsigned short & matrixcolumn);
      
  /**
   *
   * @return true until finished then false.
   */
  bool  iterationHessenberg
  (Cure::BlockMatrix & h, Cure::BlockMatrix &u,
   Cure::BlockMatrix & a, Cure::BlockMatrix &b,
   unsigned long & blockrow,
   unsigned short & matrixrow,
   unsigned long & blockcolumn,
   unsigned short & matrixcolumn);
  
  void  factorQR(Cure::BlockMatrix & h);
  
  bool getRowIndex(unsigned long &rindex, unsigned short &matrow,
		   const unsigned long row) {       
    unsigned long r=row;
    rindex=0;
    if (Rows==rindex)return false;
    while (r>=BlockRow[rindex].Width){
      r-=BlockRow[rindex].Width;
      rindex++;
      if (Rows==rindex)return false;
    }
    matrow=(unsigned short)r;
    return true;
  }
  bool getColumnIndex(unsigned long &cindex, unsigned short &matcol,
		      const unsigned long col) { 
      
    unsigned long r=col;
    cindex=0;
    if (Columns==cindex)return false;
    while (r>=BlockColumn[cindex].Width){
      r-=BlockColumn[cindex].Width;
      cindex++;
      if (Columns==cindex)return false;
    }
    matcol=(unsigned short)r;
    return true;
  }
  void operator +=(const Cure::BlockMatrix &m){
    if ((Rows!=m.Rows)||(Columns!=m.Columns))return;
    {
      if (Rows<Columns){
	for (unsigned long i=0;i<Rows;i++)
	  for (unsigned long j=0;j<m.BlockRow[i].Number;j++)
	    {
	      BlockMatrix::Element *e=&m.BlockRow[i](j);
	      add((*e)(),e->Row,e->Column);
	    }
      }else{
	for (unsigned long i=0;i<Columns;i++)
	  for (unsigned long j=0;j<m.BlockColumn[i].Number;j++)
	    {
	      BlockMatrix::Element *e=&m.BlockColumn[i](j);
	      add((*e)(),e->Row,e->Column);
	    }
	    
      }
    }
	
  }
  void operator -=(const Cure::BlockMatrix &m){
    if ((Rows!=m.Rows)||(Columns!=m.Columns))return;
    {
      if (Rows<Columns){
	for (unsigned long i=0;i<Rows;i++)
	  for (unsigned long j=0;j<m.BlockRow[i].Number;j++)
	    {
	      BlockMatrix::Element *e=&m.BlockRow[i](j);
	      subtract((*e)(),e->Row,e->Column);
	    }
      }else{
	for (unsigned long i=0;i<Columns;i++)
	  for (unsigned long j=0;j<m.BlockColumn[i].Number;j++)
	    {
	      BlockMatrix::Element *e=&m.BlockColumn[i](j);
	      subtract((*e)(),e->Row,e->Column);
	    }
	    
      }
    }
  }
  void operator *=(const double d){
    if (Rows<Columns){
      for (unsigned long i=0;i<Rows;i++)
	for (unsigned long j=0;j<BlockRow[i].Number;j++)
	  BlockRow[i](j)()*=d;
    }else{
      for (unsigned long i=0;i<Columns;i++)
	for (unsigned long j=0;j<BlockColumn[i].Number;j++)
	  BlockColumn[i](j)()*=d;
    }
  }
  void operator /=(const double d){
    if (Rows<Columns){
      for (unsigned long i=0;i<Rows;i++)
	for (unsigned long j=0;j<BlockRow[i].Number;j++)
	  BlockRow[i](j)()/=d;
    }else{
      for (unsigned long i=0;i<Columns;i++)
	for (unsigned long j=0;j<BlockColumn[i].Number;j++)
	  BlockColumn[i](j)()/=d;
    }
  }
  void thresholdColumn(const unsigned long blkcol,const double threshold){
    for (unsigned long j=0;j<BlockColumn[blkcol].Number;)
      {
	BlockMatrix::Element *ind=&BlockColumn[blkcol](j);
	Matrix *m=&(*ind)();
	bool keep=false;
	for (int i=0;i<m->Rows;i++)
	  for (int k=0;k<m->Columns;k++){
	    if (((*m)(i,k)>threshold)||((*m)(i,k)<-threshold)){
	      keep=true;
	    }else{
	      (*m)(i,k)=0;
	    }
	  }
	if (keep)j++;
	else zero((*ind).Row,  (*ind).Column);
      }
  }
  void thresholdRow(const unsigned long blkrow,const double threshold){
    for (unsigned long j=0;j<BlockRow[blkrow].Number;)
      {
	BlockMatrix::Element *ind=&BlockRow[blkrow](j);
	Matrix *m=&(*ind)();
	bool keep=false;
	for (int i=0;i<m->Rows;i++)
	  for (int k=0;k<m->Columns;k++){
	    if (((*m)(i,k)>threshold)||((*m)(i,k)<-threshold)){
	      keep=true;
	    }else{
	      (*m)(i,k)=0;
	    }
	  }
	if (keep)j++;
	else zero((*ind).Row,  (*ind).Column);
      }
  }
  void swapColumns(const unsigned long fromcol,const unsigned long tocol){
    BlockMatrix::Column c=(BlockColumn[tocol]);
    BlockColumn[tocol]=BlockColumn[fromcol];
    BlockColumn[fromcol]=c;
    for (unsigned long i=0;i<BlockColumn[fromcol].Number;i++)
      BlockRow[BlockColumn[fromcol](i).Row].
	remove(&BlockColumn[fromcol](i));
    for (unsigned long i=0;i<BlockColumn[tocol].Number;i++)
      BlockRow[BlockColumn[tocol](i).Row].
	remove(&BlockColumn[tocol](i));
 
   for (unsigned long i=0;i<BlockColumn[fromcol].Number;i++)
      BlockColumn[fromcol](i).Column=fromcol;
    for (unsigned long i=0;i<BlockColumn[tocol].Number;i++)
      BlockColumn[tocol](i).Column=tocol;

    for (unsigned long i=0;i<BlockColumn[fromcol].Number;i++)
      BlockRow[BlockColumn[fromcol](i).Row].
	addToRow(&BlockColumn[fromcol](i));
    for (unsigned long i=0;i<BlockColumn[tocol].Number;i++)
      BlockRow[BlockColumn[tocol](i).Row].
	addToRow(&BlockColumn[tocol](i));
  }
  void swapRows(const unsigned long fromr,const unsigned long tor){

    BlockMatrix::Column c=(BlockRow[tor]);
    BlockRow[tor]=BlockRow[fromr];
    BlockRow[fromr]=c;

    for (unsigned long i=0;i<BlockRow[fromr].Number;i++)
      BlockColumn[BlockRow[fromr](i).Column].
	remove(&BlockRow[fromr](i));
    for (unsigned long i=0;i<BlockRow[tor].Number;i++)
      BlockColumn[BlockRow[tor](i).Column].
	remove(&BlockRow[tor](i));

    for (unsigned long i=0;i<BlockRow[fromr].Number;i++)
      BlockRow[fromr](i).Row=fromr;
    for (unsigned long i=0;i<BlockRow[tor].Number;i++)
      BlockRow[tor](i).Row=tor;


    for (unsigned long i=0;i<BlockRow[fromr].Number;i++)
      BlockColumn[BlockRow[fromr](i).Column].
	addToColumn(&BlockRow[fromr](i));
    for (unsigned long i=0;i<BlockRow[tor].Number;i++)
      BlockColumn[BlockRow[tor](i).Column].
	addToColumn(&BlockRow[tor](i));
  }
  
  void print(){
    std::cerr<<"Block Matrix "<<Rows<<" "
	     <<Columns<<" "<<getTotalHeight()<<" "<<getTotalWidth()<<"\n";  
    for (unsigned short i=0;i<getTotalHeight();i++){
      for (unsigned short j=0;j<getTotalWidth();j++)
	std::cerr<<getValue(i,j)<<" ";
      std::cerr<<"\n";  
    }
  }
  /**
   * This sets block (blkcol,blkrow) equal to transpose of
   * block(blkrow,blkcol)
   */
  void transposeElement(unsigned long blkrow, unsigned long blkcol){
    BlockMatrix::Element *e=(*this)(blkrow,blkcol);
    setColumnWidth(blkrow,getRowHeight(blkrow));
    setRowHeight(blkcol,getColumnWidth(blkcol));
    if (!e)
      zero(blkcol,blkrow);
    else {
      BlockMatrix::Element *e2=(*this)(blkcol,blkrow);
      if (e2){
	e2->Block.transpose(e->Block);
      }else
	setElementTranspose(e->Block,blkcol,blkrow);
    }
  } 


  protected:
  BlockMatrix::Element * operator()(const unsigned long r, 
				    const unsigned long c)const{
    if (BlockRow[r].Number<BlockColumn[c].Number)
      return getColumn(r,c);
    return getRow(r,c);
  }
  BlockMatrix::Element * getColumn(const unsigned long r, 
				   const unsigned long c)const{
    return BlockRow[r].getColumn(c);
  }
  BlockMatrix::Element * getRow(const unsigned long r, 
				const unsigned long c)const{
    return BlockColumn[c].getRow(r);
  }
  bool getIndex( BlockMatrix::Element *&ind,
		 const unsigned long r, 
		 const unsigned long c)const{
    if (BlockRow[r].Number<BlockColumn[c].Number)
      return getColumn(ind,r,c);
    return getRow(ind,r,c);
  }
  void addRowProduct_(unsigned long r,
		      BlockMatrix::Column & row, 
		      BlockMatrix::Element *e){
    unsigned long startindex=0;
    for (unsigned long  j=0; j<row.Number;j++){
   	unsigned long c=row(j).Column;
	BlockMatrix::Element *e3=BlockRow[r].getColumn(c,startindex);
	if (e3)(*e3)().addProduct((*e)(),row(j)());
	else{
	  Cure::Matrix m;
	  m.multiply_((*e)(),row(j)());
	  setElement(m,r,c);		
	}
      }
  }

  void addRowProductTrans_(unsigned long r,
			 BlockMatrix::Column & row, 
			 BlockMatrix::Element *e,int which){
    unsigned long startindex=0;
    for (unsigned long  j=0; j<row.Number;j++){
      unsigned long c;     
      if (which&2)c=row(j).Row;
      else  c=row(j).Column;
      BlockMatrix::Element *e3=BlockRow[r].getColumn(c,startindex);
      Cure::Matrix m;
      m.multTranspose_((*e)(),row(j)(),which);
      if (e3)(*e3)()+=m;
      else setElement(m,r,c);		
    }
  }
  void addColumnProduct_(unsigned long c,
		      BlockMatrix::Column & col, 
		      BlockMatrix::Element *e){
    unsigned long startindex=0;
    for (unsigned long  j=0; j<col.Number;j++){
   	unsigned long r=col(j).Row;
	BlockMatrix::Element *e3=BlockColumn[c].getRow(r,startindex);
	if (e3)(*e3)().addProduct(col(j)(),(*e)());
	else{
	  Cure::Matrix m;
	  m.multiply_(col(j)(),(*e)());
	  setElement(m,r,c);		
	}
      }
  }
  void subtractRowProductTrans_(unsigned long r,
			 BlockMatrix::Column & row, 
			 BlockMatrix::Element *e,int which){
    unsigned long startindex=0;
    for (unsigned long  j=0; j<row.Number;j++){
      unsigned long c;     
      if (which&2)c=row(j).Row;
      else  c=row(j).Column;
      BlockMatrix::Element *e3=BlockRow[r].getColumn(c,startindex);
      Cure::Matrix m;
      m.multTranspose_((*e)(),row(j)(),which);   
      if (e3)(*e3)()-=m;
      else {
	m.minus();
	setElement(m,r,c);		
      }
    }
  }
  void subtractColumnProduct_(unsigned long c,
		      BlockMatrix::Column & col, 
		      BlockMatrix::Element *e){
    unsigned long startindex=0;
    for (unsigned long  j=0; j<col.Number;j++){
   	unsigned long r=col(j).Row;
	BlockMatrix::Element *e3=BlockColumn[c].getRow(r,startindex);
	if (e3)(*e3)().subtractProduct(col(j)(),(*e)());
	else{
	  Cure::Matrix m;
	  m.multiply_(col(j)(),(*e)());
	  m.minus();
	  setElement(m,r,c);		
	}
      }
  }
  void subtractRowProduct_(unsigned long r,
		      BlockMatrix::Column & row, 
		      BlockMatrix::Element *e){
    unsigned long startindex=0;
    for (unsigned long  j=0; j<row.Number;j++){
   	unsigned long c=row(j).Column;
	BlockMatrix::Element *e3=BlockRow[r].getColumn(c,startindex);
	if (e3)(*e3)().subtractProduct((*e)(),row(j)());
	else{
	  Cure::Matrix m;
	  m.multiply_((*e)(),row(j)());
	  m.minus();
	  setElement(m,r,c);		
	}
      }
  }
  void addColProductTrans_(unsigned long c,
			 BlockMatrix::Column & col, 
			 BlockMatrix::Element *e,int which){
    unsigned long startindex=0;
    for (unsigned long  j=0; j<col.Number;j++){
      unsigned long r;
      if (which&1) r=col(j).Column;
      else r=col(j).Row;
      BlockMatrix::Element *e3=BlockColumn[c].getRow(r,startindex);
      Cure::Matrix m;
      m.multTranspose_(col(j)(),(*e)(),which);
      if (e3)(*e3)()+=m;
      else setElement(m,r,c);		
    }
  }
  void subtractColProductTrans_(unsigned long c,
			 BlockMatrix::Column & col, 
			 BlockMatrix::Element *e,int which){
    unsigned long startindex=0;
    for (unsigned long  j=0; j<col.Number;j++){
      unsigned long r;
      if (which&1) r=col(j).Column;
      else r=col(j).Row;
      BlockMatrix::Element *e3=BlockColumn[c].getRow(r,startindex);
      Cure::Matrix m;
      m.multTranspose_(col(j)(),(*e)(),which);
      if (e3)(*e3)()-=m;
      else {
	m.minus();
	setElement(m,r,c);		
      }
    }
  }

  /**
   * This is called by the object to allocated an element.
   * Here we know that r,c is empty and just put a free
   * element into that position.
   */
  BlockMatrix::Element *setElement(const Cure::Matrix &m,
				   const unsigned long r,
				   const unsigned long c){
    BlockMatrix::Element *e=new BlockMatrix::Element(m,r,c);
    BlockRow[r].addToRow(e, m.Rows);
    BlockColumn[c].addToColumn(e, m.Columns);
    return e;
  }
  /**
   * Here we know that r,c is empty and just put a free
   * element int that position.
   */
  BlockMatrix::Element *setElementTranspose(const Cure::Matrix &m,
					    const unsigned long r,
					    const unsigned long c){
    BlockMatrix::Element *e=new BlockMatrix::Element(m,r,c, true);
    BlockRow[r].addToRow(e, m.Columns);
    BlockColumn[c].addToColumn(e, m.Rows);
    return e;
  }		
  
  BlockMatrix::Element * setMatrix(unsigned long r,
				    unsigned short height,
				    unsigned long c,
				    unsigned short width){
    BlockMatrix::Element *e=new BlockMatrix::Element();
    e->setMatrix(r,height,c,width);
    BlockRow[r].addToRow(e, height);
    BlockColumn[c].addToColumn(e, width);
   return e;
  }
  BlockMatrix::Element * setMatrix(unsigned long r,
				    unsigned long c){
    BlockMatrix::Element *e=new BlockMatrix::Element();
    e->setMatrix(r,BlockRow[r].Width,c,BlockColumn[c].Width);
    BlockRow[r].addToRow(e);
    BlockColumn[c].addToColumn(e);
    return e;
  }
  
};
    inline void Cure::BlockMatrix::Column::add
    (BlockMatrix::Element *p, short w){
      if (w>-1)Width=w;
      if (Number==AllocatedNumber)
	{
	  BlockMatrix::Element **e=Index;
	  if (AllocatedNumber==0)e=0;
	  AllocatedNumber+=5;
	  
	  Index=new BlockMatrix::Element*[AllocatedNumber];
	  if (e){
	    if (Number>0)
	      memcpy(Index,e,sizeof(BlockMatrix::Element*)*Number);
	    delete []e;
	  }
	}
      Index[Number]=p;
      Number++;
    }

    inline void Cure::BlockMatrix::Column::addToRow
    (BlockMatrix::Element *p, short w){
      if (w>-1)Width=w;
      if (Number==AllocatedNumber)
	{
	  BlockMatrix::Element **e=Index;
	  if (AllocatedNumber==0)e=0;
	  AllocatedNumber+=5;
	  Index=new BlockMatrix::Element*[AllocatedNumber];
	  if (e){
	    if (Number>0)
	      memcpy(Index,e,sizeof(BlockMatrix::Element*)*Number);
	    delete []e;
	  }
	}
      for (unsigned long i=0;i<Number;i++)
	{
	  if (Index[i]->Column>p->Column)
	    {
	      if (Number>i)
		memmove(&Index[i+1],&Index[i],sizeof(BlockMatrix::Element*)*
		      (Number-i));
	      Index[i]=p;
	      Number++;
	      return;
	    }
	}
      Index[Number]=p;
      Number++;
    }
  inline void Cure::BlockMatrix::Column::addToColumn
  (BlockMatrix::Element *p, short w){
    if (w>-1)Width=w;
    if (Number==AllocatedNumber)
      {
	BlockMatrix::Element **e=Index;
	if (AllocatedNumber==0)e=0;
	  AllocatedNumber+=3;
	  Index=new BlockMatrix::Element*[AllocatedNumber];
	  if (e){
	    if (Number>0)
	      memcpy(Index,e,sizeof(BlockMatrix::Element*)*Number);
	    delete []e;
	  }
	}
      for (unsigned long i=0;i<Number;i++)
	{
	  if (Index[i]->Row>p->Row)
	    {
	    if (Number>i)
	      memmove(&Index[i+1],&Index[i],sizeof(BlockMatrix::Element*)*
		      (Number-i));
	      Index[i]=p;
	      Number++;
	      return;
	    }
	}
      Index[Number]=p;
      Number++;
    }
      inline void Cure::BlockMatrix::Column::remove
      (const BlockMatrix::Element *p){
	for (unsigned long i=0;i<Number;i++)
	  {
	    if (Index[i]==p){
	      int n=Number-i-1;
	      if (n>0)
		memmove(&Index[i],&Index[i+1],sizeof(BlockMatrix::Element*)*n);
	      Number--;
	      return;
	  }
	  }
      }

    inline BlockMatrix::Element * Cure::BlockMatrix::Column::getColumn
    (unsigned long c,unsigned long &startindex)
    {
      while(startindex<Number){
	if (Index[startindex]->Column>=c){
	  if(Index[startindex]->Column==c){
	    return Index[startindex];
	  }
	  else{ 
	    if (startindex>0)startindex--;
	    return 0;
	  }
	}
	startindex++;
      }
      return 0;      
    }

  inline BlockMatrix::Element * Cure::BlockMatrix::Column::getRow
  (unsigned long r,unsigned long &startindex)
  {
    while(startindex<Number){
      if (Index[startindex]->Row>=r){
	  if(Index[startindex]->Row==r){
	    return Index[startindex];
	  }
	  else {
	  if (startindex>0)startindex--;
	  return 0;
	  }
      }
      startindex++;
    }
      if (startindex>0)startindex--;
      return 0;
      
  }

  inline void BlockMatrix::reallocate(unsigned long r,unsigned long c){
    if ((r!=AllocatedRows)||(c!=AllocatedColumns)){
      zero();
      if (r!=AllocatedRows){
	if (AllocatedRows)
	  if (BlockRow)delete[]BlockRow;
	AllocatedRows=r;
	if (r)
	  BlockRow=new BlockMatrix::Column[AllocatedRows];
	else BlockRow=0;
      }
      if (c!=AllocatedColumns){
	if (AllocatedColumns)
	  if (BlockColumn)delete[]BlockColumn;
	AllocatedColumns=c;
	if (c)
	  BlockColumn=new BlockMatrix::Column[AllocatedColumns];
	else BlockColumn=0;
      }
    }
    Rows=r;
    Columns=c;
  }
  inline void BlockMatrix::operator =(Cure::BlockMatrix &bm){
    zero();
    reallocate(bm.Rows,bm.Columns);
    copyColumnWidths(bm);
    copyRowHeights(bm);
    if (Rows<Columns){
      for (unsigned long r=0; r<Rows; r++){
	for (unsigned long i=0;i<bm.BlockRow[r].Number;i++){
	  BlockMatrix::Element *e=&bm.BlockRow[r](i);
	  setElement((*e)(),r,e->Column);	
	}	
      }	    
    }else{
      for (unsigned long c=0; c<Columns; c++){
	for (unsigned long j=0;j<bm.BlockColumn[c].Number;j++){
	  BlockMatrix::Element *e=&bm.BlockColumn[c](j);
	  setElement((*e)(),e->Row,c);		
	}
      }
    }
  }
  inline void BlockMatrix::transpose(const BlockMatrix &bm){
    zero();
    reallocate(bm.Columns,bm.Rows);
    copyColumnWidthsToRowHeights(bm);
    copyRowHeightsToColumnWidths(bm);
    if (Rows<Columns){
      for (unsigned long r=0; r<Rows; r++){
	for (unsigned long i=0;i<bm.BlockColumn[r].Number;i++){
	  BlockMatrix::Element *e=&bm.BlockColumn[r](i);
	  setElementTranspose((*e)(),r,e->Row);		
	}	    
      }
	
    }else{
      for (unsigned long c=0; c<Columns; c++){
	for (unsigned long j=0;j<bm.BlockRow[c].Number;j++){
	  BlockMatrix::Element *e=&bm.BlockRow[c](j);
	  setElementTranspose((*e)(),e->Column,c);		
	}
      }
    }
  }
  inline double BlockMatrix::getValue(unsigned long r,
		unsigned long c)const{
    if ((Rows==0)||(Columns==0))return 0;
    unsigned long i=0;
    while (r>=BlockRow[i].Width){
      r-=BlockRow[i].Width;
      i++;
      if (i>=Rows)return 0;
    }		  
    unsigned long j=0;
    while (c>=BlockColumn[j].Width){
      c-=BlockColumn[j].Width;
      j++;
      if (j>=Columns)return 0;
    }		  
    BlockMatrix::Element * e=(*this)(i,j);
     if (!e)
      return 0;
    return (*e)()(r,c);
  }
  inline void BlockMatrix::setValue(const double d,unsigned long r,
				    unsigned long c){
    if ((Rows==0)||(Columns==0))return;
    unsigned long i=0;
    while (r>=BlockRow[i].Width){
      r-=BlockRow[i].Width;
      i++;
      if (i>=Rows)return;
    }		  
    unsigned long j=0;
    while (c>=BlockColumn[j].Width){
      c-=BlockColumn[j].Width;
      j++;
      if (j>=Columns)return;
    }		  
    BlockMatrix::Element * e=(*this)(i,j);
    if (!e) e=setMatrix(i,j);
    if (e)(*e)()(r,c)=d;
  }

  void invertPositiveDef(Cure::Matrix &m,double minEigen=20);  
  
}
#endif

