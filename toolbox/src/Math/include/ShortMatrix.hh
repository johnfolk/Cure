// = AUTHOR(S)
//    John Folkesson
//    
// = COPYRIGHT
//    Copyright (c) 2005 John Folkesson
//    

#ifndef CURE_SHORTMATRIX_HH
#define CURE_SHORTMATRIX_HH
#include <iostream>
#include <fstream>
#include <math.h>
#include "Matrix.hh"

namespace Cure {
   /**
   * One can set this to integers from 0 (no debug tests) to 200 (All tests)
   * More tests make slower code.
   * This changes this header file so all code that links to this 
   * need to be recompiled if this is changed.  It has to be changed here as
   * this line will redefine it.
   *
   * There is a similar constant for the cc file with the property that 
   * it can be changed when (re)compiling ShortMatrix.cc and one should not 
   * have to recompiled all linked files. 
   *
   * The debug messages seem to be the best thing about using this class
   * as they warn for all sorts of out-of-bounds situations that commonly
   * lead to seg fault.
   */
#define CURESHORTMATRIXHHDEBUG 200

  /**
   * The ShortMatrix class will simplify code with matrices of shorts.
   * ShortMatrix class to organize allocation of memory and avoiding
   * segmentation faults.  Instead of the seg fault one gets a specific
   * error message that usually helps pin point the bug quickly.  
   * 
   *  Row and column numbering starts from 0 (not
   * 1 as in MATLAB).
   * 
   * One can temporarily assign the matrix to refer to only a sub-matrix
   * of the full array.  So that a block can be used to do all the matrix
   * operations.
   * @see ShortMatrix::offset, @see ShortMatrix::reset
   *
   * The underscore at the end of some methods has a special significance.
   * There is always a 'value safe' version without the underscore.
   * Thus for speed critical multiplications use:
   *
   * A.Multiply_(B,C); 
   *
   * rather than:
   *
   * A=B*C;  or A.Multiply(B,C)
   *
   * This avoids the intermediate copy.  The problem is that one can not do:
   *
   * NEVER DO!! 
   * A.Multiply_(A,C)
   * NEVER DO!!
   *
   * This will overwrite the operand 'A' during computation of the
   * product.  This will then give the wrong answer.  Instead do:
   *
   * A.Multiply(A,C) (no underscore)
   * 
   * which is just fine.
   *
   * Some of the methods below have two variants.  One is by value and the
   * other by ref.
   * The 'unsafe' method is signaled by the '_' in the Name_.
   * They take a ShortMatrix& as an argument which can not be 'this'.
   * 
   * Of course the by ref calls are more efficient by not making 
   * intermediate copies.  Which is the only difference.
   * So if you don't have any part of 'this' ShortMatrix in the 
   * arguments you should call the _ variants.
   * 
   * On the other hand if this is all gobbly gook to you don't use 
   * the _ variants at all.
   *
   * @author John Folkesson
   */
  class ShortMatrix
  {  
  public:
    /**
     * The number of rows in the matrix.  This number can be changed by
     * calling the offset methods.  Notice that there is an implied 
     * size limit here.
     */
    long Rows;
    /**
     * The number of columns in the matrix.  This number can be changed by
     * calling the offset methods.
     */
    long Columns;  
    /**
     * The number of columns in the memory allocation.  This transforms
     * the linear memory into a 2D array.  RowInc must be equal or
     * larger than Columns.
     */
    long  RowInc;  
    /**
     * The pointer to the current upper left corner of the matrix.  This
     *can be changed by calling offset.
     */
    short *Element;
  protected:
    /**
     * If the matrix has allocated its own memory this will store the pointer.
     * It is needed to free the memory later and for the reset method.
     */ 
    short *AllocatedMatrix;
    /**
     * This is the size of the short array pointed to by AllocatedMatrix.
     * It is needed to generate error messages when offset is out-of-bounds.
     */
    long AllocatedSize;

  public:
    /**
     * Create a matrix with 0 rows and columns.
     */
    ShortMatrix();
    /**
     * Create a matrix that allocates its own memory and is equal to the 
     * currently defined matrix b.  (ie. it can be that b has been
     * set to refer to only a sub-matrix of its full allocated array.
     * In that case only the sub-matrix will be allocated in the new matrix.)
     * @param b a matrix to be used as the initial values.   
     */
    ShortMatrix(const ShortMatrix& b);
  
    /**
     * This will allocate its own array (default is c=r).
     * All initial values are set to 0.
     * @param r the number of rows to allocate.
     * @param c the number of columns (default c=r).
     */ 
    ShortMatrix(const int r,const int c=-1);
  
