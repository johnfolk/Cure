// = AUTHOR(S)
//    John Folkesson
//    
// = COPYRIGHT
//    Copyright (c) 2007 John Folkesson
//    

#ifndef CURE_LONGARRAY_HH
#define CURE_LONGARRAY_HH
#include <string.h>  // memmove
#include <iostream>

namespace Cure {
  
  /**
   * The LongArray class will simplify code with lists of
   * longs of variable length.  It is best for lists that tend to grow
   * slowly over time.  This code contains no lists.  It 
   * implements a double index array of longs.
   *
   * One can iterate over all elements like this
   *
   * for (unsigned int i=0 i<a.rows();i++)
   *
   *   for(unsigned int j=0; j<a.columns(i); j++)
   *
   *     a(i,j)=mybigtoe;
   *  
   *
   * Very slightly more effiecient 
   * (avoid checks on i and j that you have done):
   *
   * for (unsigned int i=0 i<a.rows();i++){
   * 
   * unsigned long top=a.columns(i);
  
   * if (top){
   *
   *  long *m=&a(i,0);
   *
   *  for(unsigned int j=0; j<top; j++)
   *
   *    m[j]=mylittletoe; 
   *
   * }}
   *
   * One can also associate pairs of longs where one is the key and the 
   * other the value.  The LongArray can then be used as a hash table
   * One must call the constructor with an arguement first to set up
   * the hash mask.  
   *
   * @author John Folkesson
   */
  class LongArray{
  public:

  protected:
    unsigned long m_Mask;
    long ** m_Array;
    long m_Dummy;
    unsigned long m_AllocatedRows;
    unsigned long m_Rows;
    

    /**
     * This finds a value on a row but skips over
     * ever other column.
     * @param key the number to find
     * @param row the row to look on
     * @return -1 if fail else the column of the value
     */ 
   long findHash(long key,unsigned long row)const {
     unsigned long c=columns(row);
     for(unsigned long j=0; j<c; j+=2)
       if (m_Array[row][j]==key)return (long)j;
     return -1;				
   } 
  public:
    LongArray();
    LongArray(LongArray &l);
      ~LongArray();
    
    /**
     * 
     * Call with a depth if you want to use this as a 
     * hash table of lenth 2^hashdepth
     *
     * @param hashdepth size will be 2^hashdepth
     */
    void setHashDepth(const unsigned long hashdepth);
    /** 
     * gets the number of rows
     */
    long rows()const {return (long)m_Rows;} 
    /*
     * Gets the number of rows allocated.
     * @return number of rows allocated
     */
    unsigned long allocatedRows()const {return m_AllocatedRows;} 
    /**
     * gets the number of longs stored in a row.
     *
     * @param r the row 
     * @return the number of columns in row r
     */
    long columns(const unsigned long r)const {
      if (r<m_Rows)return m_Array[r][-1];
      return 0;
    }
    /**
     * Makes all rows empty, zero length.
     */
    void clear(){
      long r=rows();
      grow(0);
      grow(r);
    }
    /**
     * This adds the value to the list 
     * as a new last column on a row.
     * It adds a rows if they don't exists
     *
     * @param value the number to store
     * @param row the row to append it to
     *
     */
    void add(unsigned long row, long value){
      long c=columns(row);
      makeCell(row,c)=value;
    }
    /**
     * This adds the value to the list 
     * as a new last column on a row.
     * It adds a rows if they don't exists
     * It orderes the elements from lowest to highest
     * @param value the number to store
     * @param row the row to append it to
     *
     */
    void addOrdered(unsigned long row, long value){
      long c=columns(row);
      makeCell(row,c)=value;
      for( int i=c-1;i>-1;i--)
	if (value<(*this)(row,i)){
	  (*this)(row,i+1)=(*this)(row,i);
	} else {
	  (*this)(row,i+1)=value;
	  return;
	}
      (*this)(row,0)=value;
    }
    void addUniqueOrdered(unsigned long row, long value){
      if (find(value,row)==-1){
	addOrdered(row,value);
      }
    }
    /**
     * This copies one row of array to a row of this.
     * @param torow the row of this to add fraomarray(fromRow,...)to
     * @param fromarray the array to append one of its rows to this
     * @param fromrow the row of array to append to this
     * 
     */
    void addRow(long torow,
		LongArray &fromarray,long fromrow){
      unsigned long c=fromarray.columns(fromrow);
      if (torow>=rows())grow(torow+1);
      long c2=columns(torow);
      if (c>0){
	c--;
	makeCell(torow,c2+c)=(fromarray(fromrow,c));
	for (unsigned long i=0;i<c;i++)
	 (*this)(torow,c2+i)=fromarray(fromrow,i);
      }
    }

