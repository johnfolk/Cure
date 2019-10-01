// = AUTHOR(S)
//    John Folkesson
//    
// = COPYRIGHT
//    Copyright (c) 2009 John Folkesson
//    
#ifndef CURE_SPARSEMATRIX_HH
#define CURE_SPARSEMATRIX_HH
#include <iostream>
#include <fstream>
#include <string.h>  // memmove
#include "Matrix.hh"
#include "BinaryMatrix.hh"
#include "LongArray.hh"
namespace Cure {

#define CURESPARSEMATRIXHHDEBUG 200

  /**
   * @author John Folkesson 
   */
  class SparseMatrix
  {  
  public:
    /**
     * The number of rows in the sparsematrix.  
     *  Notice that there is an implied 
     * size limit here.
     */
    long m_Rows;
    /**
     * The number of columns in the sparsematrix.  
     *
     */
    long m_Columns;  
    /**
     * The number of 1K double  blocks in the memory allocation.
     */
    long  m_NumberOfElements;  
    /**
     * The pointer to the array of 1K double array pointers.
     */
    double **m_Elements;
 
    /**
     * Each row of rows lists the index of the rows elements in the
     * m_Elements allocation.  the format of the row is
     * (col1,index1,col2,index2,...,colN,indexN) for a row with N none
     * zero elements.
     * 
     */  
    LongArray  m_Row;
    /**
     * The same as Row but for columns
     * 
     */
    LongArray m_Column;
    unsigned long m_NextIndex;
  protected:
    /**
     * A single row of flags.
     * This keeps track of which m_Elements[i] pointers point to
     * something that must eventually be deleted.  That is some can be
     * part of a larger allocation of many blocks at once.  
     */
    BinaryMatrix m_Allocated;
    /**
     * These temp varibles make some inline calls faster by avoiding
     * malloc and free.
     */
    long m_TempL,m_TempL2;;
    short m_TempS;
    unsigned short m_TempUS;
    unsigned long m_TempUL;
  public:
    bool getLocation(unsigned long &block,unsigned short &i,
		     const unsigned long index)
    { 
      block=(index>>10);
      if (block>=(unsigned long)m_NumberOfElements)return false;
      i=(index&0x3FF);
      return true;
    }
    double getValue(const unsigned long index){
      if(getLocation(m_TempUL,m_TempUS,index))
	return m_Elements[m_TempUL][m_TempUS];
      return 0;
    }
    double *getPointer(const unsigned long index){
      if(getLocation(m_TempUL,m_TempUS,index))
	return &m_Elements[m_TempUL][m_TempUS];
      return 0;
    }