    /**
     * This will delete arrays allocated by the ShortMatrix but not those 
     * assigned by the user.
     */
    virtual ~ShortMatrix();
    /**
     * The = operator allocates its own memory if needed and sets it
     * equal to the currently defined matrix b.  (ie. it can be that
     * b has been set to refer to only a submatrix of its full
     * allocated array.  In that case only the sub-matrix will be
     * allocated in the new matrix.)  
     *
     * @param b a matrix to be used as the initial values.
     */
    void operator = (const ShortMatrix& b);
    /**
     * The = operator allocates its own memory if needed and sets it
     * equal to the currently defined matrix b.  (ie. it can be that
     * b has been set to refer to only a submatrix of its full
     * allocated array.  In that case only the sub-matrix will be
     * allocated in the new matrix.) It rounds all values;  
     *
     * @param b a matrix to be used as the initial values.
     */
    void operator = (const Matrix& b);
    /**
     * This set the matrix to be all zero except for d down the diagonal
     * starting from the upper left corner (ie (*this)(i,i)=d).
     * If d is 0 this just sets the matrix to 0.
     *   @param d the value fro the diagonal elements.
     */  
    void operator = (const short d);
    /**
     * This gets a copy of a matrix element at (r,c).
     * @param r the row of the element
     * @param c the column.
     */
    short operator() (const int r, const int c) const { 
      return( (*(ShortMatrix *)this).operator()(r,c) );
    }
    /**
     * This retrieves a long from the two adjacent columns.
     * I don't do this by casing the pointer as that would 
     * imply a prefered endian-ness.
     * @param r must be less that Rows.
     * @param c must be less that Columns-1
     * @return the long that has high byte in (r,c) and low byte in (r,c+1)
     */
    long getLong(const unsigned long r, const unsigned long c)
    {
      if ((r>=(unsigned long)Rows)||((c+1)>=(unsigned long)Columns)){
	std::cerr<<"ShortMatrix::getLong out of bounds"
		 <<r<<" "<<c<<" "<<Rows<<" "<<Columns<<"\n";
	return 0;
      }      
      unsigned short j=(unsigned short)(*(Element+r*RowInc+c));
      unsigned long k=(unsigned long) j;
      k=(k<<16);
      k=(0xFFFF0000&k)|
	(unsigned long)((0xFFFF)&
			((unsigned short)(*(Element+r*RowInc+c+1))));
      return (long)k;
    }
    /**
     * The long is stored: high bytes in (r,c+1) and low bytes in (r,c)
     *
     * @param val the number to store.
     * @param r must be less that Rows.
     * @param c must be less that Columns-1
     * @return 1 if out of bounds else 0. 
     */
    int setLong(const long val,const unsigned long r, const unsigned long c)
    {
      if ((r>=(unsigned long)Rows)||((c+1)>=(unsigned long)Columns)){
	std::cerr<<"ShortMatrix::setLong out of bounds"
		 <<r<<" "<<c<<" "<<Rows<<" "<<Columns<<"\n";
	return 1;
      }     
      (*(Element+r*RowInc+c))=((val>>16)&(0xFFFF));
      (*(Element+r*RowInc+c+1))=((val)&(0xFFFF));
      return 0;
    }
    /**
     * This retrieves a long from the two adjacent rows.
     * I don't do this by casing the pointer as that would 
     * imply a prefered endian-ness.
     * @param r must be less that Rows-1.
     * @param c must be less that Columns
     * @return the long that has high byte in (r,c) and low byte in (r,c+1)
     */
    long getLongColumn(const unsigned long r, const unsigned long c)
    {
      if (((1+r)>=(unsigned long)Rows)||(c>=(unsigned long)Columns)){
	std::cerr<<"ShortMatrix::getLongColumn out of bounds"
		 <<r<<" "<<c<<" "<<Rows<<" "<<Columns<<"\n";
	return 0;
      }      
      unsigned short j=(unsigned short)(*(Element+r*RowInc+c));
      unsigned long k=(unsigned long) j;
      k=(k<<16);
      k=(0xFFFF0000&k)|
	(unsigned long)((0xFFFF)&
			((unsigned short)(*(Element+(r+1)*RowInc+c))));
      return (long)k;
    }
    /**
     * The long is stored: high bytes in (r,c) and low bytes in (r+1,c)
     *
     * @param val the number to store.
     * @param r must be less that Rows-1.
     * @param c must be less that Columns
     * @return 1 if out of bounds else 0. 
     */
    int setLongColumn(const long val,const unsigned long r, const unsigned long c)
    {
      if (((1+r)>=(unsigned long)Rows)||(c>=(unsigned long)Columns)){
	std::cerr<<"ShortMatrix::setLongColumn out of bounds"
		 <<r<<" "<<c<<" "<<Rows<<" "<<Columns<<"\n";
	return 1;
      }     
      (*(Element+r*RowInc+c))=((val>>16)&(0xFFFF));
      (*(Element+(r+1)*RowInc+c))=((val)&(0xFFFF));
    }

