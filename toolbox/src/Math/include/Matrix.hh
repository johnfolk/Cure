// = AUTHOR(S)
//    John Folkesson
//    
// = COPYRIGHT
//    Copyright (c) 2004 John Folkesson
//    
//The code was heavily inspired by Matrix.cpp originally written by
//Henrik Christensen and modified by Patric Jensfelt

#ifndef CURE_MATRIX_HH
#define CURE_MATRIX_HH
#include "MatrixStuff.hh"
#include <iostream>
#include <fstream>
#include <string.h>  // memmove

namespace Cure {
  class LongArray;
   /**
   * One can set this to integers from 0 (no debug tests) to 200 (All tests)
   * More tests make slower code.
   * This changes this header file so all code that links to this 
   * need to be recompiled if this is changed.  It has to be changed here as
   * this line will redefine it.
   *
   * There is a similar constant for the cc file with the property that 
   * it can be changed when (re)compiling Matrix.cc and one should not 
   * have to recompiled all linked files. 
   *
   * The debug messages seem to be the best thing about using this class
   * as they warn for all sorts of out-of-bounds situations that commonly
   * lead to seg fault.
   */
#define CUREMATRIXHHDEBUG 200

  /**
   * The Matrix class will, simplify code with matrices of doubles,
   * give valuable debug messages and implements matrix algorithms
   * such as inverse and finding eigenvectors.  One can rely on the
   * Matrix class to organize allocation of memory and avoiding
   * segmentation faults.  Instead of the seg fault one gets a specific
   * error message that usually helps pin point the bug quickly.  
   * 
   * One can code with the Matrix class and avoid messy for loops and
   * trivial but treacherous index calculations.  The inverse and
   * eigen vector algorithms are as fast or faster than those we
   * compared with for reasonable size matrices.  The Matrix class is
   * very efficient with no penalty for using it to do mult and
   * addition and so on.  One can have the Matrix allocate its own
   * array (and delete it automatically).  The array Element is public
   * so you can just grab the pointer or you can do A(i,j)=.3 also to
   * access the elements.  Row and column numbering starts from 0 (not
   * 1 as in MATLAB).
   * 
   * One can temporarily assign the matrix to refer to only a sub-matrix
   * of the full array.  So that a block can be used to do all the matrix
   * operations.
   * @see Matrix::offset, @see Matrix::reset
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
   * They take a Matrix& as an argument which can not be 'this'.
   * 
   * Of course the by ref calls are more efficient by not making 
   * intermediate copies.  Which is the only difference.
   * So if you don't have any part of 'this' Matrix in the 
   * arguments you should call the _ variants.
   * 
   * On the other hand if this is all gobbly gook to you don't use 
   * the _ variants at all.
   *
   * @author John Folkesson (Henrik Christensen/Patric Jensfelt)
   */
  class Matrix
  {  
  public:
    /**
     * The number of rows in the matrix.  This number can be changed by
     * calling the offset methods.  Notice that there is an implied 
     * size limit here.
     */
    int Rows;
    /**
     * The number of columns in the matrix.  This number can be changed by
     * calling the offset methods.
     */
    int Columns;  
    /**
     * The number of columns in the memory allocation.  This transforms
     * the linear memory into a 2D array.  RowInc must be equal or
     * larger than Columns.
     */
    int  RowInc;  
    /**
     * The pointer to the current upper left corner of the matrix.  This
     *can be changed by calling offset.
     */
    double *Element;
  protected:
    /**
     * If the matrix has allocated its own memory this will store the pointer.
     * It is needed to free the memory later and for the reset method.
     */  
    double *AllocatedMatrix;
    /**
     * This is the size of the double array pointed to by AllocatedMatrix.
     * It is needed to generate error messages when offset is out-of-bounds.
     */
    long AllocatedSize;

  public:
    /**
     * Create a matrix with 0 rows and columns.
     */
    Matrix();
    /**
     * Create a matrix that allocates its own memory and is equal to the 
     * currently defined matrix b.  (ie. it can be that b has been
     * set to refer to only a sub-matrix of its full allocated array.
     * In that case only the sub-matrix will be allocated in the new matrix.)
     * @param b a matrix to be used as the initial values.   
     */
    Matrix(const Matrix& b);
  
    /**
     * This will allocate its own array (default is c=r).
     * All initial values are set to 0.
     * @param r the number of rows to allocate.
     * @param c the number of columns (default c=r).
     */ 
    Matrix(const int r,const int c=-1);
  
    /**
     * A special constructor very helpful in some situations but not as
     * easy to work with.  The user must be in charge of memory
     * allocation.  This sets the double array Element as mat that the
     * user has allocated and specifies number of rows r, number of
     * columns c (default is c=r), and the offset to move down a row (ME
     * RowInc is set to this), default rowOffset=c.  The caller is then
     * responsible to delete mat at the right time.  Many debug features
     * when using offset and reset methods will not be active as one can
     * not tell the size of the allocation.
     * 
     *
     * @param mat the double array to use as Element, no memory
     *            allocation is done.  
     * @param r the number of rows.
     * @param c the number of columns (default c=r).  
     * @param rowOffset This will be the RowInc, the number of double
     *  memory locations between rows in mat, (default rowOffset=c)..
     */ 
    Matrix(double *mat,const int r, 
	   const int c=-1,const int rowOffset=-1 );
   
    /**
     * This constructor allows you to create a copy of a submatrix of
     * the original matrix.
     * 
     * @param src the matrix to copy a submatrix from
     * @param rowOffset The Matrix will start at a point offset by this many
     *                     Rows from the current upper left corner.
     * @param columnOffset The Matrix will start at a point offset by this many
     *                     Columns from the current upper left corner.
     * @param Rows  Number of rows
     * @param Columns  Number of columns
     */
    Matrix(Matrix &src,
           int rowOffset, int columnOffset, int Rows, int Columns);

