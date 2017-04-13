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

#include "vifIndex.hpp"
#include "global/matrix.hpp"
#include "std/math.hpp"
#include "std/stdio.hpp"

#ifndef M_PI
#define M_PI 3.14159326
#endif

/// Weights
const DOUBLE vifIndex::K1 = 0.01;
const DOUBLE vifIndex::K2 = 0.03;
const DOUBLE vifIndex::K  = 0.04; // the value in the article is 0.01, the one in the matlab code is 0.04;
#undef DO_WINDOWING // set to enable filtering over a gaussian window function.
///

/// vifIndex::CreateGaussFilter
// Create the gauss windowing function
Matrix<double> vifIndex::CreateGaussFilter(ULONG w, ULONG h)
{
  Matrix<double> gauss(w,h);
  ULONG d1    = w/2;
  ULONG d2    = h/2;
  ULONG x,y;
  double Norm = 0.0;
  
  for (x = 0;x<w;x++){
    for(y = 0;y<h;y++){
      double g = exp(-(((x - d1) * (x - d1) + (y - d2) * (y - d2))/2.25));
      Norm    += g;
      gauss.Put(x,y,g);
    }
  }
  for (x = 0;x<w;x++){
    for(y = 0;y<h;y++){
      gauss.At(x,y) = gauss.Get(x,y) / Norm;
    }
  }
  return gauss;
}
///

/// vifIndex::CreateHammingWindow
// This method creates a Hamming window of the given size.
Matrix<DOUBLE> vifIndex::CreateHammingWindow(ULONG w, ULONG h)
{
  ULONG x,y;
  Matrix<double> window(w,h);
  double xz = double(w - 1) * 0.5;
  double yz = double(h - 1) * 0.5;
  double xf = double(w + 1) * 0.5;
  double yf = double(h + 1) * 0.5;
  double norm = 0.0;

  for(y = 0;y < h;y++) {
    for(x = 0;x < w;x++) {
      double g = cos((x - xz) / xf * M_PI * 0.5) * cos((y - yz) / yf * M_PI * 0.5);
      norm     += g;
      window.At(x,y) = g;
    }
  }
  
  norm /= 8 * 8;
  
  for(y = 0;y < h;y++) {
    for(x = 0;x < w;x++) {
      window.At(x,y) = window.At(x,y) / norm;
    }
  }
  
  return window;
}
///

/// vifIndex::vifFactor
// compute the ssim index for a complete image
double vifIndex::vifFactor(const Image& img1,const Image& img2,int ncpus,bool bylevel) const
{
  double numerator   = 0.0;
  double denominator = 0.0;

  switch (img1.ComponentCountOf()) {
  case 1:
    if (bylevel)
      printf("Y component:\n");
    vifFactorScales(img1.ComponentOf(0),img2.ComponentOf(0),img1.ComponentOf(0).ScaleOf(),ncpus,bylevel,
		    numerator,denominator);
    break;
  case 3:
    {
      DOUBLE yn = 0.0,cbn = 0.0,crn = 0.0;
      DOUBLE yd = 0.0,cbd = 0.0,crd = 0.0;
      
      if (bylevel)
	printf("Y component:\n");
      vifFactorScales(img1.ComponentOf(0),img2.ComponentOf(0),img1.ComponentOf(0).ScaleOf(),ncpus,bylevel,
		 yn,yd);
      if (bylevel)
	printf("\nCb component:\n");
      vifFactorScales(img1.ComponentOf(1),img2.ComponentOf(1),img1.ComponentOf(1).ScaleOf(),ncpus,bylevel,
		cbn,cbd);
      if (bylevel)
	printf("\nCr component:\n");
      vifFactorScales(img1.ComponentOf(2),img2.ComponentOf(2),img1.ComponentOf(2).ScaleOf(),ncpus,bylevel,
		crn,crd);
      numerator   = yn * 0.54 + cbn * 0.19 + crn * 0.27;
      denominator = yd * 0.54 + cbd * 0.19 + crd * 0.27;
    }
    break;
  default:
    return 0;
  }
  return numerator / denominator;
}
///