    /**
     * Adds this to b and returns the result as a new matrix of right size.
     * If the size of b is not the same as this a matrix of 0's is returned 
     * and an error message printed.
     * @param b the matrix to add to this.
     * @see ShortMatrix::add_
     */
    ShortMatrix operator + (const ShortMatrix &b) const;
    /**
     * Subtracts b from this matrix and returns the result as a new
     * matrix of right size.  If the size of b is not the same as this
     * a matrix of 0's is returned and an error message printed.
     *  
     * @param b the matrix to subtract from this.
     * see ShortMatrix::subtract_   
     */
    ShortMatrix operator - (const ShortMatrix &b) const;
    /**
     * Mutiplies b by this matrix and returns the result as a new matrix of
     * right size.  If the size of b is wrong a matrix
     * of 0's is returned and an error message printed.
     *
     * @param a the matrix to multiply by this.
     * @see ShortMatrix::multiply_  
     */
    ShortMatrix operator * (const ShortMatrix &b) const;
    /**
     * Adds b to this matrix and returns the result in this matrix.
     * If the size of b is wrong a matrix nothing is changed and an
     * error message printed.
     *
     * @param b the matrix to add to this.
     * @see ShortMatrix::add_
     */
    void operator += (const ShortMatrix & b);
    /**
     * Subtract b from this matrix and returns the result in this
     * matrix.  If the size of b is wrong a matrix nothing is changed
     * and an error message printed.
     *
     * @param b the matrix to subtract from this.
     * @see ShortMatrix::subtract_  
     */
    void operator -= (const ShortMatrix& b);
    /**
     * Multiply b by this matrix from the left and returns the result in
     * this matrix.  If the size of b is wrong a matrix nothing is changed
     * and an error message printed.  (this=this*mat).
     *
     * @param b the matrix to multiply by this.
     * @see ShortMatrix::multiply_
     */
    void operator *= (const ShortMatrix& b);
    /**
     * Multiply all elements  by d returns the result in
     * this.  
     *
     * @param d the number to multiply  this by.
     */
    void operator *= (const short d);
    /**
     * Divide all elements  by d returns the result in
     * this.  
     *
     * @param d the number to divide  this by.
     */
    void operator /= (const short d);
    /*
     * Adds d times I, the identity matrix, to this.
     * If this is not square the I is has the smaller dimension. 
     *
     * @param d the number to add to  this.
     */ 
    void operator += (const short d);
    /**
     * This matrix becomes equal to the element by element 
     * product between this matrix and b.  This is like .* in MATLAB.
     *  
     * @param b any matrix
     */
    void arrayMult(const ShortMatrix& b);
    /**
     * This matrix becomes equal to the element by element ratio
     * *this(i,j)=*this(i,j)/b(i,j).  This is like ./ in MATLAB.
     *  
     * @param b any matrix
     */
    void arrayDivide(const ShortMatrix& b);
    /**
     * This matrix is transposed.
     * @see ShortMatrix::transpose_;
     */
    void transpose(void);
    /**
     * This matrix becomes equal to the transpose of a.
     * @param a the matrix.  
     */
    void transpose(const ShortMatrix a);
    /**
     * This matrix becomes equal to the transpose of a.  
     *   
     * @param a Here a should not be this matrix.
     */
    void transpose_(const ShortMatrix & a);
    /**
     * This matrix is multiplied by -1.
     */
    void minus();
    /**
     * Set this = (this + transpose(this))/2
     */
    void symetrize();
    /**
     * The theshold method sets all elements with absolute value less
     * than a threshold to zero.
     * @param pthreshold The minimum absolute value for non-zero elements.
     */
    void threshold(short pthreshold);
    /**
     * The reallocate method allows you to change the array memory
     * allocation.  If the Rows=r and Columns are 'correct' this
     * method does nothing.  It does not copy any values.  The default
     * (if you only specify r), is Columns=Rows.  If a new memory
     * allocation is needed all values will be set to zero.
     *
     * @param r the number of rows.
     * @param c the number of coulmns (default c=r).
     * @see ShortMatrix::grow, @see ShortMatrix::offset, @see ShortMatrix::reset  
     */
    void reallocate(const int r,const int c=-1 );
    /**
     * All values will be set to zero.
     * The reallocateZero method allows you to change the array memory
     * allocation.  If the Rows=r and Columns are 'correct' this
     * method does nothing.  It does not copy any values.  The default
     * (if you only specify r), is Columns=Rows.  
     * 
     *
     * @param r the number of rows.
     * @param c the number of coulmns (default c=r).
     * @see ShortMatrix::grow, @see ShortMatrix::offset, @see ShortMatrix::reset  
     */
    void reallocateZero(const int r,const int c=-1 );
    /**
     * The grow method allows you to extend the array memory beyond the 
     * originally allocation, or even copy it to a smaller
     * allocation freeing memory. 
     * 
     * It allocates a new array and copies the matrix elements from the 
     * original allocation to the new one in the approriate
     * row and column (staring from (0,0) being the same)
     * The default (if you only specify r), is Columns=Rows.
     *  One should really not use this unless 
     * one explictly knows that the ShortMatrix needs to grow,
     * (in other cases its a waste of time, although it may
     * reduce amount of memory in use).
     *   
     * @param r the number of rows for the extended matrix.
     * @param c the number of columns for the extended matrix,default is c=r.
     * @see ShortMatrix::reallocate, @see ShortMatrix::offset, @see ShortMatrix::reset  
     */
    void grow(const int r,const int c=-1);
    /**
     * Copy the values in rows>r up one row and then subtract 1 from
     * Rows. 
     * @see ShortMatrix::grow, @see ShortMatrix::offset, @see ShortMatrix::reset  
     */
    void deleteRow(int r);
    /**
     * Copy the values in columns>c to the column to the left and then 
     * subtract 1 from Columns.
     * @see ShortMatrix::grow, @see ShortMatrix::offset, @see ShortMatrix::reset  
     */
    void deleteColumn(int c);
    /**
     * Moves the data to the left and changes the size of the matrix
     * @param startcolumn the first column to delete 
     * @param numberofcolumns the total number of columns to delete 
     *
     * @see ShortMatrix::grow, @see ShortMatrix::offset, @see ShortMatrix::reset  
     */
    void deleteColumns(int startcolumn, int numberofcolumns );
    /**
     * This actually creates a new short array and deletes the old
     * one.  This makes this rather heavy and not to be called
     * without really needing this.
     * Moves the data to the right and changes the size of the matrix.
     * the inserted columns have 0's set.
     * @param startcolumn the place to insert column to delete 
     * @param numberofcolumns the total number of columns to insert 
     *
     * @see ShortMatrix::grow, @see ShortMatrix::offset, @see ShortMatrix::reset  
     */
    void insertColumns(int startcolumn, int numberofcolumns );
    /**
     * Copy the values from row r1 to r2 and visa versa.
     * @param startrow a row of the matrix.
     * @param numberofrows number to insert
     * @see ShortMatrix::grow, @see ShortMatrix::offset, @see ShortMatrix::reset  
     */
    void insertRows(int startrow,int numberofrows=1 );
    /**
     * Copy the values from row r1 to r2 and visa versa.
     * @param r1 a row of the matrix.
     * @param r2 another row.
     * @see ShortMatrix::grow, @see ShortMatrix::offset, @see ShortMatrix::reset  
     */
    void swapRows(int r1,int r2);
    /**
     * Copy the values from column c1 to c2 and visa versa.
     * @param c1 a column of the matrix.
     * @param c2 another column.
     */
    void swapColumns(int c1,int c2);
    /**
     * Moves num rows from r1 to r2.
     * @return 0 if ok else 1;
     */
    int moveRows(unsigned long r1,unsigned long r2, 
		 unsigned short num=1);
    
