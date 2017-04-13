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
** $Id: matrixbase.hpp,v 1.5 2013/05/20 20:21:06 Administrator Exp $
**
*/

#ifndef GLOBAL_MATRIXBASE_HPP
#define GLOBAL_MATRIXBASE_HPP

/// Includes
#include "global/types.hpp"
#include "global/exceptions.hpp"
#include "std/stdlib.hpp"
#include "std/assert.hpp"
///

/// Defines
#ifdef NO_SYNC_OPERATIONS
#define __sync_sub_and_fetch(a,b) ((*(a) -= (b)))
#define __sync_add_and_fetch(a,b) ((*(a) += (b)))
#endif

/// Matrix base class
class MatrixBase {
  //
protected:  
  // Memory management for the entry data. This holds the memory
  // to be kept here.
  struct MemKeeper {
    APTR   m_pMem;       // where's the memory held?
    ULONG  m_ulRefCount; // reference counter
    //
    MemKeeper(size_t size)
      : m_pMem(malloc(size)), m_ulRefCount(1)
    { 
      if (m_pMem == NULL)
	Throw(NoMem,"MemKeeper::MemKeeper","out of memory");
    }
    //
    ~MemKeeper(void)
    {
      free(m_pMem);
    }
  }   *m_pMemory;
  //
  // This defines the number of entries I've to add to the index of
  // the (one dimensional!) data array to move from one row to the 
  // row below.
  // Note that this is in the coefficient type, so you'll need to multiply
  // by sizeof(type) to get the BytesPerRow.
  // Example:  A 4x4 (long) matrix usually has an EntriesPerRow==4.
  ULONG m_ulEntriesPerRow;
  // The dimensions of the array. Humm, do we need to keep these?
  // At least for consistency checking, we should.
  ULONG m_ulWidth,m_ulHeight;
  //
  // Build a new matrix allocating the indicated amount of storage.
  MatrixBase(size_t size)
    : m_pMemory(new MemKeeper(size))
  { 
    m_ulWidth         = 0;
    m_ulHeight        = 0;
    m_ulEntriesPerRow = 0;
  }
  //
  // Construct an empty matrix.
  MatrixBase(void)
    : m_pMemory(NULL)
  { 
    m_ulWidth         = 0;
    m_ulHeight        = 0;
    m_ulEntriesPerRow = 0;
  }
  //
  // Copy construct a matrix from another source, reusing its
  // memory.
  MatrixBase(const MatrixBase &o)
  {
    if (o.m_pMemory) {
	  m_pMemory         = o.m_pMemory;
      __sync_add_and_fetch(&m_pMemory->m_ulRefCount,1); // used once more
      //m_pMemory->m_ulRefCount++; // used once more
    } else {
      m_pMemory         = NULL;
    }
    m_ulEntriesPerRow   = o.m_ulEntriesPerRow;
    m_ulWidth           = o.m_ulWidth;
    m_ulHeight          = o.m_ulHeight;
  }
  //
  // Assign the memory from another matrix to this matrix.
  MatrixBase &operator=(const MatrixBase &o)
  {
    // First increment the ref-count of the original
    // if that has memory. Otherwise, copy an empty
    // matrix.
    if (o.m_pMemory) {
      // This also protects for self-assignments.
      __sync_add_and_fetch(&o.m_pMemory->m_ulRefCount,1);
      //o.m_pMemory->m_ulRefCount++;
    }
    //
    // Now release our memory.
    if (m_pMemory && __sync_sub_and_fetch(&m_pMemory->m_ulRefCount,1) == 0) {
      delete m_pMemory;
    }
    m_pMemory         = o.m_pMemory;
    m_ulEntriesPerRow = o.m_ulEntriesPerRow;
    m_ulWidth         = o.m_ulWidth;
    m_ulHeight        = o.m_ulHeight;
    return *this;
  }
  //
  // Equip a matrix with its own memory
  void Allocate(size_t s)
  {
    // Release the current memory.
    if (m_pMemory && __sync_sub_and_fetch(&m_pMemory->m_ulRefCount,1) == 0) {
      delete m_pMemory; m_pMemory = NULL;
    }
    m_pMemory         = new MemKeeper(s);
    m_ulWidth         = 0;
    m_ulHeight        = 0;
    m_ulEntriesPerRow = 0;
  }
  //
  // Release the contents of a matrix, making it empty again.
  void Release(void)
  {
    if (m_pMemory && __sync_sub_and_fetch(&m_pMemory->m_ulRefCount,1) == 0) {
      delete m_pMemory; m_pMemory = NULL;
    }
    m_ulWidth         = 0;
    m_ulHeight        = 0;
    m_ulEntriesPerRow = 0;
  }
  //
  // Give up a matrix, release the memory reference.
  ~MatrixBase(void)
  {
    if (m_pMemory && __sync_sub_and_fetch(&m_pMemory->m_ulRefCount,1) == 0) {
      delete m_pMemory;
    }
  }
  //
public:
  // Return the coordinates given the index.
  // This should be avoided in frequent calling since it
  // requires multiplication and division.
  void CoordinatesOf(ULONG idx,ULONG &x,ULONG &y) const
  {
    y = idx/m_ulEntriesPerRow;
    x = idx-y*m_ulEntriesPerRow;    
    assert(x < m_ulWidth && y < m_ulHeight);
  }
  //
  // The following method returns FALSE if the matrix is not yet
  // established
  bool IsEmpty(void) const
  {
    return (m_pMemory == NULL);
  }
  //
  //
  // Simple dimension querry functions
  //
  ULONG WidthOf(void) const
  {
    return m_ulWidth;
  }
  //
  ULONG HeightOf(void) const
  {
    return m_ulHeight;
  }  
};
///

///
#endif