/// vifIndex::pthread_entry
// The entry point to start a vif thread.
#ifndef NO_POSIX
void *vifIndex::pthread_entry(void *arg)
{
  struct vifTask *task = (struct vifTask *)arg;

  task->that->vifFactor(*task->img1,*task->img2,task->scale,
			task->offset,task->interleave,task->var,
			task->numerator,task->denominator);

  return task;
}
#endif
///

/// vifIndex::vifFactor
// Compute the vif factor for a multi-core CPU with potentially using threads.
void vifIndex::vifFactorScales(Component& img1,Component& img2,DOUBLE scaling,int ncpus,bool bylevel,
			       double &numerator,double &denominator) const
{
  int scale,nscales = img1.ScalesOf();
  int band;
  
  for(scale = 1;scale <= nscales;scale++) {
    if (scale == nscales) {
      vifMultiCoreFactor(img1.GetScale(scale),img2.GetScale(scale),scaling,ncpus,numerator,denominator);
    } else {
      for(band = 1;band <= 3;band++) {
	vifMultiCoreFactor(img1.GetBand(scale,band),img2.GetBand(scale,band),scaling,ncpus,numerator,denominator);
      }
    }
    if (bylevel)
      printf("log vif value for scale %d: %f\n",scale,-20.0 * log(1.0 - numerator/denominator) / log(10.0));
  }
}
///

/// vifIndex::vifMultiCoreFactor
// Compute the vif factor for a multi-core CPU with potentially using threads.
void vifIndex::vifMultiCoreFactor(const Matrix<FLOAT>& c1,const Matrix<FLOAT>& c2,DOUBLE scaling,int ncpus,
				  double &numerator,double &denominator) const
{    
  double var = variance(c1);

#ifdef NO_POSIX
  vifFactor(c1,c2,scaling,0,1,var,numerator,denominator);
#else
  if (ncpus == 1) {
    vifFactor(c1,c2,scaling,0,1,var,numerator,denominator);
  } else {
    int i;
    struct vifTask *tasks = new vifTask[ncpus];
    //
    for(i = 0;i < ncpus;i++) {
      tasks[i].that        = this;
      tasks[i].img1        = &c1;
      tasks[i].img2        = &c2;
      tasks[i].offset      = i;
      tasks[i].interleave  = ncpus;
      tasks[i].scale       = scaling;
      tasks[i].numerator   = 0.0;
      tasks[i].denominator = 0.0;
      tasks[i].var         = var;
      pthread_attr_init(&tasks[i].attr);
      pthread_attr_setdetachstate(&tasks[i].attr,PTHREAD_CREATE_JOINABLE);
      pthread_create(&tasks[i].pid,&tasks[i].attr,&vifIndex::pthread_entry,tasks+i);
    }
    //
    // Now wait for all threads to complete, and collect their results.
    for(i = 0;i < ncpus;i++) {
      pthread_join(tasks[i].pid,NULL);
      numerator   += tasks[i].numerator;
      denominator += tasks[i].denominator;
    }
    delete[] tasks;
  }
#endif
}
///

/// vifIndex::variance
// Computes the variance = \sigma_x^2 of the given matrix.
double vifIndex::variance(const Matrix<FLOAT> &img)
{
  ULONG x,y;
  ULONG width  = img.WidthOf();
  ULONG height = img.HeightOf();
  double mean  = 0.0;
  double sum   = 0.0;
  double var;

  for(y = 0;y < height;y++) {
    for(x = 0;x < width;x++) {
      mean += img.At(x,y);
      sum  += img.At(x,y) * img.At(x,y);
    }
  }

  mean /= width * height;
  sum  /= width * height;
  var   = sum - mean * mean;

  if (var < 0.0)
    var = 0.0;

  return var;
}
///