    /**
     * This gets a copy of a sparsematrix element at (r,c).
     * @param r the row of the element
     * @param c the column.
     */
    double operator() (const unsigned long r, const unsigned long c){ 
      double *p=getPointer(r,c);
      if (p)
	return (*p);
      else return 0;
    }
    /**
     * This gets a reference to a sparsematrix element at (r,c).
     * @param r the row of the element
     * @param c the column.
     */
    double* getPointer(const unsigned long r, const unsigned long c){
      if ((r>=(unsigned long)m_Rows)||(c>=(unsigned long)m_Columns)){
	std::cerr<<"SparseMatrix::operator()(r,c) out of bounds. "
		 <<r<<" "<<c<<" "<<m_Rows<<" "<<m_Columns<<"\n";
	return 0;
      }
      if (c<r){
	m_TempL=m_Row.findPairOrdered(r,c);
	if (m_TempL==-1)return 0;
	if (getLocation(m_TempUL,m_TempUS,m_Row(r,m_TempL+1))){
	  return &m_Elements[m_TempUL][m_TempUS];
	}
      } else {
	m_TempL=m_Column.findPairOrdered(c,r);
	if (m_TempL==-1)return 0;
	if (getLocation(m_TempUL,m_TempUS,m_Column(r,m_TempL+1))){
	  return &m_Elements[m_TempUL][m_TempUS];
	}
      }
      return 0;
    }
    /** 
     * This will grow the matrix automatically if r and c are out of bounds.
     */
    void addValue(const unsigned long r,const unsigned long c,const double val)
    {
      double *p=getPointer(r,c);
      if (p){
	(*p)+=val;
	return;
      }
      makeElement(r,c,val);
    }
    /** 
     * This will grow the matrix automatically if r and c are out of bounds.
     */
    double & getElement(const unsigned long r,const unsigned long c)
    {
      double *p=getPointer(r,c);
      if (p){
	return *p;
      }
      return makeElement(r,c);
    }
    /** 
     * This will grow the matrix automatically if r and c are out of bounds.
     */
    void setValue(const unsigned long r, const unsigned long c, double val)
    {
      double *p=getPointer(r,c);
      if (p){
	(*p)=val;
	return;
      }
      if (val!=0)
	makeElement(r,c,val);
    }
    void clear(){
      m_NextIndex=0;
      m_Row.clear();
      m_Column.clear();
    }
    /**
     * This should only be called if the matrix is becoming badly
     * disorganized with many small or zeron values or widely
     * scattered memory allocation.  This rellocates the array as a
     * single block of memory, removes all small values below
     * threshold in magnitude, and puts values into the allocation in
     * row order.
     */
    void reorder(double threshold=0);
    /**
     * 
     */
    void setToZero(const unsigned long r, const unsigned long c){
      if ((r>=(unsigned long)m_Rows)||(c>=(unsigned long)m_Columns)){
	return;
      }
      m_TempL2=m_Row.findPairOrdered(r,c);
      if (m_TempL2==-1)return;
      m_TempUL=m_Row(r,m_TempL2+1);
      if (!getLocation(m_TempUL,m_TempUS,m_TempUL))return; 
      m_Row.removeCell(r,m_TempL2);
      m_Row.removeCell(r,m_TempL2);
      m_TempL2=m_Column.findPairOrdered(c,r);
      m_Column.removeCell(c,m_TempL2);
      m_Column.removeCell(c,m_TempL2);
    }
    /**
     * This moves all elements to keep the non-zero elements in the
     * begining of the allocated arrays.  This is rather complex you
     * can call setValue(r,c,0) will be effectively the same but keep
     * the allocation for the zero element.
     */
    void clearValue(const unsigned long r, const unsigned long c){
      if ((r>=(unsigned long)m_Rows)||(c>=(unsigned long)m_Columns)){
	std::cerr<<"SparseMatrix::clearValue(r,c) out of bounds. "
		 <<r<<" "<<c<<" "<<m_Rows<<" "<<m_Columns<<"\n";
	return;
      }
      m_TempL2=m_Row.findPairOrdered(r,c);
      if (m_TempL2==-1)return;
      m_TempL=m_Row(r,m_TempL2+1);
      if (!getLocation(m_TempUL,m_TempUS,m_TempL))return; 
      m_Row.removeCell(r,m_TempL2);
      m_Row.removeCell(r,m_TempL2);
      m_TempL2=m_Column.findPairOrdered(c,r);
      m_Column.removeCell(c,m_TempL2);
      m_Column.removeCell(c,m_TempL2);
      m_NextIndex--;
      unsigned long top=((m_NextIndex)>>10);
      unsigned short topi=(m_NextIndex&&0x3FF);
      short n=(1023-m_TempS);
      if (m_TempUL==top)n=topi-m_TempUS;
      if (n>0)
	memmove(&m_Elements[m_TempUL][m_TempUS],
		&m_Elements[m_TempUL][m_TempUS+1],
		n*sizeof(double));
      if (m_TempUL<top)
	m_Elements[m_TempUL][1023]=m_Elements[m_TempUL+1][0];
      m_TempUL++;
      while (m_TempUL<top){
	memmove(&m_Elements[m_TempUL][0],&m_Elements[m_TempUL][1],
		  1023*sizeof(double));
	m_Elements[m_TempUL][1023]=m_Elements[m_TempUL+1][0];
	m_TempUL++;
      }
      if ((m_TempUL==top)&&(topi>0))
	memmove(&m_Elements[m_TempUL][0],&m_Elements[m_TempUL][1],
		topi*sizeof(double));
     
      for (long j=0;j<m_Rows;j++){
	long c=m_Row.columns(j);
	for (int i=0;i<c;i+=2){
	  if (m_Row(j,i+1)>m_TempL){
	    m_Row(j,i+1)--;
	    m_TempL2=m_Column.findPairOrdered(m_Row(j,i),j);
	    m_Column(m_Row(j,i),m_TempL2+1)--;
	  }
	}
      }
    }
    void makeElement(const unsigned long r,const unsigned long c,
		     const double val)
    {
      makeElement(r,c)=val;
    }
    double & makeElement(const unsigned long r,const unsigned long c)
    {
      if ((r>=(unsigned long)m_Rows)||(c>=(unsigned long)m_Columns)){
#if CURESPARSEMATRIXHHDEBUG >200
	std::cerr<<"SparseMatrix::makeElement(r,c) out of bounds.  "<<
	  "Auto call to grow. r="
		 <<r<<" c="<<c<<" m_Rows="<<m_Rows<<
	  " m_Columns="<<m_Columns<<"\n";
#endif
	if (c<(unsigned long)m_Columns)grow(r,m_Columns);
	else if (r<(unsigned long)m_Rows)grow(m_Rows,c);
	else grow(r,c);
      }
      m_TempL=(m_NextIndex>>10);
      if (m_TempL==m_NumberOfElements){
	double **d=m_Elements;
	m_Elements=new double*[m_NumberOfElements+1];
	if (d){
	  if (m_NumberOfElements>0)
	    memcpy(m_Elements,d,m_NumberOfElements*sizeof(double*));
	  delete []d;
	}
	m_Elements[m_NumberOfElements]=new double[1024];
	m_Allocated.grow(1,m_NumberOfElements+1);
	m_Allocated.setBit(0,m_NumberOfElements,true);
	m_NumberOfElements++;
      }
      m_TempS=(m_NextIndex&0x3FF);
      m_Row.addPairOrdered(r,c,m_NextIndex);
      m_Column.addPairOrdered(c,r,m_NextIndex);
      m_NextIndex++;
      return m_Elements[m_TempL][m_TempS];
    }
   
