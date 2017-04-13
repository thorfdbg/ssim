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
** $Id: colortransformer.cpp,v 1.11 2009-04-07 19:36:01 thor Exp $
**
*/

/// Includes
#include "ctrafo/colortransformer.hpp"
#include "img/component.hpp"
#include "global/exceptions.hpp"
#include "std/math.hpp"
#include "std/assert.hpp"
///

/// Defines
//#define LINEAR_YCBCR
#define OUTPUT_GAMMA 1.0
///

/// ColorTransformer::ColorTransformer
ColorTransformer::ColorTransformer(void)
  : m_pdLookup(NULL), m_pdLMS(NULL)
{
  
}
///

/// ColorTransformer::~ColorTransformer
ColorTransformer::~ColorTransformer(void)
{
  delete[] m_pdLookup;
}
///

/// ColorTransformer::CreateLookup
// Create the lookup table.
void ColorTransformer::CreateLookup(ULONG scale)
{
  ULONG i;

  if (m_pdLookup == NULL) {
    m_pdLookup = new DOUBLE[scale];
    
    for(i = 0; i < scale;i++) {
      m_pdLookup[i] = sRGBTransfer(i,scale - 1);
    }
  }
}
///

/// ColorTransformer::CreateLMSLookup
// Create the LMS lookup table.
void ColorTransformer::CreateLMSLookup(ULONG scale)
{
  ULONG i;

  if (m_pdLMS == NULL) {
    m_pdLMS = new DOUBLE[scale];
    for(i = 0;i < scale;i++) {
      m_pdLMS[i] = LMSTransfer(DOUBLE(i) / (scale - 1));
    }
  }
}
///

