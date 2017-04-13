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
** $Id: string.hpp,v 1.5 2013/05/20 20:21:10 Administrator Exp $
**
*/

#ifndef STD_STRING_HPP
#define STD_STRING_HPP

/// Includes
#include "config.h"


#if HAVE_STRING_H
# if !STDC_HEADERS && HAVE_MEMORY_H
#  include <memory.h>
# endif
# include <string.h>
#endif
#if HAVE_STRINGS_H
# include <strings.h>
#endif
///

/// Check for memchr function and provide a replacement if we don't have it.
#if !HAVE_MEMCHR
inline static void *memchr(const void *s, int c, size_t n) throw()
{
  const unsigned char *in = (const unsigned char *)s;

  if (n) {
    do {
      if (*in == c) return const_cast<void *>(s);
      in++;
    } while(--n);
  }
  return 0;
}
#endif
///

/// Check for the memmove function and provide a replacement if we don't have it.
#if !HAVE_MEMMOVE
// This is not overly efficient, but it should at least work...
inline static void *memmove(void *dest, const void *src, size_t n) throw()
{
  unsigned char       *d = (unsigned char *)dest;
  const unsigned char *s = (const unsigned char *)src;

  if (n) {
    if (s > d) {
      // Source is behind dest, copy ascending.
      do {
	*d = *s;
	d++,s++;
      } while(--n);
    } else if (s < d) {
      // Source is in front of dest, copy descending.
      s += n, d += n;
      do {
	--d,--s;
	*d = *s;
      } while(--n);
    }
  }
  return dest;
}
#endif
///

/// Check for the memset function and provide a replacement if we don't
#if !HAVE_MEMSET
inline static void *memset(void *s, int c, size_t n) throw()
{
  unsigned char *d = (unsigned char *)s;
  
  if (n) {
    do {
      *d = c;
      d++;
    } while(--n);
  }

  return const_cast<void *>(s);
}
#endif
///

/// Check for availibility of strcasecmp and implement it if it does not exist.
#ifdef _WIN32
# define strcasecmp(a,b) stricmp((a),(b))
#endif
#if !HAVE_STRCASECMP
#include <ctype.h>
inline static int strcasecmp(const char *s1, const char *s2) throw()
{
  int d;
  
  while(*s1) {
    d = toupper(*s1) - toupper(*s2); // this is possibly naive for some languages...
    if (d) return d;
    s1++,s2++;
  }
  return *s2;
}
#endif
///

/// Check for availibility of the strchr function and provide a replacement if there is none.
#if !HAVE_STRCHR
inline static char *strchr(const char *s, int c) throw()
{
  while(*s) {
    if (*s == c) return const_cast<char *>(s);
    s++;
  }
  return 0;
}
#endif
///

/// Check for strerror function
#if !HAVE_STRERROR
// we cannot really do much about it as this is Os depdendent. Yuck.
#define strerror(c) "unknown error"
#endif
///

/// Check for strrchr function and provide a replacement.
#if !HAVE_STRRCHR
inline static char *strrchr(const char *s, int c) throw()
{
  const char *t = s + strlen(s);
  
  while(t > s) {
    t--;
    if (*t == c) return const_cast<char *>(t);
  }
  return 0;
}
#endif
///

///
#endif
