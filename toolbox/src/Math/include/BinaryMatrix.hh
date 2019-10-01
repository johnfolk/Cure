// = AUTHOR(S)
//    John Folkesson
//    
// = COPYRIGHT
//    Copyright (c) 2005 John Folkesson
//    

#ifndef CURE_BINARYMATRIX_HH
#define CURE_BINARYMATRIX_HH
#include "ShortMatrix.hh"
#include "Matrix.hh"
#include <math.h>

namespace Cure {
   /**
   * One can set this to integers from 0 (no debug tests) to 200 (All tests)
   * More tests make slower code.
   * This changes this header file so all code that links to this 
   * need to be recompiled if this is changed.  It has to be changed here as
   * this line will redefine it.   *
   * There is a similar constant for the cc file with the property that 
   * it can be changed when (re)compiling Matrix.cc and one should not 
   * have to recompiled all linked files. 
   *
   * The debug messages seem to be the best thing about using this class
   * as they warn for all sorts of out-of-bounds situations that commonly
   * lead to seg fault.
   */
#define CUREBINARYMATRIXHHDEBUG 200

  /**
   * The BinaryMatrix class will, simplify code with matrices of
   * binaries, give valuable debug messages and make binary operation
   * easier.  One can rely on the BinaryMatrix class to organize
   * allocation of memory and avoiding segmentation faults.  Instead
   * of the seg fault one gets a specific error message that usually
   * helps pin point the bug quickly.
   * 
   * One can code with the BinaryMatrix class and avoid messy for
   * loops and trivial but treacherous index calculations.  One can
   * have the BinaryMatrix allocate its own array (and delete it
   * automatically).  
   * 
   * One can temporarily setBit the matrix to refer to only a sub-matrix
   * of the full array.  So that a block can be used to do all the matrix
   * operations.
   * @see BinaryMatrix::offset, @see BinaryMatrix::reset
   *
   * If one needs very efficient operations one should try to
   * chose Columns and Column offsets to be multiplies of 32.  This
   * will allow most operations to be done using bit operations on longs.
   *
   * Particularly ineffiecient is a long column vector (Columns=1).
   * which allocates a long for each binary value and masks out 31 of
   * the 32 bits each time.
   *
   * @author John Folkesson
   */
  class BinaryMatrix
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
  protected:
    /**
     * The number of rows from the start of the allocated memory to
     * the current matrix start.
     */
    long  CurrentRowOffset;  
    /**
     * The number of columns from the start of the allocated memory to
     * the current matrix start column.
     */
    long  CurrentColOffset;  
    /**
     * The number of columns of longs in the memory allocation.  This
     * transforms the linear memory into a 2D array.  RowInc must be
     * equal or larger than Columns/32+((Columns%32)>0).
     */
    long  RowInc;  
    /**
     * This will store the pointer to the memory allocation.
     */  
    unsigned long *AllocatedMatrix;
    /**
     * This is the size of the long array pointed to by AllocatedMatrix.
     * It is needed to generate error messages when offset is out-of-bounds.
     */
    unsigned long AllocatedSize;

    /**
     * This is the number of rows of the long array pointed to by
     * AllocatedMatrix.  It is needed to generate error messages when
     * offset is out-of-bounds.
     */
    unsigned long AllocatedRows;
    /**
     * This is the number of columns originally allocated to
     * AllocatedMatrix.  It is needed to generate error messages when
     * offset is out-of-bounds.
     */
    unsigned long AllocatedColumns;

  public:
    /**
     * Create a matrix with 0 rows and columns.
     */
    BinaryMatrix();
    /**
     * Create a matrix that allocates its own memory and is equal to the 
     * currently defined matrix b.  (ie. it can be that b has been
     * set to refer to only a sub-matrix of its full allocated array.
     * In that case only the sub-matrix will be allocated in the new matrix.)
     * @param b a matrix to be used as the initial values.   
     */
    BinaryMatrix(const BinaryMatrix& b);
  
    /**
     * This will allocate its own array (default is c=r).
     * All initial values are set to 0.
     * @param r the number of rows to allocate.
     * @param c the number of columns (default c=r).
     */ 
    BinaryMatrix(const long r,const long c=-1, bool initvalue=false);
    /**
     * This will delete arrays allocated by the BinaryMatrix. 
     * 
     */
    virtual ~BinaryMatrix();
    /**
     * The = operator allocates its own memory if needed and sets it
     * equal to the currently defined matrix b.  (ie. it can be that
     * b has been set to refer to only a submatrix of its full
     * allocated array.  In that case only the sub-matrix will be
     * allocated in the new matrix.)  
     *
     * @param b a matrix to be used as the initial values.
     */
    void operator = (const BinaryMatrix& b);
    /**
     * This set all elements of the matrix to  d 
     *  
     * @param d the value for the elements.
     */  
    void operator = (const bool d);
    /**
     * This gets a copy of a matrix element at (r,c).
     * @param r the row of the element
     * @param c the column.
     */
    bool operator() (const long r, const long c) const { 
     unsigned long ac=c+CurrentColOffset;
      if ((c<0)||(ac>=AllocatedColumns)){
	std::cerr<<"ERROR BinaryMatrix::(r,c) c="<<c<<" is out of bounds\n";
	return 0;
      }
      unsigned long rw=(r+CurrentRowOffset);
      if ((r<0)|(rw>=AllocatedRows)){
	std::cerr<<"ERROR BinaryMatrix::(r,c) r="<<r<<" is out of bounds\n";
	return 0;
      }
      unsigned short  bitcol=(ac&0x1F);
      ac=(ac>>5);
      unsigned long a=AllocatedMatrix[rw*RowInc+ac];
      a=(a>>bitcol);
      a=(a&1);
      return (bool)a;
    }
    /**
     * Test if two matricies are equal
     * @param mat the matrix to check againat this.
     * @return true if all the bits are the same.
     */
    bool operator == (const BinaryMatrix& mat)const;
 
    /**
     * Test if one set of true bits is a subset of
     * the other.
     * @param mat the matrix to check againat this.
     * @return true if all the bits are the same.
     */
    bool operator >= (const BinaryMatrix& mat)const;
    /**
     * Test if one set of true bits is a subset of
     * the other.
     * @param mat the matrix to check againat this.
     * @return true if all the bits are the same.
     */
    bool operator <= (const BinaryMatrix& mat)const;
    /**
     * Test if one set of true bits is a proper subset of
     * the other.
     * @param mat the matrix to check againat this.
     * @return true if all the bits are the same.
     */
    bool operator > (const BinaryMatrix& mat)const;
    /**
     * Test if one set of true bits is a proper subset of
     * the other.
     * @param mat the matrix to check againat this.
     * @return true if all the bits are the same.
     */
    bool operator < (const BinaryMatrix& mat)const;
    /**
     * Test if one set of true bits is different than
     * the other.
     * @param mat the matrix to check againat this.
     * @return true if all the bits are the same.
     */
    bool operator != (const BinaryMatrix& mat)const;
    /**
     * Does a logical not on every bit of the the matrix.
     */
    void operator ~(); 
    /**
     * Does a logical not on every bit of the the matrix.
     */
    void equalsNot(BinaryMatrix & bm);
    /**
     * This treats the row as a binary number and returns true if
     * the number on row is greater than the number on first row of mat.
     */
    bool rowGreaterThan(unsigned long row, const BinaryMatrix &mat)const;
    bool rowLessThan(unsigned long row, const BinaryMatrix &mat)const;
    bool rowEqualTo(unsigned long row, const BinaryMatrix &mat)const;
    /**
     *     
     * This treats the row as a binary unsigned number with msb as
     * the highest column.  
     *
     * @return 1 if row if <. -1 if row if >, 0 of row ==, else 3 if
     * wrong size.
     */
    short rowTest(unsigned long row, const BinaryMatrix &mat)const;
    /**
     * This treats each row as a binary unsigned number with msb as
     * the highest column.  It then puts the first row of sig in as a new row
     * in increasing order with no duplicate rows.  
     *
     * @return true if a row was added, that is sig did not already exists as
     * a row of this.
     **/
    bool addOrdered(Cure::BinaryMatrix &sig);
    /**
     * This treats each row as a binary unsigned number with msb as
     * the highest column.  It then finds the row that the first row
     * of sig should be inserted into.  
     * 
     *
     * @return false if sig not already on the list else truw if row already 
     *  holds a copy of sig.
     **/
    bool  findRow(long &row,Cure::BinaryMatrix &sig);
    /** 
     * Returns the bit position in the unsigned long cooresponding to
     * element (r,c).  So 0 means it is the least significant bit of
     * the longElement and 31 means it is the msb.  This method is not
     * typically called by most users.  The details of the way that
     * the bits are stored on is that an unsigned long array is
     * allocated and the indiviual bits are accessed by the bit
     * operations in BinaryMatrix::setBit and
     * BinaryMatrix::operator(). This is a helper for setBit.
     *
     * @param r the row index of the binary element
     * @param c the column index of the binary element
     * @param longElement returns with the pointer to the long that binary 
     *       element (r,c)
     *        is within.     
     */
    unsigned short bitColumn(const long r, const long c,
			     unsigned long ** longElement);
    /**
     * This setBits a bool to a bit at position (r,c).
     *
     * @param r the row index of the binary element.
     * @param c the column index of the binary element.
     * @param value the value to setBit to the bit(r,c).
     *       
     */
    void setBit(const long r, const long c,const bool value);
    /**
     * This setBits a bool to a bit at position (r,c) and
     * returns the previous bit stored there.
     *
     * @param r the row index of the binary element.
     * @param c the column index of the binary element.
     * @param value the value to setBit to the bit(r,c).
     * @return the previous value of the bit.      
     */
    bool checkSet(const long r, const long c,const bool value);
    /**
     * Returns the bit position in the unsigned long cooresponding
     * to element (r,c).  So 0 means it is the least significant bit
     * of the long and 31 means it is the msb.
     *
     * @param r the row index of the binary element
     * @param c the column index of the binary element
     * @see BinaryMatrix::bitColumn
     */
    unsigned short bitColumn(const long r, const long c)const;
    /**
     * The bitwise or between each element in a BinaaryMatrix b and this
     * matrix is stored in this matrix.
     *
     * @param b the values to do bitwise or with.
     */
    void operator |= (const BinaryMatrix & b);
    /**
     * The bitwise xor between each element in a BinaaryMatrix b and this
     * matrix is stored in this matrix.
     *
     * @param b the values to do bitwise or with.
     */
    void operator ^= (const BinaryMatrix & b);

