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
** $Id: line.cpp,v 1.3 2008-03-04 14:12:24 thor Exp $
**
*/

/// Includes
#include "global/types.hpp"
#include "std/assert.hpp"
#include "line.hpp"
///

/// Line::MirrorExtend
// Mirror-extend the line into the boundary region.
void Line::MirrorExtend(void)
{ 
  WORD *leftsrc,*leftdst;   // copy operation on the left boundary
  WORD *rightsrc,*rightdst; // copy operation on the right boundary.
  ULONG len   = LengthOf();
  //
  if (len > 1) {
    leftsrc     = Origin() + 1;
    leftdst     = leftsrc  - 2;
    rightdst    = Origin() + len; // pixel behind the edge.
    rightsrc    = rightdst - 2; 
    //
    for(int i = 1;i <= Support;i++) {
      *leftdst--  = *leftsrc++;
      *rightdst++ = *rightsrc--;
    }
  }
}
///

/// Line::Deinterleave
// Extract the even/odd pixels from the line and copy them to the targets.
// Both must be allocated already.
void Line::Deinterleave(WORD *ep,WORD *op) const
{
  const WORD *sp  = Origin();
  ULONG len       = LengthOf();

  while(len) {
    *ep++ = *sp++;
    if (--len == 0)
      break;
    *op++ = *sp++;
    --len;
  }
}
///

/// Line::Deinterleave
// Deinterleave the even or odd part of the line into the buffer.
void Line::Deinterleave(WORD *target,bool odd) const
{
  const WORD *sp = Origin();
  ULONG len      = LengthOf();

  if (odd) {
    sp++;
    len--;
  }

  len = (len + 1) >> 1;

  while(len) {
    *target++ = *sp++;
    sp++;
    len--;
  }
}
///

/// Line::Deinterleave
// Deinterleave the even or odd part of the line into the buffer.
void Line::Deinterleave(FLOAT *target,bool odd) const
{
  const WORD *sp = Origin();
  ULONG len      = LengthOf();

  if (odd) {
    sp++;
    len--;
  }

  len = (len + 1) >> 1;

  while(len) {
    *target++ = *sp++;
    sp++;
    len--;
  }
}
///

/// Line::Store
// Store the line unmodified into the buffer
void Line::Store(FLOAT *target) const
{ 
  const WORD *sp = Origin();
  ULONG len      = LengthOf();

  while(len) {
    *target++ = *sp++;
    len--;
  }
}
///