    /**
     * Adds a row and puts value in it;
     * return the number of the row added
     */
    unsigned long appendRow(long value){
      unsigned long r=rows();
      makeCell(r,0)=value;
      return r;
    }
    /**
     * This copies one row of array to a new row at the end of this.
     * @param array the array to append one of its rows to this
     * @param row the row of array to append to this
     * 2return the new row number
     */
    unsigned long appendRow(LongArray &array,unsigned long row){
      unsigned long c=array.columns(row);
      unsigned long r=rows();
      grow(r+1);
      if (c>0){
	c--;
	makeCell(r,c)=(array(row,c));
	for (unsigned long i=0;i<c;i++)
	 (*this)(r,i)=array(row,i);
      }
      return r;
    }
    /**
     * Same as add only that this won't do anything if value 
     * is already on the list.
     *
     * @param value the number to store
     * @param row the row to append it to
     *
     */
    bool addUnique(unsigned long row, long val ){
      if (find(val,row)==-1)
	{
	  add(row,val);
	  return true;
	}
      return false;
    }
    /**
     * This reverses the key <--> val association, using the val as
     * the key over key as the value.
     * @param hash the inverted hash table is returned here.
     *
     */
    void invertHash(LongArray & hash){
      long r=0;
      long c=0;
      for (long *it=iterate(r,c);it; c+=2, it=iterate(r,c)){
	hash.putHash((*this)(r,c+1),it[0]);
      }
    }
    
    /**
     * This retrieves a value paired to a key stored in the hash.
     * @param key the key to look for.
     * @param val the stored value for this key returned.
     * @return true if found else false
     */
    bool getHash(long key, long &val)const {
      unsigned long row=(key&m_Mask);
      long c=findHash(key,row);
      c++;
      if ((c<1)||(c>=(long)columns(row)))return false;
      val=(*this)(row,(unsigned long)c);
      return true;
    }
    /**
     * This checks if  a key is stored in the hash.
     * @param key the key to look for.
       * @return true if found else false
     */
    bool findHash(long key)const {
      unsigned long row=(key&m_Mask);
      unsigned long c=columns(row);
      for(unsigned long j=0; j<c; j+=2)
	if (m_Array[row][j]==key)return true;
     return false;				
    }
    /**
     * This stores a value paired to a key in the hash.
     * You can not store the same key twice.  It will 
     * overwrite the previous value for the key.
     * @param key the key to associate to value.
     * @param val the stored value for this key.
     */
    void putHash(long key,long val)
      {
	unsigned long row=(key&m_Mask);
	long c=findHash(key,row);
	if (c<0){
	  addHash(key,row,val);
	}else{
	  makeCell(row,(unsigned long)c+1)=val;
	}
      }
    long findPairOrdered(unsigned long row,long key)const {
      long c=columns(row);
      for(long j=0; j<c; j+=2){
	if (m_Array[row][j]>=key){
	  if (m_Array[row][j]==key)return j;
	  if (m_Array[row][j]>key)return -1;
	}
      }
     return -1;				
    }
    void  addPairOrdered(unsigned long row,long key,long val)
    {
      long c=columns(row);
      for(long j=0; j<c; j+=2){
	if (m_Array[row][j]==key){
	  m_Array[row][j+1]=val;
	  return;
	}
	if (m_Array[row][j]>key){
	  makeCell(row,c+1)=0;
	  memmove(&m_Array[row][j+2],&m_Array[row][j],(c-j)*sizeof(long));
	  m_Array[row][j]=key;
	  m_Array[row][j+1]=val;
	  return;
	}
      }
      makeCell(row,c+1)=val;
      makeCell(row,c)=key;
    }
    /**
     * This is called by putHash and normally the user whould not call this.
     * @param key the hash key
     * @param value the assocaitaed long
     * @param row the row to add the pair to. This should be the correct 
     * row according to key and the hash as put computes it.  Otherwise
     * the pair will not be found by hashing.
     */
    void addHash( long key, unsigned long row,long value){
      unsigned long c=columns(row);
      makeCell(row,c+1)=value;
      makeCell(row,c)=key;
    }