    /**
     * The bitwise and between each element in a BinaaryMatrix b and this
     * matrix is stored in this matrix.
     *
     * @param b the values to do bitwise or with.
     */
    void operator &= (const BinaryMatrix& b);
    /**
     * This adds this matrix's cells (1 or 0) to m.
     * The result is stored in m.
     * @param m the matrix to add to
     * @return true if m is same size else false.
     */
    bool  addTo(ShortMatrix &m);
    /**
     * This adds not this matrix's cells ~(1 or 0) to m.
     * The result is stored in m.  So add 1 for cells that are false
     * and o for cells that are true.
     * @param m the matrix to add to
     * @return true if m is same size else false.
     */
    bool  addNotTo(ShortMatrix &m);
    /**
     * Counts the number of bits set.
     * @return the number of set bits.
     */
    long count();
    /**
     * Counts the number of bits that don't match.
     * @return the number of bits that do not match.
     */
    long match(BinaryMatrix &mat);
    /**
     * This will not work for Rows or Columns > 32767.
     * Returns the distance to the nearest true bit.
     * 
     * A point from each cluster that is within resolution of the
     * nearest is added to the list indexes.
     *
     * @param r the row index of the.
     * @param c the column index.
     * @param indexes the index row and columns of the found points.
     * @param maxdistance only points closer than this will be returned.
     * @param mindistance only points further than this will be returned.
     * @return the distance or -1 if r c out of range.
     */
    double nearest(int r, int c, ShortMatrix &indexes, double maxdistance,
		   double mindistance=0);
    /**
     * This will not work for Rows or Columns > 32767.
     *  This finds the nearest point in a given direction;
     * @param r the row index of the.
     * @param c the column index.
     * @param indexes the index row and columns of the found point.
     * @param direction a unit vector in the search direction (dr,dc).
     * @param maxdistance only points closer than this will be returned.
     * @param mindistance only points further than this will be returned.
     * @return the distance. 
     */
    double nearest(int r, int c, ShortMatrix &indexes,double direction[2],
		   double maxdistance,
		   double mindistance=0);
    /**
     * This extracts the parts of this gird that ar on the boundary of
     * regions of ones.  Thus the bound mat will have a one in every
     * cell that this matririx has a one that is adjacent to a 0.
     *
     * @param boundmat the matrix of the boundary regions is returned here.
     * @param adjacency 4 or 8 depending on how many adjacent cells to use.
     */
    void  boundary(BinaryMatrix & boundmat,short adjacency=4);
    /**
     * This will check if there is any intersection between the 
     * set bits in two matricies.
     * @return true if any cell is true in both in mat and this 
     */
    bool logicAnd(BinaryMatrix &mat);
    /**
     * This will fill a disk of radius with value in mat for every
     * cell in this that equals value. 
     * @param mat the matrix will be forced to the size of this matrix.
     * @param radius the radius of the disk in cells.
     * @param value the value to fill the cells with.
     */
    void growInto(BinaryMatrix &mat,double radius,bool value=true);
    /**
     * Fills all cells that are empty and 4 neighbors to a filled cell
     * starting by filling a seed.  
     * This will not work for Rows or Columns > 32767.
     * 
     * @param r the row of the seed.
     * @param c the cloulmn of the seed
     * @return the number of cells filled.
     */
    int cluster(int r,int c);
   
    /**
     * This will fill a disk of radius with value around cell (r,c)
     * @param r the row
     * @param c the column
     * @param radius the radius of the disk in cells.
     * @param value the value to fill the cells with.
     */
    void fillDisk(long r,long c,double radius, bool value);
    /**
     * This becomes equal to the or of this with the exclusive or of
     * a and b. this=(this)|((a^b))
     * @param a a matrix.
     * @param b a matrix.
     */
    void  orExor(BinaryMatrix &a,BinaryMatrix &b);
    /**
     * This becomes equal to the or of this with the and of
     * a and b. this=(this)|((a&b))
     * @param a a matrix.
     * @param b a matrix.
     */
    void  orAnd(BinaryMatrix &a,BinaryMatrix &b);
     /**
     * This matrix is transposed.
     * @see BinaryMatrix::transpose_;
     */
    void transpose(void);
    /**
     * This matrix becomes equal to the transpose of a.
     * @param a the matrix.  
     */
    void transpose(const BinaryMatrix a);
    /**
     * This matrix becomes equal to the transpose of a.  
     *   
     * @param a Here a should not be this matrix.
     */
    void transpose_(const BinaryMatrix & a);
  
