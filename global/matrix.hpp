/************************************************************************************
 **  Copyright (C) 2005-2007 TU Berlin, Felix Oum, Thomas Richter                  **
 **                                                                                **
 **  This software is provided 'as-is', without any express or implied             **
 **  warranty.  In no event will the authors be held liable for any damages        **
 **  arising from the use of this software.                                        **
 **                                                                                **
 **  Permission is granted to anyone to use this software for any purpose,         **
 **  including commercial applications, and to alter it and redistribute it        **
 **  freely, subject to the following restrictions:                                **
 **                                                                                **
 **  1. The origin of this software must not be misrepresented; you must not       **
 **     claim that you wrote the original software. If you use this software       **
 **     in a product, an acknowledgment in the product documentation would be      **
 **     appreciated but is not required.                                           **
 **  2. Altered source versions must be plainly marked as such, and must not be    **
 **     misrepresented as being the original software.                             **
 **  3. This notice may not be removed or altered from any source distribution.    **
 **                                                                                **
 **	Felix Oum		Thomas Richter                                     **
 **				thor@math.tu-berlin.de                             **
 **                                                                                **
 ************************************************************************************/

/*
** $Id: matrix.hpp,v 1.5 2008-03-04 14:12:24 thor Exp $
**
*/

#ifndef GLOBAL_MATRIX_HPP
#define GLOBAL_MATRIX_HPP

/// Includes
#include "global/types.hpp"
#include "global/matrixbase.hpp"
#include "std/string.hpp"
#include "std/stdio.hpp"
///

/// Type traits for the matrix class
template <typename Entry> struct ElementTraits {
  // Not valid. Create a compiler error if this is
  // used.
};
//
// Type trait for all valid entry types we may put
// into matrices.
template <> struct ElementTraits<DOUBLE> {
  enum { isValid = true };
};
template <> struct ElementTraits<FLOAT> {
  enum { isValid = true };
};
template <> struct ElementTraits<ULONG> {
  enum { isValid = true };
};
template <> struct ElementTraits<LONG> {
  enum { isValid = true };
};
template <> struct ElementTraits<UWORD> {
  enum { isValid = true };
};
template <> struct ElementTraits<WORD> {
  enum { isValid = true };
};
template <> struct ElementTraits<UBYTE> {
  enum { isValid = true };
};
template <> struct ElementTraits<BYTE> {
  enum { isValid = true };
};
// We may also keep pointers here.
template <typename T> struct ElementTraits<T *> {
  enum { isValid = true };
};
///

