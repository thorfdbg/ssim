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
** $Id: image.hpp,v 1.7 2008-12-28 22:40:30 thor Exp $
**
*/

#ifndef IMG_IMAGE_HPP
#define IMG_IMAGE_HPP

/// Includes
#include "global/types.hpp"
#include "std/assert.hpp"
///

/// Forwards
class Component;
class ByteStream;
class Line;
///

/// Image
// This simplistic class holds all components and all
// data of an image. This is typically RGB three component
// image data.
class Image {
  //
  // Number of components we have here.
  UWORD            m_usComponents;
  //
  // An array of components.
  class Component **m_ppComponentArray;
  //
  // An array of lines. They are temporaries.
  class Line      **m_ppLineArray;
  //
  // Service (not required elsewhere): Read an ascii string from the input file,
  // encoding a number. This number gets returned. Throws on error.
  LONG ReadNumber(class ByteStream *from);
  // Write an Ascii string to a bytestream.
  void WriteNumber(class ByteStream *to,LONG number);
  // Skip blank spaces in the bytestream.
  void SkipBlanks(class ByteStream *from);
  // Skip comment lines starting with #
  void SkipComment(class ByteStream *in);
  //
public:
  // Create an image for the given number of components. It is empty
  // afterwards.
  Image(void);
  //
  // Dispose the image again.
  ~Image(void);
  //
  // Extract a component of the image. This does
  // not hand out life-time control of the component.
  class Component &ComponentOf(UWORD idx) const
  {
    assert(m_ppComponentArray);
    assert(idx < m_usComponents);
    return *m_ppComponentArray[idx];
  }
  //
  // Get the number of components in here.
  UWORD ComponentCountOf(void) const
  {
    return m_usComponents;
  }
  //
  // Puts a component of the image
  // Wavelet Transform Group, 2003-12-21, MH
  void Put(UWORD idx, class Component *pComponent);
  //
  // Load an image from an already open (binary) PPM or PGM file
  // Throw in case the file should be invalid.
  // Wavelet-transform while loading, requires the number of levels
  // of the transformation.
  void LoadPNM(class ByteStream *input,UBYTE levels,bool keelhighpasses);
};
///

///
#endif
