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

#ifndef SSIMINDEX_HPP
#define SSIMINDEX_HPP

/*
** This Class calculate the overall ssim-index between 2 pictures.
** First of all,we'll extract  the luminance,contrast and structur -values of 2 input images
** After this, we need to compare them and at least through  a combination of the result-values
** of this methods, we will give back the index as a float value.
*/

#include "global/types.hpp"
#include "global/matrix.hpp"
#include "img/image.hpp"
#include "img/component.hpp"
#ifndef NO_POSIX
extern "C" {
#include <pthread.h>
}
#endif


class ssimIndex {
  //
  static const DOUBLE K1,K2;
  //
  // The weights for the scales. We have exactly five weights, taken
  // from Simoncelli et al.
  static const DOUBLE Weights[5];
  //
  //This method represent the gausswindowfunction .
  static Matrix<DOUBLE> CreateGaussFilter(ULONG w, ULONG h);
  //
  // This method creates a Hamming window of the given size.
  static Matrix<DOUBLE> CreateHammingWindow(ULONG w, ULONG h);
  //
  // The masking value.
  double m_dMasking;
  //
  // This structure is used to start a new thread with a partial ssim computation.
  struct ssimTask {
    const  ssimIndex     *that;
    const  Matrix<FLOAT> *img1;
    const  Matrix<FLOAT> *img2;
    Matrix <UBYTE>       *mask;
    Matrix <UBYTE>       *err;
    const Image          *pic1;
    const Image          *pic2;
    ULONG  offset;
    ULONG  interleave;
    DOUBLE result;
    DOUBLE scale;
    BOOL   doluminance;
    int    size;
    int    nscales;
    DOUBLE cweight;
    DOUBLE gamma;
    //
#ifndef NO_POSIX
    // pthread helpers here.
    pthread_attr_t attr;
    pthread_t pid;
#endif
  };
  //
  // The window function.
  const Matrix<DOUBLE> m_Gauss;
  //
  // The error map.
  Matrix<FLOAT>       *m_pError;
  //
#ifndef NO_POSIX
  // The entry point to start a SSIM thread.
  static void *pthread_entry(void *arg);
#endif
  //
  // Compute the sim factor with a certain interleaving (only every n-th row) with the given start offset
  // from the start of the picture.
  double ssimFactor(const Matrix<FLOAT>& img1,const Matrix<FLOAT>& img2,DOUBLE scale,bool doluminance,
		    ULONG offset, ULONG interleave,int size,DOUBLE cweight,DOUBLE gamma) const;
  //
  //
  //This method give back the overall ssimindex for all the(with the gausswindowfunction) computed windows 
  double ssimFactor(Component& img1,Component& img2,DOUBLE scaling,int ncpus,bool bylevel,DOUBLE cweight) const;
  //
public:
  //
  // Global SIM including color with a naive color weighting.
  double ssimFactor(const Image& img1,const Image& img2,int ncpus,bool bylevel,Matrix<FLOAT> &err);
  //
  ssimIndex(double masking = 2.0);
  //
  ~ssimIndex()
  { }
};

#endif