    /**
     * Moves num columns from c1 to c2.
     * @return 0 if ok else 1;
     */
    int moveColumns(unsigned long c1,unsigned long c2, unsigned short num);

    /**
     * This stores a 'list' of shorts without fuss.
     * So this will expand to hold the number of shorts stored but will
     * never free the memory until the ShortMatrix is deleted.
     * The number is put in the last row at the column specified.
     * If col is 0 a new row is added to the matrix.
     * To clear just do reset (0,Columns).
     * Be sure to append(d,0) before any other numbers.
     * @param d the number to store.
     * @param col the column to put d in.
     */
    void append(short d, int col);
    /**
     * This will assign the matrix to refer to a submatrix of the full
     * allocation.  Changes the pointer Element to point to a submatrix
     * and changes the size (Rows and Columns).  Use this to do
     * operations on blocks of your matrix.  This allows you to then use
     * all the code for the ShortMatrix class on just a block.  You must then
     * reset the matrix to the original size when finished.  Of course
     * unexpected things will happen if you call methods that adjust the
     * size of the matrix.  If the matrix Rows and Colums is compatible
     * before calling any method in this class then the size will not
     * need to be adjusted and all is well.  If you miss calculate the
     * arguments here you will most likely get error messages about it
     * during execution.  You can not assign to anything outside the
     * current memory allocation.  The offsets are relative to the current
     * matrix (0,0) element.
     *
     * @param rowOffset The ShortMatrix will start at a point offset by this many
     *                     Rows from the current upper left corner.
     * @param columnOffset The ShortMatrix will start at a point offset by this many
     *                     Columns from the current upper left corner.
     * @param newRows  Rows will be set to this.
     * @param newColumns  Columns will be set to this.
     * @see ShortMatrix::reset
     */
    void offset(int rowOffset, int columnOffset, int newRows, 
		int newColumns);
    /**
     * Change the pointer Element to point to a submatrix and change the Size.
     * This moves the pointer by the number of shortss given in columnOffset.
     * If this exceeds RowInc it will end up pointing  to a lower row (ok?).
     *  
     * @param columnOffset The ShortMatrix will start at a point offset by this many
     *                     Columns from the current start.
     * @param newRows  Rows will be set to this.
     * @param newColumns  Columns will be set to this.
     * @see ShortMatrix::offset,@see ShortMatrix::reset
     */
    void offset(int columnOffset, int newRows, 
		int newColumns);
    /**
     * Change the pointer Element to point to a submatrix and change the Size.
     * @param rowColOffset The ShortMatrix will start at a point offset by this many
     *                     Rows and Columns from the current start.
     * @param newRowsCols both Rows and Columns will be set to this.
     * @see ShortMatrix::reset, @see ShortMatrix::offset 
     */
    void offset(int rowColOffset, int newRowsCols);
    /**
     * Change the pointer Element to point back to the original upper
     * left corner and change the Rows and Columns.  This can be used to
     * restore the matrix after calling offset and doing some block
     * operations.  The caller will have to have remembered the Rows and
     * Columns (the size of the original matrix).  By original we mean
     * the start of the memory allocation.  This does not work at all
     * for memory not automatically allocated by the ShortMatrix object, (ie
     * if you gave the allocated short array in the constructor).
     *
     * @param newRows  Rows will be set to this.
     * @param newColumns  Columns will be set to this.
     * @see ShortMatrix::offset   
     */
    void reset(int newRows,int newColumns);
    /**
     * Creates a file and saves the matrix in a format that MATLAB can load.
     * @param filename the name of the file to create.
     */
    void save(const char *filename);
    /** 
     * This writes the matrix to a file in a format that is useful for
     * saving of complex objects that contain matrices.  The format is
     *
     * Rows Columns
     * First row of ellements
     * Second row ...
     * @param fs an open for write file stream.
     * @see ShortMatrix::read, @see ShortMatrix::save
     */
    void   write(std::fstream &fs );
    /** 
     * This reads the matrix from a file in a format that is useful for
     * loading of complex objects that contain matrices.  The format is
     *
     * Rows Columns
     * First row of ellements
     * Second row ...
     * @param fs an open for read file stream.
     * @see ShortMatrix::write,@see ShortMatrix::save
     */
    void   read(std::fstream &fs );
    /**
     * This prints the matrix to the screen display.  This is useful
     * for debug.
     */
    void print();
    /**
     * Calculate the trace.
     * @return the sum of the diagonal.
     */
    long trace()const;
    /**
     * This finds a circlar arc to fit the points on a path.
     * @param radius the radius of the arc is returned here <0 for
     * left turns
     * This assumes the path is given by the elements of this
     * in form (x,y) on each row.
     * @center the center (r,c) of the arc 
     * @param startangle the angle to the x axis (increasing rows)
     *        of the tangent to the arc pointing in the direction of the path
     *        This is the direction the robot is facing.
     * @param maxdeviation the furthest that any point on path can be from
     *        the arc.
     * @return  The number of points that fit the arc.  If the radius is 
     *          infinite (straight line) then this is negative the of the
     *           number of points that fit the line.
     */
    int fitToArc( double &radius, double center[2],
		  double startangle,
		  double maxdeviation);

