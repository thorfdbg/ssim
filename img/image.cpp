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
** $Id: image.cpp,v 1.7 2008-12-28 22:40:30 thor Exp $
**
*/

/// Includes
#include "img/image.hpp"
#include "img/component.hpp"
#include "global/exceptions.hpp"
#include "wavelet/line.hpp"
#include "std/stdio.hpp"
#include "std/math.hpp"
#include "io/bytestream.hpp"
///

/// logint2
// Compute the logarithm to the base of two
// as an integer approximation.
static int logint2(unsigned int v)
{
  int c = 0;

  while(v > 1) {
    v >>= 1;
    c++;
  }
  
  return c;
}
///

/// Image::Image
// Create a new image
Image::Image(void)
  : m_usComponents(0), m_ppComponentArray(NULL), m_ppLineArray(NULL)
{
}
///

/// Image::~Image
Image::~Image(void)
{

  if (m_ppComponentArray) {
    UWORD i;
    for(i=0;i<m_usComponents;i++) {
      delete m_ppComponentArray[i];
    }
    delete[] m_ppComponentArray;
  } 
  if (m_ppLineArray) {
    UWORD i;
    for(i=0;i<m_usComponents;i++) {
      delete m_ppLineArray[i];
    }
    delete[] m_ppLineArray;
  }
}
///

/// Image::Put
void Image::Put(UWORD idx, class Component *pComponent) 
{
    UWORD i=0;
    class Component **m_ppTempComponentArray;

    // Is it really the next component?
    if ((idx-m_usComponents) == 0) 
    {
        // Creates a new component array
        m_ppTempComponentArray = new class Component*[idx+1];
        for (i=0; i<m_usComponents; i++)
             m_ppTempComponentArray[i]=m_ppComponentArray[i];
        m_ppTempComponentArray[idx] = pComponent;

        // DO NOT DISPOSE THE OLD COMPONENTS BECAUSE THEY WILL BE REUSED
        delete[] m_ppComponentArray;

        // Makes the new component array public to the class.
        m_ppComponentArray = m_ppTempComponentArray;
        
        m_usComponents++;
    } else {
        Throw(OutOfRange,"Image::Put","index is not correct");
    }
}
///

/// Image::ReadNumber
// Read an ascii string from the input file,
// encoding a number. This number gets returned. Throws on error.
LONG Image::ReadNumber(class ByteStream *from)
{
  LONG number   = 0;     // integer number (so far)
  bool negative = false; // sign of the number (true if negative);
  bool valid    = false; // gets true as soon as we get at least one valid digit.
  LONG in;

  //
  // Skip any leading blanks
  SkipBlanks(from);
  //
  in = from->Get();
  if (in == '+') {
    // number is positive, do nothing
  } else if (in == '-') {
    // number is negative, invert sign.
    negative = true;
  } else {
    // no sign, put data back.
    from->LastUnDo();
  }
  // Skip another block of blanks.
  SkipBlanks(from);
  // Now read digits, one by one.
  do {
    in = from->Get();
    if (in < '0' || in > '9')
      break; // stop on first invalid input.
    // Check whether the number would overflow.
    if (number >= 214748364)
      Throw(OutOfRange,"Image::ReadNumber","input number is too large");
    // Otherwise, add the digit.
    number = number * 10 + in - '0';
    valid  = true;
  } while(true);

  // put back the last digit, we should not have read it as it is
  // not part of the number.
  from->LastUnDo();
  //
  // Now check whether we got at least a single valid digit. If
  // not, there is no valid number to deliver.
  if (!valid)
    Throw(InvalidParameter,"Image::ReadNumber","input number is invalid");
  //
  if (negative)
    number = -number;

  return number;
}
///

/// Image::WriteNumber
// Write an Ascii string to a bytestream.
void Image::WriteNumber(class ByteStream *to,LONG number)
{
  char buf[11]; // we need at most 11 digits to represent it.
  char *out = buf;
  //
  sprintf(buf,"%d",int(number));
  //
  while(*out) {
    to->Put(*out);
    out++;
  }
}
///

