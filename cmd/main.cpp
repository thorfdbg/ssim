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
** $Id: main.cpp,v 1.19 2017/04/13 15:26:22 thor Exp $
**
*/

/// Includes
#include "cmd/main.hpp"
#include "img/image.hpp"
#include "io/filestream.hpp"
#include "img/component.hpp"
#include "std/stdio.hpp"
#include "std/stdarg.hpp"
#include "std/stdlib.hpp"
#include "ctrafo/colortransformer.hpp"
#include "global/exceptions.hpp"
#include "ssim/ssimIndex.hpp"
#include "vif/vifIndex.hpp"
#include <math.h>
///

/// StdExceptionPrinter
// A very simple class that manages the printing of exceptions.
class StdExceptionPrinter : public ExceptionPrinter {
  //
public:
  StdExceptionPrinter(void)
  {}
  ~StdExceptionPrinter(void)
  {}
  //
  // This is the real thing.
  virtual void PrintException(const char *fmt,...);
};

void StdExceptionPrinter::PrintException(const char *fmt,...)
{
  va_list args;

  va_start(args,fmt);
  vfprintf(stderr,fmt,args);
  va_end(args);
}
///

/// A structure keeping the local settings
// The following structure keeps the selected command line
// arguments for the imco project. It is filled in by command
// line parsing and only used locally here.
struct Settings {

  bool Log;
  //
  // The first input file name. This should be a ppm/pgm file. 
  char *m_pcInputName_1;
  

  // The second input file name. This should be a ppm/pgm file. 
  char *m_pcInputName_2;
  
  // The number of CPUs to run in parallel
  int  ncpus;
  
  // Print SSIM separately for each level?
  bool bylevel;
  
  // Use the VIF based version? (not supported yet)
  bool vif;

  // Output linear VIF?
  bool linear;

  //
  // Output ssim instead of multiscale-ssim?
  bool nowavelet;
  
  //
  // Create an image describing the non-masked image parts.
  char *m_pcMask;

  //
  // Create an image describing the error map.
  char *m_pcError;

  //
  // Masking exponent if any. Default would be a masking exponent of 2.
  double m_dMasking;
public:
  Settings(void)
    : Log(false),
      m_pcInputName_1(NULL), m_pcInputName_2(NULL),
      ncpus(1), bylevel(false), vif(false),
      linear(false), nowavelet(false),
      m_pcMask(NULL), m_pcError(NULL),
      m_dMasking(2.0)
  { 
  }
  //
  ~Settings(void)
  {
    delete[] m_pcInputName_1;
    delete[] m_pcInputName_2;
    delete[] m_pcMask;
    delete[] m_pcError;
  }
  //
  // Print the usage rules for this program.
  void Usage(const char *progname);
  //
  // Parse off the arguments from argc,argv.
  void ParseArgs(int argc,char **argv);
  //
};
///

/// Settings::Usage
// Print th command line synopsis for this
// fine program.
void Settings::Usage(const char *progname)
{
  printf("Usage: %s\n"
#ifndef NO_POSIX
	 "\t[-CC #]\t: set the number of CPUs to use in parallel\n"
#endif
	 "\t[-bl]  \t: print SSIM separately for each level\n"
	 //"\t[-vif] \t: use VIF instead of SSIM\n"
	 //"\t[-msk file] \t: create an image showing the noise mask\n"
	 "\t[-exp val ] \t: use masking exponent \"val\", default is 2.0\n"
	 "\t[-err file] \t: create an image showing the error distribution\n"
	 "\t[-lin]      \t: compute the (m)ssim as linear value, not in dB as by default\n"
	 "\t[-nowav]    \t: compute a single scale ssim instead of multiscale mssim\n"
	 "\tinfile1:\t the original file name.\n"
	 "\tinfile2:\t the distorted file name.\n"
	 "%s currently understands .ppm and .pgm files.default:ssim\n",
	 progname,progname);
}
///