    void add(const ShortMatrix m1,const ShortMatrix m2){
      add_(m1,m2);
    }
    /**
     * Efficiently add two matricies and put the result in this matrix.
     * This is the better way to do it than, *this= m1+m2.  
     * @param m1 any matrix but not this.
     * @param m2 any matrix but not this.
     */
    void add_(const ShortMatrix& m1,const ShortMatrix& m2);
    /**
     * Subtract two matricies and put the result in this matrix.
     * this= m1-m2.
     * @param m1 any matrix
     * @param m2 any matrix
     */
    void subtract(const ShortMatrix m1,const ShortMatrix m2){
      subtract_(m1,m2);
    }      
    /**
     * Subtract two matricies and put the result in this matrix.
     * This is the better way to do it than,*this= m1-m2.
     * @param m1 any matrix but not this.
     * @param m2 any matrix but not this.
     */
    void subtract_(const ShortMatrix& m1,const ShortMatrix &m2);      
    /**
     * This matrix becomes equal to the product of m1*m2, changing size
     * if needed, (no need to call reallocate to set Rows and Columns
     * correctly before calling this).   Of course, m1 and m2 must be
     * compatable with matrix multply.
     *  
     * @param m1 any matrix
     * @param m2 any matrix
     */
    void multiply(const ShortMatrix m1,const ShortMatrix m2){
      multiply_(m1,m2);
    }  
    /**
     * This matrix becomes equal to the product of m1*m2, changing size
     * if needed, (no need to call reallocate to set Rows and Columns
     * correctly before calling this).  Of course, m1 and m2 must be
     * compatable with matrix multply.
     *
     * This is the better way to do it than,*this= m1*m2.
     * @param m1 any matrix but not this.
     * @param m2 any matrix but not this.
     */
    void multiply_(const ShortMatrix& m1,const ShortMatrix& m2);
    