    /**
     * This removes a value paired to a key in the hash.
     * @param key the key to look for
     */
    void removeHash(long key){
      unsigned long row=(key&m_Mask);
      long c=findHash(key,row);
      while (c>-1){
	removeCell(row,c);
	removeCell(row,c);
	c=findHash(key,row);
      }
    }
    /**
     * Adds or removes rows from the array.
     * No data is lost here and the allocation never shrinks it only grows.
     * The part of the allocation that is considered in use can shrink.
     * New Rows will be set to zero length;
     * @param r the new number of rows.
     */
    void grow( unsigned long r){      
      if (r>m_AllocatedRows){
	long **m=m_Array;
	m_Array=new long*[r];
	if (m_AllocatedRows){
	  memcpy(m_Array,m,m_AllocatedRows*sizeof(long*));
	}
	if (m)delete[]m;
	for (unsigned long i=m_AllocatedRows; i<r;i++){
	  m_Array[i]=new long[2];
	  m_Array[i]+=2;
	  m_Array[i][-2]=0;
	  m_Array[i][-1]=0;
	}
	m_AllocatedRows=r;
      }
      for (unsigned long i=m_Rows; i<r;i++){
	m_Array[i][-1]=0;
      }
      m_Rows=r;
    }

    /**
     * The = operator allocates its own memory if needed and sets it
     * equal to the currently defined  b. 
     * It may allocate a lot less than b has.
     * @param b a LongArray to be used as the initial values.
     */
    void operator = (const LongArray& b){
      grow(b.m_Rows);
      m_Mask=b.m_Mask;
      for (unsigned long i=0;i<m_Rows; i++)
	{
	  if (b.m_Array[i][-1]>0){
	    if (b.m_Array[i][-1]>m_Array[i][-2]){
	      delete [] (m_Array[i]-2);
	      m_Array[i]=new long[b.m_Array[i][-1]+2];
	      m_Array[i]+=2;
	      m_Array[i][-2]=b.m_Array[i][-1];
	    }
	    memcpy(&m_Array[i][0],&b.m_Array[i][0],
		   (b.m_Array[i][-1])*sizeof(long));
	    m_Array[i][-1]=b.m_Array[i][-1];	    
	  }else{
	    eraseRow(i);
	  }
	}      
    }

    /**
     * Gets the number rstored in the cell.
     * @param r the the row to look in
     * @param c the column
     * @return the value in the cell or 0 if the cell 
     * has not been initialized.
     */
    long operator() (const unsigned long r, const unsigned long c) const { 
      if (columns(r)>(long)c)
	return  m_Array[r][c];
      return 0;
    }