    /**
     * This will delete arrays allocated by the Matrix but not those 
     * assigned by the user.
     */
    virtual ~Matrix();
    /**
     * The = operator allocates its own memory if needed and sets it
     * equal to the currently defined matrix b.  (ie. it can be that
     * b has been set to refer to only a submatrix of its full
     * allocated array.  In that case only the sub-matrix will be
     * allocated in the new matrix.)  
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
    void operator = (const double d);
    /**
     * This gets a copy of a matrix element at (r,c).
     * @param r the row of the element
     * @param c the column.
     */
    double operator() (const int r, const int c) const { 
      return( (*(Matrix *)this).operator()(r,c) );
    }
    /**
     * Adds this to b and returns the result as a new matrix of right size.
     * If the size of b is not the same as this a matrix of 0's is returned 
     * and an error message printed.
     * @param b the matrix to add to this.
     * @see Matrix::add_
     */
    Matrix operator + (const Matrix &b) const;
    /**
     * Subtracts b from this matrix and returns the result as a new
     * matrix of right size.  If the size of b is not the same as this
     * a matrix of 0's is returned and an error message printed.
     *  
     * @param b the matrix to subtract from this.
     * see Matrix::subtract_   
     */
    Matrix operator - (const Matrix &b) const;
    /**
     * Mutiplies b by this matrix and returns the result as a new matrix of
     * right size.  If the size of b is wrong a matrix
     * of 0's is returned and an error message printed.
     *
     * @param a the matrix to multiply by this.
     * @see Matrix::multiply_  
     */
    Matrix operator * (const Matrix &b) const;
    /**
     * Adds b to this matrix and returns the result in this matrix.
     * If the size of b is wrong a matrix nothing is changed and an
     * error message printed.
     *
     * @param b the matrix to add to this.
     * @see Matrix::add_
     */
    void operator += (const Matrix & b);
    /**
     * Subtract b from this matrix and returns the result in this
     * matrix.  If the size of b is wrong a matrix nothing is changed
     * and an error message printed.
     *
     * @param b the matrix to subtract from this.
     * @see Matrix::subtract_  
     */
    void operator -= (const Matrix& b);
    /**
     * Multiply b by this matrix from the left and returns the result in
     * this matrix.  If the size of b is wrong a matrix nothing is changed
     * and an error message printed.  (this=this*mat).
     *
     * @param b the matrix to multiply by this.
     * @see Matrix::multiply_
     */
    void operator *= (const Matrix& b);
    /**
     * Multiply all elements  by d returns the result in
     * this.  
     *
     * @param d the number to multiply  this by.
     */
    void operator *= (const double d);
    /**
     * Divide all elements  by d returns the result in
     * this.  
     *
     * @param d the number to divide  this by.
     */
    void operator /= (const double d);
    /*
     * Adds d times I, the identity matrix, to this.
     * If this is not square the I is has the smaller dimension. 
     *
     * @param d the number to add to  this.
     */ 
    void operator += (const double d);
    /**
     * This matrix becomes equal to the element by element 
     * product between this matrix and b.  This is like .* in MATLAB.
     *  
     * @param b any matrix
     */
    void arrayMult(const Matrix& b);
    /**
     * This matrix becomes equal to the element by element ratio
     * *this(i,j)=*this(i,j)/b(i,j).  This is like ./ in MATLAB.
     *  
     * @param b any matrix
     */
    void arrayDivide(const Matrix& b);
    /**
     * @return sum of elements squared.
     */
    double sumSquares(){
      double d=0;
      unsigned long top=Rows*RowInc;
      for (unsigned short  irow=0; irow<top; irow+=RowInc)
	for (unsigned short j=0;j<Columns; j++)
	  d+=(Element[irow+j]*Element[irow+j]);
      return d;
    }

    /**
     * Returns a submatrix of the matrix. Empty matrix if dimensions
     * are wrong
     *
     * @param rowOffset The Matrix will start at a point offset by this many
     *                     Rows from the current upper left corner.
     * @param columnOffset The Matrix will start at a point offset by this many
     *                     Columns from the current upper left corner.
     * @param Rows  Number of rows
     * @param Columns  Number of columns
     *
     * @return submatrix of this matrix
     */
    Matrix getSubmatrix(int rowOffset, int columnOffset, 
                        int Rows, int Columns);

    /**
     * Sets the current matrix equal to a submatrix of another matrix
     *
     * @param rowOffset The Matrix will start at a point offset by this many
     *                     Rows from the current upper left corner.
     * @param columnOffset The Matrix will start at a point offset by this many
     *                     Columns from the current upper left corner.
     * @param Rows  Number of rows
     * @param Columns  Number of columns
     *
     * @return 0 if ok, else 1
     */
    int setToSubmatrix(Matrix &src,
                       int rowOffset, int columnOffset, 
                       int Rows, int Columns);

    /**
     * This matrix is transposed.
     * @see Matrix::transpose_;
     */
    void transpose(void);

    /**
     * This matrix becomes equal to the transpose of a.
     * @param a the matrix.  
     */
    void transpose(const Matrix a);

    /**
     * This matrix becomes equal to the transpose of a.  
     *   
     * @param a Here a should not be this matrix.
     */
    void transpose_(const Matrix & a);

    /**
     * @return transpose of the matrix
     */
    Matrix getTranspose()
    {
      Matrix ret(*this);
      ret.transpose();
      return ret;
    }

    /**
     * Short form name for getTranspose
     *
     * @return transpose of the matrix
     */
    Matrix T()
    {
      Matrix ret(*this);
      ret.transpose();
      return ret;
    }

    /**
     * This matrix becomes equal to the minus of a.  
     *   
     * @param a the arg
     */
    void minus(const Matrix & a);

    /**
     * This matrix is multiplied by -1.
     */
    void minus();

    /**
     * This matrix is set to equal its inverse.
     * @return 0 if ok else 1;
     */
    int invert(void);

    /**
     * @return the inverse if this matrix. The matrix will be 0-dim if
     * the matrix is not square or cannot be inverted.
     */
    Matrix getInverse(void)
    {
      Matrix ret(*this);
      if (ret.invert()) {
        std::cerr << "Matrix::inverse cannot invert matrix\n";
        ret.reallocate(0,0);
      }
      return ret;
    }

    /**
     * Short form name for getInverse
     *
     * @return the inverse if this matrix. The matrix will be 0-dim if
     * the matrix is not square or cannot be inverted.
     */
    Matrix inv(void)
    {
      Matrix ret(*this);
      if (ret.invert()) {
        std::cerr << "Matrix::inv cannot invert matrix\n";
        ret.reallocate(0,0);
      }
      return ret;
    }