    /**
     * Create a sparsematrix with 0 rows and columns.
     */
    SparseMatrix();
    /**
     * Create a sparsematrix that allocates its own memory and is equal to the 
     * currently defined sparsematrix b.  (ie. it can be that b has been
     * set to refer to only a sub-sparsematrix of its full allocated array.
     * In that case only the sub-sparsematrix will be allocated in the new sparsematrix.)
     * @param b a sparsematrix to be used as the initial values.   
     */
    SparseMatrix(const SparseMatrix& b);
  
    /**
     * This will allocate its own array (default is c=r).
     * All initial values are set to 0.
     * @param r the number of rows to allocate.
     * @param c the number of columns (default c=r).
     */ 
    SparseMatrix(const int r,const int c=-1);
  
    virtual ~SparseMatrix();
    /**
     * The = operator allocates its own memory if needed and sets it
     * equal to the currently defined sparsematrix b.  (ie. it can be that
     * b has been set to refer to only a subsparsematrix of its full
     * allocated array.  In that case only the sub-sparsematrix will be
     * allocated in the new sparsematrix.)  
     *
     * @param b a sparsematrix to be used as the initial values.
     */
    void operator = (const SparseMatrix& b);
    /**
     * This set the sparsematrix to be all zero except for d down the diagonal
     * starting from the upper left corner (ie (*this)(i,i)=d).
     * If d is 0 this just sets the sparsematrix to 0.
     *   @param d the value fro the diagonal elements.
     */  
    void operator = (const double d);
    /**
     * Adds b to this sparsematrix and returns the result in this sparsematrix.
     * If the size of b is wrong a sparsematrix nothing is changed and an
     * error message printed.
     *
     * @param b the sparsematrix to add to this.
     * @see SparseMatrix::add_
     */
    void operator += ( SparseMatrix & b);
    /**
     * Subtract b from this sparsematrix and returns the result in this
     * sparsematrix.  If the size of b is wrong a sparsematrix nothing is changed
     * and an error message printed.
     *
     * @param b the sparsematrix to subtract from this.
     * @see SparseMatrix::subtract_  
     */
    void operator -= (SparseMatrix& b);
    /**
     * Multiply all elements  by d returns the result in
     * this.  
     *
     * @param d the number to multiply  this by.
     */
    void operator *= (const double d)
    {
      m_TempL=m_NextIndex;
      m_TempL=(m_TempL>>10);
      for (m_TempL2=0; m_TempL2<m_TempL; m_TempL2++)
	for (m_TempS=0;m_TempS<1024;m_TempS++)
	  m_Elements[m_TempL2][m_TempS]*=d;
      m_TempL2=(m_NextIndex&0x3FF);
      for (m_TempS=0;m_TempS<m_TempL2;m_TempS++)
	m_Elements[m_TempL][m_TempS]*=d;
    }
    /**
     * Divide all elements  by d returns the result in
     * this.  
     *
     * @param d the number to divide  this by.
     */
    void operator /= (const double d){
      if (d==0) {
#if CURESPARSEMATRIXDEBUG >10
	cerr << "Tried to divide SparseMatrix::operator/= 0\n";
#endif
	return; 
      }
      (*this)*=(1/d);
    }
    /**
     * This sparsematrix becomes equal to the element by element 
     * product between this sparsematrix and b.  This is like .* in MATLAB.
     *  
     * @param b any sparsematrix
     */
    void arrayMult( SparseMatrix& b);
    /**
     * This sparsematrix becomes equal to the element by element ratio
     * *this(i,j)=*this(i,j)/b(i,j).  This is like ./ in MATLAB.
     *  
     * @param b any sparsematrix
     */
    void arrayDivide( SparseMatrix& b);
    /**
     * @return sum of elements squared.
     */
    double sumSquares(){
      if (m_NextIndex==0)return 0;
      m_TempL=(m_NextIndex);
      m_TempL=(m_TempL>>10);
      double d=0;
      for (m_TempL2=0;m_TempL2<m_TempL;m_TempL2++)
	for (m_TempS=0;m_TempS<1024;m_TempS++)
	  d+=(m_Elements[m_TempL2][m_TempS]*m_Elements[m_TempL2][m_TempS]);
      m_TempL2=(m_TempL&0x3FF);
      for (m_TempS=0;m_TempS<m_TempL2;m_TempS++)
	d+=(m_Elements[m_TempL][m_TempS]*m_Elements[m_TempL][m_TempS]);
      return d;
    }
    /**
     * This sparsematrix is transposed.
     * @see SparseMatrix::transpose_;
     */
    void transpose(void);

    /**
     * This sparsematrix becomes equal to the transpose of a.
     * @param a the sparsematrix.  
     */
    void transpose(const SparseMatrix a);

    /**
     * This sparsematrix becomes equal to the transpose of a.  
     *   
     * @param a Here a should not be this sparsematrix.
     */
    void transpose_(const SparseMatrix & a);



