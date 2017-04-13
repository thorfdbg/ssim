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
** $Id: bytestream.cpp,v 1.4 2008-03-04 14:12:24 thor Exp $
**
*/

/// Includes
#include "std/string.hpp"
#include "io/bytestream.hpp"
///

/// ByteStream::Read
LONG ByteStream::Read(UBYTE *buffer,ULONG size)
{
  ULONG bytesread = 0;

  while (size>=m_ulBufBytes) { // more bytes to read than in the buffer
    if (m_ulBufBytes) {
      memcpy(buffer,m_pucBufPtr,m_ulBufBytes); // copy all data over
      m_pucBufPtr  += m_ulBufBytes;         // required for correct fill
      bytesread    += m_ulBufBytes;
      buffer       += m_ulBufBytes;
      size         -= m_ulBufBytes;
      m_ulBufBytes  = 0;  // buffer is now empty
    }

    // Now fill up the buffer again.
    if (Fill() == 0) { // read zero bytes -> EOF. Don't loop forever
      return bytesread;
    }
  }

  // only a partial read from the buffer
  // now is size <= m_ulBufBytes, guaranteed.
  if (size) {
    memcpy(buffer,m_pucBufPtr,size);
    m_pucBufPtr  += size;
    m_ulBufBytes -= size;
    // buffer    += size;  // not needed
    // size      -= size;  // ditto
    bytesread    += size;
  }

  return bytesread;
}
///

/// ByteStream::Write
LONG ByteStream::Write(const UBYTE *buffer,ULONG size)
{
  ULONG byteswritten = 0;

  while(size>=m_ulBufBytes) { // write more bytes than fit into the buffer?
    if (m_ulBufBytes) {
      memcpy(m_pucBufPtr,buffer,m_ulBufBytes);  // copy the data over
      m_pucBufPtr  += m_ulBufBytes;         // required for correct flush
      byteswritten += m_ulBufBytes;
      buffer       += m_ulBufBytes;
      size         -= m_ulBufBytes;
      m_ulBufBytes  = 0;       // buffer is now empty
    }
    // Now write the buffer (or allocate one, at least)
    Flush();

    m_pucBufPtr  = m_pucBuffer;  // re-initiate the buffer pointer
    m_ulBufBytes = m_ulBufSize;  // re-initiate the byte counter
  }

  // size is now smaller than m_ulBufBytes, guaranteed!
  if (size) {
    memcpy(m_pucBufPtr,buffer,size);
    m_pucBufPtr  += size;
    m_ulBufBytes -= size;
    // buffer    += size;  // not needed
    // size      -= size;  // ditto
    byteswritten += size;
  }

  return byteswritten;
}
///