    /**
     * This matrix becomes its square root,only for symmetric matricies.
     */
    void symmetricSqrt(){
      if (Rows==0)return;
      Cure::Matrix lam,ev,v(Rows);
      symmetricEigen(lam,ev);
      double min=lam(0,0)*1E-12;
      for (int i=0;i<Rows;i++)
	{
	  double d=0;
	  if (lam(i,i)>min)
	    d=sqrt(lam(i,i));
	  for (int j=0;j<Rows;j++)
	    v(i,j)=ev(j,i)*d;
	}
      multiply_(ev,v);
    }
    /**
     * This matrix becomes its inverse square root,only for symmetric
     * matricies.
     */
    void symmetricInvSqrt(){
      if (Rows==0)return;
      Cure::Matrix lam,ev,v(Rows);
      symmetricEigen(lam,ev);
      double min=lam(0,0)*1E-12;
      for (int i=0;i<Rows;i++)
	{
	  double d=0;
	  if (lam(i,i)>min)
	    d=1/sqrt(lam(i,i));
	  for (int j=0;j<Rows;j++)
	    v(i,j)=ev(j,i)*d;
	}
      multiply_(ev,v);
    }
    /**
     * This method does a Gram-Schmidt orthogonalization on the rows.
     * Treats each row as a vector and subtracts the components of 
     * the ith row in the directions of the jth rows j<i from the
     * ith vector and then normalizes the vectors.  
     */
    void makeOrthonormal();
    /**
     * Set this = (this + transpose(this))/2
     */
    void symetrize();
    /**
     * The theshold method sets all elements with absolute value less
     * than a threshold to zero.
     * @param pthreshold The minimum absolute value for non-zero elements.
     */
    void threshold(double pthreshold);
    /**
     * The reallocate method allows you to change the array memory
     * allocation.  If the Rows=r and Columns are 'correct' this
     * method does nothing.  It does not copy any values.  The default
     * (if you only specify r), is Columns=Rows.  If a new memory
     * allocation is needed all values will be set to zero.
     *
     * @param r the number of rows.
     * @param c the number of coulmns (default c=r).
     * @see Matrix::grow, @see Matrix::offset, @see Matrix::reset  
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
     * It allocates a new array and copies the matrix elements from the 
     * original allocation to the new one in the approriate
     * row and column (staring from (0,0) being the same)
     * The default (if you only specify r), is Columns=Rows.
     *  One should really not use this unless 
     * one explictly knows that the Matrix needs to grow,
     * (in other cases its a waste of time, although it may
     * reduce amount of memory in use).
     *   
     * @param r the number of rows for the extended matrix.
     * @param c the number of columns for the extended matrix,default is c=r.
     * @see Matrix::reallocate, @see Matrix::offset, @see Matrix::reset  
     */
    void grow(const int r,const int c=-1);
    /** 
     *
     * @return The Allocated Size of the Matrix in doubles
     */
    long getAllocatedSize(){
      return AllocatedSize;
    }
    /**
     * Copy the values in rows>r up one row and then subtract 1 from
     * Rows. 
     * @see Matrix::grow, @see Matrix::offset, @see Matrix::reset  
     */
    void deleteRow(int r);
    /**
     * Copy the values in columns>c to the column to the left and then 
     * subtract 1 from Columns.
     * @see Matrix::grow, @see Matrix::offset, @see Matrix::reset  
     */
    void deleteColumn(int c);
    /**
     * Moves the data to the left and changes the size of the matrix
     * @param startcolumn the first column to delete 
     * @param numberofcolumns the total number of columns to delete 
     *
     * @see Matrix::grow, @see Matrix::offset, @see Matrix::reset  
     */
    void deleteColumns(int startcolumn, int numberofcolumns );
    /**
     * This actually creates a new double array and deletes the old
     * one.  This makes this rather heavy and not to be called
     * without really needing this.
     * Moves the data to the right and changes the size of the matrix.
     * the inserted columns have 0's set.
     * @param startcolumn the place to insert column to delete 
     * @param numberofcolumns the total number of columns to insert 
     *
     * @see Matrix::grow, @see Matrix::offset, @see Matrix::reset  
     */
    void insertColumns(int startcolumn, int numberofcolumns );
    void insertColumn(int startcolumn);
    void insertRow(int startRow);
    /**
     * Copy the values from row r1 to r2 and visa versa.
     * @param r1 a row of the matrix.
     * @param r2 another row.
     * @see Matrix::grow, @see Matrix::offset, @see Matrix::reset  
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
     * Moves num columns from c1 to c2. It slides the columns  in between
     * over towards c1 to make room.
     * 
     * @return 0 if ok else 1;
     */
    int moveColumns(unsigned long c1,unsigned long c2, 
			    unsigned short num=1);
    /**
     * This will assign the matrix to refer to a submatrix of the full
     * allocation.  Changes the pointer Element to point to a submatrix
     * and changes the size (Rows and Columns).  Use this to do
     * operations on blocks of your matrix.  This allows you to then use
     * all the code for the Matrix class on just a block.  You must then
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
     * @param rowOffset The Matrix will start at a point offset by this many
     *                     Rows from the current upper left corner.
     * @param columnOffset The Matrix will start at a point offset by this many
     *                     Columns from the current upper left corner.
     * @param newRows  Rows will be set to this.
     * @param newColumns  Columns will be set to this.
     * @see Matrix::reset
     */
    void offset(int rowOffset, int columnOffset, int newRows, 
		int newColumns);
    /**
     * Change the pointer Element to point to a submatrix and change the Size.
     * This moves the pointer by the number of doubles given in columnOffset.
     * If this exceeds RowInc it will end up pointing  to a lower row (ok?).
     *  
     * @param columnOffset The Matrix will start at a point offset by this many
     *                     Columns from the current start.
     * @param newRows  Rows will be set to this.
     * @param newColumns  Columns will be set to this.
     * @see Matrix::offset,@see Matrix::reset
     */
    void offset(int columnOffset, int newRows, 
		int newColumns);
    /**
     * Change the pointer Element to point to a submatrix and change the Size.
     * @param rowColOffset The Matrix will start at a point offset by this many
     *                     Rows and Columns from the current start.
     * @param newRowsCols both Rows and Columns will be set to this.
     * @see Matrix::reset, @see Matrix::offset 
     */
    void offset(int rowColOffset, int newRowsCols);
    /**
     * Change the pointer Element to point back to the original upper
     * left corner and change the Rows and Columns.  This can be used to
     * restore the matrix after calling offset and doing some block
     * operations.  The caller will have to have remembered the Rows and
     * Columns (the size of the original matrix).  By original we mean
     * the start of the memory allocation.  This does not work at all
     * for memory not automatically allocated by the Matrix object, (ie
     * if you gave the allocated double array in the constructor).
     *
     * @param newRows  Rows will be set to this.
     * @param newColumns  Columns will be set to this.
     * @see Matrix::offset   
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
     * @see Matrix::read, @see Matrix::save
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
     * @see Matrix::write,@see Matrix::save
     */
    void   read(std::fstream &fs );
    /**
     * This prints the matrix to the screen display.  This is useful
     * for debug.
     */
    void print();
    /**
     * Calculate the determinate.  
     * 
     * @return the determinate
      */
    double determinant( void ) const ;
    /**
     * Calculate the trace.
     * @return the sum of the diagonal.
     */
    double trace()const;
    /**
     * Return the rank of the matrix
     * @return the rank of the matrix
     */
    int rank() const ;
    /**
     * For symetric Matricies only this can calculate the eigen
     * vectors and eigen values, this=ev*lamba*ev^T. If one needs the
     * eigen vectors more exactly one can set iterations to 1 (or 2).
     * This does not repeat the calculation but rather does an
     * iterative eigenvector projection a number of times (a Rows^3
     * operation).
     *
     * @param lambda the eigen values are returned along the diagonal in
     * decreasing absolute value order (the same order as ev).
     * @param ev the eigen vectors are the columns.  
     * @param iterations number of additional projections of the eigen
     * values.
     * @return 0 if Rows==Columns else 1.
     * @see Matrix::eigenFactor
     */
    int symmetricEigen(Matrix &lambda, Matrix &ev, int iterations=0);
    /**
     * For symmetric matricies only this can calculate the  
     * eigen values.  
     * 
     * @param lambda the eigen values are returned along the diagonal in 
     *        decreasing absolute value order.
     * @return 0 if Rows==Columns else 1. 
     * @see Matrix::eigenFactor 
     */
    int symmetricEigen(Matrix &lambda);
    /**
     * This returns absolute value of the eigenvalues (the singular values)
     * in w[Row].
     * @see Matrix::symmetricEigen, @see Matrix::eigenFactor.
     */  
    int  singularValues(double *w) const; 
    /**
     * This calculates the eigen vectors by a power method. It uses
     * repeated multiplication by the matrix.  Real eigenvalues only!
     * Imaginary Eigenvalues give meaningless results.  Symetric
     * Matricies have real eigenvalues.  This works best for well
     * spaced eigenvalues.
     *
     * @param ev The eigen vectors are returned as ev[Rows*i+j]
     * cooresponding to the values lambda[i];
     * @param lambda The eigen values in decending maginitude.   
     * @return 0 if happy, >0 indicates that it
     * failed to converge properly but may be approx correct anyhow.
     * This is normaly due to degenerate eigenvalues.  
     */
    //    int pca(double *ev, double *lambda);
    /**
     * This does the factoring A=h * B * h^T where B is hessenberg (it has
     * everything below the first subdiagonal zero). 
     * This starts as A and becomes B.  
     * @param h The orthogonal transformation of A to B.
     */
    void hessenberg(Matrix & h);
    /**
     * This is only for symetric matricies and 
     * does the factoring A=h * B * h^T where B is tridiagonal.  
     * This starts as A and becomes B.  
     * @param h The orthogonal transformation of A to B.
     */
    void symmetricHessenberg(Matrix & h);
    /**
     * This is only for symetric matricies and 
     * does the factoring A=h * B * h^T where B is tridiagonal.  
     * This starts as A and becomes B.  
     */
    void symmetricHessenberg();
    /**
     * Factor this into an uppertriangular matrix with the eigen values
     * on the diagonal multiplied right by an orthogonal matrix u and left 
     * by u^T.  This is the best one can hope for as eigen values/vectors
     * for a general matrix.
     * this = u*uppertri *u^T.
     * @param uppertri the tranformed result is returned here.
     * @param u the orthogonal tranformation is returned here.
     * @return 1 if not square else 0.
     * @see Matrix::symetricEigen 
     */
    int eigenFactor(Matrix &uppertri,Matrix &u) const;
    /**
     * Normalizes the columns to one so that (this.transpose())*this 
     * has one along the diagonal.
     */
    void normalize();
    /**
     * Does Grahm-Smit on the columns to one so that (this.transpose())*this=I 
     */
    void orthogonalize();
    /**
     * Projects out the eigenvectors by power method.
     * @param ev the eigen vector guess is in the columns in decending 
     *           (absolute value of) eigen value order.  This is then improved
     *           by multipling by this twice.
     * @param lambda if included the eigen values will be calculated for you.
     */
    void eigenize(Matrix &ev, Matrix * lambda=0);
    /**
     * Does This=QR
     * This becomes uppertriangle R.
     * This makes no requirement on the Rows and Columns.
     * 
     * @param q the orthogonal matrix Q that transformed R back to original 
     *  is multiplied by the initial q matrix like this q = q Q
     * @param startcol the if all rows/columns above startrow are known
     * to be upper-triangular then this hint will skip those columns
     */
    void triangle(Matrix & q, long startcol=0);
    /**
     * Does permuted(This)=QR This becomes a column permuted version of an
     * uppertriangle R.  This makes no requirement on the Rows and
     * Columns.
     * 
     * So at the end R=(*this)*P where P is a desired permutation of
     * the columns that is input by the permute parameter.(If q
     * initialially is the Rows x Rows Identity matrix then q at retun
     * is q=Q^T).
     * 
     * Does This*P=QR decomposition.
     * This becomes uppertriangle R*P^-1.
     * triangle(...) makes no requirement on the Rows and Columns.
     * 
     * @param q the orthonormal matrix Q is multiplied by the initial
     * q matrix like this, q = q Q^T.
     *
     * @param permute input the column permutation, so permute[j]
     * gives the column of this matrix that will be the jth column of
     * the trianglur matrix R.  The array is assumed to be as long as Columns.
     * So to get the (i,j)  element of R one does (*this)(i,permute[j])
     *
     * @param startcol assumes that all rows/columns above startrow
     * are known to be upper-triangular.  This hint will allow skiping
     * those columns.  Note this is a column of the permuted matrix R
     * (or (*this)*P) not this matrix.
     */
    void triangle(Matrix & q,long *permute, long startcol=0);
    /**
     * Does permuted(This)=QR This becomes a column permuted version of an
     * uppertriangle R.  This makes no requirement on the Rows and
     * Columns.
     * 
     * So at the end R=(*this)*P where P is a desired permutation of
     * the columns that is input by the permute parameter.(If q
     * initialially is the Rows x Rows Identity matrix then q at retun
     * is q=Q^T).
     * 
     * Does This*P=QR decomposition.
     * This becomes uppertriangle R*P^-1.
     * triangle(...) makes no requirement on the Rows and Columns.
     * 
     *  This allows for sparse matricies by including the rows and
     * columns lists.  So if there is a lot of 0's in this matrix
     * complexity can drop to as little as linear by using rows and
     * columns array lists
     * 
     *
     * @param q the orthonormal matrix Q is multiplied by the initial
     * q matrix like this, q = q Q^T.
     *
     * @param permute input the column permutation, so permute[j]
     * gives the column of this matrix that will be the jth column of
     * the trianglur matrix R.  The array is assumed to be as long as Columns.
     * So to get the (i,j)  element of R one does (*this)(i,permute[j])
     *
     * @param columns the columns array is supplied here.  The columns
     * array lists all the rows for each column that have a non =zero
     * element. It is an array with length Columns. Order of rows in
     * each column list is not important.
     * 
     * @param rows the rows array is given here.  The
     * rows array is the same but accross each row instead. 
     * 
     *
     * @param startcol assumes that all rows/columns above startrow
     * are known to be upper-triangular.  This hint will allow skiping
     * those columns.  Note this is a column of the permuted matrix R
     * (or (*this)*P) not this matrix.
     */
    void triangle(Matrix & q,long *permute,
		  LongArray & columns, 
		  LongArray &rows,
		  long startcol=0);
    /**
     * A helper method for triangle but can be used allone if matrix
     * is already in hessenberg form.  This applys the final rotation
     * to bring a Hessenberg matrix into upper triangular form.
     *
     * Does This=QR
     * This becomes uppertriangle R.
     * This makes no requirement on the Rows and Columns but assumes that
     * this is hessenberg form when called.
     * 
     * @param q the orthonormal matrix Q that transforms R back to original 
     *  is multiplied by the initial q matrix like this q = q Q^T
     * @param startcol the if all rows/columns above startrow are known
     * to be upper-triangular then this hint will skip those columns
     */
    void doQR(Matrix & q, long startcol=0);
     /**
      * A helper method for triangle but can be used allone if matrix
      * is already in hessenberg form.  This applys the final rotation
      * to bring a Hessenberg matrix into upper triangular form. Does
      * permuted(This)=QR.  This becomes a column permuted version of an
      * uppertriangle R.  This makes no requirement on the Rows and
      * Columns but assumes that this is hessenberg form when called.
      * * So at the end R=(*this)*P and Q^T=q where P is a desired *
      * permutation of the columns that is input by the permute *
      * parameter.
     * 
     * @param q the orthonormal matrix Q is multiplied by the initial
     * q matrix like this q_final = (q_init Q^T)
     *
     * @param permute input the column permutation, so permute[i]
     * gives the column of this matrix that will be the ith column of
     * the trianglur matrix R.
     *
     * @param startcol the if all rows/columns above startrow are known
     * to be upper-triangular then this hint will skip those columns
     */
    void doQR(Matrix & q, long *permute, long startcol=0);
  /**
      * A helper method for triangle but can be used allone if matrix
      * is already in hessenberg form.  This applys the final rotation
      * to bring a Hessenberg matrix into upper triangular form. Does
      * permuted(This)=QR.  This becomes a column permuted version of
      * an uppertriangle R.  This makes no requirement on the Rows and
      * Columns but assumes that this is hessenberg form when called.
      * * So at the end R=(*this)*P and Q^T=q where P is a desired *
      * permutation of the columns that is input by the permute *
      * parameter. This allows for sparse matricies by including the
      * rows and columns lists.
     * 
     * @param q the orthonormal matrix Q is multiplied by the initial
     * q matrix like this q_final = (q_init Q^T)
     *
     * @param permute input the column permutation, so permute[i]
     * gives the column of this matrix that will be the ith column of
     * the trianglur matrix R.
     *
     * @param columns the columns array is supplied here.  The columns
     * array lists all the rows for each column that have a non =zero
     * element. It is an array with length Columns. Order of rows in
     * each column list is not important.
     * 
     * @param rows the rows array is given here.  The
     * rows array is the same but accross each row instead. 
     * 
     * @param startcol the if all rows/columns above startrow are known
     * to be upper-triangular then this hint will skip those columns
     */
    void doQR(Matrix & q, long *permute, 
	      LongArray & columns, 
	      LongArray &rows,
	      long startcol=0);
    /**
     * This sets up the array's for triangle with sparse matricies.
     * The order of the lists is not specified.  This is a Row*Columns
     * complexity call but the constant is very small if this matrix
     * is mostly 0's.
     *
     * @param columns the columns array is returned here.  The columns
     * array lists all the rows for each column that have a non =zero
     * element. It is an array with length Columns. Order of rows in
     * each column list is not important.
     * 
     * @param rows the rows array is returned here.  The
     * rows array is the same but accross each row instead. 
     * 
     * @param num a hint of the number of non-zero elements across
     * each row and down each colum helps to speed up by allocating
     * blocks of memeory up front.
     */
    void makeSparseLists(LongArray & columns, 
			 LongArray & rows,
			 long num=100);
    /**
     * (this)x=rhs^T
     * Does the back substitution back to and including the block 'stopblk'.
     * Assums this to be upper triangle form
     *
     * @param x answer is returned here with dim rhs^T
     *
     * @param rhs can be a row vector  right hand side
     * 
     * @param stoprow the backsubstituion stops after
     * solving this row.  
     *
     * @param oneovermaxx will adjust x if |x(i,j)|>1/oneovermaxx==maxx 
     *  for some element of x. maxx the largest x element that you
     *  want to allow.  So the x will be maxx if answer of the equation
     * equals maxx and then maxx*(maxx/ansx) if ansx>maxx. (Avoid a
     * discontinuity)
     *
     * @return the lowest row of x that was unadjusted. So return
     * =stoprow indicates that the equation was completly solved with
     * no numerical problems.
     *
     */      
    long backSubstitute(Matrix &x, 
			const Matrix &rhs,
			double oneovermaxx=0,
			unsigned long stoprow=0);
    /**
     * This identical to backSubstitute except that it simple aborts
     * if the answer x has a value > 1/oneovermaxx.  It then returns the
     * last row that was solvable.
     *
     * Assums this to be upper triangle form so that (*this)(i,j)=0
     * for i>j.  One must have columns<=Rows or this will fail.
     *
     * (this)x=rhs^T
     * Does the back substitution back to and including the block 'stopblk'.
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
     * @return the highest row of x that was solved for. So return
     * =stopcol indicates that the equation was completly solved with
     * no numerical problems and return=Columns indicates nothing was solved.
     *
     */      
    unsigned long tryBackSubstitute(Matrix &x, 
                                    const Matrix &rhs,
                                    double oneovermaxx,
                                    unsigned long stoprow);
    /**
     * This is like tryBackSubstitute(Matrix &x, const Matrix &rhs,
     * double oneovermaxx, unsigned long stoprow) with the rows
     * permuted and the ability to treat the rightmost columns of the
     * permuted matrix, R, and th bottom rows of x as not part of this
     * (ie x(i,j) is assumed 0 for this rows i)
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
     * @param startrow the x values for this row and higher are
     * considered as given to be 0.
     *
     * @param permute the columns of this are permuted to give the
     * upper triangle matrix R as R(i,j)=(*this)(i,permute[j]), x rows
     * are similarly permuted.
     *
     * @return the highest row of P^-1 x that was solved for. So return
     * =stopcol indicates that the equation was completly solved with
     * no numerical problems and return=Columns indicates nothing was solved.
     *
     *
     */
    unsigned long
    tryBackSubstitute(Matrix &x, 
		      const Matrix &rhs,
		      double oneovermaxx,
		      unsigned long stoprow, 
		      unsigned long startrow,
		      long *permute);
    