     /**
     * The reallocate method allows you to change the array memory
     * allocation.  If the Rows=r and Columns are 'correct' this
     * method does nothing.  It does not copy any values.  The default
     * (if you only specify r), is Columns=Rows.  If a new memory
     * allocation is needed all values will be set to initvalue.
     *
     * @param r the number of rows.
     * @param c the number of coulmns (default c=r).
     * @param initvalue if new memory is allocated this is used.
     * @see BinaryMatrix::grow, @see BinaryMatrix::offset, 
     * @see BinaryMatrix::reset  
     */
   void reallocate(const long r,const long c=-1,bool initvalue=false);
    /**
     * The grow method allows you to extend the array memory beyond the 
     * originally allocation, or even copy it to a smaller
     * allocation freeing memory. 
     * 
     * It allocates a new array and copies the matrix elements from the 
     * original allocation to the new one in the approriate
     * row and column (starting from (0,0) being the same)
     * The default (if you only specify r), is Columns=Rows.
     *
     *   
     * @param r the number of rows for the extended matrix.
     * @param c the number of columns for the extended matrix,default is c=r.
     * @param value the value to fill insert rows/columns with 
     * @see BinaryMatrix::reallocate, @see BinaryMatrix::offset, 
     * @see BinaryMatrix::reset  
     */
    void grow(const long r,const long c=-1, bool value=false);
    /**
     * Copy the values in rows>r up one row and then subtract 1 from
     * Rows. 
     * @see BinaryMatrix::grow, @see BinaryMatrix::offset, 
     * @see BinaryMatrix::reset  
     */
    void deleteRow(long r);
    /**
     * Copy the values in columns>c to the column to the left and then 
     * subtract 1 from Columns.
     * @see BinaryMatrix::grow, @see BinaryMatrix::offset, 
     * @see BinaryMatrix::reset  
     */
    void deleteColumn(long c);
    /**
     * Moves the data to the left and changes the size of the matrix
     * @param startcolumn the first column to delete 
     * @param numberofcolumns the total number of columns to delete 
     *
     * @see BinaryMatrix::grow, @see BinaryMatrix::offset, 
     * @see BinaryMatrix::reset  
     */
    void deleteColumns(long startcolumn, long numberofcolumns );
    void deleteRows(long startrow, long numberofrows );
    /**
     * This actually creates a new double array and deletes the old
     * one.  This makes this rather heavy and not to be called
     * without really needing this.
     * Moves the data to the right and changes the size of the matrix.
     * the inserted columns have 0's set.
     * @param startcolumn the place to insert columns
     * @param numberofcolumns the total number of columns to insert 
     * @param value the value to fill insert columns with 
     * @see BinaryMatrix::grow, @see BinaryMatrix::offset, 
     * @see BinaryMatrix::reset  
     */
    void insertColumns(long startcolumn, long numberofcolumns, bool value=false );
    /**
     * This actually creates a new long array and deletes the old
     * one.  This makes this rather heavy and not to be called
     * without really needing this.
     * Moves the data to the down and changes the size of the matrix.
     * the inserted rows have 0's set.
     * @param startrow the place to insert row  
     * @param numberofrows the total number of rowss to insert
     * @param value the value to fill insert rows with  
     *
     * @see BinaryMatrix::grow, @see BinaryMatrix::offset, 
     * @see BinaryMatrix::reset  
     */
    void insertRows(long startrow, long numberofrowss,bool value=false);
    /**
     * Copy the values from row r1 to r2 and visa versa.
     * @param r1 a row of the matrix.
     * @param r2 another row.
     * @see BinaryMatrix::grow, @see BinaryMatrix::offset, 
     * @see BinaryMatrix::reset  
     */
    void swapRows(long r1,long r2);
    /**
     * Copy the values from column c1 to c2 and visa versa.
     * @param c1 a column of the matrix.
     * @param c2 another column.
     */
    void swapColumns(long c1,long c2);
    /**
     * This will setBit the matrix to refer to a submatrix of the full
     * allocation.  Changes the upper left corner to point to a
     * submatrix and changes the size (Rows and Columns).  Use this to
     * do operations on blocks of your matrix.  This allows you to
     * then use all the code for the BinaryMatrix class on just a
     * block.  You must then reset the matrix to the original size
     * when finished.  Of course unexpected things will happen if you
     * call methods that adjust the size of the matrix.  If the matrix
     * Rows and Colums is compatable before calling any method in this
     * class then the size will not need to be adjusted and all is
     * well.  If you miss calculate the arguments here you will most
     * likely get error messages about it during execution.  You can
     * not setBit to anything outside the current memory allocation.
     * The offsets are relative to the current matrix (0,0) element.
     *
     * @param rowOffset The BinaryMatrix will start at a point offset
     * by this many Rows from the current upper left corner.
     * @param columnOffset The BinaryMatrix will start at a point
     * offset by this many Columns from the current upper left corner.
     * @param newRows  Rows will be set to this.
     * @param newColumns  Columns will be set to this.
     * @see BinaryMatrix::reset
     */
    void offset(long rowOffset, long columnOffset, long newRows, 
		long newColumns);
    /**
     * Change the (0,0) to point back to the original upper
     * left corner and change the Rows and Columns.  This can be used to
     * restore the matrix after calling offset and doing some block
     * operations.  The caller will have to have remembered the Rows and
     * Columns (the size of the original matrix).  By original we mean
     * the start of the memory allocation. 
     *
     * @param newRows  Rows will be set to this.
     * @param newColumns  Columns will be set to this.
     * @see BinaryMatrix::offset   
     */
    void reset(long newRows,long newColumns);
    /**
     * Change the (0,0) to point back to the original upper
     * left corner and change the Rows and Columns back to originally
     * allocated values.  This can be used to restore the matrix after
     * calling offset and doing some block operations.  By original we
     * mean the start of the memory allocation.
     * 
     *
     * @see BinaryMatrix::offset   
     */
    void reset();
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
     * @see BinaryMatrix::read, @see BinaryMatrix::save
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
     * @see BinaryMatrix::write,@see BinaryMatrix::save
     */
    void   read(std::fstream &fs );
    /**
     * This prints the matrix to the screen display.  This is useful
     * for debug.
     */
    void print();
    /**
     * Performs a rotation of bm counter-clockwise by angle in
     * degrees.  The size is adjusted to include all of bm.  The parts
     * that the rotated bm does not cover are set to (!which).  The
     * origin has the translation of the origin. So the origin will
     * need to shift by an amount.  You give the value you want in
     * origin as an input parameter.  This amount will be shifted by
     * an integer number of rows and columns in order to make the
     * original matrix all fit in the new one with positive row and
     * column indexes.  The fractional offset will howver match the
     * input value of origin.  The actual row and column that the
     * (0,0) of the original matrix ended up in is returned in origin
     *  
     *
     * @param angle the angle in radians.
     * @param origin the new row and column of the origin is minus the
     * numbers returned here, (this is minus where (0,0) ends up).  
     * @param  which sets whether true:to rotate the ones (on a field of zeros)
     * or false: rotate the zeros.
     * @param extent 3 sets any cell that is covered by any part of the 
     *        rotated cell. 2 is a little less and 0 is minimal.
     */
    void rotate(double angle, double origin[2],	const bool which=true, 
		const unsigned short extent=3)
    {
      BinaryMatrix bm;
      copy(bm);
      Rows=0;
      Columns=0;
      AllocatedSize=0;
      AllocatedMatrix=0;
      rotate(bm,angle,origin,which,extent);
    }

    /**
     * Performs a rotation of bm counter-clockwise by angle in degrees.
     *
     * @param angle the angle in DEGREES.
     */
    void rotate(int angle);
    /**
     * This removes all empty rows from the top down and the bottom up.
     * @param which if false it removes empty (all false) rows if true
     * it removes all full rows.
     * @return the number of rows removed from row zero to the first
     * non empty row.
     */
    long trimRows(bool which=false);
    
    /**
     * This removes all empty columns from the left and the right.
     * @param which if false it removes empty (all false) columns if true
     * it removes all full columns.
     * @return the number of columns removed from column zero to the first
     * non empty column.
     */
    long trimColumns(bool which=false);
    /**
     * This will not work for Rows or Columns > 32767.
     * This will set to o all bits within this polygon.
     * @param verticies A nx2 matrix with the verticies of the polygon 
     *        traversed in a counter-clockwise sense.
     */
    void clearPolygon(ShortMatrix &verticies);
    /**
     * This will not work for Rows or Columns > 32767.
     * This will set to 1 all bits around the edge of this polygon.
     * It can handle any shape as long as the edges do not cross one another.
     * @param verticies A nx2 matrix with the verticies of the polygon 
     *        traversed in a counter-clockwise sense.
     */
    void setPolygon(ShortMatrix &verticies);

    /**
     * This will set to value all bits along an arc of this radius center
     * and start and end angles.
     *
     *
     * @param center The center pixel of the arc.
     * @param radius The radius of the arc.
     * @param startangle the radian from the x axis (x is the row index).
     * @param endangle the radian from the x axis (x is the row index).
     * @param thickness the arc will be set from radius to radius+thickness 
     */
    void setArc(double center[2], double radius, 
		double startangle, double endangle, bool value=true, double thickness=0);
    
    /**
     * This will set to 1 all bits along an arc of this radius center
     * and start and end angles.  The center can be above or below the
     * the plane, so its like a cone with these angles intersecting a
     * plane.  So take a sphere draw a circle on its surface and then
     * intersect it with a plane that is z=-center[2] distance above the
     * center of the sphere.  The plane has z-axis as normal.  The cone 
     * has its axis perpendicular to the z-axis.and the point at center.
     * So the cone axis can not be tilted up or down but rather is horizontal.
     * 
     * Its a sonar beam with solid angle endangle-startangle pointing
     * parellel to this plane but the sonar is below the plane.  
     * 
     *
     *
     * @param center The center pixel of the arc.
     * @param radius The radius of the arc.
     * @param startangle the radian from the x axis (x is the row index).
     * @param endangle the radian from the x axis (x is the row index).
     */
    void setHarc(double center[3], double radius, 
		double startangle, double endangle);
  
