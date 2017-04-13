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
** $Id: ptrarray.hpp,v 1.3 2008-03-04 14:12:24 thor Exp $
**
*/

#ifndef GLOBAL_PTRARRAY_HPP
#define GLOBAL_PTRARRAY_HPP

/// Includes
#include "global/types.hpp"
#include "std/assert.hpp"
///

/// class PtrArray
// A templated class defining an array of pointers to be automatically deleted.
template<typename Entry>
class PtrArray {
  //
  // The number of entries.
  size_t  m_Size;
  //
  Entry **m_ppEntries;
  //
  // No public assignment and copy constructor.
  PtrArray(const PtrArray<Entry> &)
  { }
  //
  PtrArray<Entry> &operator=(const PtrArray<Entry> &)
  { }
  //
public:
  explicit PtrArray(size_t num)
    : m_Size(num), m_ppEntries(new Entry*[num])
  {
    Entry **t = m_ppEntries;

    while(num) {
      *t++ = NULL;
      num--;
    }
  }
  //
  ~PtrArray(void)
  {
    Entry **t  = m_ppEntries;
    size_t num = m_Size;

    while(num) {
      delete *t;
      t++;
    }
    delete[] m_ppEntries;
  }
  //
  // Access operator.
  Entry *&operator[](size_t offset)
  {
    assert(offset < m_Size);
    return m_ppEntries[offset];
  }
  //
  // Size operator
  size_t LengthOf(void) const
  {
    return m_Size;
  }
};
///

///
#endif