/// ColorTransformer::ForwardsTransform
void ColorTransformer::ForwardsTransform(class Matrix<FLOAT> &rm,class Matrix<FLOAT> &gm,class Matrix<FLOAT> &bm,
					 ULONG max,ULONG lmsscale)
{ 
  ULONG xp,yp,width,height;
#ifdef LINEAR_YCBCR
  double kneeslope =  (1.055 * pow( 0.0031308, OUTPUT_GAMMA * 1./2.4 ) - 0.055) / 0.0031308;  //
#endif
  width  = rm.WidthOf();
  height = rm.HeightOf();
  //
  for(yp = 0;yp<height;yp++) {
    for(xp = 0;xp<width;xp++) {
      DOUBLE r,g,b;
      // Get color values. (The slow way).
      r  = rm.Get(xp,yp);
      g  = gm.Get(xp,yp);
      b  = bm.Get(xp,yp);
      //
#if defined(ITP)
      DOUBLE x,y,z;
      DOUBLE l,m,s,i,t,p;
      //
      // Clip overflows.
      // First step: Transform sRGB to R'G'B'
      if (r <= 0.0) {
	r = 0;
      } else if (r >= max) {
	r = 1.0;
      } else {
	r = m_pdLookup[ULONG(r)];
      }
      if (g <= 0.0) {
	g = 0;
      } else if (g >= max) {
	g = 1.0;
      } else {
	g = m_pdLookup[ULONG(g)];
      }
      if (b <= 0.0) {
	b = 0;
      } else if (b >= max) {
	b = 1.0;
      } else {
	  b = m_pdLookup[ULONG(b)];
      }
      //
      // Note that we need the D65 illuminant, which is the
      // illuminant of sRGB / Rec. 709.
      x  = 0.412453 * r + 0.357580 * g + 0.180423 * b;
      y  = 0.212671 * r + 0.715160 * g + 0.072169 * b;
      z  = 0.019334 * r + 0.119193 * g + 0.950227 * b;
      //
      // Convert from xyz to LMS. Yes, this could be done in one
      // step, but for simplicity...
      l  =  0.4002 * x + 0.7075 * y - 0.0807 * z;
      m  = -0.2280 * x + 1.1500 * y + 0.0612 * z;
      s  = 0.9184  * z;
      //
      // Nonlinear transformation into primed coordinates.
      l *= lmsscale;
      m *= lmsscale;
      s *= lmsscale;
      l  = (l >= 0.0)?(m_pdLMS[ULONG(l)]):(-m_pdLMS[ULONG(-l)]);
      m  = (m >= 0.0)?(m_pdLMS[ULONG(m)]):(-m_pdLMS[ULONG(-m)]);
      s  = (s >= 0.0)?(m_pdLMS[ULONG(s)]):(-m_pdLMS[ULONG(-s)]);
      //
      // Transfer again from lms to IPT
      i  = 0.4    * l + 0.4    * m + 0.2    * s;
      p  = 4.4550 * l - 4.8510 * m + 0.3960 * s;
      t  = 0.8056 * l + 0.3572 * m - 1.1628 * s;
      //
      // Put the stuff back.
      rm.Put(xp,yp,i);
      gm.Put(xp,yp,p);
      bm.Put(xp,yp,t);
#elif defined(LUV)
      //
      // First convert sRGB->XYZ
      DOUBLE x,y,z;
      DOUBLE xn,yn,zn,n;
      DOUBLE l,u,v;
      DOUBLE un,vn;
      //
      // Clip overflows.
      // First step: Transform sRGB to R'G'B'
      if (r <= 0.0) {
	r = 0;
      } else if (r >= max) {
	r = 1.0;
      } else {
	r = m_pdLookup[ULONG(r)];
      }
      if (g <= 0.0) {
	g = 0;
      } else if (g >= max) {
	g = 1.0;
      } else {
	g = m_pdLookup[ULONG(g)];
      }
      if (b <= 0.0) {
	b = 0;
      } else if (b >= max) {
	b = 1.0;
      } else {
	b = m_pdLookup[ULONG(b)];
      }
      //
      // Note that we need the D65 illuminant, which is the
      // illuminant of sRGB / Rec. 709.
      x  = 0.412453 * r + 0.357580 * g + 0.180423 * b;
      y  = 0.212671 * r + 0.715160 * g + 0.072169 * b;
      z  = 0.019334 * r + 0.119193 * g + 0.950227 * b;
      //
      // Initialize with the D65 white-point.
      xn = 0.95056;
      yn = 1.0;
      zn = 1.089050;
      //
      // Compute L*,u*,v*
      l  = 116.0 * pow(y,1.0/3.0) - 16.0;
      n  = (x + 15*y + 3 * z);
      // Color coordinates for black are arbitrary. Set them zero.
      u  = (n != 0.0)?(4 * x / n):(0.0);
      v  = (n != 0.0)?(9 * y / n):(0.0);
      n  = (xn + 15 * yn + 3 * zn);
      un = 4 * xn / n;
      vn = 4 * yn / n;
      u  = 13 * l * (u - un);
      v  = 13 * l * (v - vn);
      
      assert(!isnan(l) && !isnan(u) && !isnan(v));
      
      rm.Put(xp,yp,l);
      gm.Put(xp,yp,u);
      bm.Put(xp,yp,v);
#elif defined(LINEAR_YCBCR)
      DOUBLE yv,cb,cr;
      DOUBLE rp,gp,bp;
      //
      r  /= 255.0;
      g  /= 255.0;
      b  /= 255.0;
      // Linearized RGB/YCbCr
      rp  = (r <= 0.04045 ? r / 12.92 : pow( (r + 0.055) / 1.055, 2.4));
      gp  = (g <= 0.04045 ? g / 12.92 : pow( (g + 0.055) / 1.055, 2.4));
      bp  = (b <= 0.04045 ? b / 12.92 : pow( (b + 0.055) / 1.055, 2.4));
      //
      // Transform to linear YCbCr, note that Y is in the *linear* domain.
      yv =  0.299*rp   + 0.587  *gp + 0.114*bp;
      cb = -0.16875*r  - 0.33126*g  + 0.5*b;
      cr =  0.5*r      - 0.41869*g  - 0.08131*b;
      yv = (yv <= 0.0031308 ? yv * kneeslope : 1.055 * pow( yv, OUTPUT_GAMMA * 1./2.4 ) - 0.055);
      // Put the stuff back.
      rm.Put(xp,yp,yv * 255.0);
      gm.Put(xp,yp,cb * 255.0);
      bm.Put(xp,yp,cr * 255.0);	
#else
      //
      // Regular YCbCr
      DOUBLE yv,cb,cr;
      // Run the RGB->YCbCr transformation
      yv =  0.299*r   + 0.587  *g + 0.114*b;
      cb = -0.16875*r - 0.33126*g + 0.5*b;
      cr =  0.5*r     - 0.41869*g - 0.08131*b;
      // Put the stuff back.
      rm.Put(xp,yp,yv);
      gm.Put(xp,yp,cb);
      bm.Put(xp,yp,cr);
#endif
    }
  }
}
///

