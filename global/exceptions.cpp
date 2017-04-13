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

/// Includes
#include "global/types.hpp"
#include "global/exceptions.hpp"
#include "std/stdio.hpp"
#include "std/stdarg.hpp"
///

/// CodecException::PrintException
// Print an exception reason into a string for informing the user
void CodecException::PrintException(class ExceptionPrinter &to) const
{
  if (type == Ex_IoErr) {
#if CHECK_LEVEL > 0
    to.PrintException("Error: %s in %s:\n%s\n",
		      ioerr,object,reason);
#else
     to.PrintException("Error: %s:\n%s\n",
		      ioerr,reason);
#endif
 } else {
    const char *exname;
    switch(type) {
    case Ex_Eof:
      exname = "unexpected eof";
      break;
    case Ex_InvalidParameter:
      exname = "invalid parameter";
      break;
    case Ex_OutOfRange:
      exname = "parameter out of range";
      break;
    case Ex_ObjectExists:
      exname = "object exists";
      break;
    case Ex_ObjectDoesntExist:
      exname = "object doesn't exist";
      break;
    case Ex_NotImplemented:
      exname = "not implemented";
      break;
    case Ex_NoMem:
      exname = "out of memory";
      break;
    case Ex_PhaseError:
      exname = "phase error";
      break;
    default: // shut up compiler, IoErr is handled above
      exname = "unknown error";
      break;
    }
#if CHECK_LEVEL > 0
    to.PrintException("Error: %s in %s, file %s line %d :\n%s\n",
		      exname,object,file,line,reason);
#else
    to.PrintException("Error: %s:\n%s\n",
		      exname,reason);
#endif
  }
}
///

/// CodecException::CodecException (varargs version)
CodecException::CodecException(const char *io,const char *who,const char *fmt,...)    
  : file(NULL), object(who), reason(NULL), 
    line(0), type(Ex_IoErr), ioerr(io), buffer(NULL)
{
  int size;
  char buf[4];
  va_list args;
  
  va_start(args,fmt);
  size   = vsnprintf(buf,1,fmt,args) + 1;
  // some buggy versions of vsnprintf return -1 on error/overflow. In this case,
  // truncate to 256 characters here
  if (size <= 0) size = 256;
  buffer = new char[size];
  vsnprintf(buffer,size,fmt,args);
  reason = buffer;
  va_end(args);
}
///

/// CodecException::CodecException (non-const char)
CodecException::CodecException(char *,ExType why,const char *who,const char *source,
			       int where,char *when)
: file(source), object(who), reason(NULL),
  line(where), type(why), ioerr(NULL), buffer(new char[strlen(when) + 1])
{ 
  strcpy(buffer,when);
  reason = buffer;
}  
///

/// CodecException::CodecException (non-const char, IO)
CodecException::CodecException(int,const char *io,const char *who,char *when)
  : file(NULL), object(who), reason(NULL), 
    line(0), type(Ex_IoErr), ioerr(io), buffer(new char[strlen(when) + 1])
{ 
  strcpy(buffer,when);
  reason = buffer;
}
///

/// CodecException::CodecException (copy constructor)
CodecException::CodecException(const CodecException &origin)
  : buffer(origin.buffer?(new char[strlen(origin.buffer) + 1]):NULL)
{  
  file   = origin.file;
  object = origin.object;
  line   = origin.line;
  type   = origin.type;
  ioerr  = origin.ioerr;
  reason = origin.reason; // might be NULL or not
  if (buffer) {
    strcpy(buffer,origin.buffer);
    reason = buffer;
  }
}
///