    /**
     * This sparsematrix becomes equal to the minus of a.  
     *   
     * @param a the arg
     */
    void minus(const SparseMatrix & a)
    {
      (*this)=a;
      minus();
    }

    /**
     * This sparsematrix is multiplied by -1.
     */
    void minus(){ 
      m_TempL=m_NextIndex;
      m_TempL=(m_TempL>>10);
      for (m_TempL2=0; m_TempL2<m_TempL; m_TempL2++)
	for (m_TempS=0;m_TempS<1024;m_TempS++)
	  m_Elements[m_TempL2][m_TempS]=-m_Elements[m_TempL2][m_TempS];
      m_TempL2=(m_NextIndex&0x3FF);
      for (m_TempS=0;m_TempS<m_TempL2;m_TempS++)
	m_Elements[m_TempL][m_TempS]=-m_Elements[m_TempL][m_TempS];
    }

    /**
     * This sparsematrix is set to equal its inverse.
     * @return 0 if ok else >0;
     */
    int invert(Cure::Matrix &inv);

    /**
     * Set this = (this + transpose(this))/2
     */
    void symetrize();
    /**
     * The reallocate method allows you to change the array memory
     * allocation.  If the m_Rows=r and m_Columns are 'correct' this
     * method does nothing.  It does not copy any values.  The default
     * (if you only specify r), is m_Columns=m_Rows.  If a new memory
     * allocation is needed all values will be set to zero.
     *
     * @param r the number of rows.
     * @param c the number of coulmns (default c=r).
     * @see SparseMatrix::grow, @see SparseMatrix::offset, @see SparseMatrix::reset  
     */
    void reallocate(const int r,const int c=-1 );
    /**
     * Same as reallocate except it sets to 0 for sure
     */
    void  reallocateZero(const int r,const int c=-1);
    /**
     * The grow method allows you to extend the array memory beyond the 
     * originally allocation, or even copy it to a smaller
     * allocation freeing memory. 
     * 
     * It allocates a new array and copies the sparsematrix elements from the 
     * original allocation to the new one in the approriate
     * row and column (staring from (0,0) being the same)
     * The default (if you only specify r), is m_Columns=m_Rows.
     *   
     * @param r the number of rows for the extended sparsematrix.
     * @param c the number of columns for the extended sparsematrix,default is c=r.
     * @see SparseMatrix::reallocate, @see SparseMatrix::offset, @see SparseMatrix::reset  
     */
    void grow(const int r,const int c=-1);
    /**
     * Copy the values in rows>r up one row and then subtract 1 from
     * m_Rows. 
     * @see SparseMatrix::grow, @see SparseMatrix::offset, @see SparseMatrix::reset  
     */
    void deleteRow(long r);
    void deleteRows(long r, long num);
    /**
     * Copy the values in columns>c to the column to the left and then 
     * subtract 1 from m_Columns.
     * @see SparseMatrix::grow, @see SparseMatrix::offset, @see SparseMatrix::reset  
     */
    void deleteColumn(long c);
    /**
     * Moves the data to the left and changes the size of the sparsematrix
     * @param startcolumn the first column to delete 
     * @param numberofcolumns the total number of columns to delete 
     *
     * @see SparseMatrix::grow, @see SparseMatrix::offset, @see SparseMatrix::reset  
     */
    void deleteColumns(long startcolumn, long numberofcolumns );
    /**
     * This actually creates a new double array and deletes the old
     * one.  This makes this rather heavy and not to be called
     * without really needing this.
     * Moves the data to the right and changes the size of the sparsematrix.
     * the inserted columns have 0's set.
     * @param startcolumn the place to insert column to delete 
     * @param numberofcolumns the total number of columns to insert 
     *
     * @see SparseMatrix::grow, @see SparseMatrix::offset, @see SparseMatrix::reset  
     */
    void insertColumns(long startcolumn, long numberofcolumns);
    void insertColumn(long startcolumn);
    void insertRow(long startRow);
    void insertRows(long startRow, long numberofrows);
    /**
     * Copy the values from row r1 to r2 and visa versa.
     * @param r1 a row of the sparsematrix.
     * @param r2 another row.
     * @see SparseMatrix::grow, @see SparseMatrix::offset, @see SparseMatrix::reset  
     */
    void swapRows(long r1,long r2);
    /**
     * Copy the values from column c1 to c2 and visa versa.
     * @param c1 a column of the sparsematrix.
     * @param c2 another column.
     */
    void swapColumns(long c1,long c2);
    /**
     * Moves num rows from r1 to r2.
     * @return 0 if ok else 1;
     */
    int moveRows(unsigned long r1,unsigned long r2, 
			 unsigned short num=1);
    /**
     * Moves num columns from c1 to c2. It slides the columns  in between
     * over towards c1 to make room.
     * 
     * @return 0 if ok else 1;
     */
    int moveColumns(unsigned long c1,unsigned long c2, 
			    unsigned short num=1);
      /**
     * Creates a file and saves the sparsematrix in a format that MATLAB can load.
     * @param filename the name of the file to create.
     */
    void save(const char *filename);
    /** 
     * This writes the sparsematrix to a file in a format that is useful for
     * saving of complex objects that contain matrices.  The format is
     *
     * m_Rows m_Columns
     * First row of ellements
     * Second row ...
     * @param fs an open for write file stream.
     * @see SparseMatrix::read, @see SparseMatrix::save
     */
    void   write(std::fstream &fs );
    /** 
     * This reads the sparsematrix from a file in a format that is useful for
     * loading of complex objects that contain matrices.  The format is
     *
     * m_Rows m_Columns
     * First row of ellements
     * Second row ...
     * @param fs an open for read file stream.
     * @see SparseMatrix::write,@see SparseMatrix::save
     */
    void   read(std::fstream &fs );
    /**
     * Calculate the determinate.  
     * 
     * @return the determinate
      */
    double determinant( void );
    /**
     * Calculate the trace.
     * @return the sum of the diagonal.
     */
    double trace();
    void normalize();
    /**
     * A helper method for triangle but can be used allone if sparsematrix
     * is already in hessenberg form.  This applys the final rotation
     * to bring a Hessenberg sparsematrix into upper triangular form.
     *
     * Does This=QR
     * This becomes uppertriangle R.
     * This makes no requirement on the m_Rows and m_Columns but assumes that
     * this is hessenberg form when called.
     * 
     * @param q the orthonormal matrix Q that transforms R back to original 
     *  is multiplied by the initial q matrix like this q = q Q^T
     * @param startcol the if all rows/columns above startrow are known
     * to be upper-triangular then this hint will skip those columns
     */
    void doQR(Matrix & q, long *permute, 
	      long startcol=0);
    void doQR(Matrix & q, long startcol=0);
    void doQR();
    /**
     * Does permuted(This)=QR This becomes a column permuted version of an
     * uppertriangle R.  This makes no requirement on the m_Rows and
     * m_Columns.
     * 
     * So at the end R=(*this)*P where P is a desired permutation of
     * the columns that is input by the permute parameter.(If q
     * initialially is the m_Rows x m_Rows Identity sparsematrix then q at retun
     * is q=Q^T).
     * 
     * Does This*P=QR decomposition.
     * This becomes uppertriangle R*P^-1.
     * triangle(...) makes no requirement on the m_Rows and m_Columns.
     * 
     * @param q the orthonormal sparsematrix Q is multiplied by the initial
     * q sparsematrix like this, q = q Q^T.
     *
     * @param permute input the column permutation, so permute[j]
     * gives the column of this sparsematrix that will be the jth
     * column of the trianglur sparsematrix R.  The array is assumed
     * to be as long as m_Columns.  So to get the (i,j) element of R
     * one does (*this)(i,permute[j])
     *
     * @param startcol assumes that all rows/columns above startrow
     * are known to be upper-triangular.  This hint will allow skiping
     * those columns.  Note this is a column of the permuted sparsematrix R
     * (or (*this)*P) not this sparsematrix.
     */
    void triangle(Matrix & q,long *permute, 
		  long startcol=0);
      