/// ColorTransformer::ForwardsTransform
// Run a forwards transformation of the data
// in case we have three or more components. 
// Run it only on the first three.
void ColorTransformer::ForwardsTransform(class Image *img)
{
  FLOAT scale;
  UWORD i,j;
  ULONG width,height;
  ULONG max = 0,lmsscale = 0;
  class Component *red,*green,*blue;
  UBYTE levels;
  //
  assert(img);
  if (img->ComponentCountOf() == 1) { 
    img->ComponentOf(0).NameOf()   = "Y";
    img->ComponentOf(0).WeightOf() = 1.0;
    return; // nothing to do.
  }
  if (img->ComponentCountOf() < 3)
    Throw(NotImplemented,"ColorTransformer::ForwardsTransform",
	  "no color transformation for a two-component image available");
  //
  // Get the components.
  red    = &img->ComponentOf(0);
  levels = img->ComponentOf(0).ScalesOf();
  green  = &img->ComponentOf(1);
  blue   = &img->ComponentOf(2);
  assert(red && green && blue);
  //
#if defined(LUV) || defined(ITP)
  // Create the input lookup table to convert RGB to R'G'B'.
  max = (1UL << red->BitDepthOf());
  CreateLookup(max);
  // Precision of 16 bit should be hopefully sufficient.
  CreateLMSLookup(lmsscale = (1UL<<16));
#endif
  //
  // Ok, here we have at least three components. Check for their scales. They
  // should be approximately equal.
  scale  = red->ScaleOf();
  width  = red->WidthOf();
  height = red->HeightOf();
  //
  for(i = 0;i<3;i++) {
    if (fabs(img->ComponentOf(i).ScaleOf() - scale) > 1)
      Throw(NotImplemented,"ColorTransformer::ForwardsTransform",
	    "no color transform for unequally scaled components available");
    if (img->ComponentOf(i).WidthOf()  != width ||
	img->ComponentOf(i).HeightOf() != height)
      Throw(InvalidParameter,"ColorTransformer::ForwardsTransform",
	    "components have unequal dimensions, cannot transform");
  }
  //
  // Now run the transformation process.
  for(i = 1;i <= levels;i++) {
    if (i == levels || red->KeepsSubbands() == false) {
      Matrix<FLOAT> &rm = red->GetScale(i);
      Matrix<FLOAT> &gm = green->GetScale(i);
      Matrix<FLOAT> &bm = blue->GetScale(i);
      ForwardsTransform(rm,gm,bm,max,lmsscale);
    } else {
      for(j = 1;j < 3;j++) { 
	Matrix<FLOAT> &rm = red->GetBand(i,j);
	Matrix<FLOAT> &gm = green->GetBand(i,j);
	Matrix<FLOAT> &bm = blue->GetBand(i,j);
	ForwardsTransform(rm,gm,bm,max,lmsscale);
      }
    }
  }
  img->ComponentOf(1).IsSigned() = true;
  img->ComponentOf(2).IsSigned() = true; 
  //
  // Define the weights - used from the current reference implementation.
#if defined(ITP)
  // TO BE DONE
#elif defined(LINEAR_YCBCR)
  img->ComponentOf(0).WeightOf() = 0.91;
  img->ComponentOf(1).WeightOf() = 0.02;
  img->ComponentOf(2).WeightOf() = 0.07;
  img->ComponentOf(0).NameOf()   = "Y";
  img->ComponentOf(1).NameOf()   = "Cb";
  img->ComponentOf(2).NameOf()   = "Cr"; 
#else
  img->ComponentOf(0).WeightOf() = 0.95;
  img->ComponentOf(1).WeightOf() = 0.02;
  img->ComponentOf(2).WeightOf() = 0.03;
  img->ComponentOf(0).NameOf()   = "Y";
  img->ComponentOf(1).NameOf()   = "Cb";
  img->ComponentOf(2).NameOf()   = "Cr";
#endif      
}
///