    /**
     * This is like tryBackSubstitute(Matrix &x, const Matrix &rhs,
     * double oneovermaxx, unsigned long stoprow) with the rows
     * permuted and the ability to use the sparseness information in rows.
     *
     * @param x answer is returned here with dim rhs^T
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
     * upper triangle matrix R as R(i,j)=(*this)(i,permute[j]), x rows
     * are similarly permuted.
     *
     * @return the highest row of P^-1 x that was solved for. So return
     * =stopcol indicates that the equation was completly solved with
     * no numerical problems and return=Columns indicates nothing was solved.
     *
     *
     */
    unsigned long
    tryBackSubstitute(Matrix &x, 
		      const Matrix &rhs,
		      double oneovermaxx,
		      unsigned long stoprow, 
		      LongArray &rows,
		      long *permute);
   
    /**
     * solve xt(*this)=(rhs)^T @param columns row i of columns lists
     * the rows of this that are non-zero.
     *
     * @param permute a matrix defined by A(i,j)=*this(i,permute(j))
     * is upper triangular.
     *
     * @return last column of xt sucessfully solved for so Rows-1 if
     * total sucess.
     */
    long
    tryTransposedBackSubstitute(Matrix &xt, 
				const Matrix &rhs,
				double oneovermaxx,
				LongArray &columns,
				long *permute);
    /**
     * This identical to backSubstitute except that it moves any low
     * information columns to the right side and solves the smaller
     * remaining system.  So this always finds as much of the solution
     * as is possible given the information available. The remainder
     * of x is simple set to zero.
     *
     * This matrix is not assumed to be upper-triangular when this is
     * called but it is returned as an upper triangular matrix but
     * with the columns possibly rearranged.
     *
     * The equation solved is (this)x=rhs^T.
     * Does the back substitution back to and including the 'stoprow'.
     * Assumes this to be upper triangle form so that (*this)(i,j)=0 for i>j.
     * One must have columns<=Rows or this will fail.
     *
     *
     * So your code might look like this:
     *
     * T.triangle(rhs);
     * 
     * Matrix tempx;
     *
     * long columns[T.Columns];
     *
     * long dim=T.solveTo(tempx,columns,rhs);
     *
     * if (dim<0){//bad news, didn't work, matricies are wrong sizes}
     * 
     * x.reallocateZero(tempx.Rows,tempx.Columns)
     *
     * for (long i=0; i<dim;i++)
     *   
     *   for(long j=0;j<x.Columns;j++)
     *
     *     x(columns[i],j)=tempx(i,j);
     *
     * 
     *
     * Of course now T has its columns all mixed up if that matters.
     *
     * @param x answer is returned here with dim g^T
     *
     * @param columns An array of length Columns that will return with
     * columns[i]=the column of the original matrix that has now moved
     * to column i.  
     *
     * @param rhs can be a row vector  right hand side
     * 
     * @param oneovermaxx will test if |x(i,j)|>1/oneovermaxx==maxx
     * for some element of x. maxx the largest x element that you want
     * to allow.  This is the flag to check numerical stability.  It
     * forces the column to be moved to the rightmost of the columns
     * with the diagonal then checked against minsquarediagonal.
     *
     * @param minsquarediagonal the minimum diagonal term when the
     * other columns have been marginalized out and only the simple
     * ax=b equation is left for the tested row.  If this is a
     * measurment equation with (this)^T(this)= informationmatrix then
     * 1/minsquarediagonal is a minimum on the diagonal of the
     * covariance.  If this minimum is exceeded after the oneovermaxx
     * flag was thrown the column is considered low info and its x
     * element(s) set to zero.
     *
     * @param mininformation the minimum diagonal in the information
     * matrix.  This is strickly >= the square diagonal.  This
     * preliminary test will also throw the flag to move the column
     * right and check the diagonal element.  This can be used to
     * remove unobservable dimensions.  It will not remove dimensions
     * that were observable but only relative to some other
     * dimensions.
     *
     * @param stoprow the backsubstituion stops after solving this
     * row/col.
     *
     * @return the maximum column (row of x) that was solved for.
     * Rows of x above this are zero and had insufficient information
     * to solve. -1 if fails.
     */
    long solveTo(Matrix &x, long *columns,
                 Matrix &rhs, double oneovermaxx=.02,
                 double minsquarediagonal=.1,
                 double mininformation=10,
                 unsigned long stoprow=0);