    void triangle(Matrix & q,long startcol=0);
    void triangle();
      

    /**
     * (this)x=rhs^T
     * Does the back substitution back to and including the block 'stopblk'.
     * Assums this to be upper triangle form
     *
     * It simple aborts
     * if the answer x has a value > 1/oneovermaxx.  It then returns the
     * last row that was solvable.
     *
     * Assums this to be upper triangle form so that (*this)(i,j)=0
     * for i>j.  One must have columns<=m_Rows or this will fail.
     *
     * (this)x=rhs^T
     * Does the back substitution back to and including the block 'stopblk'.
     *
     *
     *
     *
     *
     * @param x answer is returned here with dim g^T
     *
     * @param rhs can be a row vector  right hand side
     * 
     * @param stoprow the backsubstituion stops after
     * solving this row of x.  
     *
     * @param oneovermaxx will abort x if |x(i,j)|>1/oneovermaxx==maxx 
     *  for some element of x. maxx the largest x element that you
     *  want to allow.  
     *
     * @param rows the array that gives the non-zero columns on each
     * row of this.
     * 
     *
     * @param permute the columns of this are permuted to give the
     * upper triangle sparsematrix R as R(i,j)=(*this)(i,permute[j]), x rows
     * are similarly permuted.
     *
     * @return the highest row of P^-1 x that was solved for. So return
     * =stopcol indicates that the equation was completly solved with
     * no numerical problems and return=m_Columns indicates nothing was solved.
     *
     *
     */
    unsigned long
    tryBackSubstitute(Matrix &x, 
		      const Matrix &rhs,
		      double oneovermaxx,
		      unsigned long stoprow);

    unsigned long
    tryBackSubstitute(Matrix &x, 
		      const Matrix &rhs,
		      double oneovermaxx,
		      unsigned long stoprow, 
		      long *permute);
    unsigned long
    tryBackSubstitute(SparseMatrix &x, 
		      SparseMatrix &rhs,
		      double oneovermaxx,
		      unsigned long stoprow, 
		      long *permute);