/// vifIndex::vifFactor
// Compute the sim factor with a certain interleaving (only every n-th row) with the given start offset
// from the start of the picture.
void vifIndex::vifFactor(const Matrix<FLOAT>& img1,const Matrix<FLOAT>& img2,DOUBLE scale,
			 ULONG offset, ULONG interleave,double var,
			 double &numerator,double &denominator) const
{
  //the 2 images will be transform in a "float" matrix.
  ULONG width  = img1.WidthOf();
  ULONG height = img1.HeightOf();
  
  //The GaussFilter will be apply
  ULONG w  = m_Gauss.WidthOf();
  ULONG h  = m_Gauss.HeightOf();
  ULONG sz = w * h;
  ULONG ct = 0;
  
  double C2 = (K2*scale)*(K2*scale);
  double C3 = C2 / 2.0; //* C2 / 4.0; // Note that we need s^2 here, not s.
  double C4 = K; // according to the VIF matlab code, this value of K is already scaled to 8bpp.

  for(ULONG y1 = offset;y1 <= height-h;y1 += interleave){
    for(ULONG x1 = 0;x1 <= width-w;x1++){
      ULONG x,y;
      double lumvalue_1  = 0.0,lumvalue_2  = 0.0;
      double con2value_1 = 0.0,con2value_2 = 0.0;
      double corrvalue   = 0.0;
      double numadd,denadd;
      double gv,vv;
      
      // Collect moments. Note that we do that in one
      // loop. The above code does in several, but this
      // is definitely quicker.
      for( y=0;y<h;y++){
	for( x=0;x<w;x++){
	  double k1    = img1.Get(x+x1,y+y1);
	  double k2    = img2.Get(x+x1,y+y1);
#ifdef DO_WINDOWING
	  double valv  = m_Gauss.Get(x,y);
	  lumvalue_1  += k1*valv;
	  lumvalue_2  += k2*valv;
	  con2value_1 += k1*k1*valv;
	  con2value_2 += k2*k2*valv;
	  corrvalue   += k1*k2*valv;
#else
	  lumvalue_1  += k1;
	  lumvalue_2  += k2;
	  con2value_1 += k1*k1;
	  con2value_2 += k2*k2;
	  corrvalue   += k1*k2;
#endif
	}
      }
#ifndef DO_WINDOWING
      // m_Gauss normalized the values to one. We do this now manually.
      lumvalue_1  /= sz;
      lumvalue_2  /= sz;
      con2value_1 /= sz;
      con2value_2 /= sz;
      corrvalue   /= sz;
#endif
      //
      // Fixup the moments so we really get what is needed.
      con2value_1     -= lumvalue_1 * lumvalue_1;
      con2value_2     -= lumvalue_2 * lumvalue_2;
      corrvalue       -= lumvalue_1 * lumvalue_2;
      // Fixup round-off errors. Variances should be positive.
      if (con2value_1  < 0.0)
	con2value_1    = 0.0;
      if (con2value_2  < 0.0)
	con2value_2    = 0.0;
      //
      //
      // Compute V and G.
      if (con2value_1 > 0.0 && con2value_2 > 0.0) {
	gv = corrvalue / con2value_1;
      } else {
	gv = 0.0;
      }
      if (con2value_1 > 0.0) {
	vv = (con2value_2 - corrvalue * corrvalue / con2value_1) / (w * h);
      } else {
	vv = con2value_2 / (w * h);
      }
      // Constrain the gain to be positive.
      if (gv < 0.0) {
	vv = con2value_2 / (w * h);
	gv = 0.0;
      }
      //
      numerator += log(1.0 + (gv * var) / (vv + K));
      ct++;
    }
  }
  //
  // The numerator does not depend on the samples at all.
  denominator += ct * log(1.0 + var / K);
}
///

/// vifIndex::vifIndex
vifIndex::vifIndex(void) 
  : m_Gauss(CreateGaussFilter(WindowSize,WindowSize))
{
}
///

