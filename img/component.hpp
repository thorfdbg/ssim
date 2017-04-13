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
** $Id: component.hpp,v 1.8 2009-04-07 18:14:36 thor Exp $
**
*/

#ifndef IMG_COMPONENT_HPP
#define IMG_COMPONENT_HPP

/// Includes
#include "global/types.hpp"
#include "global/matrix.hpp"
#include "wavelet/band.hpp"
///

/// Class Component
// A component is for us a rectangular domain of
// image data, represented as FLOATs (for simplicity)
// We also could support arbitrary data types, but that
// just makes things more complicated.
class Component {
  //
  // The hierarchy of bands in here.
  class Band  m_Band;
  //
  // This here is set if the component is signed. Otherwise,
  // it is false.
  BOOL        m_bIsSigned;
  //
  // The maximum amplitude we expect here. For signed,
  // this is the positive maximum, and the negative
  // maximum is expected to be -max. Beware!
  FLOAT       m_fMaxScale;
  //
  // The bit depth of this component, resulting in the above scale.
  UBYTE       m_ucBitDepth;
  //
  // The component weight in the final assignment/computation of the SSIM
  DOUBLE      m_dWeight;
  //
  // Name of this component.
  const char *m_pcName;
  //
public:
  // Unlike the matrix, the component really
  // carries the data with it. Thus, we need to give
  // width and height if we want to handle components.
  // Also requires the bit depth and the decomposition depth of the band in here.
  Component(ULONG width,ULONG height,BOOL sign,UBYTE bitdepth,FLOAT scale,UBYTE decdepth,bool keephp);
  //
  // Ditto. Also destroys the matrix data.
  ~Component(void);
  //
  // Return wether this component is signed (or not).
  BOOL &IsSigned(void)
  {
    return m_bIsSigned;
  }
  //
  // Return the scale of this component.
  FLOAT ScaleOf(void) const
  {
    return m_fMaxScale;
  }
  //
  // Return the bit detph of this component. Corresponds to the
  // scale returned above.
  UBYTE BitDepthOf(void) const
  {
    return m_ucBitDepth;
  }
  //
  bool KeepsSubbands(void) const
  {
    return m_Band.KeepsSubbands();
  }
  //
  // Push a line into the component and by that, wavelet transform it.
  // Due to the way how the transformer works, only every other pixel in the
  // line carries a sample.
  void PushLine(const class Line *line)
  {
    m_Band.PushLine(line);
  }
  //
  // Return the dimensions of the component.
  ULONG WidthOf(void) const
  {
    return m_Band.WidthOf();
  }
  //
  ULONG HeightOf(void) const
  {
    return m_Band.HeightOf();
  }
  //
  // Return the number of scales in here.
  UBYTE ScalesOf(void) const
  {
    return m_Band.ResolutionOf() + 1;
  }
  //
  // Return the indicated scale of the image. The original image
  // scale is scale #1, the next lower scale is scale #2 and so on.
  Matrix<FLOAT> &GetScale(UBYTE scale);
  //
  // Return the indicated subband of the image. Band == 0 is the LL band.
  Matrix<FLOAT> &GetBand(UBYTE scale,UBYTE band);
  //
  // Set or retrieve the component weight.
  DOUBLE &WeightOf(void)
  {
    return m_dWeight;
  }
  //
  // Retrieve the weight.
  DOUBLE WeightOf(void) const
  {
    return m_dWeight;
  }
  //
  // Return or set the name.
  const char *&NameOf(void)
  {
    return m_pcName;
  }
  //
  // Return the name.
  const char *NameOf(void) const
  {
    return m_pcName;
  }
};
///

///
#endif
