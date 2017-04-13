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
** $Id: filestream.cpp,v 1.4 2008-03-04 14:12:24 thor Exp $
**
*/

/// Includes
#include "io/filestream.hpp"
#include "std/assert.hpp"
#include "std/errno.hpp"
#include "global/exceptions.hpp"
///

/// FileStream::~FileStream
// Dispose the filestream. This will
// also try to flush the buffer, then
// shuts the thing down. No exceptions are
// allowed here.
FileStream::~FileStream(void)
{
  try {
    Close();
  } catch(...) {
    // We can't do anything useful here anyhow...
  }
  //
  // Check whether, due to an error, the stream
  // is still open. If so, shut it down now.
  if (m_pFile)
    fclose(m_pFile);
  delete[] m_pucBuffer;
}
///

/// FileStream::Fill
// Fill the buffer from the stream,
// return the number of bytes read.
LONG FileStream::Fill(void)
{
  size_t n;
  assert(m_pFile);
  assert(m_bOpenForWrite == false);
  
  n = fread(m_pucBuffer,1,m_ulBufSize,m_pFile);
  if (n == 0) {
    if (ferror(m_pFile)) {
      ThrowIo("FileStream::Fill","failed reading from the file");
    }
  }
  m_ulBufBytes = n;
  m_ulCounter += n;
  m_pucBufPtr  = m_pucBuffer;

  return n;
}
///

/// FileStream::Flush
// Write the buffer contents out to the file,
// the reset the buffer for the next block to write.
void FileStream::Flush(void)
{
  ULONG bytes;
  assert(m_pFile);
  assert(m_bOpenForWrite == true);
  //
  // Perform only an operation if there is something
  // in the buffer.
  if ((bytes = ULONG(m_pucBufPtr - m_pucBuffer))) {
    if (fwrite(m_pucBuffer,1,bytes,m_pFile) != bytes)
      ThrowIo("FileStream::Flush","failed writing bytes to the stream");
  }
  m_ulCounter += bytes;
  m_ulBufBytes = m_ulBufSize;
  m_pucBufPtr  = m_pucBuffer;
}
///

/// FileStream::OpenForRead
// Open a filestream for reading binary data.
void FileStream::OpenForRead(const char *path,ULONG bufsize)
{
  assert(m_pFile == NULL);
  assert(m_pucBuffer == NULL);
  //
  // Now perform the real open.
  m_pFile = fopen(path,"rb");
  if (m_pFile == NULL)
    ThrowIo("FileStream::OpenForRead","failed opening the file for reading");
  //
  // Now initalize the buffer for reading.
  m_ulBufSize     = bufsize;
  m_pucBuffer     = new UBYTE[bufsize];
  m_pucBufPtr     = m_pucBuffer;
  m_ulBufBytes    = 0;
  m_ulCounter     = 0;
  m_bOpenForWrite = false;
}
///

/// FileStream::OpenForWrite
// Open a filestream for reading binary data.
void FileStream::OpenForWrite(const char *path,ULONG bufsize)
{
  assert(m_pFile == NULL);
  assert(m_pucBuffer == NULL);
  //
  // Now perform the real open.
  m_pFile = fopen(path,"wb");
  if (m_pFile == NULL)
    ThrowIo("FileStream::OpenForWrite","failed opening the file for writing");
  //
  // Now initalize the buffer for reading.
  m_ulBufSize     = bufsize;
  m_pucBuffer     = new UBYTE[bufsize];
  m_pucBufPtr     = m_pucBuffer;
  m_ulBufBytes    = bufsize;
  m_ulCounter     = 0;
  m_bOpenForWrite = true;
}
///

/// FileStream::Close
// Close a filestream
void FileStream::Close(void)
{
  if (m_pFile) {
    int rc;
    // If it is open for writing, flush it out.
    if (m_bOpenForWrite)
      Flush();
    // Now close it.
    rc = fclose(m_pFile);
    m_pFile = NULL;
    delete[] m_pucBuffer;
    m_pucBuffer = NULL;
    m_pucBufPtr = NULL;
    // Check whether we collected an Io error.
    // If so, throw.
    if (rc != 0)
      ThrowIo("FileStream::Close","failed flushing the buffer");
  }
}
///
