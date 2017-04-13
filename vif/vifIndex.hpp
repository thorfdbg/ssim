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

#ifndef VIFINDEX_HPP
#define VIFINDEX_HPP

/*
** This Class calculate the overall vif-index between 2 pictures.
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


class vifIndex {
  //
  // K is the variance of the perception noise, picked as 0.1
  static const DOUBLE K,K1,K2;
  //
  // The window size.
  enum {
    WindowSize = 3 // this is what the article does.
  };
  //
  //This method represent the gausswindowfunction .
  static Matrix<DOUBLE> CreateGaussFilter(ULONG w, ULONG h);
  //
  // This method creates a Hamming window of the given size.
  static Matrix<DOUBLE> CreateHammingWindow(ULONG w, ULONG h);
  //
  // This structure is used to start a new thread with a partial ssim computation.
  struct vifTask {
    const  vifIndex      *that;
    const  Matrix<FLOAT> *img1;
    const  Matrix<FLOAT> *img2;
    Matrix <UBYTE>       *mask;
    Matrix <UBYTE>       *err;
    const Image          *pic1;
    const Image          *pic2;
    ULONG  offset;
    ULONG  interleave;
    DOUBLE numerator;    // will return the numerator of the VIF, information capacity for distorted image
    DOUBLE denominator;  // information capacity for the reference image.
    DOUBLE scale;
    DOUBLE var;          // variance
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
#ifndef NO_POSIX
  // The entry point to start a SSIM thread.
  static void *pthread_entry(void *arg);
#endif
  //
  // Compute vif with a certain interleaving (only every n-th row) with the given start offset
  // from the start of the picture, return numerator and denominator.
  void vifFactor(const Matrix<FLOAT>& img1,const Matrix<FLOAT>& img2,DOUBLE scale,
		 ULONG offset, ULONG interleave,DOUBLE var,
		 DOUBLE &numerator,DOUBLE &denominator) const;
  //
  //
  // Compute the vif factor for a multi-core CPU with potentially using threads.
  void vifMultiCoreFactor(const Matrix<FLOAT>& img1,const Matrix<FLOAT>& img2,DOUBLE scaling,int ncpus,
			  double &numerator,double &denominator) const;
  //
  // Return the global vif for the given images.
  void vifFactorScales(Component& img1,Component& img2,DOUBLE scaling,int ncpus,bool bylevel,
		       DOUBLE &numerator,DOUBLE &denominator) const;
  //
  // Computes the variance = \sigma_x^2 of the given matrix.
  static double variance(const Matrix<FLOAT> &img);
  //
public:
  //
  // Global SIM including color with a naive color weighting.
  double vifFactor(const Image& img1,const Image& img2,int ncpus = 1,bool bylevel = false) const;
  //
  //
  vifIndex(void);
  //
  ~vifIndex()
  { }
};

#endif