/// vifIndex::estimateGV
#if 0
// Estimate the gain field and the transmission noise
// Filtersize is the dimension of the window within which the correlation etc.
// are measured. The step size is defined by WindowSize.
// The filter size is given by the filter itself. Note that the filter must be
// normalized to have the total l^1-norm of 1.0.
void vifIndex::estimateGV(const Matrix<FLOAT>& img1,const Matrix<FLOAT>& img2,
			  Matrix<FLOAT> &g,Matrix<FLOAT> &v,const Matrix<FLOAT> &filter)
{
  ULONG w,h,x,y,xt,yt;
  ULONG xn,yn;
  ULONG filtersize = filter.WidthOf();
  double mean[WindowSize][WindowSize];
  double corr[WindowSize][WindowSize][WindowSize][WindowSize];
  ULONG blockcnt = 0;
  //
  assert(filter.WidthOf() == filter.HeightOf());
  assert(img1.WidthOf()   == img2.WidthOf());
  assert(img1.HeightOf()  == img2.HeightOf());
  //
  // Allocate the target matrices.
  w = (img1.WidthOf()  - filtersize + 1) / WindowSize;
  h = (img1.HeightOf() - filtersize + 1) / WindowSize;
  //
  g.Allocate(w,h);
  v.Allocate(w,h);
  //
  // Clean the mean and square matrices.
  memset(mean,0,sizeof(mean));
  memset(corr,0,sizeof(corr));
  //
  for(y = 0,yn = 0,yt = 0;y < img1.HeightOf();y++) {
    for(x = 0,xn = 0,xt = 0;x < img1.WidthOf();x++) {
      ULONG i,j;
      //
      // Compute the variance on overlapping MxM blocks, unfiltered (M = WindowSize is too small anyhow)
      // This is only on the original image.
      if (x + WindowSize < img1.WidthOf() && y + WindowSize < img1.HeightOf()) {
	for(j = 0;j < WindowSize;j++) {
	  for(i = 0;i < WindowSize;i++) {
	    ULONG k,l;
	    mean[i][j] += img1.At(x+i,y+j);
	    for(l = 0;l < WindowSize;l++) {
	      for(k = 0;k < WindowSize;k++) {
		corr[i][j][k][l] += img1.At(x+i,y+j) * img1.At(x+k,y+l);
	      }
	    }
	  }
	}
	blockcnt++;
      }
      //
      if (x == xn && y == yn) {
	double gv,vv;
	double lumvalue_1  = 0.0,lumvalue_2  = 0.0;
	double con2value_1 = 0.0,con2value_2 = 0.0;
	double corrvalue   = 0.0;
	// Collect moments. Note that we do that in one
	// loop.
	for(j = 0;j < filtersize;j++) {
	  for(i = 0;i < filtersize;i++) {
	    double k1    = img1.Get(x+i,y+j);
	    double k2    = img2.Get(x+i,y+j);
	    double valv  = filter.Get(x,y);
	    lumvalue_1  += k1*valv;
	    lumvalue_2  += k2*valv;
	    con2value_1 += k1*k1*valv;
	    con2value_2 += k2*k2*valv;
	    corrvalue   += k1*k2*valv;
	  }
	}
	// Fixup the moments so we really get what is needed.
	con2value_1     -= lumvalue_1 * lumvalue_1;
	con2value_2     -= lumvalue_2 * lumvalue_2;
	corrvalue       -= lumvalue_1 * lumvalue_2;
	// Fixup round-off errors. Variances should be positive.
	if (con2value_1  < 0.0)
	  con2value_1    = 0.0;
	if (con2value_2  < 0.0)
	  con2value_2    = 0.0;
	//
	// Regression analysis for the gain field.
	if (con2value_1 > 0.0 && con2value_2 > 0.0) {
	  gv = corrvalue / con2value_1;
	} else {
	  gv = 0.0;
	}
	if (con2value_1 > 0.0) {
	  vv = (con2value_2 - corrvalue * corrvalue / con2value_1) * (filtersize * filtersize);
	} else {
	  vv = con2value_2;
	}
	// Constrain the gain to be positive.
	if (gv < 0.0) {
	  vv = con2value_2;
	  gv = 0.0;
	}
	//
	g.At(xt,yt) = gv;
	v.At(xt,yt) = gv;
	//
	// To the next sample position.
	xt++;
	xn += WindowSize;
      }
    }
    //
    // To the next sample position.
    if (y == yn) {
      yt++;
      yn += WindowSize;
    }
  }
  //
  // Compute and fixup the correlation matrix.
  if (blockcnt) {
    ULONG i,j;
    ULONG k,l;
    //
    for(j = 0;j < WindowSize;j++) {
      for(i = 0;i < WindowSize;i++) {
	for(l = 0;l < WindowSize;l++) {
	  for(k = 0;k < WindowSize;k++) {
	    corr[i][j][k][l] = (corr[i][j][k][l] - mean[i][j] * mean[k][l]) / (blockcnt * blockcnt);
	  }
	}
      }
    }
  }
}
#endif
///
