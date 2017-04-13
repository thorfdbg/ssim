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
** $Id: filter.hpp,v 1.3 2008-03-04 14:12:24 thor Exp $
**
*/

#ifndef WAVELET_FILTER_HPP
#define WAVELET_FILTER_HPP

/// Includes
#include "global/types.hpp"
#include "std/assert.hpp"
///

/// Forwards
class Line;
///

/// class Filter
// This class implements the forwards lifting steps
// for the 13-7 filter. This is an integer filter that
// performs almost as good as the 9/7 float. We do it
// in integer here because that's faster.
class Filter {
  //
  // Hmm. Really no data here...
  //
public:
  //
  // Perform the horizontal lifting steps on the given line.
  static void HLift(class Line *line);
  //
  // Perform the first vertical lifting step on the given lines.
  static void VLift1(const class Line *top2,const class Line *top1,class Line *target,
		     const class Line *bot1,const class Line *bot2);
  //
  // Perform the second vertical lifting step on the given lines.
  static void VLift2(const class Line *top2,const class Line *top1,class Line *target,
		     const class Line *bot1,const class Line *bot2);
  //
};
///

///
#endif