    /**
     * This method allows transposing the arguments before multiplying them.
     * This matrix =  (tranpose) a * (transpose)b, changing
     * size if needed, (no need to call reallocate to set Rows and 
     * Columns correctly before calling this).
     *
     * Where which  
     * @param which tells which matrix, (a or b)  to transpose:
     *              1 transpose a, 
     *              2 transpose b
     *              3 both
     * @param a any matrix.
     * @param b any matrix.
     */
    void multTranspose(const ShortMatrix a,const ShortMatrix b,const int which){
      multTranspose_(a,b,which);
    }
    /**
     * This method allows transposing the arguments before multiplying them.
     * This matrix =  (tranpose) a * (transpose)b, changing
     * size if needed, (no need to call reallocate to set Rows and 
     * Columns correctly before calling this).
     *
     * This is a whole lot more effiecient than for instance
     * c.transpose(b);
     * this=a*c;
     *
     * Where which  
     * @param which tells which matrix, (a or b)  to transpose:
     *              1 transpose a, 
     *              2 transpose b
     *              3 both
     * @param a any matrix.
     * @param b any matrix.
     */
    void multTranspose_(const ShortMatrix& a,const ShortMatrix& b,const int which);
    /**
     * This is like multiply but puts in a minus sign.
     * This = -(a * b), changing
     * size if needed, (no need to call reallocate to set Rows and 
     * Columns correctly before calling this).
     *
     * @param a any matrix
     * @param b any matrix
     */
    void minusMult(const ShortMatrix a,const ShortMatrix b){
      minusMult(a,b);
    }
    /**
     * This is like multiply but puts in a minus sign.
     * Much better than *this = -(a * b). It changes
     * size if needed, (no need to call reallocate to set Rows and 
     * Columns correctly before calling this).
     *
     * @param a any matrix but not this.
     * @param b any matrix but not this.
     */
    void minusMult_(const ShortMatrix& a,const ShortMatrix& b);

    /**
     * Adds the product of two matricies to this matrix.
     * This += a * b.
     *
     * @param a any matrix
     * @param b any matrix
     */
    void addProduct(const ShortMatrix a, const ShortMatrix b){
      addProduct_(a, b);
    }
    /**
     * Adds the product of two matricies to this matrix.
     * Better than: *this += a * b.
     * @param a any matrix but not this.
     * @param b any matrix but not this.
     */
    void addProduct_(const ShortMatrix& a, const ShortMatrix& b);

    /**
     * Subtracts the product of two matricies from this matrix.
     * This -= a * b.
     *
     * @param a any matrix
     * @param b any matrix
     */
    void subtractProduct(const ShortMatrix a,const ShortMatrix b){
      subtractProduct_(a,b);
    }
    /**
     * Subtracts the product of two matricies from this matrix.
     * Better than: *this -= a * b.
     * @param a any matrix but not this.
     * @param b any matrix but not this.
     */
    void subtractProduct_(const ShortMatrix& a,const ShortMatrix& b);