    /**
     * Add two matricies and put the result in this sparsematrix.
     * this= m1+m2.
     * @param m1 any sparsematrix
     * @param m2 any sparsematrix
     */
    void add(SparseMatrix m1,SparseMatrix m2){
      add_(m1,m2);
    }
    /**
     * Efficiently add two matricies and put the result in this sparsematrix.
     * This is the better way to do it than, *this= m1+m2.  
     * @param m1 any sparsematrix but not this.
     * @param m2 any sparsematrix but not this.
     */
    void add_(SparseMatrix& m1,SparseMatrix& m2);
    /**
     * Subtract two matricies and put the result in this sparsematrix.
     * this= m1-m2.
     * @param m1 any sparsematrix
     * @param m2 any sparsematrix
     */
    void subtract( SparseMatrix m1, SparseMatrix m2){
      subtract_(m1,m2);
    }      
    /**
     * Subtract two matricies and put the result in this sparsematrix.
     * This is the better way to do it than,*this= m1-m2.
     * @param m1 any sparsematrix but not this.
     * @param m2 any sparsematrix but not this.
     */
    void subtract_( SparseMatrix& m1, SparseMatrix &m2);      
    /**
     * This sparsematrix becomes equal to the product of m1*m2, changing size
     * if needed, (no need to call reallocate to set m_Rows and m_Columns
     * correctly before calling this).   Of course, m1 and m2 must be
     * compatable with sparsematrix multply.
     *  
     * @param m1 any sparsematrix
     * @param m2 any sparsematrix
     */
    void multiply( SparseMatrix m1, SparseMatrix m2){
      multiply_(m1,m2);
    }  
    /**
     * This sparsematrix becomes equal to the product of m1*m2, changing size
     * if needed, (no need to call reallocate to set m_Rows and m_Columns
     * correctly before calling this).  Of course, m1 and m2 must be
     * compatable with sparsematrix multply.
     *
     * This is the better way to do it than,*this= m1*m2.
     * @param m1 any sparsematrix but not this.
     * @param m2 any sparsematrix but not this.
     */
    void multiply_( SparseMatrix& m1, SparseMatrix& m2){
      clear();
      addProduct_(m1,m2);
    }

	 
    /**
     * This method allows transposing the arguments before multiplying them.
     * This sparsematrix =  (tranpose) a * (transpose)b, changing
     * size if needed, (no need to call reallocate to set m_Rows and 
     * m_Columns correctly before calling this).
     *
     * Where which  
     * @param which tells which sparsematrix, (a or b)  to transpose:
     *              1 transpose a, 
     *              2 transpose b
     *              3 both
     * @param a any sparsematrix.
     * @param b any sparsematrix.
     */
    void multTranspose( SparseMatrix a, SparseMatrix b, int which){
      multTranspose_(a,b,which);
    }
    /**
     * This method allows transposing the arguments before multiplying them.
     * This sparsematrix =  (tranpose) a * (transpose)b, changing
     * size if needed, (no need to call reallocate to set m_Rows and 
     * m_Columns correctly before calling this).
     *
     * This is a whole lot more effiecient than for instance
     * c.transpose(b);
     * this=a*c;
     *
     * Where which  
     * @param which tells which sparsematrix, (a or b)  to transpose:
     *              1 transpose a, 
     *              2 transpose b
     *              3 both
     * @param a any sparsematrix.
     * @param b any sparsematrix.
     */
    void multTranspose_( SparseMatrix& a, SparseMatrix& b, int which);
    /**
     * This is like multiply but puts in a minus sign.
     * This = -(a * b), changing
     * size if needed, (no need to call reallocate to set m_Rows and 
     * m_Columns correctly before calling this).
     *
     * @param a any sparsematrix
     * @param b any sparsematrix
     */
    void minusMult( SparseMatrix a, SparseMatrix b){
      minusMult(a,b);
    }
    /**
     * This is like multiply but puts in a minus sign.
     * Much better than *this = -(a * b). It changes
     * size if needed, (no need to call reallocate to set m_Rows and 
     * m_Columns correctly before calling this).
     *
     * @param a any sparsematrix but not this.
     * @param b any sparsematrix but not this.
     */
    void minusMult_( SparseMatrix& a, SparseMatrix& b){
      clear();
      subtractProduct(a,b);
    }
    /**
     * Adds the product of two matricies to this sparsematrix.
     * This += a * b.
     *
     * @param a any sparsematrix
     * @param b any sparsematrix
     */
    void addProduct( SparseMatrix a,  SparseMatrix b){
      addProduct_(a, b);
    }
    /**
     * Adds the product of two matricies to this sparsematrix.
     * Better than: *this += a * b.
     * @param a any sparsematrix but not this.
     * @param b any sparsematrix but not this.
     */
    void addProduct_( SparseMatrix& a,  SparseMatrix& b);

    /**
     * Subtracts the product of two matricies from this sparsematrix.
     * This -= a * b.
     *
     * @param a any sparsematrix
     * @param b any sparsematrix
     */
    void subtractProduct( SparseMatrix a, SparseMatrix b){
      subtractProduct_(a,b);
    }
    /**
     * Subtracts the product of two matricies from this sparsematrix.
     * Better than: *this -= a * b.
     * @param a any sparsematrix but not this.
     * @param b any sparsematrix but not this.
     */
    void subtractProduct_( SparseMatrix& a, SparseMatrix& b);