    /**
     * This will set to 1 all bits within an arc of this radius center
     * and start and end angles, a pie slice.
     * It will do this by growing inwards from the outside of the 
     * sector so if some cells in the sector are already cleared it will
     * not pass them. 
     * M&=temp;
     *
     * @param center The center pixel of the arc.
     * @param radius The radius of the arc.
     * @param startangle the radian from the x axis (x is the row index).
     * @param endangle the radian from the x axis (x is the row index).
     */
    void setSector(double center[2], double radius, 
		     double startangle, double endangle);

    /**
     * This will set to 1 all bits within an area of this radius center
     * and start and end angles.
     * The center can be above or below the plane of this.
     *
     *      
     * BinaryMatrix temp(M.Rows,M.Columns,true);
     *
     *
     * M&=temp;
     *
     * @see BinaryMatrix::setHarc
     * @param center The center pixel of the 'cone'.
     * @param radius The radius of the arc.
     * @param startangle the radian from the x axis (x is the row index).
     * @param endangle the radian from the x axis (x is the row index).
     */
    void setHsection(double center[3], double radius, 
		       double startangle, double endangle);
    /**
     * This will set to 0 all bits within an arc of this radius center
     * and start and end angles, a pie slice.
     * @param center The center pixel of the arc.
     * @param radius The radius of the arc.
     * @param startangle the radian from the x axis (x is the row index).
     * @param endangle the radian from the x axis (x is the row index).
     */
    void clearSector(double center[2], double radius, 
		     double startangle, double endangle);

    /**
     * This will set to 0 all bits within an area of this radius center
     * and start and end angles.
     * The center can be above or below the plane of this.
     *
     * @see BinaryMatrix::setHarc
     * @param center The center pixel of the 'cone'.
     * @param radius The radius of the arc.
     * @param startangle the radian from the x axis (x is the row index).
     * @param endangle the radian from the x axis (x is the row index).
     */
    void clearHsection(double center[3], double radius, 
		       double startangle, double endangle);
    /**
     * 
     * This will set to 'value' all bits out to this radius from  the center
     * and at this angle.
     *
     * @param center The base of the ray.
     * @param radius The length of the ray.
     * @param angle the radian from the x axis (x is the row index).
     * @param value the value to set the ray to.
     * @param minradius the distance from center to start seting cells
     */
    void setRay(double center[2], 
		double radius, 
		double angle, 
		bool value, double minradius=0);
    /**
     * Test if an arc crosses a filled cell.
     *
     * @param velocity (linearvelocity,angularvelocity, timelength).
     * @param r start cell row.
     * @param c start cell column.
     * @param startangle the angle with the x axis (increasing rows)
     * @return false if no cell along arc is true. else true
     */
    bool testArc(Cure::Matrix &velocity,
		 long r,long c,double startangle);

   /**
    * This will not work for Rows or Columns > 32767.
    * Does a search in the grid filling in neighborhood with ones
    * for all cells visited.  Will not move thru any cells that are filled
    * in this matrix. This calculates the actual path distances.
    * @param cells called with the seed in first row and returns with all the
    * cells in the neighborhod, (across each row format: row, column)
    * @param distancelimit the search will stop at this limiting distance.
    * @param neighborhood returns with 1 in every cell that can be reach 
    * with a path of distancelimit or less. 
    * @param boundary ones where obstacles were encountered 
    * @param mindistance the row and column of the closest obstacle
    * @param celldistance returns with the distance from the seed to each
    * of the cells in neighborhood.
    * @return the minimum distance to obstacles or -1 if none
    */
    double distanceSearch(Cure::ShortMatrix &cells,
                          double distancelimit, 
                          BinaryMatrix &neighborhood,
                          Cure::BinaryMatrix &boundary,
                          short mindistance[2],
                          Cure::Matrix & celldistance);
    /**
     * This will not work for Rows or Columns > 32767.
     * The iterative step for distanceSearch.
     * @param cells the list of cells grown so far in format r,c 
     * 
     * @next the index of the current growth cell into the cells matrix 
     * 
     * @visited  It will end with cells filled.
     * @boundary the filled cells
     * from this Matrix that have been encountered during growth
     * @param celldistance returns with the distance from the seed to each
     * of the cells in neighborhood.
     * @return 0 if no more growth possible;
     *        -1 if no obstacle encounterd
     *        1 if obstacle 
     */
    int distanceSearchstep(Cure::ShortMatrix &cells, int next, 
                           BinaryMatrix &visited,
                           Cure::BinaryMatrix &boundary, 
                           Cure::Matrix & celldistance);
    /**
     * This will not work for Rows or Columns > 32767.
     * This grows a cluster of adjacent cells from a seed.
     *
     * This is meant to be called iteratively to fill an entire region
     * @param cells the queue of cells to be called next far in format r,c
     * starts with the seed and has the new cells added.
     * @visited returns with 1 in every cell adjacent to 
     * (cells(0,0),cells(0,1)) that is not set in this. 
     * @param top If the growth is to move diagonally set this to 8 else 4 
     * @return 0 if no more growth possible;
     *         1 otherwise
     */
    int clusterGrowth(Cure::ShortMatrix &cells, 
		      BinaryMatrix &visited, int top);

    /**
     * This will not work for Rows or Columns > 32767.
     * This grows a cluster from a seed to fill a whole enclosed region
     * with no concern for path distance.  
     * This is meant to be called iteratively to fill an entire region
     * @param cells the queue of cells to be called next far in format r,c
     * starts with the seed.
     * @visited returns with 1 in every cell in cells 
     * 
     * @param eightneighbor true if the growth is to move diagonally.
     * @param r the row for the seed @param c the column for the seed
     * @param if >0 the cluster will grow to this maximum distance
     * from the seed
     * @return number of cells filled.
     */
    int clusterGrowth( BinaryMatrix &visited, bool eightneighbor, 
			int r, int c,
			double distancelimit=-1);
    /**
     * This will not work for Rows or Columns > 32767.
     * Finds the shortest path. Beware of huge time dependance on searchlimit.
     * @param r1 the row index of the start point.
     * @param c1 the column index of the start point.
     * @param r2 the row index of the end point.
     * @param c2 the column index of the end point.
     * @path the path indexes are returned here with first row
     *       =(r1,c1) and last row = (r2,c2)
     * @searchlimit paths further than this will not be found
     * @return -1 if path not found else path distance
     */
    double path(short r1,short c1,short r2,short c2,
                Cure::ShortMatrix &path, double searchlimit);
    /**
     * This will not work for Rows or Columns > 32767.
     * The iterative step for pathSearch.  celldis ahould be 0 for all
     * unvisited cells and >0 for visited ones
     * @param cells the list of cells grown so far in format r,c 
     * @next the index of the current growth cell into the cells matrix 
     * @param celldis returns with the distance from the seed to each
     * of the cells in neighborhood.
     * @param celldist2 the cell distances from the other end of the path,
     * @param minindex the r,c of the current cell that joins the two celldis
     * implied paths.
     * @param mindist the minimum path distance between the two ends is 
     * maintained here =celldis(minindex(0,0),minindex(0,1))+
     *                  celldis2(minindex(0,0),minindex(0,1)).
     * @return 0 if no more growth possible;
     *         1 else
     */
    int pathSearchstep(Cure::ShortMatrix &cells, int next, 
		       Cure::Matrix &celldis,
		       Cure::Matrix &celldis2,
		       Cure::ShortMatrix &minindex,
		       double &mindist);
    /**
     * Find if an adjacent bit is set.
     * @param r the row
     * @param c ths column.
     * @param neighbors if 4 only checks 4 adjancency
     * @return true if any adjacent bit has value.
     */
    bool adjacentBit(long r,long c,short neighbors=4,bool value=true);    
    /**Return true if r,c are in the rance of this Matrix*/
    bool inRange(long r, long c){
      return(((r<Rows)&&(r>=0))&&((c<Columns)&&(c>=0)));
      }
  protected: 
    /**
     * Performs a rotation of bm counter-clockwise by angle in degrees.
     * The center point of the new matrix cooresponds to the center of bm.
     * The size is adjusted to include all of bm.
     * The parts that the rotated bm does not cover are set to 0.
     *
     * @param bm the matrix to be rotated.
     * @param a the angle in rads.
     * @param  which sets whether to rotate the ones (on a field of zeros)
     * or the zeros.
     * @param extent 3 sets any cell that is covered by any part of the 
     *        rotated cell. 2 is a little less and 0 is minimal.
     */
    void rotate(BinaryMatrix &bm, const double a, double origin[2],
		const bool which=true, 
		const unsigned short extent=3);
    /**
     * This will not work for Rows or Columns > 32767.
     * Propogate distance change in cell r,c thoughout celldis.
     */
    void   checkdis(Cure::Matrix &celldis,int r,int c);
    /**
     * This will not work for Rows or Columns > 32767.
     * Propogate distance change in cell r,c thoughout celldis and
     * maintain the shortest path in mindist and the common index in minindex.
     */
    void   checkdis(Cure::Matrix &celldis,int r,int c,
		    Cure::Matrix &celldis2,
		    Cure::ShortMatrix &minindex,
		    double &mindist);
    int cluster(ShortMatrix &clusters,ShortMatrix &temp);
    unsigned short  getTop()const;
    unsigned long  getIndex(unsigned long &top,
			     unsigned long &last) const;
      