    /**
     * This is the same as SolveTo, except for permuting the columns,
     * which identical to backSubstitute except that it moves any low
     * information columns to the right side and solves the smaller
     * remaining system.  So this always finds as much of the solution
     * as is possible given the information available. The remainder
     * of x is simple set to zero.
     *
     * So this does not move any matrix elements but instead changes permute.
     * SolveTo will, in contrast, move the columns and then tell the
     * caller what moved with columns.
     *
     * This matrix is not assumed to be upper-triangular when this is
     * called but it is returned as an upper triangular matrix but
     * with the columns possibly rearranged.
     *
     * The equation solved is (this)x=rhs^T.
     * Does the back substitution back to and including the 'stoprow'.
     * Assumes this to be upper triangle form so that (*this)(i,j)=0 for i>j.
     * One must have columns<=Rows or this will fail.
     *
     *
     * So your code might look like this:
     *
     * T.triangle(rhs);
     * 
     * Matrix tempx;
     *
     * long columns[T.Columns];
     *
     * long dim=T.solveTo(tempx,columns,rhs);
     *
     * if (dim<0){//bad news, didn't work, matricies are wrong sizes}
     * 
     * x.reallocateZero(tempx.Rows,tempx.Columns)
     *
     * for (long i=0; i<dim;i++)
     *   
     *   for(long j=0;j<x.Columns;j++)
     *
     *     x(columns[i],j)=tempx(i,j);
     *
     * 
     *
     * Of course now T has its columns all mixed up if that matters.
     *
     * @param x answer is returned here with dim g^T
     *
     * @param permute An array of length Columns that holds in
     * permute[i]=the column of the original matrix that has now moved
     * to column i.  
     *
     * @param rhs can be a row vector  right hand side
     * 
     * @param oneovermaxx will test if |x(i,j)|>1/oneovermaxx==maxx
     * for some element of x. maxx the largest x element that you want
     * to allow.  This is the flag to check numerical stability.  It
     * forces the column to be moved to the rightmost of the columns
     * with the diagonal then checked against minsquarediagonal.
     *
     * @param minsquarediagonal the minimum diagonal term when the
     * other columns have been marginalized out and only the simple
     * ax=b equation is left for the tested row.  If this is a
     * measurment equation with (this)^T(this)= informationmatrix then
     * 1/minsquarediagonal is a minimum on the diagonal of the
     * covariance.  If this minimum is exceeded after the oneovermaxx
     * flag was thrown the column is considered low info and its x
     * element(s) set to zero.
     *
     * @param mininformation the minimum diagonal in the information
     * matrix.  This is strickly >= the square diagonal.  This
     * preliminary test will also throw the flag to move the column
     * right and check the diagonal element.  This can be used to
     * remove unobservable dimensions.  It will not remove dimensions
     * that were observable but only relative to some other
     * dimensions.
     *
     * @param startrow the rows below this will be considered 'external' 
     * and x for them will be forced to be 0.
     *
     * @param stoprow the backsubstituion stops after solving this
     * row/col.
     *
     * @return the maximum column (row of x) that was solved for.
     * Rows of x above this are zero and had insufficient information
     * to solve. -1 if fails.
     */
    long solveToPermuted(Matrix &x, long *permute,
			 Matrix &rhs, double oneovermaxx=.02,
			 double minsquarediagonal=.1,
			 double mininformation=10,
			 long startrow=-1,
			 unsigned long stoprow=0);

