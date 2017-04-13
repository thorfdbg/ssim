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
** $Id: filter.cpp,v 1.3 2008-03-04 14:12:24 thor Exp $
**
*/

/// Includes
#include "filter.hpp"
#include "line.hpp"
///

/// Filter::HLift
// Perform the horizontal lifting steps on the given line.
void Filter::HLift(class Line *line)
{
  WORD *p   = line->Origin();;
  ULONG len = line->LengthOf();
  WORD *dp,*last;
  //
  // Special one-point case: Do not filter then.
  if (len > 1) {
    //
    // First the odd pixels.
    dp   = p - 3;
    last = p + len + 3;
    do {
      *dp += (dp[-3] - 9 * dp[-1] - 9 * dp[1] + dp[3]) >> 4;
      dp  += 2;
    } while(dp < last);
    //
    // Then the even pixels.
    dp   = p;
    last = p + len;
    do {
      *dp += (-dp[-3] + 9 * dp[-1] + 9 * dp[1] - dp[3]) >> 5;
      dp  += 2;
    } while(dp < last);
  }
}
///

/// Filter::VLift1
// Perform the first vertical lifting step on the given lines.
void Filter::VLift1(const class Line *top2,const class Line *top1,class Line *target,
		    const class Line *bot1,const class Line *bot2)
{  
  ULONG len       = target->LengthOf();
  const WORD *t2  = top2->Origin();
  const WORD *t1  = top1->Origin();
  WORD *p         = target->Origin();
  const WORD *b1  = bot1->Origin();
  const WORD *b2  = bot2->Origin();
  
  assert(len == top2->LengthOf() && len == top1->LengthOf() && len == bot1->LengthOf() && len == bot2->LengthOf());

  do {
    *p += (*t2 - 9 * *t1 - 9 * *b1 + *b2) >> 4;
    t2++,t1++,p++,b1++,b2++;
  } while(--len);
}
///

/// Filter::VLift2
// Perform the second vertical lifting step on the given lines.
void Filter::VLift2(const class Line *top2,const class Line *top1,class Line *target,
		    const class Line *bot1,const class Line *bot2)
{  
  ULONG len       = target->LengthOf();
  const WORD *t2  = top2->Origin();
  const WORD *t1  = top1->Origin();
  WORD *p         = target->Origin();
  const WORD *b1  = bot1->Origin();
  const WORD *b2  = bot2->Origin();
  
  assert(len == top2->LengthOf() && len == top1->LengthOf() && len == bot1->LengthOf() && len == bot2->LengthOf());

  do {
    *p += (-*t2 + 9 * *t1 + 9 * *b1 - *b2) >> 5;
    t2++,t1++,p++,b1++,b2++;
  } while(--len);
}
///