  /** 
     * Returns the bit position in the unsigned long cooresponding to
     * element (r,c).  So 0 means it is the least significant bit of
     * the longElement and 31 means it is the msb.  This method is not
     * typically called by most users.  The details of the way that
     * the bits are stored on is that an unsigned long array is
     * allocated and the indiviual bits are accessed by the bit
     * operations in BinaryMatrix::setBit and
     * BinaryMatrix::operator(). This is a helper for setBit.
     *
     * @param r the row index of the binary element
     * @param c the column index of the binary element
     * @param longElement returns the offset to the long that binary element (r,c)
     *        is within.     
     */
    unsigned short bitColumnOffset(const long r, const long c,
				   unsigned long *longElement) const ;
      /**
     * This will conditionally allocate a new unsigned long array to
     * AllocatedMatrix.  It allways sets Rows and Columns to r and c.
     * If the current allocation is not large enough it will delete it
     * and allocate a new array.  If it allocates a new array it sets
     * all values to 0.  If it does not allocate no values are
     * changed.  
     * @param r number of rows 
     * @param c number of colunms
     */
    void setupElement(const long r, const long c, bool initvalue=false);
  private:
    bool clearRow(int step, BinaryMatrix &visitup, 
		  BinaryMatrix &visitdown,int r, int c);
    bool  setVisit(int step, BinaryMatrix &visitup, 
		   BinaryMatrix &visitdown,int r, int c, bool f=true);
    void move(const BinaryMatrix& mat);
    void copy(BinaryMatrix &cm);
  };
  inline void BinaryMatrix::reallocate(const long r,const long c, bool initvalue){
    if (r==Rows)
      {
	if(c==Columns)return;
	if ((c<0)&&(Columns==Rows))return;  
      }
    setupElement(r,c,initvalue);
  }
  
  inline void BinaryMatrix::operator = (const BinaryMatrix& mat) 
  { 
    if ((Columns!=mat.Columns)||( Rows!=mat.Rows))
      reallocate(mat.Rows,mat.Columns);
    if ((mat.Columns==0)||(mat.Rows==0))return;
    unsigned long top,last;
    unsigned long endcol=getIndex(top,last);
    if ((top==mat.getTop())){
      unsigned long ac=CurrentColOffset+top;
      ac=(ac>>5);
      unsigned long bc=mat.CurrentColOffset+top;
      bc=(bc>>5);
      unsigned long *a=&AllocatedMatrix[(CurrentRowOffset*RowInc)+ac];
      unsigned long b=((mat.CurrentRowOffset*mat.RowInc)+bc);
      for (long i=0; i<Rows; i++){
	for (long j=0; j<(long)top; j++) 
	  setBit(i,j,mat(i,j));
	if (endcol>0)
	  memcpy(a+i*RowInc,mat.AllocatedMatrix+b+i*mat.RowInc,
		 (sizeof(unsigned long)*(endcol)));
	for (long j=last; j<Columns; j++) 
	  setBit(i,j,mat(i,j));
      }
    }
  else
    for (long i=0; i<Rows; i++)
      for (long j=0; j<Columns; j++) 
	setBit(i,j,mat(i,j));
  }
  inline void BinaryMatrix::move(const BinaryMatrix& mat) 
  {
    if ((Columns!=mat.Columns)||( Rows!=mat.Rows))
      reallocate(mat.Rows,mat.Columns);
    unsigned long top,last;
    unsigned long endcol=getIndex(top,last);
    if ((top==mat.getTop())){
      unsigned long ac=CurrentColOffset+top;
      ac=(ac>>5);
      unsigned long bc=mat.CurrentColOffset+top;
      bc=(bc>>5);
      unsigned long *a=&AllocatedMatrix[(CurrentRowOffset*RowInc)+ac];
      unsigned long b=((mat.CurrentRowOffset*mat.RowInc)+bc);
      for (int i=0; i<Rows; i++){
	for (long j=0; j<(long)top; j++) 
	  setBit(i,j,mat(i,j));
	if (endcol>0)
	  memmove(a+i*RowInc,mat.AllocatedMatrix+b+i*mat.RowInc,
		  (sizeof(unsigned long)*(endcol)));
	for (long j=last; j<Columns; j++) 
	  setBit(i,j,mat(i,j));
      }
    }
    else
      for (long i=0; i<Rows; i++)
	for (long j=0; j<Columns; j++) 
	  setBit(i,j,mat(i,j));
  }
  inline void BinaryMatrix::operator &= (const BinaryMatrix& mat)
  {
#if CUREBINARYMATRIXHHDEBUG >20
    if ((Columns!=mat.Columns)||(Rows!=mat.Rows))
      {
	std::cerr<<"\nERROR BinaryMatrix::&= INCOMPATABLE\n";
	return;
      }
#endif 
    if ((Rows==0)||(Columns==0))return;
    unsigned long top,last;
    unsigned long endcol=getIndex(top,last);
    if ((top==mat.getTop())){
      unsigned long ac=CurrentColOffset+top;
      ac=(ac>>5);
      unsigned long bc=mat.CurrentColOffset+top;
      bc=(bc>>5);
      unsigned long *a=&AllocatedMatrix[(CurrentRowOffset*RowInc)+ac];
      unsigned long b=((mat.CurrentRowOffset*mat.RowInc)+bc);
      for (int i=0; i<Rows; i++){
	for (long j=0; j<(long)top; j++) 
	  setBit(i,j,((*this)(i,j)&(mat(i,j))));
	for (long j=0; j<(long)endcol; j++)
	  a[i*RowInc+j]=(a[i*RowInc+j]&
			 (mat.AllocatedMatrix[b+i*mat.RowInc+j]));
	for (long j=last; j<Columns; j++) 
	  setBit(i,j,((*this)(i,j)&(mat(i,j))));
      }
    }
    else
      for (long i=0; i<Rows; i++)
	for (long j=0; j<Columns; j++) 
	  setBit(i,j,((*this)(i,j)&(mat(i,j))));
    
  }
  inline void BinaryMatrix::operator |= (const BinaryMatrix& mat)
  {
#if CUREBINARYMATRIXHHDEBUG  >20
    if ((Columns!=mat.Columns)||(Rows!=mat.Rows))
      {
	std::cerr<<"\nERROR BinaryMatrix::|= INCOMPATABLE\n";
	return;
      }
#endif 
    if ((Rows==0)||(Columns==0))return;
    unsigned long top,last;
    unsigned long endcol=getIndex(top,last);
    if ((top==mat.getTop())){
      unsigned long ac=CurrentColOffset+top;
      ac=(ac>>5);
      unsigned long bc=mat.CurrentColOffset+top;
      bc=(bc>>5);
      unsigned long *a=&AllocatedMatrix[(CurrentRowOffset*RowInc)+ac];
      unsigned long b=((mat.CurrentRowOffset*mat.RowInc)+bc);
      for (long i=0; i<Rows; i++){
	for (long j=0; j<(long)top; j++) 
	  setBit(i,j,((*this)(i,j)|(mat(i,j))));
	for (long j=0; j<(long)endcol; j++)
	  a[i*RowInc+j]=(a[i*RowInc+j]|
			 (mat.AllocatedMatrix[b+i*mat.RowInc+j]));
	for (long j=last; j<Columns; j++) 
	  setBit(i,j,((*this)(i,j)|(mat(i,j))));
      }
    }
    else
      for (long i=0; i<Rows; i++)
	for (long j=0; j<Columns; j++) 
	  setBit(i,j,((*this)(i,j)|(mat(i,j))));
    
  }
  inline bool  BinaryMatrix::addTo(ShortMatrix &mat){
#if CUREBINARYMATRIXHHDEBUG  >20
    if ((Columns!=mat.Columns)||(Rows!=mat.Rows))
      {
	std::cerr<<"\nERROR BinaryMatrix::|= INCOMPATABLE\n";
	return false;
      }
#endif 
    if ((Columns==0))return true;
    for (long r=0; r<Rows; r++){
      long rw=(r+CurrentRowOffset);
      rw*=RowInc;
      for (long c=0; c<Columns;) {
	unsigned long ac=c+CurrentColOffset;
	unsigned short  bitcol=(ac&0x1F);
	ac=(ac>>5);
	unsigned long a=AllocatedMatrix[rw+ac];
	a=(a>>bitcol);
	unsigned short top=32-bitcol;
	top+=c;
	if (top>Columns)top=Columns;
	while (c<top){
	  if (a&1) mat(r,c)++;
	  a=(a>>1);
	  c++;
	}
      }      
  
    }
    return true;  
  }
  inline bool  BinaryMatrix::addNotTo(ShortMatrix &mat){
#if CUREBINARYMATRIXHHDEBUG  >20
    if ((Columns!=mat.Columns)||(Rows!=mat.Rows))
      {
	std::cerr<<"\nERROR BinaryMatrix::|= INCOMPATABLE\n";
	return false;
      }
#endif 
    if ((Columns==0))return true;
    for (long r=0; r<Rows; r++){
      long rw=(r+CurrentRowOffset);
      rw*=RowInc;
      for (long c=0; c<Columns;) {
	unsigned long ac=c+CurrentColOffset;
	unsigned short  bitcol=(ac&0x1F);
	ac=(ac>>5);
	unsigned long a=AllocatedMatrix[rw+ac];
	a=(a>>bitcol);
	unsigned short top=32-bitcol;
	top+=c;
	if (top>Columns)top=Columns;
	while (c<top){
	  if (!(a&1)) mat(r,c)++;
	  a=(a>>1);
	  c++;
	}
      }      
   }
    return true;
  }
  inline short BinaryMatrix::rowTest(unsigned long row, 
				     const BinaryMatrix &mat)const{
    if ((Columns!=mat.Columns)||(1>mat.Rows)||(Rows<=(long)row))
      return 3;
    unsigned long top,last;
    unsigned long endcol=getIndex(top,last);
    if ((top==mat.getTop())){
      unsigned long ac=CurrentColOffset+top;
      ac=(ac>>5);
      unsigned long bc=mat.CurrentColOffset+top;
      bc=(bc>>5);
      unsigned long *a=&AllocatedMatrix[((CurrentRowOffset+row)*RowInc)+ac];
      unsigned long *b=&mat.AllocatedMatrix[(mat.CurrentRowOffset*mat.RowInc)+bc];
      for (long j=Columns-1; j>=(long)last; j--) 
	if ((*this)(row,j)&&(!mat(0,j))) return -1;
	else if (!(*this)(row,j)&&(mat(0,j))) return 1;
      for (long j=endcol-1;j>=0; j--)
	if ((a[j]>b[j]))return -1;
   	else  if (a[j]<b[j])return 1;
      for (long j=top-1;j>=0;  j--) 
	if ((*this)(row,j)&&(!mat(0,j))) return -1;
	else if (!(*this)(row,j)&&(mat(0,j))) return 1;
    } else {
      for (long j=Columns-1; j>=0; j--) 
	if ((*this)(row,j)&&(!mat(0,j))) return -1;
	else if (!(*this)(row,j)&&(mat(0,j))) return 1;
    }
    return 0;
  }



  inline bool BinaryMatrix::rowGreaterThan(unsigned long row, 
					   const BinaryMatrix &mat)const{
    if ((Columns!=mat.Columns)||(1>mat.Rows)||(Rows<=(long)row))
      return false;
    unsigned long top,last;
    unsigned long endcol=getIndex(top,last);
    if ((top==mat.getTop())){
      unsigned long ac=CurrentColOffset+top;
      ac=(ac>>5);
      unsigned long bc=mat.CurrentColOffset+top;
      bc=(bc>>5);
      unsigned long *a=&AllocatedMatrix[(CurrentRowOffset*RowInc)+ac];
      unsigned long b=((mat.CurrentRowOffset*mat.RowInc)+bc);
      for (long j=0; j<(long)top; j++) 
	if ((*this)(row,j)&&(!mat(0,j))) return true;
	else if (!(*this)(row,j)&&(mat(0,j))) return false;
      for (long j=0; j<(long)endcol; j++)
	if ((a[row*RowInc+j]>
	     (mat.AllocatedMatrix[b+j])))return true;
	else  if ((a[row*RowInc+j]<
		   (mat.AllocatedMatrix[b+j])))return false;
      for (long j=last; j<Columns; j++) 
	if ((*this)(row,j)&&(!mat(0,j))) return true;
	else if (!(*this)(row,j)&&(mat(0,j))) return false;
    } else {
      for (long i=0; i<Rows; i++)
	for (long j=0; j<Columns; j++) 
	  if ((*this)(row,j)&&(!mat(0,j))) return true;
	  else if (!(*this)(row,j)&&(mat(0,j))) return false;
    }
    return false;
  }
  inline bool BinaryMatrix::rowLessThan(unsigned long row, 
					   const BinaryMatrix &mat)const{
    if ((Columns!=mat.Columns)||(1>mat.Rows)||(Rows<=(long)row))
      return false;
    unsigned long top,last;
    unsigned long endcol=getIndex(top,last);
    if ((top==mat.getTop())){
      unsigned long ac=CurrentColOffset+top;
      ac=(ac>>5);
      unsigned long bc=mat.CurrentColOffset+top;
      bc=(bc>>5);
      unsigned long *a=&AllocatedMatrix[(CurrentRowOffset*RowInc)+ac];
      unsigned long b=((mat.CurrentRowOffset*mat.RowInc)+bc);
      for (long j=0; j<(long)top; j++) 
	if ((*this)(row,j)&&(!mat(0,j))) return false;
	else if (!(*this)(row,j)&&(mat(0,j))) return true;
      for (long j=0; j<(long)endcol; j++)
	if ((a[row*RowInc+j]>
	     (mat.AllocatedMatrix[b+j])))return false;
	else  if ((a[row*RowInc+j]<
		   (mat.AllocatedMatrix[b+j])))return true;
      for (long j=last; j<Columns; j++) 
	if ((*this)(row,j)&&(!mat(0,j))) return false;
	else if (!(*this)(row,j)&&(mat(0,j))) return true;
    } else {
      for (long i=0; i<Rows; i++)
	for (long j=0; j<Columns; j++) 
	  if ((*this)(row,j)&&(!mat(0,j))) return false;
	  else if (!(*this)(row,j)&&(mat(0,j))) return true;
    }
    return false;
  }
  inline bool BinaryMatrix::rowEqualTo(unsigned long row, 
					   const BinaryMatrix &mat)const{
    if ((Columns!=mat.Columns)||(1>mat.Rows)||(Rows<=(long)row))
      return false;
    unsigned long top,last;
    unsigned long endcol=getIndex(top,last);
    if ((top==mat.getTop())){
      unsigned long ac=CurrentColOffset+top;
      ac=(ac>>5);
      unsigned long bc=mat.CurrentColOffset+top;
      bc=(bc>>5);
      unsigned long *a=&AllocatedMatrix[(CurrentRowOffset*RowInc)+ac];
      unsigned long b=((mat.CurrentRowOffset*mat.RowInc)+bc);
      for (long j=0; j<(long)top; j++) 
	if ((*this)(row,j)&&(!mat(0,j))) return false;
	else if (!(*this)(row,j)&&(mat(0,j))) return false;
      for (long j=0; j<(long)endcol; j++)
	if ((a[row*RowInc+j]!=
	     (mat.AllocatedMatrix[b+j])))return false;
      for (long j=last; j<Columns; j++) 
	if ((*this)(row,j)&&(!mat(0,j))) return false;
	else if (!(*this)(row,j)&&(mat(0,j))) return false;
    } else {
      for (long i=0; i<Rows; i++)
	for (long j=0; j<Columns; j++) 
	  if ((*this)(row,j)&&(!mat(0,j))) return false;
	  else if (!(*this)(row,j)&&(mat(0,j))) return false;
    }
    return true;
  }
  inline bool BinaryMatrix::operator == (const BinaryMatrix& mat)const{
    if ((Columns!=mat.Columns)||(Rows!=mat.Rows))
      return false;
    unsigned long top,last;
    unsigned long endcol=getIndex(top,last);
    if ((top==mat.getTop())){
      unsigned long ac=CurrentColOffset+top;
      ac=(ac>>5);
      unsigned long bc=mat.CurrentColOffset+top;
      bc=(bc>>5);
      unsigned long *a=&AllocatedMatrix[(CurrentRowOffset*RowInc)+ac];
      unsigned long b=((mat.CurrentRowOffset*mat.RowInc)+bc);
      for (long i=0; i<Rows; i++){
	for (long j=0; j<(long)top; j++) 
	  if((*this)(i,j)^(mat(i,j))) return false;
	for (long j=0; j<(long)endcol; j++)
	 if ((a[i*RowInc+j]^
	      (mat.AllocatedMatrix[b+i*mat.RowInc+j])))return false;
	for (long j=last; j<Columns; j++) 
	  if((*this)(i,j)^(mat(i,j)))return false;
      }
    }
    else
      for (long i=0; i<Rows; i++)
	for (long j=0; j<Columns; j++) 
	  if(((*this)(i,j)^(mat(i,j))))return false;    
    return true;
  }
  inline bool BinaryMatrix::operator != (const BinaryMatrix& mat)const{
    return (!((*this)==mat));
  }
  inline bool BinaryMatrix::operator >= (const BinaryMatrix& mat)const{
    BinaryMatrix bm=mat;
    bm&=(*this);
    return (bm==mat);
  }
  inline bool BinaryMatrix::operator <= (const BinaryMatrix& mat)const{
    BinaryMatrix bm=mat;
    bm&=(*this);
    return (bm==(*this));
  }
  inline bool BinaryMatrix::operator < (const BinaryMatrix& mat)const{
    return (mat!=(*this))&&(mat<=(*this));
  }
  inline bool BinaryMatrix::operator > (const BinaryMatrix& mat)const{
    return (mat!=(*this))&&(mat>=(*this));
  }

  inline void BinaryMatrix::operator ^= (const BinaryMatrix& mat)
  {
#if CUREBINARYMATRIXHHDEBUG  >20
    if ((Columns!=mat.Columns)||(Rows!=mat.Rows))
      {
	std::cerr<<"\nERROR BinaryMatrix::^= INCOMPATABLE\n";
	return;
      }
#endif 
    unsigned long top,last;
    unsigned long endcol=getIndex(top,last);
    if ((top==mat.getTop())){
      unsigned long ac=CurrentColOffset+top;
      ac=(ac>>5);
      unsigned long bc=mat.CurrentColOffset+top;
      bc=(bc>>5);
      unsigned long *a=&AllocatedMatrix[(CurrentRowOffset*RowInc)+ac];
      unsigned long b=((mat.CurrentRowOffset*mat.RowInc)+bc);
      for (long i=0; i<Rows; i++){
	for (long j=0; j<(long)top; j++) 
	  setBit(i,j,((*this)(i,j)^(mat(i,j))));
	for (long j=0; j<(long)endcol; j++)
	  a[i*RowInc+j]=(a[i*RowInc+j]^
			 (mat.AllocatedMatrix[b+i*mat.RowInc+j]));
	for (long j=last; j<Columns; j++) 
	  setBit(i,j,((*this)(i,j)^(mat(i,j))));
      }
    }
    else
      for (long i=0; i<Rows; i++)
	for (long j=0; j<Columns; j++) 
	  setBit(i,j,((*this)(i,j)^(mat(i,j))));    
  }

