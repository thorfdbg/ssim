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
** $Id: band.hpp,v 1.5 2009-05-28 13:53:29 thor Exp $
**
*/

#ifndef WAVELET_BAND_HPP
#define WAVELET_BAND_HPP

/// Includes
#include "global/types.hpp"
#include "std/assert.hpp"
#include "global/matrix.hpp"
#include "global/ptrarray.hpp"
///

/// Forwards
class Line;
///

/// class Band
// This class keeps a stack of lines forming one wavelet band of the
// image. It also carries the wavelet transformation out.
class Band {
  //
  // The sub-band if any. The life-time of the sub-bands
  // is controlled here.
  class Band   *m_pSubBand;
  //
  // Constants for the register size
  enum {
    RegisterSize   = 12, // number of entries.
    OddWorkingPos  = 7,  // If a line is in this position, do work at odd clocks
    EvenWorkingPos = 4   // If a line is in this position, do work at even clocks
  };
  //
  // The line shift register.
  class Line    *m_pRegister[RegisterSize];
  //
  // The mirror-extended registers. Contains alternative
  // (mirrored) sources whenever the above contains NULL.
  class Line    *m_pMirrored[RegisterSize];
  //
  // The resolution level of this band.
  UBYTE          m_ucResolution;
  //
  // Dimensions of the band.
  ULONG          m_ulWidth,m_ulHeight;
  //
  // The coefficient matrix, if we need it.
  Matrix<FLOAT>  m_Coefficients;
  //
  // The subbands if we need it.
  Matrix<FLOAT>  m_HL;
  Matrix<FLOAT>  m_LH;
  Matrix<FLOAT>  m_HH;
  //
  // The current line we expect as input.
  LONG           m_lY;
  //
  // The output Y generated.
  ULONG          m_ulYO;
  //
  // Set in case empty lines are in the register that require
  // extension.
  bool           m_bExtend;
  //
  // Set in case the high-pass output should be kept as well.
  bool           m_bKeepHP;
  //
  // Advance the line shift register by two, push lines at the exit
  // positions into the child bands, make room for new lines.
  bool ShiftLineRegister(void);
  //
  // Accquire a new line for the indicated Y position, or recycle one.
  class Line *&NewLine(LONG y);
  //
  // extend the lines by mirroring by inserting line pointers to otherwise
  // NULL-lines in the register.
  void MirrorExtend(void);
  //
#if CHECK_LEVEL > 0
  // Save this band as PGM image for cross-check.
  void SaveBand(void);
#endif
  //
public:
  //
  // Setup a band of the given dimensions and the given decomposition depth.
  Band(ULONG width,ULONG height,UBYTE reslvl,bool keephp);
  //
  // Destroy this sub-band and the entire subband hierarchy.
  ~Band(void);
  //
  // Get the sub-band of this band or NULL in case there is none.
  class Band *SubBandOf(void);
  //
  // Push a line for transformation into this band.
  void PushLine(const class Line *data);
  //
  // Number of bands below this scale. For the topmost band, this
  // is the number of decomposition levels.
  UBYTE ResolutionOf(void) const
  {
    return m_ucResolution;
  }
  //
  // Gain access to the coefficient matrix.
  Matrix<FLOAT> &CoefficientsOf(void)
  {
    return m_Coefficients;
  }
  // 
  bool KeepsSubbands(void) const
  {
    return m_bKeepHP;
  }
  //
  // Return the dimensions of the band.
  ULONG WidthOf(void) const
  {
    return m_ulWidth;
  }
  ULONG HeightOf(void) const
  {
    return m_ulHeight;
  }
  //
  // Return the indicated subband.
  Matrix<FLOAT> &SubBandOf(UBYTE orientation)
  {
    switch(orientation) {
    case 1:
      return m_HL;
    case 2:
      return m_LH;
    case 3:
      return m_HH;
    }
    assert(false);
    return m_HH;
  }
};
///

///
#endif
