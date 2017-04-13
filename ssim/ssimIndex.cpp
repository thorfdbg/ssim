/************************************************************************************
 **  Copyright (C) 2005-2010 Thomas Richter                                        **
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
 **	           		Thomas Richter                                     **
 **				thor@math.tu-berlin.de                             **
 **                                                                                **
 ************************************************************************************/

#include "ssimIndex.hpp"
#include "global/matrix.hpp"
#include "std/math.hpp"
#include "std/stdio.hpp"

#ifndef M_PI
#define M_PI 3.14159326
#endif

/// Weights
// The weights for the scales. We have exactly five weights, taken
// from Simoncelli et al.
const DOUBLE ssimIndex::Weights[5] = {0.0448,0.2856,0.3001,0.2363,0.1333};
const DOUBLE ssimIndex::K1 = 0.01;
const DOUBLE ssimIndex::K2 = 0.03;
///

/// ssimIndex::CreateGaussFilter
// Create the gauss windowing function
Matrix<double> ssimIndex::CreateGaussFilter(ULONG w, ULONG h)
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

/// ssimIndex::CreateHammingWindow
// This method creates a Hamming window of the given size.
Matrix<DOUBLE> ssimIndex::CreateHammingWindow(ULONG w, ULONG h)
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

/// ssimIndex::ssimFactor
// compute the ssim index for a complete image
double ssimIndex::ssimFactor(const Image& img1,const Image& img2,int ncpus,bool bylevel,Matrix<FLOAT> &err)
{

  int i;
  DOUBLE ssim = 0.0,w;

  m_pError = &err;
  if (!err.IsEmpty()) {
    err.CleanMatrix();
    // During the accumulation phase, err contains the logarithm of the probability of not detecting
    // an error. 0.0 means: log(1-p) = 0.0, thus p = 0.0 -> error detection probability p = 0.0: no error.
    ncpus = 1; // requires single-CPU usage to pool up the error correctly.
  }
  
  for(i = 0;i < img1.ComponentCountOf();i++) {
    if (bylevel)
      printf("\n%s component:\n",img1.ComponentOf(i).NameOf());
    w = ssimFactor(img1.ComponentOf(i),img2.ComponentOf(i),img1.ComponentOf(i).ScaleOf(),ncpus,bylevel,
		   img1.ComponentOf(i).WeightOf());
    ssim += img1.ComponentOf(i).WeightOf() * w;
  }

  if (!err.IsEmpty()) {
    int x,y,w = err.WidthOf(),h = err.HeightOf();
    for(y = 0;y < h;y++) {
      for(x = 0;x < w;x++) {
	err.At(x,y) = 1.0 - exp(err.At(x,y)); // Compute the detection probability
      }
    }
  }
  return ssim;
}
///

/// ssimIndex::pthread_entry
// The entry point to start a SSIM thread.
#ifndef NO_POSIX
void *ssimIndex::pthread_entry(void *arg)
{
  struct ssimTask *task = (struct ssimTask *)arg;

  task->result = task->that->ssimFactor(*task->img1,*task->img2,task->scale,task->doluminance,
					task->offset,task->interleave,
					task->size,task->cweight,task->gamma);

  return task;
}
#endif
///

/// ssimIndex::ssimFactor
// Compute the ssim factor for a multi-core CPU with potentially using threads.
double ssimIndex::ssimFactor(Component& img1,Component& img2,DOUBLE scaling,int ncpus,bool bylevel,DOUBLE cweight) const
{
  int scale,nscales = img1.ScalesOf();
  DOUBLE result = 1.0;

  for(scale = 1;scale <= nscales;scale++) {
    const Matrix<FLOAT> &c1 = img1.GetScale(scale);
    const Matrix<FLOAT> &c2 = img2.GetScale(scale);
    DOUBLE thissim;

#ifdef NO_POSIX
    thissim = ssimFactor(c1,c2,scaling,scale == nscales,0,1,scale-1,cweight,Weights[scale-1]);
#else
    if (ncpus == 1) {
      thissim = ssimFactor(c1,c2,scaling,scale == nscales,0,1,scale-1,cweight,Weights[scale-1]);
    } else {
      int i;
      struct ssimTask *tasks = new ssimTask[ncpus];
      double sum = 0.0;
      //
      for(i = 0;i < ncpus;i++) {
	tasks[i].that        = this;
	tasks[i].img1        = &c1;
	tasks[i].img2        = &c2;
	tasks[i].offset      = i;
	tasks[i].interleave  = ncpus;
	tasks[i].scale       = scaling;
	tasks[i].doluminance = (scale == nscales);
	tasks[i].size        = scale-1;
	tasks[i].nscales     = nscales;
	tasks[i].cweight     = cweight;
	tasks[i].gamma       = Weights[scale-1];
	pthread_attr_init(&tasks[i].attr);
	pthread_attr_setdetachstate(&tasks[i].attr,PTHREAD_CREATE_JOINABLE);
	pthread_create(&tasks[i].pid,&tasks[i].attr,&ssimIndex::pthread_entry,tasks+i);
      }
      //
      // Now wait for all threads to complete, and collect their results.
      for(i = 0;i < ncpus;i++) {
	pthread_join(tasks[i].pid,NULL);
	sum += tasks[i].result;
      }
      delete[] tasks;
      thissim = sum / ncpus;
    }
#endif
    if (bylevel)
      printf("log ssim value for scale %d: %f\n",scale,-20.0 * log(1.0 - thissim) / log(10.0));
    //
    // If this is a single-scale ssim, no exponent.
    if (nscales > 1) {
      result *= pow(thissim,Weights[scale-1]);
    } else {
      result *= thissim;
    }
  }

  return result;
}
///

