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
** $Id: colortransformer.hpp,v 1.7 2008-12-28 22:40:30 thor Exp $
**
*/

#ifndef CTRAFO_COLORTRANSFORMER_HPP
#define CTRAFO_COLORTRANSFORMER_HPP

/// Includes
#include "img/image.hpp"
#include "std/math.hpp"
#include "global/matrix.hpp"
///

/// ColorTransformer
// This class describes a simplistic color transformer
// for RGB->YC_bC_r transformation.
class ColorTransformer {
  //
  // The lookup table used for the transfer function lookup.
  DOUBLE *m_pdLookup;
  //
  // The lookup table for the LMS transfer function.
  DOUBLE *m_pdLMS;
  //
  // The RGB->R'B'G' transfer function
  static DOUBLE sRGBTransfer(DOUBLE in,DOUBLE scale)
  {
    if (in < 0.04045 * scale) {
      // The linear region
      return in / (12.92 * scale);
    } else {
      return pow((in / scale + 0.055) / 1.055,2.4);
    }
  }
  //
  // The LMS to L'M'S' lookup function.
  static DOUBLE LMSTransfer(DOUBLE in)
  {
    return pow(in,0.43);
  }
  //
  // Create the lookup table.
  void CreateLookup(ULONG scale);
  //
  // Create the LMS lookup table.
  void CreateLMSLookup(ULONG scale);
  //
  // Transform a single matrix
  void ForwardsTransform(class Matrix<FLOAT> &rm,class Matrix<FLOAT> &gm,class Matrix<FLOAT> &bm,
			 ULONG max,ULONG lmsscale);
  //
public:
  ColorTransformer(void);
  ~ColorTransformer(void);
  //
  // Forwards transform, i.e. RGB->YC_bC_r
  // This touches only the first three components.
  void ForwardsTransform(class Image *img);
};
///

///
#endif