    /**
     * Subtracts the product of a sparsematrix and a double from this sparsematrix.
     * *this -= d * b.
     * @param b any sparsematrix but not this.
     * @param d any double number.
     */
    void subtractProduct( SparseMatrix b, double d){
      addProduct_(b,-d);
    }
    /**
     * Subtracts the product of a sparsematrix and a double from this sparsematrix.
     * Better than: *this -= d * b.
     * @param b any sparsematrix but not this.
     * @param d any double number.
     */
    void subtractProduct_( SparseMatrix& b, double d){
      addProduct_(b,-d);
    }
    /**
     * Adds the product of a sparsematrix and a double from this sparsematrix.
     * *this += d * b.
     * @param b any sparsematrix.
     * @param d any double number.
     */
    void addProduct( SparseMatrix b, double d){
      addProduct_(b,d);
    }
    /**
     * Adds the product of a sparsematrix and a double from this sparsematrix.
     * Better than: *this += d * b.
     * @param b any sparsematrix but not this.
     * @param d any double number.
     */
    void addProduct_( SparseMatrix& b, double d);
  /**
     * This atkes three 'information matricies' in a T circuit
     * and converts to two equicvalent matricies in a L circuit.
     * This is the first horizontal leg of the T while pe is the other leg
     * and rn is the leg to 'ground'.  The rtype is zero if there is no 
     * rn information.  The return is to change this to the horizontal leg
     * and rn to the new grounded leg effectively removing pe from the circuit.
     *
     *  this becomes pn = 1/2[a^t * this + this * a]
     *  a=s^-1 * pe
     *  s = this + rn + pe
     *  rn becomes pe(I-a) - pn 
     *
     * All three input matricies must be symetric square.
     * All three should be same size.
     * All three should be positive semi-definate.
     * This+pe should be positive definate.
     * 
     * @param pe the symetric information sparsematrix to be eliminated.
     * @param rn the symetric information to the absoulte reference frame
     * that is to be converted from a position between pe and this to a 
     * series coupling to this.
     * @return 1 if this is not square else 0.
     */
    int convertInformationT2L(Cure::SparseMatrix &pe,
			       Cure::SparseMatrix &rn);
  protected: 
    /**
     * This will conditionally allocate a new double array to AllocatedElement.
     * It allways sets m_Rows and m_Columns to r and c.  If the current allocation
     * is not large enough it will delete it and allocate a new array.
     * If it allocates a new array it sets all values to 0.  
     * If it does not allocate no values are changed.
     * @param r number of rows
     * @param c number of colunms
     */
    void setupElement(const int r, const int c);
  private:
    bool inRange(int r, int c){
      return(((r<m_Rows)&&(r>=0))&&((c<m_Columns)&&(c>=0)));
    }
    void move(const SparseMatrix& mat);
  };
  inline void SparseMatrix::reallocate(const int r,const int c){
    if (r==m_Rows)
      {
	if(c==m_Columns)return;
	if ((c<0)&&(m_Columns==m_Rows))return;  
      }
    setupElement(r,c);
  }
  inline void SparseMatrix::reallocateZero(const int r,const int c){
    if (r==m_Rows)
      {
	if ((c==m_Columns)||((c<0)&&(m_Columns==m_Rows))){
	  (*this)=0;
	  return;
	}
      }
    setupElement(r,c);
  }
  
