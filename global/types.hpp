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
** $Id: types.hpp,v 1.5 2012-04-24 14:10:51 thor Exp $
**
*/

#ifndef GLOBAL_TYPES_HPP
#define GLOBAL_TYPES_HPP

#include "config.h"

// The following definitions setup a couple of
// user specific types to be used within the codec.
//
// Ideally, this should be generated thru autoconf. 
// (This has to happen later).

// 8 bit data types, signed, unsigned.
// Note that these should be compiler independent,
// requiring this to be adapted, e.g. by autoconf.

#if SIZEOF_CHAR != 1
# error "no 8 bit datatype available"
#endif
// unsigned 8 bit type
typedef unsigned char UBYTE;
// signed 8 bit type
typedef signed char BYTE;

#if SIZEOF_INT == 2
// unsigned 16 bit type
typedef unsigned int UWORD;
// signed 16 bit type
typedef signed int WORD;
#elif SIZEOF_SHORT == 2
// unsigned 16 bit type
typedef unsigned short UWORD;
// signed 16 bit type
typedef signed short WORD;
#else
# error "no 16 bit datatype available"
#endif

#if SIZEOF_LONG == 4
// unsigned 32 bit type
typedef unsigned long ULONG;
// signed 32 bit type
typedef signed long LONG;
#elif SIZEOF_INT == 4
// unsigned 32 bit type
typedef unsigned int ULONG;
// signed 32 bit type
typedef signed int LONG;
#endif

#ifdef HAVE_LONG_LONG
// unsigned 64 bit type
typedef unsigned long long UQUAD;
// signed 64 bit type
typedef signed long long QUAD;
#elif HAVE___INT64
typedef signed __int64 QUAD;
typedef unsigned __int64 UQUAD;
#else
# error "No 64 bit datatype available"
#endif


// short IEEE floating point type
typedef float FLOAT;
// long IEEE floating point type
typedef double DOUBLE;

// generic pointer type (A PoiNTeR)
typedef void *APTR;

// boolean
typedef bool BOOL;

// NULL pointer type. We use
// the GNU extension to get more warnings
// if something's wrong with it. (If available)
#ifndef NULL
#if HAS__NULL_TYPE
#define NULL (__null)
#else
#define NULL (0)
#endif
#endif

///
#endif