    /**
     * This finds a value on a row.
     * @param value the number to store
     * @param row the row to append it to
     * @return -1 if fail else the column of the value
     */ 
   long find(long val, unsigned long row)const{
     unsigned long c=columns(row);
     for(unsigned long j=0; j<c; j++)
       if (m_Array[row][j]==val)return (long)j;
     return -1;				
   } 
    /**
     * This will remove the c'th element in row 
     * and shorten the row.  
     * @param c the index to the element to remove 
     * @param row the row to remove it from
     */
    void removeCell(unsigned long row,unsigned long c){
      unsigned long col=columns(row);
      long n=col;
      n-=c;
      n--;
      if (n>0)
	memmove(&m_Array[row][c],
	       &m_Array[row][c+1],
	       sizeof(long)*(n));
      if (col>0)m_Array[row][-1]--;
    }
    /**
     * This will find the all instances of value if it is on the row
     * and shorten the row.  
     * @param value the number to remove
     * @param row the row to remove it from
     */
    void removeValue(long value,unsigned long row){
      long c=find(value,row);
      while (c!=-1){
	removeCell(row,(unsigned long)c);
	c=find(value,row);
      }
    }
    /** To be symmetric with add*/
    void remove(unsigned long row,long value){
      removeValue(value,row);
    }
    /**
     * This will adjust the row's columns to 0;
     */
    void eraseRow(unsigned long r){
      if (r<m_Rows)m_Array[r][-1]=0;
    }
    /**
     * Copy the values in rows>r up one row and then subtract 1 from
     * Rows. 
     * @see LongArray::grow
     */
    void deleteRows(unsigned long r,unsigned long numberofrows=1){
      if (moveRows(r, m_Rows-numberofrows,numberofrows)){
	m_Rows-=numberofrows;
	for (unsigned long i=0;i<numberofrows;i++)
	  m_Array[i+m_Rows][-1]=0;;
      }
    }
    void insertRows(unsigned long startrow,unsigned long numberofrows=1 )
    {
      grow(m_Rows+numberofrows);
      moveRows(m_Rows-numberofrows,startrow,numberofrows);
    }
    /**
     * Copy the values from row r1 to r2 and visa versa.
     * @param r1 a row of the matrix.
     * @param r2 another row.
     * @see LongArray::grow, 
     */
    void swapRows(unsigned long r1,unsigned long  r2){
	if ((r1<m_Rows)&&(r2<m_Rows)){
	  long *m=m_Array[r1];
	  m_Array[r1]=m_Array[r2];	
	  m_Array[r1]=m;
	}
      }
      /**
       * This will move the rows from r1 to r2 
       * The rows in  between r1 and r2 will move up or down to make place.
       *
       * @return true if possible;
       */
      bool moveRows(unsigned long r1,unsigned long  r2, 
		    unsigned long numberofrows){
	if (((r1+numberofrows)<=m_Rows)&&((r2+numberofrows)<=m_Rows)){
	  if (r1==r2)return true;
	  if (numberofrows==0)return true;
	  long **m[numberofrows];
	  memcpy(m,&m_Array[r1],sizeof(long*)*numberofrows);
	  if (r2>r1){
	    memmove(&m_Array[r1],&m_Array[r1+numberofrows],
		  sizeof(long*)*(r2-r1));
	  }else{
	    memmove(&m_Array[r2+numberofrows],&m_Array[r2],
		    sizeof(long*)*(r1-r2));
	  }
	  memcpy(&m_Array[r2],m,sizeof(long*)*numberofrows);
	  return true;
	}
	return false;
      }
    
