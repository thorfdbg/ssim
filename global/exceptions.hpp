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
** $Id: exceptions.hpp,v 1.4 2008-03-04 14:12:24 thor Exp $
**
*/

#ifndef GLOBAL_EXCEPTION_HPP
#define GLOBAL_EXCEPTION_HPP

/// Includes
#include "std/string.hpp"
///

/// Class ExceptionPrinter
// This defines a class that can print output to somewhere
class ExceptionPrinter {
  //
  //
public:
  ExceptionPrinter(void)
  {}
  //
  virtual ~ExceptionPrinter(void)
  {}
  //
  //
  // The following method will be called to print out the exception.
  virtual void PrintException(const char *fmt,...) = 0;
  //
};
///

/// Class CodecException
// These classes are "thrown" on an exception
class CodecException {
public:
  enum ExType {
    //
    // Real exceptions start
    Ex_IoErr,             // IoError. The system specific errnum below says more
    Ex_Eof,               // unexpected eof
    Ex_InvalidParameter,  // a parameter was invalid
    Ex_OutOfRange,        // a parameter was out of range
    Ex_ObjectExists,      // created an object that exists already
    Ex_ObjectDoesntExist, // object did not yet exist
    Ex_NotImplemented,    // failed because method is not available (internal)
    Ex_NoMem,             // out of memory error
    Ex_PhaseError         // inconistent internal state
  };
private:
  //
  const char *file;       // name of the source file affected
  const char *object;     // name of the object that threw
  const char *reason;     // further textual description
  int         line;       // line number that caused the exception
  ExType      type;       // type of the exception
  const char *ioerr;      // system specific IO exception
  char       *buffer;     // for complex error messages: An allocated buffer
  //
public:
  // Exception cloner: We must also duplicate the buffer as we release it
  // within the destructor of the origin
  CodecException(const CodecException &origin);
  //
  //
  // Generate an exception by an emulator internal cause
  CodecException(char *,ExType why,const char *who,const char *source,
		 int where,const char *when)
    : file(source), object(who), reason(when),
      line(where), type(why), ioerr(NULL), buffer(NULL)
  { }  
  // The same, but without a constant source we need to copy
  CodecException(char *,ExType why,const char *who,const char *source,
		 int where,char *when);
  //
  // Generate an exception by an io error
  CodecException(int,const char *io,const char *who,const char *when)
    : file(NULL), object(who), reason(when), 
      line(0), type(Ex_IoErr), ioerr(io), buffer(NULL)
  { }
  // The same, but with non-constant reason argument that must be copied.
  CodecException(int,const char *io,const char *who,char *when);
  //
  // Generate a complex IoError exception
  CodecException(const char *io,const char *who,const char *fmt,...);
  //
  //
  ~CodecException(void)
  {   
    delete[] buffer;
  }
  //
  // Print an exception reason into a string for informing the user
  void PrintException(class ExceptionPrinter &to) const;
  //
  // Return the type of the exception for selective handling
  ExType TypeOf(void) const
  {
    return type;
  }
};
///

/// Exception macros
#define Throw(why,object,txt) throw(CodecException(NULL,CodecException::Ex_ ## why,object,__FILE__,__LINE__,txt))
#define ThrowIo(object,desc)  throw(CodecException(0,strerror(errno),object,desc))
///

///
#endif
    