/// ssimIndex::ssimFactor
// Compute the sim factor with a certain interleaving (only every n-th row) with the given start offset
// from the start of the picture.
double ssimIndex::ssimFactor(const Matrix<FLOAT>& img1,const Matrix<FLOAT>& img2,DOUBLE scale,bool doluminance,
			     ULONG offset, ULONG interleave,
			     int size,DOUBLE cweight,DOUBLE gamma) const
{
  ULONG counter     = 0;
  double ssimsum    = 0.0;
  bool includevis   = (m_dMasking < 2.0); // include the visibility coefficient.

  //the 2 images will be transform in a "float" matrix.
  ULONG width  = img1.WidthOf();
  ULONG height = img1.HeightOf();
  
  //The GaussFilter will be apply
  ULONG w = m_Gauss.WidthOf();
  ULONG h = m_Gauss.HeightOf();
  
  double C1 = (K1*scale)*(K1*scale);
  double C2 = (K2*scale)*(K2*scale);
  double C3 = C2/2;  

#if 0
  {
    char name[256];
    snprintf(name,255,"scale_%d.pgm",size);
    FILE *out = fopen(name,"wb");
    fprintf(out,"P5\n%d\t%d\n255\n",width,height);
    for(ULONG y = 0;y < height;y++) {
      for(ULONG x = 0;x < width;x++) {
	fputc(img1.Get(x,y),out);
      }
    }
    fclose(out);
  }
#endif


  for(ULONG y1 = offset;y1 <= height-h;y1 += interleave){
    for(ULONG x1 = 0;x1 <= width-w;x1++){
      ULONG x,y;
      double lumvalue_1  = 0.0,lumvalue_2  = 0.0;
      double con2value_1 = 0.0,con2value_2 = 0.0;
      double corrvalue   = 0.0,con12value;
      double visibility  = 1.0; // the new factor in SSIM
      double complum;
      double compcon;
      double compstruct; 

      // Collect moments. Note that we do that in one
      // loop. The above code does in several, but this
      // is definitely quicker.
      for( y=0;y<h;y++){
	for( x=0;x<w;x++){
	  double k1    = img1.Get(x+x1,y+y1);
	  double k2    = img2.Get(x+x1,y+y1);
	  double valv  = m_Gauss.Get(x,y);
	  lumvalue_1  += k1*valv;
	  lumvalue_2  += k2*valv;
	  con2value_1 += k1*k1*valv;
	  con2value_2 += k2*k2*valv;
	  corrvalue   += k1*k2*valv;
	}
      }
      //
      if (includevis) {
	double l2norm1 = 0.0;
	double l2norm2 = 0.0;
	double lpnorm1 = 0.0;
	double lpnorm2 = 0.0;
	double sscale  = w * h; // note that this is the l^2 norm, not the variance (which would be averaged)
	double C3      = C2 * pow(sscale,2.0 / m_dMasking - 1.0); // the scaling of the stabilizer.
	//
	// Include the visibility mask. Note that the gaussian window
	// has average 1.0, thus lumvalue is really the average (with weights)
	//
	for( y=0;y<h;y++){
	  for( x=0;x<w;x++){
	    double k1    = img1.Get(x+x1,y+y1);
	    double k2    = img2.Get(x+x1,y+y1);
	    double valv  = m_Gauss.Get(x,y) * sscale;
	    double v1    = k1 - lumvalue_1; // value minus average
	    double v2    = k2 - lumvalue_2;
	    //
	    l2norm1     += v1 * v1 * valv;
	    l2norm2     += v2 * v2 * valv;
	    lpnorm1     += pow(fabs(v1),m_dMasking) * valv;
	    lpnorm2     += pow(fabs(v2),m_dMasking) * valv;
	  }
	}
	//
	lpnorm1 = pow(lpnorm1,2.0 / m_dMasking);
	lpnorm2 = pow(lpnorm2,2.0 / m_dMasking);
	//
	// Plus stabilizer
	visibility  = (l2norm1 + l2norm2 + C3) / (lpnorm1 + lpnorm2 + C3);
	visibility  = pow(visibility,m_dMasking / 2.0);
	//printf("%g\t",visibility);
	// Should almost never overrun. In case it is due to numerical problems,
	// confine it.
	if (visibility > 1.0)
	  visibility = 1.0;
	assert(visibility > 0.0 && visibility <= 1.0);
	// This should scale like samples^(2/p - 1), thus multiply by that to bring it back into a useful range.
	//visibility *= pow(scale,2.0 / m_dMasking - 1.0);
      } else {
	visibility = 1.0;
      }
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
      con12value       = sqrt(con2value_1 * con2value_2);
      
      complum    = (2.0 * lumvalue_1 * lumvalue_2 + C1)/(lumvalue_1 * lumvalue_1 + lumvalue_2 * lumvalue_2 + C1);
      compcon    = (2.0 * con12value + C2)/(con2value_1 + con2value_2 + C2);
      compstruct = (corrvalue + C3)/(con12value + C3); 
      //
      // If the luminance is suppressed (on all but the smallest scale), set this contribution to 1.0.
      if (!doluminance)
	complum = 1.0;
      //ssim index for a window:
      /* NOTE: The following would be correct, but since Alpha = Beta = Gamma = 1.0, no sweat,
      ** and we're in a hurry.
      ** float locssim = pow(complum,Alpha) * pow(compcon,Beta) * pow(compstruct,Gamma);
      */ 
      //
      // If visibility is included, modify accordingly. Note that the term is constructed in a way
      // to reproduce the understood "classical" masking term.
      if (includevis) {
	//locssim = 1.0 - ((1.0 - locssim) * visibility);
	//locssim = (1.0 - visibility) + locssim * visibility;
	compstruct = (1.0 - visibility) + compstruct * visibility;
      }
      double locssim = complum * compcon * compstruct;
      //
      //sum of the ssim indexes for all windows.
      ssimsum +=locssim;
      counter++;
      //
      if (m_pError && !m_pError->IsEmpty()) {
	int xmin,xmax,ymin,ymax,i,j;
	int exponent = size;
	DOUBLE xmid,ymid;
	DOUBLE p = (locssim + 1.0) * 0.5,f;
	if (p <= 0.0) {
	  p = -HUGE_VAL; // In this simple model, the probabililty of *not* detecting an error.
	  // Note that the local SSIM may get negative if source and reconstructed structure
	  // are anti-correlated.
	} else {
	  // However, we need the log since that accumulated additively.
	  p = cweight * gamma * log(p);
	}
	//p *= -1E+3;
	//printf("%g\t",p);
	//
	if (exponent > 0) {
	  xmin = ((x1 + (w >> 1)) << exponent) - ((1 << exponent) >> 1);
	  ymin = ((y1 + (h >> 1)) << exponent) - ((1 << exponent) >> 1);
	  xmax = xmin + (2 << exponent);
	  ymax = ymin + (2 << exponent);
	  xmid = (xmin + xmax - 1) * 0.5;
	  ymid = (ymin + ymax - 1) * 0.5;
	  f    = M_PI / (xmax - xmin);
	  if (xmin < 0)
	    xmin = 0;
	  if (ymin < 0)
	    ymin = 0;
	  if (xmax > int(m_pError->WidthOf()))
	    xmax = m_pError->WidthOf();
	  if (ymax > int(m_pError->HeightOf()))
	    ymax = m_pError->HeightOf();
	  for(j = ymin;j < ymax;j++) {
	    for(i = xmin;i < xmax;i++) {
	      // Note that the following works only for single-CPU usage - no locking.
	      m_pError->At(i,j) += p * cos(f * (i - xmid)) * cos(f * (j - ymid));
	    }
	  }
	} else {
	  m_pError->At(x1,y1) += p;
	}
      }
    }
  }

  if (counter > 0)
    return ssimsum / counter;
  return 1.0;
}
///

/// ssimIndex::ssimIndex
ssimIndex::ssimIndex(double masking) 
  : m_dMasking(masking), m_Gauss(CreateGaussFilter(11,11))
{
}
///

/// PrintMatrix
// Print out the matrix
void PrintMatrix(const Matrix<double> &m)
{
  if (!m.IsEmpty()) {
    ULONG x,y;
    for(y = 0;y < m.HeightOf();y++) {
      for(x = 0;x < m.WidthOf();x++) {
	double v = double(m.Get(x,y));
	printf("%f\t",v);
      }
      printf("\n");
    }
  } else {
    printf("<NULL>\n");
  }
}
///
