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
** $Id: stdio.hpp,v 1.4 2008-03-04 14:12:24 thor Exp $
**
*/

#ifndef STD_STDIO_HPP
#define STD_STDIO_HPP

/// Includes
#include "config.h"
#include <stdio.h>
#if HAVE_STDARG_H
#include <stdarg.h>
#else
#error "requires the stdarg.h header"
#endif
///

/// Replacements for snprintf and vsnprintf
#if !HAVE_SNPRINTF
int snprintf(char *str, size_t size, const char *format, ...);
#elif defined( _WIN32 )
#define snprintf _snprintf
#endif
///

/// Replacements for vsnprintf
#if !HAVE_VSNPRINTF
int vsnprintf(char *str, size_t size, const char *format,va_list ap);
#elif defined( _WIN32 )
#define vsnprintf _vsnprintf
#endif
///

///
#endif