  inline void SparseMatrix::operator = (const SparseMatrix& mat) 
  {
    if (m_Columns!=mat.m_Columns || m_Rows!=mat.m_Rows )
      reallocate(mat.m_Rows,mat.m_Columns);
    clear();
    m_Row=mat.m_Row;
    m_Column=mat.m_Column;
    m_TempL=(mat.m_NextIndex>>10);
    m_TempL2=(mat.m_NextIndex&0x3FF);
    if (m_TempL2>0)
      m_TempL++;
    if (m_NumberOfElements<m_TempL){
      double **m=m_Elements;
      if (m_Elements)delete []m_Elements;
      m_Elements=new double*[m_TempL];
      if (m){
	if (m_NumberOfElements>0)
	  memcpy(m_Elements,m,m_NumberOfElements*sizeof(double*)); 
	delete []m;
      }
      
      m_Elements[m_NumberOfElements]=
	new double[((m_TempL-m_NumberOfElements)<<10)];
      for (m_TempL2=m_NumberOfElements+1;m_TempL2<m_TempL;m_TempL2++){
	m_Elements[m_TempL2]=m_Elements[m_TempL2-1]+1024;
	m_Allocated.grow(1,m_TempL,false);
	m_Allocated.setBit(0,m_NumberOfElements,true);
	m_NumberOfElements=m_TempL;
      }
    }
    m_TempUL=sizeof(double);
    m_TempUL=(m_TempUL<<10);
    for (int i=0;i<m_TempL;i++){
      memcpy(m_Elements[i],mat.m_Elements[i],m_TempUL);
    }
    m_NextIndex=mat.m_NextIndex;
  }
  inline void SparseMatrix::addProduct_(SparseMatrix& a, 
				        SparseMatrix& b)
  {
#if CURESPARSEMATRIXDEBUG >10
    if (((m_Rows!=a.m_Rows)||(m_Columns!=b.m_Columns))||
	(b.m_Rows!=a.m_Columns))
      {
	cerr << "\nSPARSEMATRIX ERROR Incompatible matrices for addProduct\n";
	return;
      }
#endif
    reallocate(a.m_Rows,b.m_Columns);
    long i,r,k,k2,c,col;
    for (i=0;i<a.m_Rows;i++){
      r=a.m_Row.columns(i);
      if (r>0)
	for (k=0;k<r;k+=2){
	  col=a.m_Row(i,k);
	  c=b.m_Row.columns(col);
	  if (c>0){
	    for (k2=0;k2<c;k2+=2){
	      addValue(i,b.m_Row(col,k2),
		       (a.getValue(a.m_Row(i,k+1))*
			b.getValue(b.m_Row(col,k2+1))));
	    } 
	  }	
	}
    }
  }
  inline void SparseMatrix::subtractProduct_(SparseMatrix& a, 
					     SparseMatrix& b)
  {
#if CURESPARSEMATRIXDEBUG >10
    if (((m_Rows!=a.m_Rows)||(m_Columns!=b.m_Columns))||(b.m_Rows!=a.m_Columns))
      {
	cerr << "\nSPARSEMATRIX ERROR Incompatible matrices for subtractProduct\n";
	return;
      }
#endif
    reallocate(a.m_Rows,b.m_Columns);
    long i,r,k,k2,c,col;
    for (i=0;i<a.m_Rows;i++){
      r=a.m_Row.columns(i);
      if (r>0)
	for (k=0;k<r;k+=2){
	  col=a.m_Row(i,k);
	  c=b.m_Row.columns(col);
	  if (c>0){
	    for (k2=0;k2<c;k2+=2){
	      addValue(i,b.m_Row(col,k2),
		       (-a.getValue(a.m_Row(i,k+1))*
			b.getValue(b.m_Row(col,k2+1))));
	    } 
	  }	
	}
    }
  }

  inline void SparseMatrix::add_(SparseMatrix& a,SparseMatrix& b)
  {
#if CURESPARSEMATRIXHHDEBUG >20
    if (( b.m_Columns!=a.m_Columns) || (b.m_Rows!=a.m_Rows ))
      {
	std::cerr << "\nERROR Incompatible matrices for add_\n";
	return;
      }
#endif 
    clear();
    reallocate(a.m_Rows,a.m_Columns);
    long i,r,k;
    for (i=0;i<a.m_Rows;i++){
      r=a.m_Row.columns(i);
      if (r>0)
	for (k=0;k<r;k+=2)
	  setValue(i,a.m_Row(i,k),a.getValue(a.m_Row(i,k+1)));
    }
    for (i=0;i<b.m_Rows;i++){
      r=b.m_Row.columns(i);
      if (r>0)
	for (k=0;k<r;k+=2)
	  addValue(i,b.m_Row(i,k),b.getValue(b.m_Row(i,k+1)));
    }
  }
  inline void SparseMatrix::subtract_(SparseMatrix& a,SparseMatrix& b)
  {
    
#if CURESPARSEMATRIXHHDEBUG >20
    if (( b.m_Columns!=a.m_Columns) || (b.m_Rows!=a.m_Rows ))
      {
	std::cerr << "\nERROR Incompatible matrices for subtract_\n";
	return;
      }
#endif 
    clear();
    reallocate(a.m_Rows,a.m_Columns);
    long i,r,k;
    for (i=0;i<a.m_Rows;i++){
      r=a.m_Row.columns(i);
      if (r>0)
	for (k=0;k<r;k+=2)
	  setValue(i,a.m_Row(i,k),a.getValue(a.m_Row(i,k+1)));
    }
    for (i=0;i<b.m_Rows;i++){
      r=b.m_Row.columns(i);
      if (r>0)
	for (k=0;k<r;k+=2)
	  addValue(i,b.m_Row(i,k),-b.getValue(b.m_Row(i,k+1)));
    }
  }
  inline void   SparseMatrix::write(std::fstream &fs ){
    fs<<"\n"<<m_Rows<<" "<<m_Columns<<"\n"; 
    for (long i=0;i<m_Rows; i++){
      long c=m_Row.columns(i);
      fs<<c<<" ";     
      for (long j=0;j<c;j+=2)
	fs<<m_Row(i,j)<<" "<<getValue(m_Row(i,j+1))<<" ";
      fs<<"\n";     
    }
  }
  inline void SparseMatrix::read(std::fstream &fs ){
    long r=0;
    long c=0;
    long k=0;
    fs>>r>>c;
    reallocate(r,c);
    double d=0;
    for (int i=0;i<r; i++){
      fs>>c;
      for (int j=0;j<c; j++){
	fs>>k>>d;
	setValue(i,k,d);
      }
    }
  }

   
} // namespace Cure;



#endif // CURE_SPARSEMATRIX_HH
