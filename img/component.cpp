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
** $Id: component.cpp,v 1.7 2009-04-07 18:14:36 thor Exp $
**
*/

/// Includes
#include "img/component.hpp"
///

/// Component::Component
// Build a new component with given dimensions.
Component::Component(ULONG width,ULONG height,bool sign,UBYTE depth,FLOAT scale,UBYTE decdepth,bool keephp)
  : m_Band(width,height,decdepth-1,keephp),
    m_bIsSigned(sign), m_fMaxScale(scale), m_ucBitDepth(depth),
    m_dWeight(1.0), m_pcName("Y")
{
}
///

/// Component::~Component
// Release the component again.
Component::~Component(void)
{
}
///

/// Component::GetScale
// Return the indicated scale of the image. The original image
// scale is scale #1, the next lower scale is scale #2 and so on.
Matrix<FLOAT> &Component::GetScale(UBYTE scale)
{
  class Band *band = &m_Band;

  while(scale > 1) {
    band = band->SubBandOf();
    scale--;
  }

  return band->CoefficientsOf();
}
///

/// Component::GetBand
// Return the indicated scale of the image. The original image
// scale is scale #1, the next lower scale is scale #2 and so on.
Matrix<FLOAT> &Component::GetBand(UBYTE scale,UBYTE orient)
{
  class Band *band = &m_Band;

  while(scale > 1) {
    band = band->SubBandOf();
    scale--;
  }

  return band->SubBandOf(orient);
}
///
