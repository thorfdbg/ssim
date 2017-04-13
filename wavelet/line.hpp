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
** $Id: line.hpp,v 1.3 2008-03-04 14:12:24 thor Exp $
**
*/

#ifndef WAVELET_LINE_HPP
#define WAVELET_LINE_HPP

/// Includes
#include "global/types.hpp"
#include "std/assert.hpp"
///

/// class Line
// This class describes one horizontal line of the image data
class Line {
  //
  // The amount of boundary overlap - extend.
  enum {
    Support = 6
  };
  //
  // The pointer to the allocated memory.
  WORD *m_pData;
  //
  // The pointer to the first nominal pixel to be filtered.
  WORD *m_pOrigin;
  //
  // The nominal number of pixels, not inluding the boundary of the line.
  ULONG m_ulSize;
  //
  //
public:
  // Create a new line of the given nominal size.
  Line(ULONG length)
    : m_pData(new WORD[length + Support * 2]), m_pOrigin(m_pData + Support), m_ulSize(length)
  { }
  //
  ~Line()
  {
    delete[] m_pData;
  }
  //
  // Get the first nominal pixel of the line.
  WORD *Origin(void)
  {
    return m_pOrigin;
  }
  //
  const WORD *Origin(void) const
  {
    return m_pOrigin;
  }
  //
  // Return the pixel at the given offset.
  WORD &At(int offset)
  {
    assert(offset >= 0 && ULONG(offset) < m_ulSize);

    return m_pOrigin[offset];
  }
  //
  // Return the size of the line in pixels, not including the extend.
  ULONG LengthOf(void) const
  {
    return m_ulSize;
  }
  //
  // Mirror-extend the line into the boundary region.
  void MirrorExtend(void);
  //
  // Extract the even/odd pixels from the line and copy them to the targets.
  // Both must be allocated already.
  void Deinterleave(WORD *even,WORD *odd) const;
  //
  // Deinterleave the even or odd part of the line into the buffer.
  void Deinterleave(WORD *target,bool odd) const;
  //
  // Deinterleave the even or odd part of the line into the buffer.
  void Deinterleave(FLOAT *target,bool odd) const;
  //
  // Store the line unmodified into the buffer
  void Store(FLOAT *target) const;
  //
#if CHECK_LEVEL > 0
  // For debugging only: The Y position of the line.
  LONG m_lY;
#endif
};
///

///
#endif