/// templated Matrix class
template <typename Entry> class Matrix : public MatrixBase {
  //
  // The type buffer goes here: This is the origin of the
  // matrix array.
  Entry *m_pEntries;
  //
public:
  //
  // Default constructor, do not allocate any memory.
  Matrix(void)
    : MatrixBase()
  {
    assert(ElementTraits<Entry>::isValid);
  }
  //
  // Destructor: Also cleans up the memory allocated here.
  ~Matrix(void)
  {
    // ...but in matrixbase.
  }
  //
  // Allocator for a matrix with given dimensions.
  Matrix(ULONG w,ULONG h)
    : MatrixBase(w * h * sizeof(Entry))
  { 
    assert(ElementTraits<Entry>::isValid);
    m_pEntries = static_cast<Entry *>(m_pMemory->m_pMem);
    m_ulWidth         = w;
    m_ulHeight        = h;
    m_ulEntriesPerRow = w;
  }
  //
  // Allocate a matrix that has been empty before.
  void Allocate(ULONG width,ULONG height)
  {
    assert(m_pMemory == NULL);
    //
    // First get the memory
    MatrixBase::Allocate(width * height * sizeof(Entry));
    m_pEntries = static_cast<Entry *>(m_pMemory->m_pMem);
    m_ulWidth         = width;
    m_ulHeight        = height;
    m_ulEntriesPerRow = width;
  }
  //
  // Release the memory of a matrix explicitly.
  void Dispose(void)
  {
    MatrixBase::Release();
    m_pEntries = NULL;
  }
  //
  // Copy a matrix over. The two matrices will afterwards use
  // the same memory. Use DuplicateFrom for a deep copy.
  Matrix(const Matrix<Entry> &o)
    : MatrixBase(o), m_pEntries(o.m_pEntries)
  {  
    assert(ElementTraits<Entry>::isValid);
  }
  //
  // Assign this matrix to the second matrix. The two matrices
  // will use the same matrix coefficients afterwards.
  Matrix<Entry> &operator=(const Matrix<Entry> &o)
  {
    // First assign the base
    MatrixBase::operator=(o);
    // Then the element pointer.
    m_pEntries = o.m_pEntries;
    //
    return *this;
  }
  //
  // Build a new matrix out of a larger matrix by sharing a
  // coefficient window between the two. 
  // x and y is the anchor position of the new matrix within the
  // old, w and h the dimension of the new matrix.
  Matrix(Matrix<Entry> &o,ULONG x,ULONG y,ULONG w,ULONG h)
    : MatrixBase(o)
  {  
    assert(ElementTraits<Entry>::isValid);
    assert(x + w <= o.m_ulWidth);
    assert(y + h <= o.m_ulHeight);
    //
    // Now assign the matrix contents.
    m_pEntries = &o.At(x,y);
    m_ulEntriesPerRow = o.m_ulEntriesPerRow;
    m_ulWidth  = w;
    m_ulHeight = h;
  }
  //
  // Assign this matrix to a sub-window of an existing
  // matrix, then sharing the memory between the two.
  void Extract(Matrix<Entry> &o,ULONG x,ULONG y,ULONG w,ULONG h)
  {
    assert(x + w <= o.m_ulWidth);
    assert(y + h <= o.m_ulHeight);
    //
    MatrixBase::operator=(o);
    m_pEntries = &o.At(x,y);
    m_ulEntriesPerRow = o.m_ulEntriesPerRow;
    m_ulWidth  = w;
    m_ulHeight = h;
  }
  //
  // Get: Extract an element from the matrix
  Entry Get(ULONG x,ULONG y) const
  {
    assert(m_pEntries);
    assert(x < m_ulWidth && y < m_ulHeight);
    return m_pEntries[x+y*m_ulEntriesPerRow];
  }
  //
  const Entry *Origin(void) const
  {
    return m_pEntries;
  }
  //
  // The same as a const lvalue:
  const Entry &At(ULONG x,ULONG y) const
  {   
    assert(m_pEntries);
    assert(x < m_ulWidth && y < m_ulHeight);
    return m_pEntries[x+y*m_ulEntriesPerRow];
  }
  //
  // The same as a non-const lvalue 
  Entry &At(ULONG x,ULONG y)
  {   
    assert(m_pEntries);
    assert(x < m_ulWidth && y < m_ulHeight);
    return m_pEntries[x+y*m_ulEntriesPerRow];
  }
  //
  // Put: Place an element into the matrix.
  void Put(ULONG x,ULONG y,Entry v)
  {    
    assert(m_pEntries);
    assert(x < m_ulWidth && y < m_ulHeight);
    m_pEntries[x+y*m_ulEntriesPerRow] = v;
  }
  //
  // The same again, for the purists that like to call this
  // "set" instead of "put".
  void Set(ULONG x,ULONG y,Entry v)
  {
    assert(m_pEntries);
    assert(x < m_ulWidth && y < m_ulHeight);
    m_pEntries[x+y*m_ulEntriesPerRow] = v;
  }
  //
  // Make a one-to-one copy from the original, creating
  // two indpendent copies, i.e. make a "deep copy". The
  // old contents is lost. This only works for PODs.
  void DuplicateFrom(const Matrix<Entry> &o)
  {
    Allocate(o.m_ulWidth * o.m_ulHeight * sizeof(Entry));
    //
    m_ulWidth         = o.m_ulWidth;
    m_ulHeight        = o.m_ulHeight;
    m_ulEntriesPerRow = o.m_ulWidth;
    m_pEntries        = static_cast<Entry *>(m_pMemory->m_pMem);
    if (o.m_ulWidth == o.m_ulEntriesPerRow) {
      memcpy(m_pEntries,o.m_pEntries,
	     o.m_ulEntriesPerRow * o.m_ulHeight * sizeof(Entry));
    } else {
      const Entry *src = o.m_pEntries;
      Entry *dst       = m_pEntries;
      ULONG h  = m_ulHeight;
      ULONG w = sizeof(Entry) * m_ulWidth;
      //
      while(h) {
	memcpy(dst,src,w);
	src += o.m_ulEntriesPerRow;
	dst += m_ulEntriesPerRow;
	h--;
      }
    }
  }
  //  
  // Clean the contents of this matrix completely.
  // Warning! This works only well for PODs.
  void CleanMatrix(void)
  {
    if (m_pEntries) {
      // Quick method: Matrix is continous
      if (m_ulEntriesPerRow == m_ulWidth) {
	memset(m_pEntries,0,m_ulWidth * m_ulHeight * sizeof(Entry));
      } else {
	Entry *row = m_pEntries;
	ULONG h    = m_ulHeight;
	ULONG w    = m_ulWidth * sizeof(Entry);
	while(h) {
	  memset(row,0,w);
	  row     += m_ulEntriesPerRow;
	  h--;
	}
      }
    }
  }
};
///

///
#endif