    /**
     * This gets a reference to a array element at (r,c).
     * if out of bounds it will return 0 and print warning;
     * 
     *
     * @param r the row of the element
     * @param c the column.
     * @return the array element.
     */
    long & operator() (const unsigned long r, const unsigned long c){
      
      if ((r<m_Rows)&&(c<(unsigned long)m_Array[r][-1]))
	return m_Array[r][c];
      std::cerr<<"Warning LongArray index Out of Bounds "
	       <<r<<" "<<c<<" "<<m_Rows<<" "<<columns(r)<<"\n";
         return m_Dummy;
    }
    /**
     * This helps you no to forget to set row and col to 
     * zero at the start.  Do not use this if you want to start  in the
     * middle.
     *
     * @param row the row parameter will be set to zero to start iteration
     * from begining of LongArray.
     * @param col the column parameter will be set to zero to start iteration
     * from begining of LongArray.
     * @return see iterate.
     */
    long * initIterate(long &row, long &col)
      {
	row=0;
	col=0;
	return iterate(row,col);
      }
    /**
     *  Iterate over all elements in the Array.
     *
     * use like this:
     * 
     * long r=0;
     *
     *long c=0;
     *
     * for (long *it=la.iterate(r,c);it; c++, it=la.iterate(r,c){
     *    std::cerr<<"element "<<r<<" "<<c<<" = "<it[0]<<"\n";
     *  }
     *
     * @row the row to start looking is input the row found is output.
     * @col the column to start looking is input the column found is output.
     * 
     * @return the first non empty cell starting from (row,col).
     * If at end then returns 0
     */
    long * iterate(long &row, long &col){
      if (row>=(long)m_Rows)return 0;
      if (row<0)row=0;
      long c=columns(row);
      if (c<=(long)col){
	c=0;
	col=0;
	while(c==(long)col){
	  row++;
	  if (row==(long)m_Rows)return 0;
	  c=columns(row);
	}
      }
      if (c>(long)col){
	return &m_Array[row][col];
      }
      return 0;
    }
    bool equalRows(unsigned long r1,unsigned long r2){
      long c1=columns(r1);
      if (c1!=(long)columns(r2))return false;
      for (long i=0;i<c1;i++)
	if (m_Array[r1][i]!=m_Array[r2][i])return false;
      return true;
    }
    void print();
    void printRow(long i);
    /**
     * This forms all combinations of a row of this.  
     * So *this(row) should have a list of longs
     * and combinations will get appended at r with all 
     * combinations of those longs staring with *this(row)
     * and ending with an empty row.  No combination is repated.
     * 
     * @param combinations the array to store the combinations.
     * @parmam r the row of combinations to start from. All data 
     * below this is wiped out first.
     * @param row the row of this to form combinations from.
     */
    void combinations(LongArray &combinations,long r=0, long row=0)
    {
      long n=columns(row);
      if (n==0){
	if (r>-1)
	  combinations.grow(r);
	return;
      }
      long nhyp=n;
      nhyp=(1<<nhyp);
      combinations.grow(r);
      combinations.grow(r+nhyp);
      long period=(nhyp>>1);
      int iter=1;
      for (int i=0;i<n;i++,iter=(iter<<1),period=(period>>1))
	{
	  long index=r;
	  for (int j=0; j<iter;j++,index+=period){
	    for (int k=0;k<period;k++,index++){
	      combinations.add(index,(*this)(row,i));
	    }
	  }
	}	
    }
    /**
     * This forms all pairwise combinations of a row of this.  
     * So *this(row) should have a list of pairs (ie matches)
     * and combinations will get appended at r with all 
     * combinations of those pairs staring with *this(row)
     * and ending with an empty row.  No combination is repated.
     * 
     * @param combinations the array to store the combinations.
     * @parmam r the row of combinations to start from. All data 
     * below this is wiped out first.
     * @param row the row of this to form combinations from.
     */
    void pairwiseCombinations(LongArray &combinations,long r=0, long row=0)
    {
      long n=columns(row);
      n=(n>>1);
      long nhyp=n;
      nhyp=(1<<nhyp);
      combinations.grow(r);
      combinations.grow(r+nhyp);
      if (n<1)return;
      n=(n<<1);
      long period=(nhyp>>1);
      int iter=1;
      for (int i=0;i<n;i+=2,iter=(iter<<1),period=(period>>1))
	{
	  long index=r;
	  for (int j=0; j<iter;j++,index+=period){
	    for (int k=0;k<period;k++,index++){
	      combinations.add(index,(*this)(row,i));
	      combinations.add(index,(*this)(row,i+1));
	    }
	  }
	}	
    }

   /**
     * This gets a reference to a array element at (r,c).
     * if out of bounds it will try to grow to right size,
     * filling with 0's.
     *
     * @param r the row of the element
     * @param c the column.
     * @return the array element even if out of bounds.
     */
    long & makeCell(const unsigned long r, const unsigned long c){
      if (r>=m_Rows) grow(r+1);
      if (c>=(unsigned long)m_Array[r][-1]){
	if (c>=(unsigned long)m_Array[r][-2]){
	    long* m=m_Array[r];
	    m_Array[r]=new long[c+3];
	    m_Array[r]+=2;
	    m_Array[r][-2]=c+1;
	    m_Array[r][-1]=c+1;
	    memcpy(&m_Array[r][0],m,m[-1]*sizeof(long));
	    memset(&m_Array[r][m[-1]],0,(c+1-m[-1])*sizeof(long));
	    m-=2;
	    delete [] (m);
	  }else{
	    memset(&m_Array[r][m_Array[r][-1]],0,
		   (c+1-m_Array[r][-1])*sizeof(long));
	    m_Array[r][-1]=c+1;
	  }
      }
      return m_Array[r][c];
    }
  protected:      
    
};
} // namespace Cure;


#endif 