    /**
     * Subtracts the product of a matrix and a ahort from this matrix.
     * *this -= d * b.
     * @param b any matrix but not this.
     * @param d any short number.
     */
    void subtractProduct(const ShortMatrix b,const short d){
      subtractProduct_(b,d);
    }
    /**
     * Subtracts the product of a matrix and a double from this matrix.
     * Better than: *this -= d * b.
     * @param b any matrix but not this.
     * @param d any short number.
     */
    void subtractProduct_(const ShortMatrix& b,const short d);
    /**
     * Adds the product of a matrix and a short from this matrix.
     * *this += d * b.
     * @param b any matrix.
     * @param d any short number.
     */
    void addProduct(const ShortMatrix b,const short d){
      addProduct_(b,d);
    }
    /**
     * Adds the product of a matrix and a short from this matrix.
     * Better than: *this += d * b.
     * @param b any matrix but not this.
     * @param d any short number.
     */
    void addProduct_(const ShortMatrix& b,const short d);
    /**
     * This gets a reference to a matrix element at (r,c).
     * @param r the row of the element
     * @param c the column.
     */
    short& operator() (const int r, const int c){
#if CURESHORTMATRIXHHDEBUG >30
      if ((r<0)||(c<0))
	std::cerr<<"\nShortMatrix index below 0\n";
      if ((r>=Rows)&(c>=Columns))
	std::cerr<<"\nShortMatrix index above bounds\n";
#if  CURESHORTMATRIXHHDEBUG >50
      if(AllocatedMatrix==Element)
	if ((r*RowInc+c)>=AllocatedSize){
	  std::cerr<<"\nERROR ShortMatrix::(r,c) index above Allocated bounds\n";
	}
#if  CURESHORTMATRIXHHDEBUG >100
      if ((AllocatedMatrix<=Element)&&((AllocatedMatrix+AllocatedSize)>Element))
	if ((Element+r*RowInc+c)>=AllocatedMatrix+AllocatedSize){
	  std::cerr<<"\nERROR ShortMatrix index out of Allocated bounds\n";
	}
#endif
#endif
#endif
      return Element[r*RowInc+c];
    }
  protected: 
    /**
     * This will conditionally allocate a new short array to AllocatedElement.
     * It allways sets Rows and Columns to r and c.  If the current allocation
     * is not large enough it will delete it and allocate a new array.
     * If it allocates a new array it sets all values to 0.  
     * If it does not allocate no values are changed.
     * @param r number of rows
     * @param c number of colunms
     */
    void setupElement(const int r, const int c);
  private:
    bool inRange(int r, int c){
      return(((r<Rows)&&(r>=0))&&((c<Columns)&&(c>=0)));
      }
    void copy(Cure::ShortMatrix & a);
    void move(const ShortMatrix& mat);
  };
  
  inline void ShortMatrix::reallocate(const int r,const int c){
    if (r==Rows)
      {
	if(c==Columns)return;
	if ((c<0)&&(Columns==Rows))return;  
      }
    setupElement(r,c);
  }
  
  inline void ShortMatrix::reallocateZero(const int r,const int c){
    if (r==Rows)
      {
	if(c==Columns)return;
	if ((c<0)&&(Columns==Rows))return;  
      }
    setupElement(r,c);
    if (AllocatedMatrix)
      memset(AllocatedMatrix, 0, sizeof(short) * AllocatedSize);
    
  }
  