/// Image::SkipBlanks
// Skip blank spaces in the bytestream.
void Image::SkipBlanks(class ByteStream *from)
{
  LONG ch;

  do {
    ch = from->Get();
  } while(ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r');
  //
  // Put the last read character back into the stream.
  from->LastUnDo();
}
///

/// Image::SkipComment
// Skip comment lines starting with #
void Image::SkipComment(class ByteStream *in)
{
  LONG c;
  //
  //
  do {
    c = in->Get();
    // Skip blanks, white spaces, etc.
  } while(c == ' ' || c == '\t' || c == '\r');
  //
  // Check the last character. If a new line, check for comments.
  if (c == '\n') {
    // Possibly a # is following.
    while((c = in->Get()) == '#') {
      // A comment line.
      do {
	c = in->Get();
      } while(c != '\n' && c != ByteStream::Eof && c != '\r');
    }
    // put it back
    in->LastUnDo();
  } else {
    in->LastUnDo();
  }
}
///

/// Image::LoadPNM
// Load an image from an already open (binary) PPM or PGM file
// Throw in case the file should be invalid.
void Image::LoadPNM(class ByteStream *input,UBYTE declevels,bool keephp)
{
  LONG data;
  UWORD i;
  LONG width,height,precision;
  LONG x,y;
  UBYTE bits;
  //
  assert(m_ppComponentArray == NULL);
  //
  // Read the header of the file. This must be P6 for a
  // color image, and P5 for a grey-scale image. We currently
  // do not support ASCII images.
  data = input->Get();
  if (data != 'P')
    Throw(InvalidParameter,"Image::LoadPNM","input image stream is no valid PNM file");
  data = input->Get();
  if (data == '6') {
    // A color image. Allocate three components.
    m_usComponents     = 3;
  } else if (data == '5') {
    // A grey scale image. Allocate only one component.
    m_usComponents     = 1;
  } else {
    Throw(InvalidParameter,"Image::LoadPNM","input image is either invalid or an unsupported PNM type");
  }
  // Initialize the component array now.
  m_ppComponentArray = new class Component*[m_usComponents];
  for(i = 0;i<m_usComponents;i++)
    m_ppComponentArray[i] = NULL;
  //
  // Initialize the line array.
  m_ppLineArray      = new class Line*[m_usComponents];
  for(i = 0;i<m_usComponents;i++)
    m_ppLineArray[i]      = NULL;
  //
  SkipComment(input);
  // Read the width and the height off the stream. This will also
  // strip blanks we might have in the stream now.
  width     = ReadNumber(input);
  SkipComment(input);
  height    = ReadNumber(input);
  SkipComment(input);
  precision = ReadNumber(input);
  bits      = logint2(precision + 1);
  //
  // Make some consistency checks.
  if (width <= 0 || height <= 0)
    Throw(OutOfRange,"Image::LoadPNM","image dimensions are out of range");
  //
  if (precision <= 0 || precision >= 65536)
    Throw(OutOfRange,"Image::LoadPNM","image precision/bitdepth is out of range");
  //
  // Skip a single whitespace character.
  data = input->Get();
  // Check for MS-Dos line separator \r\n
  if (data == '\r') {
    data = input->Get();
    if (data != '\n') {
      // no MS-Dos separator, MacOs separator! Iek!
      input->LastUnDo();
      data = '\r';
    }
  }
  if (data != ' ' && data != '\n' && data != '\r' && data != '\t')
    Throw(InvalidParameter,"Image::LoadPNM","input image is not a valid PNM file");
  //
  // Now allocate the components.
  for(i=0;i<m_usComponents;i++) {
    m_ppComponentArray[i] = new class Component(width,height,false,bits,FLOAT(precision),declevels,keephp);
    m_ppLineArray[i]      = new class Line(width << 1); // requires twice the width, yuck!
  }
  //
  // Now read the data, component wise interleaved.
  for(y=0;y<height;y++) {
    for(x=0;x<width;x++) {
      for(i=0;i<m_usComponents;i++) {
	data = input->Get();
	// Fall over if this is an EOF.
	if (data == ByteStream::Eof)
	  Throw(Eof,"Image::LoadPNM","unexpected EOF detected in input image");
	//
	if (bits > 8) {
	  LONG dt = input->Get();
	  if (dt == ByteStream::Eof)
	    Throw(Eof,"Image::LoadPNM","unexpected EOF detected in input image");
	  data = (data << 8) | dt;
	}
	//
	if (data < 0 || data > precision)
	  Throw(OutOfRange,"Image::LoadPNM","the input image contains invalid pixels");
	//
	// Insert the data. We could do that a bit faster with pointers, but
	// so what...
	m_ppLineArray[i]->At(x<<1) = data;
      }
    }
    for(i=0;i<m_usComponents;i++) {
      m_ppComponentArray[i]->PushLine(m_ppLineArray[i]);
    }
  }
}
///