inline unsigned long  BinaryMatrix::getIndex(unsigned long &top,
			       unsigned long &last) const
  {
    if ((Columns<1)||(Rows<1))
      {
	top=0;
	last=0;
	return 0;
      }
    unsigned short bitcol=bitColumn(0,0);
    top=32-bitcol; 
    if ((bitcol==0)&&(Columns>31))
      top=0;
    if (Columns<(long)top)
      {  
	top=Columns;
	last=top;
      }
    else {
      unsigned short ebitcol=bitColumn(0,Columns-1)+1;
      if (ebitcol==32){
	ebitcol=0;
      }
      last=Columns-ebitcol;
    }
    return ((last-top)>>5);
  }
  inline unsigned short  BinaryMatrix::getTop()const
  {
    if ((Columns<1)||(Rows==0))
      return 0;
    unsigned short top=32-bitColumn(0,0);
    if ((top==32)&&(Columns>31))
      return 0;
    if (Columns<top)
      return (unsigned short)Columns;
    return top;
  }
  inline void BinaryMatrix::operator ~()
  {
  unsigned long top,last;
  unsigned long endcol=getIndex(top,last);
  unsigned long ac=CurrentColOffset+top;
  ac=(ac>>5);
  unsigned long *a=&AllocatedMatrix[(CurrentRowOffset*RowInc)+ac];
  for (long i=0; i<Rows; i++){
    for (long j=0; j<(long)top; j++) 
      setBit(i,j,!(*this)(i,j));
    for (long j=0; j<(long)endcol; j++)
      a[i*RowInc+j]=~(a[i*RowInc+j]);	  
    for (long j=last; j<Columns; j++) 
      setBit(i,j,!(*this)(i,j));
  }
  }
  
  inline void BinaryMatrix::offset(long rowOffset, 
				   long columnOffset, 
				   long newRows, 
				   long newColumns){
    long ro=rowOffset+CurrentRowOffset;
    long co=columnOffset+CurrentColOffset;
#if  CUREBINARYMATRIXHHDEBUG  >100
    unsigned long ec=(co+newColumns);
    if (((long)AllocatedRows<(ro+newRows))||(ec>AllocatedColumns))
      if ((newColumns>0)&&(newRows>0)){
	std::cerr<<"\nBinaryMatrix::offset index above bounds\n";
	}
    if((co<0)|(ro<0)){
      std::cerr<<"\nBinaryMatrix::offset index below bounds\n";
    }
    if (newRows<0){
      std::cerr<<"\nBinaryMatrix::offset Rows set to negative\n";
      newRows=0;
    }
    if (newColumns<0){
      std::cerr<<"\nBinaryMatrix::offset Columns set to negative\n";
      newColumns=0;
    }
#endif
    CurrentRowOffset=ro;
    CurrentColOffset=co;
    Rows=newRows;
    Columns=newColumns;
  }
  inline  void BinaryMatrix::reset(long newRows,long newColumns){
#if  CUREBINARYMATRIXHHDEBUG >40
    if ((newRows>(long)AllocatedRows)||(newColumns>(long)AllocatedColumns))
      if ((newColumns>0)&&(newRows>0))
	  std::cerr<<"\nBinaryMatrix::reset index above bounds\n";
#endif
    CurrentRowOffset=0;
    CurrentColOffset=0;
    Rows=newRows;
    Columns=newColumns;
  }
  inline  void BinaryMatrix::reset(){
    CurrentRowOffset=0;
    CurrentColOffset=0;
    Rows=AllocatedRows;
    Columns=AllocatedColumns;
  }
  inline void   BinaryMatrix::write(std::fstream &fs ){
    fs<<"\n"<<Rows<<" "<<Columns<<"\n"; 
    for (long i=0;i<Rows; i++){
      for (long j=0;j<Columns; j++)
	fs<<(*this)(i,j);
      fs<<"\n";     
    }
  }
  inline void BinaryMatrix::read(std::fstream &fs ){
    int r=0;
    int c=0;
    fs>>r>>c;
    reallocate(r,c);
    bool b;
    for (long i=0;i<Rows; i++)
      for (long j=0;j<Columns; j++)
	{
	  fs>>b;
	  setBit(i,j,b);
	}
  }
  inline unsigned short BinaryMatrix::bitColumn(const long r, const long c,
					       unsigned long ** longElement)
  {
    longElement[0]=0;
    unsigned long ac=c+CurrentColOffset;
    if ((c<0)||(ac>=AllocatedColumns)){
      std::cerr<<"ERROR BinaryMatrix::bitcolumn c="<<c
	       <<" is out of bounds\n";
	return 64;
    }
    long rw=(r+CurrentRowOffset);
    if ((r<0)||(rw>=(long)AllocatedRows)){
      std::cerr<<"ERROR BinaryMatrix::bitcolumn r="<<r<<" is out of bounds\n";
          return 64;
      }
    unsigned short  bitcol=(ac&0x1F);
    ac=(ac>>5);
    longElement[0]=&AllocatedMatrix[(rw*RowInc)+ac];
    return bitcol;
  }
  inline unsigned short BinaryMatrix::bitColumnOffset(const long r, 
						      const long c,
						      unsigned long 
						      *longElement) const
  {
    longElement[0] =0;
    unsigned long ac=c+CurrentColOffset;
    if ((c<0)||(ac>=AllocatedColumns)){
      std::cerr<<"ERROR BinaryMatrix::bitcolumnOffset c="<<c
	       <<" is out of bounds\n";
	return 64;
    }
    long rw=(r+CurrentRowOffset);
    if ((r<0)||(rw>=(long)AllocatedRows)){
      std::cerr<<"ERROR BinaryMatrix::bitcolumnOffset r="<<r<<" is out of bounds\n";
      return 64;
      }
    unsigned short  bitcol=(ac&0x1F);
    ac=(ac>>5);
    longElement[0]=(rw*RowInc)+ac;
    return bitcol;
  }
  inline unsigned short BinaryMatrix::bitColumn(const long r, const long c) const
  {
      unsigned long ac=c+CurrentColOffset;
      if ((c<0)||(ac>=AllocatedColumns)){
	std::cerr<<"ERROR BinaryMatrix::bitcolumn c="<<c
		 <<" is out of bounds\n";
	return 64;
      }
      long rw=(r+CurrentRowOffset);
      if ((r<0)||(rw>=(long)AllocatedRows)){
	std::cerr<<"ERROR BinaryMatrix::bitcolumn r="<<r<<" is out of bounds\n";
	return 64;
      }
      return (ac&0x1F);
  }
  inline  bool BinaryMatrix::checkSet(const long r, const long c,
				      const bool value)
  {
    if (r>=Rows)return false;
    if (c>=Columns)return false;
    if (r<0)return false;
    if (c<0)return false;
    unsigned long *a=0;
    unsigned short  bitcol=bitColumn(r,c,&a);
    if (bitcol==64)return false;
    unsigned long v=1;
    v=(v<<bitcol);
    unsigned long t=a[0];
    t=(t>>bitcol);
    t=(t&1);    
    if (value)
      a[0]=(a[0]|v);
    else
      {
	v=~v;
	  *a=(*a&v);
      }
    return (bool)t;
  }
  inline void BinaryMatrix::setBit(const long r, 
				   const long c,const bool value){ 
    unsigned long *a=0;
    unsigned short  bitcol=bitColumn(r,c,&a);
    if (bitcol==64)return;
    unsigned long v=1;
    v=(v<<bitcol);
    if (value)
      {
	a[0]=(a[0]|v);
      }
    else
      {
	  v=~v;
	  *a=(*a&v);
      }
  }
  inline bool BinaryMatrix::setVisit(int step, BinaryMatrix &visitup, 
				     BinaryMatrix &visitdown,
				     int r, int c, bool f)
  {
    if (inRange(r,c)){  
      if (step>0) visitup.setBit(r,c,true);
      else if (step<0) visitdown.setBit(r,c,true);
      else {
	 visitup.setBit(r,c,true);
	 visitdown.setBit(r,c,true);
      }
    } else if (f) {
      
      if (r<0)
	setVisit(step,visitup,visitdown,0,c, false);
      if (r>=Rows)
	setVisit(step,visitup,visitdown,Rows-1,c,false);
      if (c<0)
	setVisit(step,visitup,visitdown,r,0,false);
      if (c>=Columns)
	setVisit(step,visitup,visitdown,r,Columns-1,false);
      return false;
    }
    if ((step!=0)&&(f))
      {
	c-=step;
	setVisit(step,visitup,visitdown,r,c,false);
      } 
     
    return true;
  }
  inline bool BinaryMatrix::clearRow(int step, BinaryMatrix &visitup, 
				     BinaryMatrix &visitdown,int r, int c)
  {
    if (r<0)return false;
    if (r>=Rows)return false;
    int rc=c;
    bool clearrow=true;
    while (clearrow)
      {
	if (inRange(r,rc)){  
	  if (!checkSet(r,rc,false))clearrow=false;	  	  
	  if ((step>0)&& (visitdown(r,rc)))clearrow=false;
	  else if ((step<0)&& (visitup(r,rc)))clearrow=false;
	  else if (step==0)clearrow=false;
	}else {
	  if ((step>0)&&(rc>=Columns))clearrow=0;
	  else if ((step<0)&&(rc<0))clearrow=0;
	  else if (step==0)clearrow=false;
	}
	rc+=step;
      }
    return true;
  }
  inline  void BinaryMatrix::copy(BinaryMatrix &cm)
  {
    cm.AllocatedMatrix=AllocatedMatrix;
    cm.AllocatedSize=AllocatedSize;
    cm.AllocatedRows=AllocatedRows;
    cm.AllocatedColumns=AllocatedColumns;
    cm.CurrentRowOffset=CurrentRowOffset;
    cm.CurrentColOffset=CurrentColOffset;  
    cm.Rows=Rows;
    cm.Columns=Columns;
    cm.RowInc=RowInc;
  }
  inline void 	BinaryMatrix::checkdis(Cure::Matrix &celldis,int r,int c)
  {   
    int i=r-1;;
    int j=c;
    double d=1.0;
    for (int it=0; it<8;it++) {
      if (inRange(i,j))
	if (celldis(i,j)>0){
	  if (((celldis(i,j)+d)<(celldis(r,c)))) {
	    celldis(r,c)=celldis(i,j)+d;
	    return checkdis(celldis,r,c);
	  }
	  else if (celldis(i,j)>(celldis(r,c)+d)) {
	    celldis(i,j)=(celldis(r,c)+d);
	    checkdis(celldis,i,j);
	  }
	}
      if (it==0)i+=2;
      else if(it==1){
	i--;
	j--;
      }
      else if (it==2) j+=2;
      else if (it==3) {
	i--;
	d=M_SQRT2;
      }
      else if (it==4)i+=2;
      else if(it==5)j-=2;
      else i-=2;
    }    
}
  inline  void   BinaryMatrix::checkdis(Cure::Matrix &celldis,int r,int c,
					Cure::Matrix &celldis2,
					Cure::ShortMatrix &minindex,
					double &mindist)
  {   
    int i=r-1;;
    int j=c;
    double d=1.0;
    for (int it=0; it<8;it++) {
      if (inRange(i,j))
	if (celldis(i,j)>0){
	  if (((celldis(i,j)+d)<(celldis(r,c)))) {
	    celldis(r,c)=celldis(i,j)+d;
	    if (celldis2(r,c)>0){
	      double dis=celldis2(r,c)+celldis(r,c);
	      if (dis<mindist){
		mindist=dis;
		minindex(0,0)=r;
		minindex(0,1)=c;
	      }
	    }
	    return checkdis(celldis,r,c,celldis2,minindex,mindist);
	  }
	  else if (celldis(i,j)>(celldis(r,c)+d)) {
	    celldis(i,j)=(celldis(r,c)+d);
	    if (celldis2(i,j)>0){
	      double dis=celldis2(i,j)+celldis(i,j);
	      if (dis<mindist){
		mindist=dis;
		minindex(0,0)=i;
		minindex(0,1)=j;
	      }
	    }
	    checkdis(celldis,i,j,celldis2,minindex,mindist);
	  }
	}
      if (it==0)i+=2;
      else if(it==1){
	i--;
	j--;
      }
      else if (it==2) j+=2;
      else if (it==3) {
	i--;
	d=M_SQRT2;
      }
      else if (it==4)i+=2;
      else if(it==5)j-=2;
      else i-=2;
    }    
}
 
 
  inline int BinaryMatrix::clusterGrowth(Cure::ShortMatrix &cells, 
					 BinaryMatrix &visited, int top)
  {
    if (cells.Rows<1)return 0;
    int r=cells(0,0)-1;
    int c=cells(0,1);
    for (int it=0; it<top; it++)
      {
	if (inRange(r,c)){
	  if (!visited(r,c))
	    if (!(*this)(r,c))
	      {
		cells.append(r,0);
		cells.append(c,1);
		visited.setBit(r, c,true);
	      }
	}
	if (it==0)r+=2;
	else if(it==1){
	  r--;
	  c--;
	}
	else if (it==2) c+=2;
	else if (it==3) r++;
	else if (it==4)r-=2;
	else if (it==5)c-=2;
	else r+=2;
    } 
    cells.offset(1,0,cells.Rows-1,cells.Columns);
    return 1;
  }
  
} // namespace Cure;


#endif 