  inline void ShortMatrix::operator = (const ShortMatrix& mat) 
  {
    if (Columns!=mat.Columns || Rows!=mat.Rows )
      reallocate(mat.Rows,mat.Columns);
    long n=(Rows*RowInc);
    if ((n>0)&&((RowInc==Columns)&&(mat.RowInc==mat.Columns)))
      memcpy(Element,mat.Element,sizeof(short)*(n));
    else
      {
	long mati=0;
	for (long i=0; i<n;i+=RowInc,mati+=mat.RowInc)
	  memcpy(Element+i,mat.Element+mati,sizeof(short)*(Columns));
      }
  }
  inline void ShortMatrix::move(const ShortMatrix& mat) 
  {
    if (Columns!=mat.Columns || Rows!=mat.Rows )
      reallocate(mat.Rows,mat.Columns);
    long n=(Rows*RowInc);
    if ((n>0)&&((RowInc==Columns)&&(mat.RowInc==mat.Columns)))
      memmove(Element,mat.Element,sizeof(short)*(n));
    else
      {
	long mati=0;
	for (long i=0; i<n;i+=RowInc,mati+=mat.RowInc)
	  memmove(Element+i,mat.Element+mati,sizeof(short)*(Columns));
      }
  }
  inline void ShortMatrix::multiply_(const ShortMatrix& a,const ShortMatrix& b)
  {
    reallocate(a.Rows,b.Columns);
#if CURESHORTMATRIXHHDEBUG >20
    if (a.Columns!=b.Rows)
      {
	std::cerr<<"\nERROR ShortMatrix::multiply_ INCOMPATABLE\n";
	return;
      }
#endif 
    (*this)=0;
    for (int i=0; i<Rows; i++)
      for (int j=0; j<Columns; j++)
	for (int k=0; k<a.Columns; k++)
	  (*this)(i,j)+=a(i,k)+b(k,j);
 
  }
  inline void ShortMatrix::add_(const ShortMatrix& a,const ShortMatrix& b)
  {
#if CURESHORTMATRIXHHDEBUG >20
    if (( b.Columns!=a.Columns) || (b.Rows!=a.Rows ))
      {
	std::cerr << "\nERROR Incompatible matrices for add_\n";
	return;
      }
#endif 
    reallocate(a.Rows,a.Columns);
    for (int i=0; i<Rows; i++)
      for (int j=0; j<Columns; j++)
	(*this)(i,j)=a(i,j)+b(i,j);
  
  }
  inline void ShortMatrix::subtract_(const ShortMatrix& a,const ShortMatrix& b)
  {
    
#if CURESHORTMATRIXHHDEBUG >20
    if (( b.Columns!=a.Columns) || (b.Rows!=a.Rows ))
      {
	std::cerr << "\nERROR Incompatible matrices for subtract_\n";
	return;
      }
#endif 
    reallocate(a.Rows,a.Columns);
    for (int i=0; i<Rows; i++)
      for (int j=0; j<Columns; j++)
	(*this)(i,j)=a(i,j)-b(i,j);
  }
  inline void ShortMatrix::offset(int rowOffset, int columnOffset, int newRows, 
			     int newColumns){
#if  CURESHORTMATRIXHHDEBUG >100
    if ((AllocatedMatrix<=Element)&&((AllocatedMatrix+AllocatedSize)>Element)){
      if ((Element+(((rowOffset+newRows-1)*RowInc)+(columnOffset+newColumns-1)))
	  >=AllocatedMatrix+AllocatedSize)
	if ((newColumns>0)&&(newRows>0))
	  {
	    std::cerr<<"\nShortMatrix::offset index above bounds\n";
	  }
      if ((Element+(((rowOffset)*RowInc)+(columnOffset)))
	  <AllocatedMatrix){
	std::cerr<<"\nShortMatrix::offset index below bounds\n";
      }
    }
#endif
    Element+=((rowOffset*RowInc)+columnOffset);
    Rows=newRows;
    Columns=newColumns;
  }
  inline void ShortMatrix::offset(int columnOffset, int newRows, 
			     int newColumns){
#if  CURESHORTMATRIXHHDEBUG >100
    if ((AllocatedMatrix<=Element)&&((AllocatedMatrix+AllocatedSize)>Element)){
      if ((Element+(((newRows-1)*RowInc)+(columnOffset+newColumns-1)))
	  >=AllocatedMatrix+AllocatedSize)
	if ((newColumns>0)&&(newRows>0))
	  {
	    std::cerr<<"\nShortMatrix::offset index sbove bounds\n";
	  }
      if ((Element+columnOffset)<AllocatedMatrix){
	std::cerr<<"\nShortMatrix::offset index below bounds\n";
      }
    }
#endif
    Element+=(columnOffset);
    Rows=newRows;
    Columns=newColumns;
  }
  inline  void ShortMatrix::offset(int rowColOffset, int newRowsCols){
#if  CURESHORTMATRIXHHDEBUG >100
    if ((AllocatedMatrix<=Element)&&((AllocatedMatrix+AllocatedSize)>Element)){
      if ((Element+(((rowColOffset+newRowsCols-1)*(RowInc+1))))
	  >=(AllocatedMatrix+AllocatedSize))
	if (newRowsCols)
	  {
	    std::cerr<<"\nShortMatrix::offset index out of bounds\n";
	  }
      if ((Element+(((rowColOffset)*(RowInc+1))))
	  <AllocatedMatrix){
	std::cerr<<"\nShortMatrix::offset index out of bounds\n";
      }
    }
#endif
    Element+=(rowColOffset*(RowInc+1));
    Rows=newRowsCols;
    Columns =newRowsCols;
  }
  inline  void ShortMatrix::reset(int newRows,int newColumns){
#if  CURESHORTMATRIXHHDEBUG >40
    if(newRows*RowInc>AllocatedSize)
      if ((newColumns>0)&&(newRows>0))
	{
	  std::cerr<<"\nShortMatrix::reset index above bounds\n";
	}
#endif
    Element=AllocatedMatrix;
    Rows=newRows;
    Columns=newColumns;
  }
  inline void   ShortMatrix::write(std::fstream &fs ){
    int top=Rows*RowInc;
    fs<<"\n"<<Rows<<" "<<Columns<<"\n"; 
    for (int i=0;i<top; i+=RowInc){
      for (int j=0;j<Columns; j++)
	fs<<Element[i+j]<<" ";
      fs<<"\n";     
    }
  }
  inline void ShortMatrix::read(std::fstream &fs ){
    int r=0;
    int c=0;
    fs>>r>>c;
    reallocate(r,c);
    r=Rows*RowInc;
    for (int i=0;i<r; i+=RowInc)
      for (int j=0;j<Columns; j++)
	fs>>Element[i+j];
  }
  inline  void ShortMatrix::copy(ShortMatrix &a)
  {
    a.AllocatedMatrix=AllocatedMatrix;
    a.AllocatedSize=AllocatedSize;
    a.Element=Element;
    a.Rows=Rows;
    a.Columns=Columns;
    a.RowInc=RowInc;
  }
   
} // namespace Cure;


#endif 