/// Settings::ParseArgs
// Parse off program arguments from the command line arguments.
void Settings::ParseArgs(int argc,char **argv)
{
  
  const char *arg;
  const char *progname = argv[0];
  bool failure = false;
  
  //do I need this??
  argc--;
  argv++; // drop the command name.

 while((arg = *argv++)) {
    argc--;
    // Check whether we currently hold an option.
    if (arg[0] == '-') {
      // Yes, an option. o on parsing the input.
      if (!strcmp(arg,"--")) {
	// abort parsing options, go
	// to the file name arguments.
	break;
      } else if (!strcmp(arg,"-bl")) {
	bylevel = true;
      } else if (!strcmp(arg,"-lin")) {
	linear  = true;
      } else if (!strcmp(arg,"-nowav")) {
	nowavelet = true;
      } else if (!strcmp(arg,"-vif")) {
	vif     = true;
      } else if (!strcmp(arg,"-exp")) {
	if (argv[0]) {
	  char *end;
	  m_dMasking = strtod(argv[0],&end);
	  if (*end || m_dMasking <= 0.0 || m_dMasking >= 2.0) {
	    fprintf(stderr,"masking value must be a numeric argument between 0.0 and 2.0\n");
	    failure = true;
	    break;
	  } else {
	    argc--;
	    argv++;
	  }
	} else {
	  failure = true;
	  break;
	}
      } else if (!strcmp(arg,"-msk")) {
	if (argv[0] && m_pcMask == NULL) {
	  m_pcMask = new char[strlen(argv[0]) + 1];
	  strcpy(m_pcMask,argv[0]);
	  argc--;
	  argv++;
	} else {
	  failure = true;
	  break;
	} 
      } else if (!strcmp(arg,"-err")) {
	if (argv[0] && m_pcError == NULL) {
	  m_pcError = new char[strlen(argv[0]) + 1];
	  strcpy(m_pcError,argv[0]);
	  argc--;
	  argv++;
	} else {
	  failure = true;
	  break;
	}
      } else if (!strcmp(arg,"-log")) {
	// always on, only for backwards compatibility
      } else if (!strcmp(arg,"-CC")) {
	if (argv[0]) {
	  ncpus = atoi(argv[0]);
	  if (ncpus <= 0)
	    failure = true;
	  argc--;
	  argv++;
	} else
	  failure = true;
      } else {
	failure = true;
	break;
      }
    } else {
      // Put this argument back.
      argc++;
      argv--;
      break;
    }
 }
    
 // Exactly two arguments must be left:
 // the 2 input files' name.
 if (failure || argc != 2) {
   Usage(progname);
   exit(10);
 }
 //
 m_pcInputName_1  = new char[strlen(argv[0]) + 1];
 m_pcInputName_2 = new char[strlen(argv[1]) + 1];
 //
 // copy the names over.
 strcpy(m_pcInputName_1,argv[0]);
 strcpy(m_pcInputName_2,argv[1]);
}
///

/// The main program loop
int main(int argc,char **argv)
{
  struct Settings settings;
  //
  try {
    //
    // parse off the command line arguments here.
    settings.ParseArgs(argc,argv);
    //
    // Now check whether we encode or decode.
    class FileStream in1,in2;
    class Image img1,img2;
    Matrix<FLOAT> err;
    int i;
    //
    // Open the input stream for first image's reading.
    in1.OpenForRead(settings.m_pcInputName_1);
    //
    // Open the input stream for second image's reading.
    in2.OpenForRead(settings.m_pcInputName_2);
    
    
    // Read the images from the files, and transform them
    // on the way.
    // Note that MSSIM requires five stages.
    img1.LoadPNM(&in1,(settings.nowavelet)?(1):(5),settings.vif);
    in1.Close();
    img2.LoadPNM(&in2,(settings.nowavelet)?(1):(5),settings.vif);
    in2.Close();
    
    //Wir muessen hier dafuer sorgen, dass die BIlder dieselbe Dimensionen haben
    //
    if (img1.ComponentCountOf() != img2.ComponentCountOf()) {
      Throw(InvalidParameter,"main","Component counts differ, cannot compare images.\n");
    }
    // ensure that the image components all have the same
    // dimensions. Can't handle other images in imco otherwise.
    for(i = 0;i < img1.ComponentCountOf(); i++) {
      if (!(img1.ComponentOf(i).WidthOf()  == img2.ComponentOf(i).WidthOf() &&
	    img1.ComponentOf(i).HeightOf() == img2.ComponentOf(i).HeightOf())) {
	Throw(InvalidParameter,"main","Component dimensions differ, cannot compare images.\n");
      }
    }
    //
    // Generate an error map?
    if (settings.m_pcError) {
      err.Allocate(img1.ComponentOf(0).WidthOf(),img1.ComponentOf(0).HeightOf());
    }
    //
    // Check whether the image is color. If so, run a color transformation
    // and perform the computation only on the Y plane.
    switch(img1.ComponentCountOf()) {
    case 1:
      // gray-scale, fine.
      break;
    case 3:
      {
	class ColorTransformer trafo;
	//
	trafo.ForwardsTransform(&img1);
	trafo.ForwardsTransform(&img2);
      }
      break;
    default:
      Throw(NotImplemented,"main","unsupported number of components, cannot compare images.\n");
      break;
    }
    //
    double psnr;
    
    if (settings.vif) {
      class vifIndex vif;
      psnr = vif.vifFactor(img1,img2,settings.ncpus,settings.bylevel);
      if (!settings.linear)
	psnr = -10.0 * log(1.0 - psnr) / log(10.0);
    } else {
      class ssimIndex ssim1(settings.m_dMasking);
      psnr = ssim1.ssimFactor(img1,img2,settings.ncpus,settings.bylevel,err);
      if (!settings.linear)
	psnr = -10.0 * log(1.0 - psnr) / log(10.0);
    }
    printf("%g\n",psnr);
    //
    // Create the error map
    if (settings.m_pcError) {
      int x,y;
      FILE *error = fopen(settings.m_pcError,"wb");
      if (error) {
	fprintf(error,"P5\n%d\t%d\n255\n",int(err.WidthOf()),int(err.HeightOf()));
	for(y = 0;y < int(err.HeightOf());y++) {
	  for(x = 0;x < int(err.WidthOf());x++) {
	    int v = int(255 * err.Get(x,y));
	    v     = (v < 0)?(0):((v > 255)?(255):(v));
	    fputc(v,error);
	  }
	}
	fclose(error);
      }
    }
  } catch(const CodecException &ce) {
    class StdExceptionPrinter ep;
    //
    ce.PrintException(ep);
    return 5;
  }
  return 0;
}
///
