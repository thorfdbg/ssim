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
** $Id: band.cpp,v 1.4 2008-12-28 22:40:30 thor Exp $
**
*/

/// Includes
#include "band.hpp"
#include "line.hpp"
#include "filter.hpp"
///

/// Band::Band
// Setup a band of the given dimensions
Band::Band(ULONG width,ULONG height,UBYTE reslvl,bool keephp)
  : m_ucResolution(reslvl),
    m_ulWidth(width), m_ulHeight(height),
    m_Coefficients((keephp)?(0):(width),(keephp)?(0):(height)),
    m_HL((keephp)?((width + 0) >> 1):(0),(keephp)?((height + 1) >> 1):(0)),
    m_LH((keephp)?((width + 1) >> 1):(0),(keephp)?((height + 0) >> 1):(0)),
    m_HH((keephp)?((width + 0) >> 1):(0),(keephp)?((height + 0) >> 1):(0)),
    m_lY(0), m_ulYO(0), m_bExtend(true), m_bKeepHP(keephp) // starts with empty lines in the buffer.
{
  int i;
  
  m_pSubBand = NULL;

  for(i = 0;i < RegisterSize;i++) {
    m_pRegister[i] = NULL;
    m_pMirrored[i] = NULL;
  }
}
///

/// Band::~Band
// Delete this band and the entire band hierarchy.
Band::~Band(void)
{
  int i;
  
  delete m_pSubBand;
  
  // The mirrored pointers do not get deleted. No need to
  // as they are just copies of the original line pointers.
  for(i = 0;i < RegisterSize;i++) {
    delete m_pRegister[i];
  }
}
///

/// Band::SubBandOf
// Get the indicated sub-band of this band or NULL in case there is none.
class Band *Band::SubBandOf(void)
{
  if (m_ucResolution > 0 && m_pSubBand == NULL) {
    ULONG width  = ((WidthOf()  - 1) >> 1) + 1;
    ULONG height = ((HeightOf() - 1) >> 1) + 1;
    m_pSubBand   = new Band(width,height,m_ucResolution - 1,(m_ucResolution == 1)?(false):(m_bKeepHP));
  }
  return m_pSubBand;
}
///

/// Band::PushLine
// Push a line for transformation into this band.
void Band::PushLine(const class Line *data)
{
  assert(m_lY < LONG(HeightOf()));
  //
  // Store the data in the matrix before we proceed.
  if (!m_bKeepHP)
    data->Deinterleave(&m_Coefficients.At(0,m_lY),false);
  //
  if (m_ucResolution > 0) {
    if (HeightOf() == 1) {
      class Line *line = NewLine(m_lY);
      // Single line case. Do not transform.
      data->Deinterleave(line->Origin(),false);
      // Just push the data into the subbands.
      line->MirrorExtend();
      Filter::HLift(line);
      //
      // This is only the low-pass signal. Keep that in the filters if required.
      if (m_bKeepHP) {
	line->Deinterleave(&m_HL.At(0,m_ulYO),true);
	m_ulYO++;
      }
      SubBandOf()->PushLine(line);
      m_lY++;
    } else {
      bool cont;
      class Line *line = NewLine(m_lY);
      data->Deinterleave(line->Origin(),false); // Extract only the low-pass pixels.
      //
      do {
	//
	// We only work on odd clocks.
	if (m_lY & 1) {
	  class Line *top2,*top1,*bot1,*bot2;
	  class Line *center;
	  //
	  // Required to extend the lines by mirroring? If so, do now.
	  if (m_bExtend)
	    MirrorExtend();
	  //
	  // If a line run into the odd working position, grab neighbour lines and do the job.
	  if ((center = m_pRegister[OddWorkingPos])) {
	    top2 = m_pRegister[OddWorkingPos - 3]?m_pRegister[OddWorkingPos - 3]:m_pMirrored[OddWorkingPos - 3];
	    top1 = m_pRegister[OddWorkingPos - 1]?m_pRegister[OddWorkingPos - 1]:m_pMirrored[OddWorkingPos - 1];
	    bot1 = m_pRegister[OddWorkingPos + 1]?m_pRegister[OddWorkingPos + 1]:m_pMirrored[OddWorkingPos + 1];
	    bot2 = m_pRegister[OddWorkingPos + 3]?m_pRegister[OddWorkingPos + 3]:m_pMirrored[OddWorkingPos + 3];
	    //
	    Filter::VLift1(top2,top1,center,bot1,bot2);
	  }
	  //
	  // If a line run into the odd working position, grab neighbour lines and do the job.
	  if ((center = m_pRegister[EvenWorkingPos])) {
	    top2 = m_pRegister[EvenWorkingPos - 3]?m_pRegister[EvenWorkingPos - 3]:m_pMirrored[EvenWorkingPos - 3];
	    top1 = m_pRegister[EvenWorkingPos - 1]?m_pRegister[EvenWorkingPos - 1]:m_pMirrored[EvenWorkingPos - 1];
	    bot1 = m_pRegister[EvenWorkingPos + 1]?m_pRegister[EvenWorkingPos + 1]:m_pMirrored[EvenWorkingPos + 1];
	    bot2 = m_pRegister[EvenWorkingPos + 3]?m_pRegister[EvenWorkingPos + 3]:m_pMirrored[EvenWorkingPos + 3];
	    //
	    Filter::VLift2(top2,top1,center,bot1,bot2);
	  }
	  //
	  // Make room for two new lines. End of data reached?
	  cont = ShiftLineRegister();
	} else cont = true;
	//
	// Next line.
	m_lY++;
	//
	// More lines expected?
	// If at end of buffer, cleanup.
	if (cont && m_lY >= LONG(HeightOf())) {
	  class Line *&line = NewLine(m_lY);
	  delete line;
	  line      = NULL;
	  // We just inserted a NULL into the register,
	  // turn mirroring back on.
	  m_bExtend = true;
	} else break;
	//
      } while(true);
    }
  } else {
    m_lY++;
  }
#if CHECK_LEVEL > 0
  if (m_lY >= LONG(HeightOf()))
    SaveBand();
#endif
}
///