    /**
     * Does This=QR
     * This becomes uppertriangle R This assumes Rows<=Columns.
     * 
     * @param q the orthonormal matrix Q that transformed R back to original 
     */
    void decompQR(Matrix & q);
    void decompQR();
    
    /**
     * Add two matricies and put the result in this matrix.
     * this= m1+m2.
     * @param m1 any matrix
     * @param m2 any matrix
     */
    void add(const Matrix m1,const Matrix m2){
      add_(m1,m2);
    }
    /**
     * Efficiently add two matricies and put the result in this matrix.
     * This is the better way to do it than, *this= m1+m2.  
     * @param m1 any matrix but not this.
     * @param m2 any matrix but not this.
     */
    void add_(const Matrix& m1,const Matrix& m2);
    /**
     * Subtract two matricies and put the result in this matrix.
     * this= m1-m2.
     * @param m1 any matrix
     * @param m2 any matrix
     */
    void subtract(const Matrix m1,const Matrix m2){
      subtract_(m1,m2);
    }      
    /**
     * Subtract two matricies and put the result in this matrix.
     * This is the better way to do it than,*this= m1-m2.
     * @param m1 any matrix but not this.
     * @param m2 any matrix but not this.
     */
    void subtract_(const Matrix& m1,const Matrix &m2);      
    /**
     * This matrix becomes equal to the product of m1*m2, changing size
     * if needed, (no need to call reallocate to set Rows and Columns
     * correctly before calling this).   Of course, m1 and m2 must be
     * compatable with matrix multply.
     *  
     * @param m1 any matrix
     * @param m2 any matrix
     */
    void multiply(const Matrix m1,const Matrix m2){
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
    void multiply_(const Matrix& m1,const Matrix& m2);
  
	 
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
    void multTranspose(const Matrix a,const Matrix b,const int which){
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
    void multTranspose_(const Matrix& a,const Matrix& b,const int which);
    /**
     * This is like multiply but puts in a minus sign.
     * This = -(a * b), changing
     * size if needed, (no need to call reallocate to set Rows and 
     * Columns correctly before calling this).
     *
     * @param a any matrix
     * @param b any matrix
     */
    void minusMult(const Matrix a,const Matrix b){
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
    void minusMult_(const Matrix& a,const Matrix& b);

    /**
     * Adds the product of two matricies to this matrix.
     * This += a * b.
     *
     * @param a any matrix
     * @param b any matrix
     */
    void addProduct(const Matrix a, const Matrix b){
      addProduct_(a, b);
    }
    /**
     * Adds the product of two matricies to this matrix.
     * Better than: *this += a * b.
     * @param a any matrix but not this.
     * @param b any matrix but not this.
     */
    void addProduct_(const Matrix& a, const Matrix& b);

    /**
     * Subtracts the product of two matricies from this matrix.
     * This -= a * b.
     *
     * @param a any matrix
     * @param b any matrix
     */
    void subtractProduct(const Matrix a,const Matrix b){
      subtractProduct_(a,b);
    }
    /**
     * Subtracts the product of two matricies from this matrix.
     * Better than: *this -= a * b.
     * @param a any matrix but not this.
     * @param b any matrix but not this.
     */
    void subtractProduct_(const Matrix& a,const Matrix& b);

    /**
     * Subtracts the product of a matrix and a double from this matrix.
     * *this -= d * b.
     * @param b any matrix but not this.
     * @param d any double number.
     */
    void subtractProduct(const Matrix b,const double d){
      subtractProduct_(b,d);
    }
    /**
     * Subtracts the product of a matrix and a double from this matrix.
     * Better than: *this -= d * b.
     * @param b any matrix but not this.
     * @param d any double number.
     */
    void subtractProduct_(const Matrix& b,const double d);
    /**
     * Adds the product of a matrix and a double from this matrix.
     * *this += d * b.
     * @param b any matrix.
     * @param d any double number.
     */
    void addProduct(const Matrix b,const double d){
      addProduct_(b,d);
    }
    /**
     * Adds the product of a matrix and a double from this matrix.
     * Better than: *this += d * b.
     * @param b any matrix but not this.
     * @param d any double number.
     */
    void addProduct_(const Matrix& b,const double d);
    /**
     * This gets a reference to a matrix element at (r,c).
     * @param r the row of the element
     * @param c the column.
     */
    double& operator() (const int r, const int c){
#if CUREMATRIXHHDEBUG >30
      if ((r<0)||(c<0))
	std::cerr<<"\nMatrix index below 0\n";
      if ((r>=Rows)&(c>=Columns))
	std::cerr<<"\nMatrix index above bounds\n";
#if  CUREMATRIXHHDEBUG >50
      if(AllocatedMatrix==Element)
	if ((r*RowInc+c)>=AllocatedSize){
	  std::cerr<<"\nERROR Matrix::(r,c) index above Allocated bounds\n";
	}
#if  CUREMATRIXHHDEBUG >100
      if ((AllocatedMatrix<=Element)&&((AllocatedMatrix+AllocatedSize)>Element))
	if ((Element+r*RowInc+c)>=AllocatedMatrix+AllocatedSize){
	  std::cerr<<"\nERROR Matrix index out of Allocated bounds\n";
	}
#endif
#endif
#endif
      return Element[r*RowInc+c];
        
    }
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
     * @param pe the symetric information matrix to be eliminated.
     * @param rn the symetric information to the absoulte reference frame
     * that is to be converted from a position between pe and this to a 
     * series coupling to this.
     * @return 1 if this is not square else 0.
     */
    int convertInformationT2L(Cure::Matrix &pe,
			       Cure::Matrix &rn);
  protected: 
    /**
     * This will conditionally allocate a new double array to AllocatedElement.
     * It allways sets Rows and Columns to r and c.  If the current allocation
     * is not large enough it will delete it and allocate a new array.
     * If it allocates a new array it sets all values to 0.  
     * If it does not allocate no values are changed.
     * @param r number of rows
     * @param c number of colunms
     */
    void setupElement(const int r, const int c);
    /**
     * Decompose matrix into QSP, where this becomes Q a Rows x Rows
     * orthogonal matrix, A is a Rows x Columns 'diagonal type matrix'
     * and P is an Columns x Columns orthogonal matrix.
     * A restriction of this implementation is that Rows>=Columns
     * 
     * @param w An array of length Columns for the singular values, the
     * squareroot of the diagonal elements of S.  
     * @param v An array of size Rows X Columns  
     */
    int svDecomposition(double *w, double *v); 

    int luDecomposition(int* v);
    void luBackSubst(int* indx, double *b);
    /**
     * This starts as A and becomes R upper triangular of A=qR with q orthogonal.
     * If A is not square it should have more columns than rows and R will be
     * zero below the diagonal.
     *  Be sure to do this before calling:
     *  q.reallocate(Rows);
     *  q=1;
     * 
     */
    void factorQR(Matrix & q);

    /**
     * This starts as A and becomes R upper triangular of A=qR with q
     * orthogonal.  If A is not square it should have more columns
     * than rows and R will be zero below the diagonal.  Be sure to do
     * this before calling:
     *
     *  q.reallocate(Rows);
     *  q=1;
     *
     *  @param q returns with the orthogonal matrix from R to A.  This
     *  is 0 below the first subdiagonal.
     *  
     *  @param qcum returns with qcum=qcum*q where qcum Columns are
     *  limited to q's Rows before multipling, then restored.  This is
     *  a bit faster than doing the multiplication yourself it uses
     *  the construction of q to optimize.
     *             
     */
    void symmetricQR(Matrix & q, Matrix &qcum);
    /**
     * This starts as A and becomes R upper triangular of A=qR with q
     * orthogonal.  If A is not square it should have more columns
     * than rows and R will be zero below the diagonal.  Be sure to do
     * this before calling:
     *  
     * q.reallocate(Rows);
     *  q=1;
     *
     *  @param q returns with the orthogonal matrix from R to A.  This
     *  is 0 below the first subdiagonal.
     */
    void symmetricQR(Matrix & q);
  private:
    bool inRange(int r, int c){
      return(((r<Rows)&&(r>=0))&&((c<Columns)&&(c>=0)));
      }
    void copy(Cure::Matrix & a);
    void move(const Matrix& mat);
  };
  inline void Matrix::reallocate(const int r,const int c){
    if (r==Rows)
      {
	if(c==Columns)return;
	if ((c<0)&&(Columns==Rows))return;  
      }
    setupElement(r,c);
  }
  inline void Matrix::reallocateZero(const int r,const int c){
    if (r==Rows)
      {
	if ((c==Columns)||((c<0)&&(Columns==Rows))){
	  (*this)=0;
	  return;
	}
      }
    setupElement(r,c);
  }
  
  inline void Matrix::operator = (const Matrix& mat) 
  {
    if (Columns!=mat.Columns || Rows!=mat.Rows )
      reallocate(mat.Rows,mat.Columns);
    MatrixStuff::matEqual(Element,mat.Element,Rows,Columns, RowInc,mat.RowInc);
  }
  inline void Matrix::move(const Matrix& mat) 
  {
    if (Columns!=mat.Columns || Rows!=mat.Rows )
      reallocate(mat.Rows,mat.Columns);
    long n=(Rows*Columns);
    if ((n>0)&&((RowInc==Columns)&&(mat.RowInc==mat.Columns)))
      memmove(Element,mat.Element,sizeof(double)*(n));
    else
      {
	long mati=0;
	n=(Rows*RowInc);
	if (Columns>0)
	  for (long i=0; i<n;i+=RowInc,mati+=mat.RowInc)
	    memmove(Element+i,mat.Element+mati,sizeof(double)*(Columns));
      }
  }
  inline void Matrix::multiply_(const Matrix& a,const Matrix& b)
  {
    reallocate(a.Rows,b.Columns);
#if CUREMATRIXHHDEBUG >20
    if (a.Columns!=b.Rows)
      {
	std::cerr<<"\nERROR Matrix::multiply_ INCOMPATABLE\n";
	return;
      }
#endif 
    MatrixStuff::matMult(Element,a.Element,b.Element, Rows,
	    a.Columns,Columns,RowInc,a.RowInc,b.RowInc);  
  }
  inline void Matrix::add_(const Matrix& a,const Matrix& b)
  {
#if CUREMATRIXHHDEBUG >20
    if (( b.Columns!=a.Columns) || (b.Rows!=a.Rows ))
      {
	std::cerr << "\nERROR Incompatible matrices for add_\n";
	return;
      }
#endif 
    reallocate(a.Rows,a.Columns);
    MatrixStuff::matAdd(Element,a.Element,b.Element,
	   Rows,Columns,RowInc,a.RowInc,b.RowInc);  
  
  }
  inline void Matrix::subtract_(const Matrix& a,const Matrix& b)
  {
    
#if CUREMATRIXHHDEBUG >20
    if (( b.Columns!=a.Columns) || (b.Rows!=a.Rows ))
      {
	std::cerr << "\nERROR Incompatible matrices for subtract_\n";
	return;
      }
#endif 
    reallocate(a.Rows,a.Columns);
    MatrixStuff::matSubtract(Element,a.Element,b.Element,
		Rows,Columns,RowInc,a.RowInc,b.RowInc);  
  }
  inline void Matrix::offset(int rowOffset, int columnOffset, int newRows, 
			     int newColumns){
#if  CUREMATRIXHHDEBUG >100
    if ((AllocatedMatrix<=Element)&&((AllocatedMatrix+AllocatedSize)>Element)){
      if ((Element+(((rowOffset+newRows-1)*RowInc)+(columnOffset+newColumns-1)))
	  >=AllocatedMatrix+AllocatedSize)
	if ((newColumns>0)&&(newRows>0))
	  {
	    std::cerr<<"\nMatrix::offset index above bounds\n";
	  }
      if ((Element+(((rowOffset)*RowInc)+(columnOffset)))
	  <AllocatedMatrix){
	std::cerr<<"\nMatrix::offset index below bounds\n";
      }
    }
#endif
    Element+=((rowOffset*RowInc)+columnOffset);
    Rows=newRows;
    Columns=newColumns;
  }
  inline void Matrix::offset(int columnOffset, int newRows, 
			     int newColumns){
#if  CUREMATRIXHHDEBUG >100
    if ((AllocatedMatrix<=Element)&&((AllocatedMatrix+AllocatedSize)>Element)){
      if ((Element+(((newRows-1)*RowInc)+(columnOffset+newColumns-1)))
	  >=AllocatedMatrix+AllocatedSize)
	if ((newColumns>0)&&(newRows>0))
	  {
	    std::cerr<<"\nMatrix::offset index sbove bounds\n";
	  }
      if ((Element+columnOffset)<AllocatedMatrix){
	std::cerr<<"\nMatrix::offset index below bounds\n";
      }
    }
#endif
    Element+=(columnOffset);
    Rows=newRows;
    Columns=newColumns;
  }
  inline  void Matrix::offset(int rowColOffset, int newRowsCols){
#if  CUREMATRIXHHDEBUG >100
    if ((AllocatedMatrix<=Element)&&((AllocatedMatrix+AllocatedSize)>Element)){
      if ((Element+(((rowColOffset+newRowsCols-1)*(RowInc+1))))
	  >=(AllocatedMatrix+AllocatedSize))
	if (newRowsCols)
	  {
	    std::cerr<<"\nMatrix::offset index out of bounds\n";
	  }
      if ((Element+(((rowColOffset)*(RowInc+1))))
	  <AllocatedMatrix){
	std::cerr<<"\nMatrix::offset index out of bounds\n";
      }
    }
#endif
    Element+=(rowColOffset*(RowInc+1));
    Rows=newRowsCols;
    Columns =newRowsCols;
  }
  inline  void Matrix::reset(int newRows,int newColumns){
#if  CUREMATRIXHHDEBUG >40
    if(newRows*RowInc>AllocatedSize)
      if ((newColumns>0)&&(newRows>0))
	{
	  std::cerr<<"\nMatrix::reset index above bounds\n";
	}
#endif
    Element=AllocatedMatrix;
    Rows=newRows;
    Columns=newColumns;
  }
  inline void   Matrix::write(std::fstream &fs ){
    int top=Rows*RowInc;
    fs<<"\n"<<Rows<<" "<<Columns<<"\n"; 
    for (int i=0;i<top; i+=RowInc){
      for (int j=0;j<Columns; j++)
	fs<<Element[i+j]<<" ";
      fs<<"\n";     
    }
  }
  inline void Matrix::read(std::fstream &fs ){
    int r=0;
    int c=0;
    fs>>r>>c;
    reallocate(r,c);
    r=Rows*RowInc;
    for (int i=0;i<r; i+=RowInc)
      for (int j=0;j<Columns; j++)
	fs>>Element[i+j];
  }
  inline  void Matrix::copy(Matrix &a)
  {
    a.AllocatedMatrix=AllocatedMatrix;
    a.AllocatedSize=AllocatedSize;
    a.Element=Element;
    a.Rows=Rows;
    a.Columns=Columns;
    a.RowInc=RowInc;
  }

   
} // namespace Cure;

std::ostream& operator<<(std::ostream& os, const Cure::Matrix& m);

#endif // CURE_MATRIX_HH