/// Band::ShiftLineRegister
// Advance the line shift register by two, push lines at the exit
// positions into the child bands, make room for new lines.
bool Band::ShiftLineRegister(void)
{
  class Line *even,*odd;
  bool res;
  int i;
  //
  // Check whether we have output at the register.
  even = m_pRegister[0];
  odd  = m_pRegister[1];
  //
  if ((res = (odd && even))) {
    // The odd line contains the high-pass. We do not need
    // them at all.
    //
    even->MirrorExtend();
    Filter::HLift(even);
    //
    // Keep the resulting lines.
    if (m_bKeepHP) {
      even->Deinterleave(&m_HL.At(0,m_ulYO),true);
      odd->MirrorExtend();
      Filter::HLift(odd);
      odd->Deinterleave(&m_LH.At(0,m_ulYO),false);
      odd->Deinterleave(&m_HH.At(0,m_ulYO),true);
      m_ulYO++;
    }
    //
    // Now push into the low-pass.
    SubBandOf()->PushLine(even);
    // Line is done.
    //
    // As we now removed lines from the top, extension is no longer necessary.
    m_bExtend = false;
  }
  //
  // Move the lines up by two, make new room, insert old lines back into the bottom to
  // make them available as new buffers.
  for(i = 2;i < RegisterSize;i++) {
    m_pRegister[i - 2] = m_pRegister[i];
  }
  m_pRegister[i - 2] = even;
  m_pRegister[i - 1] = odd;

  return res;
}
///

/// Band::NewLine
// Accquire a new line for the indicated Y position, or recycle one.
class Line *&Band::NewLine(LONG y)
{
  int pos = (y & 1) + RegisterSize - 2;;
  class Line *&line = m_pRegister[pos];
  //
  // Is there one we can recycle?
  if (line == NULL)
    line = new Line(WidthOf());
  //
#if CHECK_LEVEL > 0
  line->m_lY = y;
#endif
  return line;
}
///

/// Band::MirrorExtend
// extend the lines by mirroring by inserting line pointers to otherwise
// NULL-lines in the register. This is not very elegant, but it should be
// working at least.
void Band::MirrorExtend(void)
{
  int i;
  bool cont;
  int topsrc,botsrc; // first and last valid line offset.
  int topdst,botdst;

  for(i = 0,topsrc = botsrc = -1;i < RegisterSize;i++) {
    if (m_pRegister[i] != NULL) {
      if (topsrc < 0)
	topsrc = i;
      botsrc   = i;
    }
  }

  //
  // Now mirror the lines over, one by another.
  topsrc++;
  topdst = topsrc - 2;
  botsrc--;
  botdst = botsrc + 2;

  do {
    cont = false;
    if (topdst >= 0 && topsrc < RegisterSize) {
      m_pMirrored[topdst] = m_pRegister[topsrc]?m_pRegister[topsrc]:m_pMirrored[topsrc];
      topdst--;
      topsrc++;
      cont = true;
    }
    if (botsrc >= 0 && botdst < RegisterSize) {
      m_pMirrored[botdst] = m_pRegister[botsrc]?m_pRegister[botsrc]:m_pMirrored[botsrc];
      botdst++;
      botsrc--;
      cont = true;
    }
  } while(cont);
}
///

/// Band::SaveBand
#if CHECK_LEVEL > 0
#include "std/stdio.hpp"
// Save this band as PGM image for cross-check.
void Band::SaveBand(void)
{
  ULONG x,y;
  char filename[30];
  FILE *stream;

  if (!m_bKeepHP) {
    snprintf(filename,30,"img_%d.pgm",m_ucResolution);
    stream = fopen(filename,"wb");
    fprintf(stream,"P5\n%u\t%u\n255\n",unsigned(WidthOf()),unsigned(HeightOf()));
    for(y = 0;y < HeightOf();y++) {
      for(x = 0;x < WidthOf();x++) {
	FLOAT f = m_Coefficients.Get(x,y);
	fputc((int)(f),stream);
      }
    }
    fclose(stream);
  }
  if (m_bKeepHP) {
    for(int band = 1;band <= 3;band++) {
      Matrix<FLOAT> &b = SubBandOf(band);
      snprintf(filename,30,"img_%d_%d.pgm",band,m_ucResolution);
      stream = fopen(filename,"wb");
      fprintf(stream,"P5\n%u\t%u\n255\n",unsigned(b.WidthOf()),unsigned(b.HeightOf()));
      for(y = 0;y < b.HeightOf();y++) {
	for(x = 0;x < b.WidthOf();x++) {
	  FLOAT f = b.Get(x,y) + 128.0;
	  fputc((int)(f),stream);
	}
      }
      fclose(stream);
    }
  }
}
#endif
///

